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

package io.github.berrydb.monitor.core;

import java.net.UnknownHostException;
import java.util.List;

import org.bson.BSONObject;

import io.github.berrydb.monitor.exception.BaseException;
import io.github.berrydb.monitor.network.ConfigOptions;
import io.github.berrydb.monitor.network.ConnectionTCPImpl;
import io.github.berrydb.monitor.network.IConnection;
import io.github.berrydb.monitor.network.ServerAddress;
import io.github.berrydb.monitor.util.DatabaseMessageHelper;
import io.github.berrydb.monitor.core.DatabaseConstants.Operation;

public class Database {
    private IConnection connection = null;
    private String ip;
    private int port;
    private int lastTimes;
    private String errMsg;

    private int[] updateTimesArr = null;
    private int updateTimes;
    private int index;
    private final int SAVE_RECORD_MAX_NUMER = 40;

    public Database() {
        index = 0;
        updateTimesArr = new int[SAVE_RECORD_MAX_NUMER];
    }

    public Database(String ip, int port) {
        index = 0;
        updateTimesArr = new int[SAVE_RECORD_MAX_NUMER];
        this.ip = ip;
        this.port = port;
    }

    public int getUpdateTimes() {
        DatabaseMessage message = getInsertTimes();
        if (message == null) {
            updateTimes = -1;
            return -1;
        }
        Snapshot snapshot = message.getSnapshot();
        if (snapshot == null) {
            return -1;
        }
        System.out.println("3");
        int insertTimes = snapshot.getInsertTimes();
        System.out.println("insertTimes:" + insertTimes);
        updateTimes = (insertTimes - lastTimes);
        lastTimes = insertTimes;

        if (index >= SAVE_RECORD_MAX_NUMER) {
            index = 0;
        }

        updateTimesArr[index % SAVE_RECORD_MAX_NUMER] = updateTimes;
        index++;
        return updateTimes;
    }

    public void disconnect() throws BaseException {
        DatabaseMessage dbMessage = new DatabaseMessage();
        dbMessage.setOperationCode(Operation.OP_DISCONNECT);
        byte[] request = DatabaseMessageHelper.buildQueryRequest(dbMessage);
        connection.sendMessage(request);
        connection.close();
    }

    public DatabaseMessage getInsertTimes() throws BaseException {
        DatabaseMessage dbMessage = new DatabaseMessage();
        dbMessage.setOperationCode(Operation.OP_INSERT_SNAPSHOT);
        byte[] request = DatabaseMessageHelper.buildInsertSnapshotRequest(dbMessage);
        try {
            boolean ret = connection.sendMessage(request);
            if (!ret) {
                errMsg = "server is closed.";
                return null;
            }
        } catch (BaseException e) {
            errMsg = String.format("[BaseException]send to %s:%d - %s", ip, port, e.getMessage());
            return null;
        }
        try {
            byte[] recvBuffer = connection.receiveMessage();
            DatabaseMessage runtimeSDBMessage = DatabaseMessageHelper.msgExtractReply(recvBuffer);
            assert runtimeSDBMessage != null;
            List<BSONObject> list = runtimeSDBMessage.getObjList();
            for (BSONObject bo : list) {
                int insertTimes = Integer.parseInt(bo.get("insertTimes").toString());
                int delTimes = Integer.parseInt(bo.get("delTimes").toString());
                int queryTimes = Integer.parseInt(bo.get("queryTimes").toString());
                int serverRunTime = Integer.parseInt(bo.get("serverRunTime").toString());
                Snapshot snapshot = new Snapshot();
                snapshot.setInsertTimes(insertTimes);
                snapshot.setDelTimes(delTimes);
                snapshot.setQueryTimes(queryTimes);
                snapshot.setServerRunTime(serverRunTime);
                runtimeSDBMessage.setSnapshot(snapshot);
                break;
            }
            return runtimeSDBMessage;
        } catch (BaseException e) {
            errMsg = String.format("[BaseException] receive from %s:%d - %s", ip, port, e.getMessage());
            return null;
        }
    }

    private boolean initConnection(String ip, int port) throws UnknownHostException {
        ServerAddress sa = new ServerAddress(ip, port);
        ConfigOptions options = new ConfigOptions();
        connection = new ConnectionTCPImpl(sa, options);
        return connection.connect();
    }

    public boolean start() {
        try {
            return initConnection(ip, port);
        } catch (UnknownHostException e1) {
            connection = null;
            errMsg = String.format("[UnknownHostException]can not connect server %s:%d", ip, port);
            return false;
        } catch (BaseException be) {
            errMsg = String.format("can not connect server %s:%d", ip, port);
            return false;
        } catch (Exception e) {
            errMsg = String.format("%s", e.getMessage().toString());
            return false;
        }
    }

    public String getErrorMsg() {
        return errMsg;
    }

    public String getIp() {
        return ip;
    }

    public int getLastTimes() {
        return lastTimes;
    }

    public int getCurInsertTimes() {
        return updateTimes;
    }

    public int getPort() {
        return port;
    }

    public int maxRecordNumber() {
        int max = 0;
        for (int j : updateTimesArr) {
            if (max < j) {
                max = j;
            }
        }
        return max;
    }
}
