#!/usr/bin/env bash
#
#  Copyright ©2020 efikarl, https://efikarl.com.
#
#  This program is just made available under the terms and conditions of the
#  MIT license: https://efikarl.com/mit-license.html.
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#  A script to build OVMF.
set -e
shopt -s nocasematch

THE_SCRIPT=${BASH_SOURCE:-$0}
SCRIPT_DIR=$(dirname ${THE_SCRIPT})
#
# Setup workspace if it is not set
#
args=$@
if [ -z $WORKSPACE ] || [ -z $EDK_TOOLS_PATH ]
then
  echo Setup workspace ...
  if [ -f $SCRIPT_DIR/../edksetup.sh ]
  then
    set --
    source $SCRIPT_DIR/../edksetup.sh
    set -- $args
  else
    echo "[ERR]:$THE_SCRIPT: no edksetup.sh found in \$WORKSPACE=$WORKSPACE!"
    exit 1
  fi
fi
#
# Filter out _BUILD_ARGS_ and _QEMUx_ARGS_
#
while [[ $# > 0 ]]
do
  case $1 in
  -p | -a | -b | -t | -n)
    _BUILD_ARGS_="$_BUILD_ARGS_ $1 $2"
    shift 2
    ;;
  *)
    _QEMUx_ARGS_="$_QEMUx_ARGS_ $1"
    shift 1
    ;;
  esac
done
#
# _BUILD_ARGS_ parsing to override rules in target.txt
#
set -- $_BUILD_ARGS_
while getopts a:b:p:t:n: arg
do
  case $arg in
  p)
    BUILD_PLAT=$OPTARG;;
  a)
    BUILD_ARGS="$BUILD_ARGS -a $OPTARG" && BUILD_ARCH=$BUILD_ARCH$OPTARG;;
  b)
    BUILD_ARGS="$BUILD_ARGS -b $OPTARG" && BUILD_TYPE=$OPTARG;;
  t)
    BUILD_ARGS="$BUILD_ARGS -t $OPTARG" && BUILD_TOOL=$OPTARG;;
  n)
    BUILD_ARGS="$BUILD_ARGS -n $OPTARG" && THREAD_NUM=$OPTARG;;
  ?)
    echo "[ERR]:$THE_SCRIPT: invalid build option: $arg!"
    exit 1
    ;;
  esac
done
#
# Setup default build arguments that not given
#
case $BUILD_ARCH in
  IA32)
    if [[ -z $BUILD_PLAT ]]
    then
      BUILD_PLAT=OvmfPkg/OvmfPkgIa32.dsc
    fi
    PLATFORM_BUILD_DIR=OvmfIa32
    ;;
  X64)
    if [[ -z $BUILD_PLAT ]]
    then
      BUILD_PLAT=OvmfPkg/OvmfPkgX64.dsc
    fi
    PLATFORM_BUILD_DIR=OvmfX64
    ;;
  IA32X64 | X64IA32)
    if [[ -z $BUILD_PLAT ]]
    then
      BUILD_PLAT=OvmfPkg/OvmfPkgIa32X64.dsc
    fi
    PLATFORM_BUILD_DIR=Ovmf3264
    ;;
  *)
    if [[ -z $BUILD_ARCH ]]
    then
      BUILD_PLAT=OvmfPkg/OvmfPkgIa32X64.dsc
      PLATFORM_BUILD_DIR=Ovmf3264
    else
      echo "[ERR]:$THE_SCRIPT: invaild build arch till now: $BUILD_ARCH!"
      exit 1
    fi
    ;;
esac
  BUILD_ARGS="$BUILD_ARGS -p $BUILD_PLAT"
[[ -z $BUILD_ARCH ]] && {
  BUILD_ARCH=X64
  BUILD_ARGS="$BUILD_ARGS -a IA32 -a X64"
}
[[ -z $BUILD_TYPE ]] && {
  BUILD_TYPE=DEBUG
  BUILD_ARGS="$BUILD_ARGS -b $BUILD_TYPE"
}
[[ -z $BUILD_TOOL ]] && {
  case `uname` in
  Darwin*)
    BUILD_TOOL=XCODE5;;
  Linux*)
    BUILD_TOOL=GCC5;;
  esac
  BUILD_ARGS="$BUILD_ARGS -t $BUILD_TOOL"
}
  BUILD_ARGS="$BUILD_ARGS -j $WORKSPACE/build.log"
#
# _QEMUx_ARGS_ parsing
#
set -- $_QEMUx_ARGS_
while [[ $# > 0 ]]
do
  case $1 in
  qemu)
    RUN_QEMU=RunGumpRun
    shift 1
    ;;
  *)
    QEMUx_ARGS="$QEMUx_ARGS $1"
    shift 1
    ;;
  esac
done
#
# Build or Run OVMF
#
if [[ $RUN_QEMU != RunGumpRun ]]
then
  echo build $BUILD_ARGS && build $BUILD_ARGS
else
  case $BUILD_ARCH in
  IA32)
    QEMU_CMD=qemu-system-i386
    ;;
  X64 | IA32X64 | X64IA32)
    QEMU_CMD=qemu-system-x86_64
    ;;
  *)
    echo "[ERR]:$THE_SCRIPT: unsupported cpu architecture: $OPTARG!"
    exit 1
    ;;
  esac
  if ! ( command -v $QEMU_CMD >/dev/null 2>&1 )
  then
    echo "[ERR]:$THE_SCRIPT: qemu is not found!"
    exit 1
  fi

  OVMF_FD=$WORKSPACE/Build/$PLATFORM_BUILD_DIR/"$BUILD_TYPE"_"$BUILD_TOOL"/FV/OVMF.fd
  [[ -f $OVMF_FD ]] || build $BUILD_ARGS
  case $QEMU_VER in
  0.*.* | 1.[0-5].*)
    QEMUx_ARGS="$QEMUx_ARGS -bios $OVMF_FD"
    ;;
  *)
    QEMUx_ARGS="$QEMUx_ARGS -drive if=pflash,format=raw,file=$OVMF_FD"
    ;;
  esac

  QEMU_DBG="-debugcon file:debug.log -global isa-debugcon.iobase=0x402"
  echo "Running: $QEMU_CMD $QEMUx_ARGS" && cd $WORKSPACE && $QEMU_CMD $QEMU_DBG $QEMUx_ARGS
fi
