#!/bin/bash
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
  echo "[ERR]:${BASH_SOURCE:-$0}: not in workspace, call this script in workspace please!"
  exit 1
fi

# rm non-ovmf pkgs
for f in $(ls -a)
do
  if [[ $f != BaseTools ]] && [[ $f != Conf ]] && [[ $f != KarlPkg ]]
  then
    cat OvmfPkg/OvmfPkgIa32X64.dsc | grep $f > /dev/null || rm -rf $f
  fi
done

# add back those should be there
git checkout HEAD -- EmbeddedPkg .gitignore .gitmodules edksetup.sh License.txt License-History.txt Maintainers.txt
