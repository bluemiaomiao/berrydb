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

import org.bson.BSONObject;

import java.util.List;

public class DatabaseMessage {
    private int 		requestLength;
    private long 		requestID;
    private int 		responseTo;
    private DatabaseConstants.Operation operationCode;
    private DatabaseConstants.Operation 	returnCode;
    private int			numReturn;
    private Snapshot	snapshot;
    private String 		messageText;
    private int 		rc;
    private BSONObject insertor;
    private BSONObject query;
    private BSONObject delete;

    private short w;
    private short padding;
    private int flags;
    private BSONObject matcher;
    private BSONObject selector;
    private BSONObject orderBy;
    private BSONObject hint;
    private int version;
    private int returnRowsCount2;
    private long returnRowsCount;
    private List<Long> contextIDList;
    private List<BSONObject> objList ;

    private byte[] nodeID = new byte[12];

    public int getRequestLength() {
        return requestLength;
    }

    public void setRequestLength(int requestLength) {
        this.requestLength = requestLength;
    }

    public long getRequestID() {
        return requestID;
    }

    public void setRequestID(long requestID) {
        this.requestID = requestID;
    }

    public int getResponseTo() {
        return responseTo;
    }

    public void setResponseTo(int responseTo) {
        this.responseTo = responseTo;
    }

    public DatabaseConstants.Operation getOperationCode() {
        return operationCode;
    }

    public void setOperationCode(DatabaseConstants.Operation operationCode) {
        this.operationCode = operationCode;
    }

    public DatabaseConstants.Operation getReturnCode() {
        return returnCode;
    }

    public void setReturnCode(DatabaseConstants.Operation returnCode) {
        this.returnCode = returnCode;
    }

    public int getNumReturn() {
        return numReturn;
    }

    public void setNumReturn(int numReturn) {
        this.numReturn = numReturn;
    }

    public Snapshot getSnapshot() {
        return snapshot;
    }

    public void setSnapshot(Snapshot snapshot) {
        this.snapshot = snapshot;
    }

    public String getMessageText() {
        return messageText;
    }

    public void setMessageText(String messageText) {
        this.messageText = messageText;
    }

    public int getRc() {
        return rc;
    }

    public void setRc(int rc) {
        this.rc = rc;
    }

    public BSONObject getInsertor() {
        return insertor;
    }

    public void setInsertor(BSONObject insertor) {
        this.insertor = insertor;
    }

    public BSONObject getQuery() {
        return query;
    }

    public void setQuery(BSONObject query) {
        this.query = query;
    }

    public BSONObject getDelete() {
        return delete;
    }

    public void setDelete(BSONObject delete) {
        this.delete = delete;
    }

    public short getW() {
        return w;
    }

    public void setW(short w) {
        this.w = w;
    }

    public short getPadding() {
        return padding;
    }

    public void setPadding(short padding) {
        this.padding = padding;
    }

    public int getFlags() {
        return flags;
    }

    public void setFlags(int flags) {
        this.flags = flags;
    }

    public BSONObject getMatcher() {
        return matcher;
    }

    public void setMatcher(BSONObject matcher) {
        this.matcher = matcher;
    }

    public BSONObject getSelector() {
        return selector;
    }

    public void setSelector(BSONObject selector) {
        this.selector = selector;
    }

    public BSONObject getOrderBy() {
        return orderBy;
    }

    public void setOrderBy(BSONObject orderBy) {
        this.orderBy = orderBy;
    }

    public BSONObject getHint() {
        return hint;
    }

    public void setHint(BSONObject hint) {
        this.hint = hint;
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public int getReturnRowsCount2() {
        return returnRowsCount2;
    }

    public void setReturnRowsCount2(int returnRowsCount2) {
        this.returnRowsCount2 = returnRowsCount2;
    }

    public long getReturnRowsCount() {
        return returnRowsCount;
    }

    public void setReturnRowsCount(long returnRowsCount) {
        this.returnRowsCount = returnRowsCount;
    }

    public List<Long> getContextIDList() {
        return contextIDList;
    }

    public void setContextIDList(List<Long> contextIDList) {
        this.contextIDList = contextIDList;
    }

    public List<BSONObject> getObjList() {
        return objList;
    }

    public void setObjList(List<BSONObject> objList) {
        this.objList = objList;
    }

    public byte[] getNodeID() {
        return nodeID;
    }

    public void setNodeID(byte[] nodeID) {
        this.nodeID = nodeID;
    }
}
