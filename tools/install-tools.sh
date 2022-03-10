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

echo "正在安装基本开发框架..."

sudo apt-get update -y

if [ $? -ne 0 ]; then
    echo "软件源更新失败, 请检查网络连接和相关权限并重试"
    exit -1
fi

sudo apt-get cmake git build-essential -y

if [ $? -ne 0 ]; then
    echo "安装失败, 请检查网络连接和相关权限并重试"
fi