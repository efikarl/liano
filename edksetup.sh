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
#  A script to setup environment for EDKII.
help_info()
{
  echo '  ------------------------------------------------------------------------  '
  echo '  usage:                                                                    '
  echo '    edksetup [-w WORKSPACE] [-e EDK_TOOLS_PATH] [-p PYTHON_COMMAND]         '
  echo '  option:                                                                   '
  echo '    -w WORKSPACE      --Setup environment variable WORKSPACE for EDKII.     '
  echo '    -t EDK_TOOLS_PATH --Setup environment variable EDK_TOOLS_PATH for EDKII.'
  echo '    -p PYTHON_COMMAND --Setup environment variable PYTHON_COMMAND for EDKII.'
  echo '  ------------------------------------------------------------------------  '
}

env_path_rel_to_abs()
{
  # as "readlink -f $path" under linux, macosx not support till 10.15.5
  pushd . >/dev/null 2>&1 && {
    cd $2 && export $1=$(pwd)
  } && popd >/dev/null 2>&1
}

param_parse()
{
  while getopts "w:t:p:" arg
  do
    case $arg in
    w)
      export WORKSPACE=$OPTARG
      ;;
    t)
      export EDK_TOOLS_PATH=$OPTARG
      ;;
    p)
      export PYTHON_COMMAND=$OPTARG
      ;;
    ?)
      help_info && return 1
      ;;
    esac
  done

  return 0
}

check_envar()
{
  # WORKSPACE: check and try to set
  # if not set
  #   try to set via pwd of this script
  if [[ ! -f $WORKSPACE/MdePkg/MdePkg.dsc ]] && [[ -f $(dirname ${BASH_SOURCE[0]})/MdePkg/MdePkg.dsc ]]
  then
    WORKSPACE=$(dirname ${BASH_SOURCE[0]})
  fi
  # WORKSPACE: check at last
  if [[ ! -f $WORKSPACE/MdePkg/MdePkg.dsc ]]
  then
    echo "[ERR]:${BASH_SOURCE[0]}: Invalid \$WORKSPACE=$WORKSPACE!"
    return 1
  fi
  env_path_rel_to_abs WORKSPACE $WORKSPACE

  # EDK_TOOLS_PATH: check and try to set
  # if not set
  #   1. try to set via BASE_TOOLS_PATH
  if [[ ! -f $EDK_TOOLS_PATH/Conf/target.template ]] && [[ -f $BASE_TOOLS_PATH/Conf/target.template ]]
  then
    EDK_TOOLS_PATH=$BASE_TOOLS_PATH
  fi
  #   2. try to set via $WORKSPACE
  if [[ ! -f $EDK_TOOLS_PATH/Conf/target.template ]] && [[ -f $WORKSPACE/BaseTools/Conf/target.template ]]
  then
    EDK_TOOLS_PATH=$WORKSPACE/BaseTools
  fi
  # EDK_TOOLS_PATH: check at last
  if [[ ! -f $EDK_TOOLS_PATH/Conf/target.template ]]
  then
    echo "[ERR]:${BASH_SOURCE[0]}: Invalid \$EDK_TOOLS_PATH=$EDK_TOOLS_PATH!"
    return 1
  fi
  env_path_rel_to_abs EDK_TOOLS_PATH $EDK_TOOLS_PATH

  # PYTHON_COMMAND: check and try to set
  # if not set
  #   1. set for it from system installation
  PYTHON_COMMAND=$(which python2)
  #   2. override it via BASE_TOOLS_PYTHON
  if [[ -n $BASE_TOOLS_PYTHON ]]
  then
    PYTHON_COMMAND=$BASE_TOOLS_PYTHON
  fi
  # PYTHON_COMMAND: check at last
  if ! ( command -v $PYTHON_COMMAND >/dev/null 2>&1 )
  then
    echo "[ERR]:${BASH_SOURCE[0]}: Invalid \$PYTHON_COMMAND=$PYTHON_COMMAND!"
    return 1
  fi
  export PYTHON_COMMAND

  export PYTHONHASHSEED=1

  return 0
}

create_conf()
{
  [[ -d $WORKSPACE/Conf ]] || mkdir $WORKSPACE/Conf
  for i in $EDK_TOOLS_PATH/Conf/*.template
  do
    cp -f $i $WORKSPACE/Conf/$(basename -s .template $i).txt
  done
  [[ -f $WORKSPACE/Conf/target.txt ]] || return 1

  return 0
}

check_tools()
{
  export PATH=$EDK_TOOLS_PATH/BinWrappers/PosixLike:$PATH

  LAST_TIME_BUILD_OS=$WORKSPACE/ltbos.log

  [[ -f $LAST_TIME_BUILD_OS ]] || echo > $LAST_TIME_BUILD_OS
  # c base tool check, if 1st time build or build os changes
  if [[ $(uname) != $(cat $LAST_TIME_BUILD_OS) ]]
  then
    if ( git submodule init && git submodule update )
    then
      rm -R -f BaseTools/Source/C/BrotliCompress/brotli
      mkdir -p BaseTools/Source/C/BrotliCompress/brotli
      cp -R -f MdeModulePkg/Library/BrotliCustomDecompressLib/brotli/* BaseTools/Source/C/BrotliCompress/brotli
    fi
    echo "clean base c tools ..."
    make -C $EDK_TOOLS_PATH/Source/C clean 1>/dev/null 2>&1
    echo "build base c tools ..."
    make -C $EDK_TOOLS_PATH/Source/C 1>/dev/null 2>&1
    result=$([[ $? == 0 ]] && echo pass || echo fail)
    echo "build base c tools end: $result"
    if [[ $result == fail ]]
    then
      return 1
    else
      echo $(uname) > $LAST_TIME_BUILD_OS
    fi
  fi
  # python base tool check, not build and use script directly
  if [[ ! -f $EDK_TOOLS_PATH/Source/Python/build/build.py ]]
  then
    echo "[ERR]:${BASH_SOURCE[0]}: $EDK_TOOLS_PATH/Source/Python!"
    return 1
  fi

  return 0
}

main()
{
  param_parse $@
  [[ $? != 0 ]] && return 1
  check_envar
  [[ $? != 0 ]] && return 2
  create_conf
  [[ $? != 0 ]] && return 3
  check_tools
  [[ $? != 0 ]] && return 4
  echo EDKII setup is done!
  return 0
}

main $@
