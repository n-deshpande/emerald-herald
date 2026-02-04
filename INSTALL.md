# Build & Setup

This is a standalone romhack fork. These instructions cover the minimal setup to build and test locally.

## Prerequisites

- `git`
- `make`
- A GCC ARM toolchain that provides `arm-none-eabi-gcc` (devkitARM via devkitPro is the common choice)

Windows users should use WSL2 for a smoother toolchain setup.

## Build

```console
make
```

The output ROM will be `pokeemerald.gba` in the repo root.

## Tests

```console
make check
```

## Parallel Builds

```console
make -j$(nproc)
```

On macOS, use `sysctl -n hw.ncpu` instead of `nproc`.

## Debug Build

```console
make debug
```

## Optional Tools

- PoryMap (maps)
- Porytiles (metatiles)
- Poryscript (scripting)

Use whichever tools match your workflow; they are not required to build.
