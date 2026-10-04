CC = cc
CFLAGS = -Wall -Wextra -O2

TARGET = civibox

COMMANDS ?= cat

SRC = main.c etc/common.c

$(foreach cmd,$(COMMANDS),$(eval SRC += informationes/$(cmd).c))

.PHONY: all clean menuconfig

all: config.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

config.h:
	@echo "#ifndef CONFIG_H" > ./include/config.h
	@echo "#define CONFIG_H" >> ./include/config.h
	@echo "" >> ./include/config.h
	@echo '#include <stddef.h>' >> ./include/config.h
	@echo '#include "common.h"' >> ./include/config.h
	@echo "" >> ./include/config.h

	@for cmd in $(COMMANDS); do \
		echo "int $${cmd}_cmd(int argc, char **argv);" >> ./include/config.h; \
	done

	@echo "" >> ./include/config.h
	@echo "static const struct command commands[] = {" >> ./include/config.h

	@for cmd in $(COMMANDS); do \
		echo '    { "'$${cmd}'", '$${cmd}'_cmd },' >> ./include/config.h; \
	done

	@echo "    { NULL, NULL }" >> ./include/config.h
	@echo "};" >> ./include/config.h
	@echo "" >> ./include/config.h
	@echo "#endif" >> ./include/config.h

clean:
	rm -f $(TARGET)
	rm -f ./include/config.h
	rm -f menuconfig

menuconfig:
	rm -f menuconfig
	./scripts/gen.sh
	$(CC) $(CFLAGS) config/menuconfig.c -lncurses -o menuconfig
	./menuconfig
	rm -f menuconfig