/*******************************************************************************
 Copyright (C) 2021 BerryDB Software Inc.
 This application is free software: you can redistribute it and/or modify it
 under the terms of the GNU Affero General Public License, Version 3, as
 published by the Free Software Foundation.

 This application is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
 details.

 You should have received a copy of the GNU Affero General Public License along
 with this application. If not, see <http://www.gnu.org/license/>
 *******************************************************************************/

package io.github.berrydb.monitor.network;

import io.github.berrydb.monitor.exception.BaseException;

public interface IConnection {
    public void close();
    public boolean connect() throws BaseException;
    public void changeConfigOptions(ConfigOptions opts) throws BaseException;
    public boolean sendMessage(byte[] msg) throws BaseException;
    public byte[] receiveMessage() throws BaseException;
}
