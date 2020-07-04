#!/usr/bin/env bash
#
#  Copyright ©2020 efikarl, https://efikarl.com.
#
#  This program is just made available under the terms and conditions of the
#  MIT license: https://efikarl.com/mit-license.html.
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

if [[ ! -d MdePkg ]]
then
  echo "[ERR]:${BASH_SOURCE:-$0}: snot in workspace, call this script in workspace please!"
  exit 1
fi

# rm all files in basetools
rm -rf ./BaseTools/*

# add back those should be there
git checkout HEAD --    \
    BaseTools/Conf/XMLSchema                                    \
    BaseTools/Conf/*.template                                   \
    BaseTools/Scripts/GccBase.lds                               \
    BaseTools/Source                                            \
    BaseTools/UserManuals                                       \
    BaseTools/GNUmakefile                                       \

# setup bin wrapper BaseTools/BinWrappers/PosixLike
C_TOOLS="Brotli DevicePath EfiRom GenCrc32 GenFfs GenFv GenFw GenSec LzmaCompress Split TianoCompress VfrCompile VolInfo"
S_TOOLS="AmlToHex BPDG Ecc GenFds GenPatchPcdTable PatchPcdValue Pkcs7Sign Rsa2048Sha256Sign TargetTool Trim UPT build"
__TOOLS="BrotliCompress GenDepex GenerateCapsule LzmaF86Compress"

mkdir -p BaseTools/BinWrappers/PosixLike
for tool in $C_TOOLS
do
  cp -f KarlPkg/EfiUtils/Scripts/1_basetools_clean/1.c.sh.template    BaseTools/BinWrappers/PosixLike/$tool
done
for tool in $S_TOOLS
do
  cp -f KarlPkg/EfiUtils/Scripts/1_basetools_clean/1.s.sh.template    BaseTools/BinWrappers/PosixLike/$tool
done
cp -f BaseTools/BinWrappers/PosixLike/Brotli                          BaseTools/BinWrappers/PosixLike/BrotliCompress
cp -f KarlPkg/EfiUtils/Scripts/1_basetools_clean/1.s.sh._.gendepex    BaseTools/BinWrappers/PosixLike/GenDepex
cp -f KarlPkg/EfiUtils/Scripts/1_basetools_clean/1.s.sh._.gencaps     BaseTools/BinWrappers/PosixLike/GenerateCapsule
cp -f KarlPkg/EfiUtils/Scripts/1_basetools_clean/1.s.sh._.lzma_f86    BaseTools/BinWrappers/PosixLike/LzmaF86Compress
