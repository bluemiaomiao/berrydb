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

public class Snapshot {
    private int insertTimes;
    private int delTimes;
    private int queryTimes;
    private int serverRunTime;

    public int getInsertTimes() {
        return insertTimes;
    }

    public void setInsertTimes(int insertTimes) {
        this.insertTimes = insertTimes;
    }

    public int getDelTimes() {
        return delTimes;
    }

    public void setDelTimes(int delTimes) {
        this.delTimes = delTimes;
    }

    public int getQueryTimes() {
        return queryTimes;
    }

    public void setQueryTimes(int queryTimes) {
        this.queryTimes = queryTimes;
    }

    public int getServerRunTime() {
        return serverRunTime;
    }

    public void setServerRunTime(int serverRunTime) {
        this.serverRunTime = serverRunTime;
    }
}
