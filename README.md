CiviBox: toolkit POSIX.

--- Getting started

You can clone this repository to compile CiviBox, so this should work:

git clone https://github.com/Civitus-Linux/CiviBox.git
cd CiviBox
make

This produces the "civibox" multicall binary.

You can run commands through CiviBox by passing the command name as
the first argument:

./civibox ls
./civibox cat file.txt
./civibox mkdir test

CiviBox is designed to provide small POSIX-style utilities through a
single executable.

--- Building CiviBox

CiviBox uses the "make menuconfig; make" idiom, similar to the Linux kernel
and BusyBox.

Usually you want:

make menuconfig
make

The "make menuconfig" command opens the CiviBox configuration interface.
This allows you to select which commands are included in the resulting
binary.

The configuration is stored in ".config".

For example:

CONFIG_COMPILE_STATIC=y

CONFIG_CAT=y
CONFIG_LS=y
CONFIG_CP=y

# CONFIG_MV is not set

CONFIG_RM=y
CONFIG_MKDIR=y

After changing the configuration, run "make" again to rebuild CiviBox.

To remove generated files and the compiled binary:

make clean

--- Using CiviBox

The CiviBox build produces a multicall binary, a program that provides
multiple commands through a single executable.

The first argume
