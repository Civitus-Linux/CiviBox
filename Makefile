CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =
LDLIBS =

BUILD_DIR = build
TARGET = $(BUILD_DIR)/civibox
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

SRC = main.c etc/common.c $(NORMAL_SOURCES) $(EXTRA_SOURCES)
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

ifneq ($(strip $(EXTRA_SOURCES)),)
BUILD_INFO = using extra commands
else
BUILD_INFO = using standard commands
endif

.PHONY: all build config menuconfig clean distclean FORCE

all: build

FORCE:

include/config.h: FORCE $(COMMAND_SOURCES)
	@mkdir -p include
	@{ \
		echo '#ifndef CONFIG_H'; \
		echo '#define CONFIG_H'; \
		echo '#include <stddef.h>'; \
		echo '#include "common.h"'; \
		for src in $(SRC); do \
			case "$$src" in main.c|etc/common.c) continue ;; esac; \
			cmd=$$(basename "$$src" .c); \
			echo "int $${cmd}_cmd(int argc, char **argv);"; \
		done; \
		echo 'static const struct command commands[] = {'; \
		for src in $(SRC); do \
			case "$$src" in main.c|etc/common.c) continue ;; esac; \
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
	@mkdir -p $(BUILD_DIR)
	@printf "Compiling CiviBox (%s)\n" "$(BUILD_INFO)"
	@set -e; \
	( \
		set -e; \
		for src in $(SRC); do \
			obj="$(BUILD_DIR)/$${src%.c}.o"; \
			mkdir -p "$$(dirname "$$obj")"; \
			$(CC) $(CFLAGS) -c "$$src" -o "$$obj"; \
		done \
	) & \
	pid=$$!; \
	while kill -0 $$pid 2>/dev/null; do \
		sleep 1; \
		kill -0 $$pid 2>/dev/null && printf "."; \
	done; \
	wait $$pid || { printf "\n"; exit 1; }; \
	printf "\n"; \
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(TARGET) || exit 1; \
	echo "ok"

menuconfig:
	@mkdir -p $(BUILD_DIR)
	@bash scripts/gen.sh
	@$(CC) $(CFLAGS) config/menuconfig.c -lncurses -o $(BUILD_DIR)/menuconfig
	@./$(BUILD_DIR)/menuconfig
	@rm -f $(BUILD_DIR)/menuconfig

config: menuconfig

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f include/config.h include/config.h.tmp
	@rm */*.o
	@echo "ok"

distclean: clean
	@rm -f $(CONFIG_FILE) etc/commands.c
	@echo "ok"
