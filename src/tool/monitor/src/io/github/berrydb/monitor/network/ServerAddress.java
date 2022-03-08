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

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;

public class ServerAddress {
    private final static String DEFAULT_HOST = "127.0.0.1";
    private final static int DEFAULT_PORT = 2022;
    private InetSocketAddress hostAddress;
    private String host;
    private int port;

    public ServerAddress() {
        this(new InetSocketAddress(DEFAULT_HOST, DEFAULT_PORT));
    }

    public ServerAddress(String host, int port) throws UnknownHostException {
        hostAddress = new InetSocketAddress(InetAddress.getByName(host).toString().split("/")[1], port);
        this.host = host;
        this.port = port;
    }

    public ServerAddress(String host) throws UnknownHostException {
        if (host.indexOf(":") > 0) {
            String[] tmp = host.split(":");
            this.host = tmp[0].trim();
            try {
                this.host = InetAddress.getByName(this.host).toString().split("/")[1];
            } catch (Exception e) {
                throw new BaseException("SDB_INVALID_ARG");
            }
            this.port = Integer.parseInt(tmp[1].trim());
        } else {
            this.host = host;
            this.port = DEFAULT_PORT;
        }
        hostAddress = new InetSocketAddress(this.host, this.port);
    }

    public ServerAddress(InetAddress addr) {
        this(new InetSocketAddress(addr, DEFAULT_PORT));
    }

    public ServerAddress(InetAddress addr, int port) {
        this(new InetSocketAddress(addr, port));
    }

    public ServerAddress(InetSocketAddress addr) {
        hostAddress = addr;
        host = addr.getHostName();
        port = addr.getPort();
    }

    public InetSocketAddress getHostAddress() {
        return hostAddress;
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }
}
