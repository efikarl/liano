#!/usr/bin/env bash
#
#  Copyright ©2020 efikarl, https://efikarl.com.
#
#  This program is just made available under the terms and conditions of the
#  MIT license: https://efikarl.com/mit-license.html.
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE MIT LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

help_info()
{
  echo '  ------------------------------------------------------------------------  '
  echo '  usage: submodule init                                                     '
  echo '    ${BASH_SOURCE:-$0} [-s]                                                 '
  echo '  option:                                                                   '
  echo '    -s                --Sync submodule for BaseTools.                       '
  echo '  ------------------------------------------------------------------------  '
}

param_parse()
{
  while getopts "s" arg
  do
    case $arg in
    s)
      export SUBMOD_BASE_TOOLS_SYNC=todo
      ;;
    ?)
      help_info && return 1
      ;;
    esac
  done
}

submod_add()
{
  echo "submod_add ..."
  for i in {1..3}
  do
    if [[ -n ${submod_key[$i]} ]] && [[ -n ${submod_url[$i]} ]]
    then
      git submodule add --name ${submod_key[$i]} ${submod_url[$i]} ${submod_dir[$i]}
    fi
  done
  echo "submod_add done"
}

submod_del()
{
  echo "submod_del ..."
  for i in {1..3}
  do
    if [[ -n ${submod_key[$i]} ]] && [[ -n ${submod_url[$i]} ]]
    then
      ( git submodule deinit ${submod_dir[$i]} || git rm -rf ${submod_dir[$i]} || rm -rf ${submod_dir[$i]} ) 1>/dev/null 2>&1 && {
        rm -rf .git/modules/${submod_key[$i]}
      }
    fi
  done
  echo "submod_del done"
}

submod_basetools_sync()
{
  if ( git submodule init && git submodule update )
  then
    rm -R -f BaseTools/Source/C/BrotliCompress/brotli
    mkdir -p BaseTools/Source/C/BrotliCompress/brotli
    cp -R -f MdeModulePkg/Library/BrotliCustomDecompressLib/brotli/* BaseTools/Source/C/BrotliCompress/brotli
  fi
}

main()
{
  if [[ ! -d MdePkg ]]
  then
    echo "[ERR]:${BASH_SOURCE:-$0}: not in workspace, call this script in workspace please!"
    exit 1
  fi

  param_parse $@
  [[ $? != 0 ]] && return 1

  # submodule configuration
  declare -a submod_key
  declare -a submod_url
  declare -a submod_dir
  submod_key[1]=openssl
  submod_url[1]=https://github.com/openssl/openssl
  submod_dir[1]=CryptoPkg/Library/OpensslLib/openssl
  submod_key[2]=brotli
  submod_url[2]=https://github.com/google/brotli
  submod_dir[2]=MdeModulePkg/Library/BrotliCustomDecompressLib/brotli
  submod_key[3]=oniguruma
  submod_url[3]=https://github.com/kkos/oniguruma
  submod_dir[3]=MdeModulePkg/Universal/RegularExpressionDxe/oniguruma

  if [[ $SUBMOD_BASE_TOOLS_SYNC == todo ]]
  then
    submod_basetools_sync
  else
    submod_del && submod_add
    # next: pls git checkout to right commit for submodules by self
  fi
}

main $@
