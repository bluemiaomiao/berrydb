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

package io.github.berrydb.core;

import io.github.berrydb.excetion.BaseException;
import io.github.berrydb.network.ConfigOptions;
import io.github.berrydb.network.ConnectionTCPImpl;
import io.github.berrydb.network.IConnection;
import io.github.berrydb.network.ServerAddress;
import io.github.berrydb.util.HashAlgorithm;
import io.github.berrydb.util.KetamaNodeLocator;

import java.io.*;
import java.net.UnknownHostException;
import java.util.*;

public class DatabaseCommon {
    public KetamaNodeLocator locator = null;
    private List<ServerAddress> serverAddressList = null;
    private Map<ServerAddress, IConnection> connectionMap = null;

    private final int VIRTUAL_NODE_COUNT = 160;

    public KetamaNodeLocator getLocator() {
        return locator;
    }

    public Map<ServerAddress, IConnection> getConnectionMap() {
        return connectionMap;
    }

    public DatabaseCommon() {
    }

    public int init(String configFile) {
        File file = new File(configFile);
        if(!file.exists()) {
            System.out.println("config file is not exist.");
            return -1;
        }
        try {
            setServerAddressVec(file);
        } catch (UnknownHostException e) {
            e.printStackTrace();
            return -1;
        }
        connectionMap = new HashMap<ServerAddress, IConnection>();
        initConnection();
        locator = new KetamaNodeLocator(serverAddressList, HashAlgorithm.KETAMA_HASH, VIRTUAL_NODE_COUNT);
        return 0;
    }

    private void initConnection() {
        ConfigOptions options = new ConfigOptions();
        int size = serverAddressList.size();
        Vector<ServerAddress> tmpVec = new Vector<ServerAddress>();
        for (ServerAddress sa : serverAddressList) {
            try {
                IConnection connection = new ConnectionTCPImpl(sa, options);
                connection.initialize();
                connectionMap.put(sa, connection);
            } catch (BaseException e) {
                System.out.printf("can't not connect server %s:%d%n", sa.getHost(), sa.getPort());
                tmpVec.add(sa);
            }
        }
        for (ServerAddress serverAddress : tmpVec) {
            serverAddressList.remove(serverAddress);
        }
    }

    private void setServerAddressVec(File file) throws UnknownHostException{
        serverAddressList = new LinkedList<ServerAddress>();
        ServerAddress sa = null;
        try {
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String tmpString = null;
            while((tmpString=reader.readLine())!=null) {
                String[] arr = tmpString.split(":");
                sa = new ServerAddress(arr[0], Integer.parseInt(arr[1]));
                serverAddressList.add(sa);
            }
        }catch(IOException e) {
            e.printStackTrace();
        }
    }

}