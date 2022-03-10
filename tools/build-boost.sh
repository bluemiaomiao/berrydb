# *******************************************************************************
#                   Copyright (C) 2021 BerryDB Software Inc.
# This application is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero General Public License, Version 3, as
# published by the Free Software Foundation.
#
# This application is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
# details.
#
# You should have received a copy of the GNU Affero General Public License along
# with this application. If not, see <http://www.gnu.org/license/>
# *******************************************************************************/

#!/usr/bin/env bash

echo "编译Boost依赖库, 这可能需要很长时间..."
cd ${BERRYDB_WORKSPACE}/boost

BERRYDB_BOOST_BUILD_LOG=${BERRYDB_WORKSPACE}/boost-build.log

./bootstrap.sh --prefix=. --with-icu=/usr/lib/x86_64-linux-gnu

if [ $? -ne 0 ]; then
    echo "执行准备任务遇到了错误"
    exit -1
fi

echo "即将生成编译日志: ${BERRYDB_BOOST_BUILD_LOG}"
echo "帮助: 使用 \"tail -f ${BERRYDB_BOOST_BUILD_LOG}\" 可以实时查看编译进度"

./bjam link=static threading=multi variant=release address-model=64 toolset=gcc runtime-link=shared architecture=ia64 cxxflags="-std=c++11" binary-format=elf > ${BERRYDB_BOOST_BUILD_LOG} 2>&1

if [ $? -eq 0 ]; then
    echo "恭喜, 编译成功!"
else
    echo "编译失败, 请查看 ${BERRYDB_BOOST_BUILD_LOG} 文件寻找可能的帮助"
fi
