/*
 * CiviBox - Minimal userspace toolkit for Linux
 *
 * Copyright (C) 2026 Pietro Carlini
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/random.h>
#include <sys/time.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENT_PORT 68
#define SERVER_PORT 67

#define DISCOVER 1
#define OFFER    2
#define REQUEST  3
#define ACK      5
#define NAK      6

struct dhcp_packet
{
    uint8_t op, htype, hlen, hops;
    uint8_t xid[4];
    uint16_t secs, flags;
    uint32_t ciaddr, yiaddr, siaddr, giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[312];
} __attribute__((packed));

static int get_mac(int sock, const char *name, uint8_t *mac)
{
    struct ifreq ifr = {0};

    if (strlen(name) >= IFNAMSIZ) return -1;

    strcpy(ifr.ifr_name, name);

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) return -1;

    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

    return 0;
}

static int create_socket(const char *name)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int one = 1;

    if (sock < 0) return -1;

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one)) < 0) goto fail;
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, name, strlen(name) + 1) < 0) goto fail;

    struct timeval tv = {8, 0};

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) goto fail;

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(CLIENT_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) goto fail;

    return sock;

fail:
    close(sock);
    return -1;
}

static int add_option(struct dhcp_packet *p, size_t *pos, uint8_t code, const void *data, size_t len)
{
    if (len > 255 || *pos + len + 2 >= sizeof(p->options)) return -1;

    p->options[(*pos)++] = code;
    p->options[(*pos)++] = len;
    memcpy(p->options + *pos, data, len);
    *pos += len;

    return 0;
}

static int get_option(struct dhcp_packet *p, size_t size, uint8_t code, void *out, size_t len)
{
    size_t off = offsetof(struct dhcp_packet, options);
    size_t pos = 4;

    if (size < off + 4 || memcmp(p->options, "\x63\x82\x53\x63", 4)) return -1;

    while (pos < size - off) {
        uint8_t id = p->options[pos++];

        if (id == 255) break;
        if (!id) continue;
        if (pos >= size - off) return -1;

        uint8_t n = p->options[pos++];

        if (pos + n > size - off) return -1;

        if (id == code) {
            if (n != len) return -1;
            memcpy(out, p->options + pos, len);
            return 0;
        }

        pos += n;
    }

    return -1;
}

static int build_packet(struct dhcp_packet *p, const uint8_t xid[4], const uint8_t mac[6], uint8_t type, uint32_t *ip, uint32_t *server)
{
    static const uint8_t cookie[] = {99, 130, 83, 99};
    static const uint8_t wanted[] = {1, 3, 6, 15, 51, 54};

    uint8_t id[7] = {1, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]};
    size_t pos = 4;

    memset(p, 0, sizeof(*p));

    p->op = 1;
    p->htype = 1;
    p->hlen = 6;
    p->flags = htons(0x8000);

    memcpy(p->xid, xid, 4);
    memcpy(p->chaddr, mac, 6);
    memcpy(p->options, cookie, 4);

    if (add_option(p, &pos, 53, &type, 1) < 0) return -1;
    if (add_option(p, &pos, 61, id, sizeof(id)) < 0) return -1;
    if (ip && add_option(p, &pos, 50, ip, 4) < 0) return -1;
    if (server && add_option(p, &pos, 54, server, 4) < 0) return -1;
    if (add_option(p, &pos, 55, wanted, sizeof(wanted)) < 0) return -1;

    p->options[pos++] = 255;

    return offsetof(struct dhcp_packet, options) + pos;
}

static int send_packet(int sock, struct dhcp_packet *p, size_t len)
{
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    return sendto(sock, p, len, 0, (struct sockaddr *)&addr, sizeof(addr)) == (ssize_t)len ? 0 : -1;
}

static int receive_packet(int sock, uint8_t xid[4], struct dhcp_packet *p, size_t *size, struct sockaddr_in *from)
{
    for (;;) {
        socklen_t slen = sizeof(*from);
        ssize_t n = recvfrom(sock, p, sizeof(*p), 0, (struct sockaddr *)from, &slen);
        uint8_t type;

        if (n < 0) return -1;
        if ((size_t)n < offsetof(struct dhcp_packet, options) + 4) continue;
        if (p->op != 2 || memcmp(p->xid, xid, 4)) continue;
        if (get_option(p, n, 53, &type, 1) < 0) continue;

        *size = n;

        return type;
    }
}

static int dhcp_client(const char *name)
{
    uint8_t mac[6], xid[4];
    uint32_t ip, server;
    size_t size;
    struct dhcp_packet p;
    struct sockaddr_in from;
    char addr[INET_ADDRSTRLEN];

    int sock = create_socket(name);

    if (sock < 0) {
        perror("socket/bind");
        return 1;
    }

    if (get_mac(sock, name, mac) < 0) {
        perror("get_mac");
        goto fail;
    }

    if (rand(xid, sizeof(xid), 0) != sizeof(xid)) {
        perror("rand");
        goto fail;
    }

    int len = build_packet(&p, xid, mac, DISCOVER, NULL, NULL);

    if (len < 0 || send_packet(sock, &p, len) < 0) {
        perror("DHCP Discover");
        goto fail;
    }

    int type = receive_packet(sock, xid, &p, &size, &from);

    if (type != OFFER) {
        fprintf(stderr, "No DHCP Offer received\n");
        goto fail;
    }

    ip = p.yiaddr;

    if (get_option(&p, size, 54, &server, 4) < 0)
        server = from.sin_addr.s_addr;

    inet_ntop(AF_INET, &ip, addr, sizeof(addr));
    printf("Offer: %s\n", addr);

    len = build_packet(&p, xid, mac, REQUEST, &ip, &server);

    if (len < 0 || send_packet(sock, &p, len) < 0) {
        perror("DHCP Request");
        goto fail;
    }

    type = receive_packet(sock, xid, &p, &size, &from);

    if (type == NAK) {
        fprintf(stderr, "DHCP NAK received\n");
        goto fail;
    }

    if (type != ACK) {
        fprintf(stderr, "No DHCP ACK received\n");
        goto fail;
    }

    ip = p.yiaddr;

    inet_ntop(AF_INET, &ip, addr, sizeof(addr));
    printf("Lease: %s\n", addr);

    close(sock);

    return 0;

fail:
    close(sock);
    return 1;
}

int ddhcpc_cmd(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ddhcpc INTERFACE\n");
        return 1;
    }

    return dhcp_client(argv[1]);
}
