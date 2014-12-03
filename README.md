# TasTools mod

This mod is used as part of the process of Half-Life TAS creation. We do not
consider the demos recorded in TasTools mod to be legitimate. The `genlegit.py`
script is required to produce legitimate scripts. This repo also contains
qconread, which is a Qt5 GUI utility that displays the console output in an
accessible way.

Assuming Qt5 is installed, you can build qconread in Linux by typing `qmake` in
`utils/qconread` to generate the Makefile. Once the Makefile is generated you
only need `make` for subsequent builds.

Currently, only Linux is supported.  To build the mod, enter the `injectlib`
folder and type `make`.  A shared library named `tasinjectlib.so` will be
created.  To inject this library into Half-Life, set `LD_PRELOAD` to the path
of this library before running the game.
