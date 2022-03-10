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

echo "欢迎来到BerryDB编译环境就绪程序!"

if [ ! -n $1 ]; then
    echo "致命的错误, 缺少必要的参数"
    exit -1
fi

if [ ! -d $1 ]; then
    echo "$1 不存在, 请指定正确的本地Git存储库位置"
    exit -1
else
    echo "使用 $1 作为工作目录"
    export BERRYDB_WORKSPACE=$1

    export BERRYDB_WORKSPACE_DEPS="${BERRYDB_WORKSPACE}/deps"
    echo "将 ${BERRYDB_WORKSPACE} 作为软件包依赖目录, 后续你可以删除该目录"
fi

echo "创建必要的目录"
mkdir -p ${BERRYDB_WORKSPACE_DEPS}

${BERRYDB_WORKSPACE}/tools/download-boost.sh
${BERRYDB_WORKSPACE}/tools/build-boost.sh

echo "恭喜, 编译环境已经就绪, 你可以使用Mircosoft VS Code/JetBrains CLion/GNU Emacs/GNU Vim等开始工作!"