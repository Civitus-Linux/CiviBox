
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =
LDLIBS =

TARGET = civibox
CONFIG_FILE = .config

STATIC_CONFIG := $(shell grep -qx 'CONFIG_COMPILE_STATIC=y' $(CONFIG_FILE) 2>/dev/null && echo y)

ifeq ($(STATIC_CONFIG),y)
LDFLAGS += -static
endif

NORMAL_SOURCES := $(shell \
	for file in informationes/*.c; do \
		[ -f "$$file" ] || continue; \
		cmd=$$(basename "$$file" .c); \
		key=$$(printf '%s' "$$cmd" | tr '[:lower:]' '[:upper:]'); \
		if [ ! -f $(CONFIG_FILE) ] || grep -qx "CONFIG_$$key=y" $(CONFIG_FILE); then \
			echo "$$file"; \
		fi; \
	done)

EXTRA_SOURCES := $(shell \
	for file in extra/*.c; do \
		[ -f "$$file" ] || continue; \
		cmd=$$(basename "$$file" .c); \
		key=$$(printf '%s' "$$cmd" | tr '[:lower:]' '[:upper:]'); \
		if [ ! -f $(CONFIG_FILE) ] || grep -qx "CONFIG_EXTRA_$$key=y" $(CONFIG_FILE); then \
			echo "$$file"; \
		fi; \
	done)

COMMAND_SOURCES = $(NORMAL_SOURCES) $(EXTRA_SOURCES)
SRC = main.c etc/common.c $(COMMAND_SOURCES)
OBJ = $(SRC:.c=.o)

ifneq ($(strip $(EXTRA_SOURCES)),)
BUILD_INFO = using extra commands
else
BUILD_INFO = using standard commands
endif

.PHONY: all build link config menuconfig clean distclean FORCE

all: build

include/config.h: FORCE $(COMMAND_SOURCES)
	@mkdir -p include
	@{ \
		echo '#ifndef CONFIG_H'; \
		echo '#define CONFIG_H'; \
		echo '#include <stddef.h>'; \
		echo '#include "common.h"'; \
		for src in $(COMMAND_SOURCES); do \
			cmd=$$(basename "$$src" .c); \
			echo "int $${cmd}_cmd(int argc, char **argv);"; \
		done; \
		echo 'static const struct command commands[] = {'; \
		for src in $(COMMAND_SOURCES); do \
			cmd=$$(basename "$$src" .c); \
			echo "    { \"$$cmd\", $${cmd}_cmd },"; \
		done; \
		echo '    { NULL, NULL }'; \
		echo '};'; \
		echo '#endif'; \
	} > include/config.h.tmp
	@cmp -s include/config.h.tmp include/config.h || mv include/config.h.tmp include/config.h
	@rm -f include/config.h.tmp

build: include/config.h
	@printf "Compiling CiviBox (%s)\n" "$(BUILD_INFO)"
	@set -e; \
	( \
		set -e; \
		for src in $(SRC); do \
			obj=$${src%.c}.o; \
			$(CC) $(CFLAGS) -c "$$src" -o "$$obj"; \
		done \
	) & \
	pid=$$!; \
	while kill -0 $$pid 2>/dev/null; do \
		sleep 1; \
		kill -0 $$pid 2>/dev/null && printf "."; \
	done; \
	wait $$pid || { printf "\n"; echo "failed"; exit 1; }; \
	printf "\n"; \
	$(MAKE) --no-print-directory link

link:
	@$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(TARGET) || exit 1
	@echo "ok"

menuconfig:
	@bash scripts/gen.sh
	@$(CC) $(CFLAGS) config/menuconfig.c -lncurses -o menuconfig
	@./menuconfig
	@rm -f menuconfig

config: menuconfig

clean:
	@find . -name '*.o' -delete
	@rm -f $(TARGET) menuconfig include/config.h include/config.h.tmp
	@echo "ok"

distclean: clean
	@rm -f $(CONFIG_FILE) etc/commands.c
	@echo "ok"

FORCE:

