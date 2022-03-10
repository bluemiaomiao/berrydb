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

BERRYDB_BOOST_VERSION="1.53.0"
_BERRYDB_BOOST_VERSION=${BERRYDB_BOOST_VERSION//./_}

echo "下载Boost依赖库...${BERRYDB_WORKSPACE_DEPS}/boost.tar.gz"
wget --no-check-certificate --output-file="${BERRYDB_WORKSPACE_DEPS}/boost.tar.gz" "https://udomain.dl.sourceforge.net/project/boost/boost/${BERRYDB_BOOST_VERSION}/boost_${_BERRYDB_BOOST_VERSION}.tar.bz2"

echo "释放Boost依赖库文件到${BERRYDB_WORKSPACE}/boost"
tar -xvf ${BERRYDB_WORKSPACE_DEPS}/boost.tar.gz -C ${BERRYDB_WORKSPACE}

echo "准备就绪, 请执行${BERRYDB_WORKSPACE}/tools/build-boost.sh构建!"
