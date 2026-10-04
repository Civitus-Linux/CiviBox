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
