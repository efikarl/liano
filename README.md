# liano ![CI.GCC](https://github.com/efikarl/liano/workflows/CI/badge.svg?branch=master)

**This is Personal Debug and Study Repo for UEFI on OVMF of EDK2.**

## User Guide

#### How to build?

See repo [liano.build](https://github.com/efikarl-docker/liano.build) please.

#### How to run?

If you're under Unix-like OS, and make sure qemu is installed as a prerequisite. Just:

```shell
# Suppose OVMF for arch X64 is built as step before
# You can simply run it by calling OvmfPkg/build.sh with 'qemu' and '-a X64' options
bash OvmfPkg/build.sh qemu -a X64
```

Then a debug log `debug.log` will be found in your workspace, you can use it to debug your idea. 

But if you're in Windows OS, This is a bit difficult.  Waiting for WSL2 GUI ready for this.

