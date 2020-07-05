# liano ![CI.GCC](https://github.com/efikarl/liano/workflows/CI/badge.svg?branch=master)

**This is Personal Debug and Study Repo for UEFI on OVMF of EDK2.**

## User Guide

#### How to build?

For convenience, I've built a docker [efikarl/ovmf.build](https://hub.docker.com/r/efikarl/ovmf.build) for this task.

```shell
# 1st, ensure you have a id for access https://hub.docker.com
docker pull efikarl/ovmf.build

# Clone https://github.com/efikarl/liano.git and cd workspace
git clone https://github.com/efikarl/liano.git && cd liano

# Build OVMF for arch IA32
docker run --rm -v ${PWD}:/ws efikarl/ovmf.build -a IA32

# Build OVMF for arch X64
docker run --rm -v ${PWD}:/ws efikarl/ovmf.build -a X64

# Build OVMF for arch IA32 X64
docker run --rm -v ${PWD}:/ws efikarl/ovmf.build -a IA32 -a X64
```

When building is done, you will find `OVMF.fd` in `Build/Ovmf*/DEBUG_GCC*/FV/OVMF.fd`.

#### How to run?

If you want to run `OVMF.fd`, you must make sure qemu is installed as a prerequisite. Atfer that:

```shell
# Suppose OVMF for arch X64 is built as step before
# You can simply run it by calling OvmfPkg/build.sh with 'qemu' and '-a X64' options
bash OvmfPkg/build.sh qemu -a X64
```

Then a debug log `debug.log` will be found in your workspace, you can use it to debug your idea. 

#### How to contribute to?

The `devel` repo is default repo for development, and master repo is a repo for tag and release. So:

- No `pull request` or `push` to master directly is allowed except me.
- If you'd like to merge your code to `devel`, be sure:
  - Your code must follow C coding standard of EDK2.
  - You are agree with Google's [The Standard of Code Review](https://google.github.io/eng-practices/review/reviewer/standard.html).

**Note: **

​    **This repo is for personal debug and study purpose. You're not encouraged to contribute to it, you can contribute to [EDKII](https://github.com/tianocore/edk2) instead. but if you'd like, you're welcome.**
