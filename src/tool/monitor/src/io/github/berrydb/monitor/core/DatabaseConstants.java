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

import io.github.berrydb.monitor.exception.BaseException;

public class DatabaseConstants {
    public final static String OP_ERRNO_FIELD = "errno";
    public final static int OP_MAX_NAME_LENGTH = 255;

    public final static int COLLECTION_SPACE_MAX_SZ = 127;
    public final static int COLLECTION_MAX_SZ = 127;
    public final static int MAX_CS_SIZE = 127;

    public static final String UNKNOWN_TYPE = "UNKNOWN";
    public static final String UNKNOWN_DESC = "Unknown Error";
    public static final int UNKNOWN_CODE = 1;

    public static final String ADMIN_PROMPT = "$";
    public static final String LIST_CMD = "list";
    public static final String CREATE_CMD = "create";
    public static final String DROP_CMD = "drop";
    public static final String SNAP_CMD = "snapshot ";
    public static final String TEST_CMD = "test";
    public static final String ACTIVE_CMD = "active";
    public static final String SHUTDOWN_CMD = "shutdown";
    public final static String COL_SPACES = "collectionspaces";
    public final static String COLLECTIONS = "collections";
    public final static String COL_SPACE = "collectionspace";
    public final static String NODE = "node";
    public final static String NODE_NAME_SEP = ":";
    public final static String CONTEXTS = "contexts";
    public final static String CONTEXTS_CUR = "contexts current";
    public final static String SESSIONS = "sessions";
    public final static String SESSIONS_CUR = "sessions current";
    public final static String STORE_UNITS = "storageunits";
    public final static String COLLECTION = "collection";
    public final static String CREATE_INX = "create index";
    public final static String DROP_INX = "drop index";
    public final static String GET_INDEX = "get indexes";
    public final static String GET_COUNT = "get count";
    public final static String DATABASE = "database";
    public final static String SYSTEM = "system";
    public final static String RESET = "reset";
    public final static String RENAME_COLLECTION = "rename collection";
    public final static String GROUP = "group";
    public final static String GROUPS = "groups";

    public final static String CMD_NAME_LIST_CONTEXTS = "list contexts";
    public final static String CMD_NAME_LIST_CONTEXTS_CURRENT = "list contexts current";
    public final static String CMD_NAME_LIST_SESSIONS = "list sessions";
    public final static String CMD_NAME_LIST_SESSIONS_CURRENT = "list sessions current";
    public final static String CMD_NAME_LIST_COLLECTIONS = "list collections";
    public final static String CMD_NAME_LIST_COLLECTION_SPACES = "list collectionspaces";
    public final static String CMD_NAME_LIST_STORAGE_UNITS = "list storageunits";
    public final static String CMD_NAME_LIST_GROUPS = "list groups";
    public final static String CMD_NAME_CREATE_GROUP = "create group";
    public final static String CMD_NAME_ACTIVE_GROUP = "active group";
    public final static String CMD_NAME_STARTUP_NODE = "startup node";
    public final static String CMD_NAME_SHUTDOWN_NODE = "shutdown node";
    public final static String CMD_NAME_SPLIT = "split";
    public final static String CMD_NAME_CREATE_CATALOG_GROUP = "create catalog group";

    public final static String FIELD_NAME_NAME = "Name";
    public final static String FIELD_NAME_OLD_NAME = "OldName";
    public final static String FIELD_NAME_NEW_NAME = "NewName";
    public final static String FIELD_NAME_PAGE_SIZE = "PageSize";
    public final static String FIELD_NAME_HOST = "HostName";
    public final static String FIELD_NAME_COLLECTION_SPACE = "CollectionSpace";
    public final static String FIELD_NAME_GROUP_NAME = "GroupName";
    public final static String FIELD_NAME_GROUP_SERVICE = "Service";
    public final static String FIELD_NAME_GROUP = "Group";
    public final static String FIELD_NAME_NODE_ID = "NodeID";
    public final static String FIELD_NAME_GROUP_ID = "GroupID";
    public final static String FIELD_NAME_PRIMARY = "PrimaryNode";
    public final static String FIELD_NAME_SERVICE_NAME = "Name";
    public final static String FIELD_NAME_SERVICE_TYPE = "Type";
    public final static String FIELD_NAME_SOURCE = "Source";
    public final static String FIELD_NAME_TARGET = "Target";
    public final static String FIELD_NAME_SPLIT_QUERY = "SplitQuery";
    public final static String FIELD_NAME_CATALOG_SHARDING_KEY = "ShardingKey";
    public final static String FIELD_COLLECTION = "Collection";
    public final static String FIELD_TOTAL = "Total";
    public final static String FIELD_INDEX = "Index";

    public final static String INDEX_MGMT_NAME = "name";
    public final static String INDEX_MGMT_KEY = "key";
    public final static String INDEX_MGMT_UNIQUE = "unique";
    public final static String INDEX_MGMT_ENFORCED = "enforced";
    public final static String INDEX_MGMT_INDEX_DEF = "IndexDef";

    public final static String PMD_OPTION_SVC_NAME = "svcname";
    public final static String PMD_OPTION_DBPATH = "dbpath";

    public final static String OID = "_id";

    public final static int FLG_UPDATE_UPSERT = 0x00000001;
    public final static int FLG_REPLY_CONTEXT_SOR_NOT_FOUND = 0x00000001;
    public final static int FLG_REPLY_SHARD_CONF_STALE = 0x00000004;

    public final static int DB_DMS_EOC = new BaseException("DB_DMS_EOC").getErrorCode();

    public final static String LITTLE_ENDIAN = "LITTLE_ENDIAN";
    public final static String BIG_ENDIAN = "BIG_ENDIAN";
    public final static String SYSTEM_ENDIAN = LITTLE_ENDIAN;

    public final static byte[] ZERO_NODE_ID = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    public enum Operation {
        OP_REPLY(1),
        OP_INSERT(2),
        OP_DELETE(3),
        OP_QUERY(4),
        OP_COMMAND(5),
        OP_DISCONNECT(6),
        OP_RETURN_OK(7),
        OP_INSERT_SNAPSHOT(8),
        RESERVED(9);

        private int code;

        private Operation(int code) {
            this.code = code;
        }

        public int getCode() {
            return code;
        }

        public static Operation getByValue(int inVal) {
            Operation runtimeOperator = Operation.RESERVED;
            for (Operation o : Operation.values()) {
                if (o.getCode() == inVal) {
                    runtimeOperator = o;
                    break;
                }
            }
            return runtimeOperator;
        }
    }

}
