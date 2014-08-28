# TasTools mod

This mod is used as part of the process of Half-Life TAS creation. We do not
consider the demos recorded in TasTools mod to be legitimate. The `genlegit.py`
script is required to produce legitimate scripts. This repo also contains
qconread, which is a Qt5 GUI utility that displays the console output in an
accessible way.

Assuming Qt5 is installed, you can build qconread in Linux by typing `qmake` in
`utils/qconread` to generate the Makefile. Once the Makefile is generated you
only need `make` for subsequent builds.

To build TasTools mod in Linux, you would do a `make` in the `linux`
directory. Then `client.so` and `hl.so` will be created in `linux/release`. To
build the mod in Windows instead, open `projects.sln` under `projects\vs2013`.
