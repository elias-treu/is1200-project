# File Structure Changes

> **Disclaimer:** The Makefile changes described here were produced with assistance from an AI coding agent and were subsequently build-tested and verified on the FPGA.

The project is organized into separate source, header, library, and build directories:

```text
src/      C and assembly source files
include/  Project headers
lib/      DTEK-V library files and softfloat.a
build/    Generated objects, ELF, binary, and disassembly
```

The Makefile searches `src/` and `lib/` for source files and uses `-Iinclude -Ilib` for headers. All generated files are placed under `build/`; the FPGA image is `build/main.bin`.

Linking is run from inside `build/` so the existing linker directive remains valid:

```ld
STARTUP(boot.o)
```

This is important because `boot.o` must place the reset and interrupt vectors at the beginning of the raw binary. Linking it later with the other objects can produce a binary that builds but does not boot correctly.

`src/main.c` provides the empty `handle_interrupt` required by `src/boot.S`. The application `main()` currently remains in `src/screen.c` while LCD functionality is being tested.

The project can be built with `make` and run with:

```sh
dtekv-run build/main.bin
```
