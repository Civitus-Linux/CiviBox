CC = gcc

CFLAGS = -Wall -Wextra -O2 -I$(CURDIR)/include -I$(CURDIR)/include/auth
LDFLAGS =
LDLIBS = -lcrypt

TARGET = civibox
BUILD_DIR = build

CONFIG_FILE := $(wildcard .config)

STATIC_CONFIG := $(shell grep -qx 'CONFIG_COMPILE_STATIC=y' .config 2>/dev/null && echo y)

ifeq ($(STATIC_CONFIG),y)
LDFLAGS += -static
endif

INFO_SOURCES := $(shell find informationes -mindepth 2 -maxdepth 2 -type f -name '*.c' 2>/dev/null | sort)

ifeq ($(CONFIG_FILE),)
COMMAND_SOURCES := $(INFO_SOURCES)
else
COMMAND_SOURCES := $(shell \
	for file in $(INFO_SOURCES); do \
		cmd=$$(basename "$$file" .c); \
		category=$$(basename "$$(dirname "$$file")" | tr '[:lower:]' '[:upper:]'); \
		key=$$(printf '%s' "$$cmd" | tr '[:lower:]' '[:upper:]'); \
		grep -qx "CONFIG_$${category}_$${key}=y" .config && printf '%s ' "$$file"; \
	done)
endif

COMMANDS := $(foreach file,$(COMMAND_SOURCES),$(basename $(notdir $(file))))

SRC = main.c etc/common.c include/auth/auth.c $(COMMAND_SOURCES)

.PHONY: all clean menuconfig

all: $(BUILD_DIR)/$(TARGET)

include/config.h: $(CONFIG_FILE) $(INFO_SOURCES)
	@mkdir -p include
	@echo "#ifndef CONFIG_H" > include/config.h
	@echo "#define CONFIG_H" >> include/config.h
	@echo "" >> include/config.h
	@echo '#include <stddef.h>' >> include/config.h
	@echo '#include "common.h"' >> include/config.h
	@echo "" >> include/config.h
	@for cmd in $(COMMANDS); do \
		echo "int $${cmd}_cmd(int argc, char **argv);" >> include/config.h; \
	done
	@echo "" >> include/config.h
	@echo "static const struct command commands[] = {" >> include/config.h
	@for cmd in $(COMMANDS); do \
		echo '    { "'$${cmd}'", '$${cmd}'_cmd },' >> include/config.h; \
	done
	@echo "    { NULL, NULL }" >> include/config.h
	@echo "};" >> include/config.h
	@echo "" >> include/config.h
	@echo "#endif" >> include/config.h

$(BUILD_DIR)/$(TARGET): include/config.h $(SRC)
	@mkdir -p $(BUILD_DIR)
	@printf "Compiling Civibox\n"
	@$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(LDLIBS) -o "$@" & \
	pid=$$!; \
	while kill -0 $$pid 2>/dev/null; do \
		printf "."; \
		sleep 0.25; \
	done; \
	wait $$pid; status=$$?; \
	printf "\n"; \
	if [ $$status -eq 0 ]; then \
		printf "Compiled!\n"; \
	else \
		rm -f "$@"; \
		exit $$status; \
	fi

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f include/config.h
	@rm -f menuconfig
	@printf "ok\n"

menuconfig:
	@rm -f $(BUILD_DIR)/menuconfig
	@mkdir -p $(BUILD_DIR)
	@./scripts/gen.sh
	@$(CC) $(CFLAGS) config/menuconfig.c -lncurses -o $(BUILD_DIR)/menuconfig
	@./$(BUILD_DIR)/menuconfig
	@rm -f $(BUILD_DIR)/menuconfig
