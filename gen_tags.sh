#!/bin/bash - 
#===============================================================================
#          FILE:  gen_tags.sh
#         USAGE:  ./gen_tags.sh 
#   DESCRIPTION:  
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR: LuoQiaofa (Luoqf), luoqiaofa@163.com
#  ORGANIZATION: 
#       CREATED: 07/03/2019 09:31:17 PM EDT
#      REVISION:  ---
#===============================================================================
set -o nounset                              # Treat unset variables as an error

filelist="flist.txt"
# script_path=`dirname $0`
# echo script_path=${script_path}
# cd ${script_path}
export GTAGSROOT=${PWD}
export GTAGSDBPATH=${PWD}
echo tags_path:${GTAGSDBPATH}
# cd ${GTAGSDBPATH}/../..
echo src_root=${GTAGSROOT}
ctags --tag-relative=yes --sort=yes --output-format=u-ctags -f ${GTAGSDBPATH}/tags -L ${GTAGSROOT}/${filelist}
gtags --gtagslabel new-ctags -f ${GTAGSROOT}/${filelist} ${GTAGSDBPATH}

