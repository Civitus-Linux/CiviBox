CC = gcc

CFLAGS = -Wall -Wextra -O2
LDFLAGS =
LDLIBS =

TARGET = civibox
CONFIG_FILE = .config

STATIC_CONFIG := $(shell \
	grep -qx 'CONFIG_COMPILE_STATIC=y' $(CONFIG_FILE) 2>/dev/null && echo y)

ifeq ($(STATIC_CONFIG),y)
LDFLAGS += -static
endif

COMMAND_FILES := $(shell \
	find informationes -mindepth 2 -maxdepth 2 \
	-type f -name '*.c' 2>/dev/null | sort)

COMMAND_SRC := $(shell \
	for file in $(COMMAND_FILES); do \
		name=$$(basename "$$file" .c); \
		category=$$(basename "$$(dirname "$$file")"); \
		key=$$(printf '%s_%s' "$$category" "$$name" | \
			tr '[:lower:]' '[:upper:]'); \
		if [ ! -f $(CONFIG_FILE) ] || \
		   grep -qx "CONFIG_$$key=y" $(CONFIG_FILE); then \
			printf '%s ' "$$file"; \
		fi; \
	done)

SRC = main.c etc/common.c $(COMMAND_SRC)

.PHONY: all clean menuconfig

all: $(TARGET)

include/config.h: $(CONFIG_FILE) $(COMMAND_FILES)
	@mkdir -p include
	@{ \
		echo '#ifndef CONFIG_H'; \
		echo '#define CONFIG_H'; \
		echo ''; \
		echo '#include <stddef.h>'; \
		echo '#include "common.h"'; \
		echo ''; \
		for file in $(COMMAND_SRC); do \
			cmd=$$(basename "$$file" .c); \
			echo "int $${cmd}_cmd(int argc, char **argv);"; \
		done; \
		echo ''; \
		echo 'static const struct command commands[] = {'; \
		for file in $(COMMAND_SRC); do \
			cmd=$$(basename "$$file" .c); \
			echo "    { \"$${cmd}\", $${cmd}_cmd },"; \
		done; \
		echo '    { NULL, NULL }'; \
		echo '};'; \
		echo ''; \
		echo '#endif'; \
	} > include/config.h

$(TARGET): include/config.h $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f include/config.h
	rm -f menuconfig

menuconfig:
	$(CC) $(CFLAGS) config/menuconfig.c -lncurses -o menuconfig
	./menuconfig
	$(MAKE) clean
	$(MAKE) all
	rm -f menuconfig
