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
import io.github.berrydb.network.IConnection;
import io.github.berrydb.network.ServerAddress;
import io.github.berrydb.util.DatabaseMessageHelper;
import io.github.berrydb.core.DatabaseConstants.Operation;
import org.bson.BSONObject;
import org.bson.util.JSON;

import java.util.HashMap;
import java.util.Map;

public class Database {
    private final DatabaseCommon databaseCommon;
    private Map<ServerAddress, Integer> nodeRecordMap = null;
    private long insertTotalTime = 0;

    public Database() throws BaseException {
        this.databaseCommon = new DatabaseCommon();
    }

    public void endStat(int total) {
        for (Map.Entry<ServerAddress, Integer> entry : this.nodeRecordMap.entrySet()) {
            System.out.println("Node name:" + entry.getKey().getPort()
                    + "-Times:" + entry.getValue()
                    + " - Percent: " + (float) entry.getValue() / total * 100 + "%");
        }
    }

    public void startStat() {
        insertTotalTime = 0;
        this.nodeRecordMap = new HashMap<ServerAddress, Integer>();
    }

    public int init(String configFilePath) {
        return this.databaseCommon.init(configFilePath);
    }

    public void disconnect() throws BaseException {
        DatabaseMessage DatabaseMessage = new DatabaseMessage();
        byte[] request = DatabaseMessageHelper.buildDisconnectRequest(DatabaseMessage);
        Map<ServerAddress, IConnection> connectionMap = this.databaseCommon.getConnectionMap();
        for (Map.Entry<ServerAddress, IConnection> entry : connectionMap.entrySet()) {
            IConnection connection = entry.getValue();
            ServerAddress servAddress = entry.getKey();
            connection.sendMessage(request);
            connection.close();
        }
    }

    public DatabaseMessage query(String key)
            throws BaseException {
        IConnection connection = getConnection(key);

        if (null == connection) {
            System.out.println("connection is failed");
            return null;
        }

        DatabaseMessage DatabaseMessage = new DatabaseMessage();
        DatabaseMessage.setOperationCode(Operation.OP_QUERY);

        BSONObject bson = (BSONObject) JSON.parse(key);
        DatabaseMessage.setQuery(bson);
        byte[] request = DatabaseMessageHelper.buildQueryRequest(DatabaseMessage);
        connection.sendMessage(request);

        byte[] recvBuffer = connection.receiveMessage();


        return DatabaseMessageHelper.msgExtractReply(recvBuffer);
    }

    public DatabaseMessage insert(String key, String record)
            throws BaseException {

        IConnection connection = getConnection(key);
        if (null == connection) {
            System.out.println("connection is failed");
            return null;
        }

        DatabaseMessage DatabaseMessage = new DatabaseMessage();
        DatabaseMessage.setOperationCode(Operation.OP_INSERT);
        DatabaseMessage.setMessageText(record);

        BSONObject bson = (BSONObject) JSON.parse(record);
        DatabaseMessage.setInsertor(bson);

        byte[] request = DatabaseMessageHelper.buildInsertRequest(DatabaseMessage);
        connection.sendMessage(request);

        byte[] recvBuffer = connection.receiveMessage();

        return DatabaseMessageHelper.msgExtractReply(recvBuffer);
    }

    public DatabaseMessage delete(String key)
            throws BaseException {
        IConnection connection = getConnection(key);

        if (null == connection) {
            System.out.println("connection is failed");
            return null;
        }

        DatabaseMessage DatabaseMessage = new DatabaseMessage();
        DatabaseMessage.setOperationCode(Operation.OP_DELETE);

        BSONObject bson = (BSONObject) JSON.parse(key);
        DatabaseMessage.setDelete(bson);
        byte[] request = DatabaseMessageHelper.buildDeleteRequest(DatabaseMessage);

        connection.sendMessage(request);

        byte[] recvBuffer = connection.receiveMessage();
        return DatabaseMessageHelper.msgExtractReply(recvBuffer);
    }

    private IConnection getConnection(String key) {
        ServerAddress addr = this.databaseCommon.getLocator().getPrimary(key);
        if (addr != null) {
            this.nodeRecordMap.merge(addr, 1, Integer::sum);
        }

        return this.databaseCommon.getConnectionMap().get(addr);
    }

}