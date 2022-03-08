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

package io.github.berrydb.util;

import com.google.gson.Gson;
import com.google.gson.internal.Primitives;
import io.github.berrydb.core.DatabaseConstants;
import io.github.berrydb.core.DatabaseMessage;
import io.github.berrydb.excetion.BaseException;
import io.github.berrydb.core.DatabaseConstants.Operation;
import org.bson.*;
import org.bson.io.BasicOutputBuffer;
import org.bson.io.OutputBuffer;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.lang.reflect.Type;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.MessageDigest;
import java.util.*;

public class DatabaseMessageHelper {
    private final static int MESSAGE_HEADER_LENGTH = 8;
    private final static int MESSAGE_OP_INSERT_LENGTH = 12;
    private final static int MESSAGE_OP_DELETE_LENGTH = 8;
    private final static int MESSAGE_OP_QUERY_LENGTH = 8;

    @SuppressWarnings("unused")
    public static byte[] buildQueryRequest(DatabaseMessage DatabaseMessage)
            throws BaseException {
        Operation opCode = DatabaseMessage.getOperationCode();
        int messageLen = 0;
        byte[] bsonData = bsonObjectToByteArray(DatabaseMessage.getQuery());
        messageLen = Helper.roundToMultipleXLength(MESSAGE_OP_QUERY_LENGTH, 4) +
                Helper.roundToMultipleXLength(bsonData.length, 4);
        List<byte[]> fieldList = new ArrayList<byte[]>();
        fieldList.add(assembleHeader(messageLen, Operation.OP_QUERY.getCode()));
        fieldList.add(Helper.roundToMultipleX(bsonData, 4));
        return Helper.concatByteArray(fieldList);
    }

    @SuppressWarnings("unused")
    public static byte[] buildInsertRequest(DatabaseMessage DatabaseMessage)
            throws BaseException {
        Operation opCode = DatabaseMessage.getOperationCode();

        int messageLen = 0;

        byte[] bsonData = bsonObjectToByteArray(DatabaseMessage.getInsertor());
        byte[] bsonRecord = bsonObjectToByteArray(DatabaseMessage.getInsertor());
        messageLen = Helper.roundToMultipleXLength(MESSAGE_OP_INSERT_LENGTH, 4)
                + Helper.roundToMultipleXLength(bsonData.length, 4);

        List<byte[]> fieldList = new ArrayList<byte[]>();
        fieldList.add(assembleHeader(messageLen, Operation.OP_INSERT.getCode()));

        int numRecord = 1;
        ByteBuffer buf = ByteBuffer.allocate(4);
        if (Objects.equals(DatabaseConstants.SYSTEM_ENDIAN, DatabaseConstants.LITTLE_ENDIAN)) {
            buf.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            buf.order(ByteOrder.BIG_ENDIAN);
        }

        buf.putInt(numRecord);
        fieldList.add(Helper.roundToMultipleX(buf.array(), 4));
        fieldList.add(Helper.roundToMultipleX(bsonRecord, 4));

        return Helper.concatByteArray(fieldList);
    }

    @SuppressWarnings("unused")
    public static byte[] buildDeleteRequest(DatabaseMessage DatabaseMessage)
            throws BaseException {
        Operation opCode = DatabaseMessage.getOperationCode();
        int messageLen = 0;

        byte[] bsonData = bsonObjectToByteArray(DatabaseMessage.getDelete());
        messageLen = Helper.roundToMultipleXLength(MESSAGE_OP_DELETE_LENGTH, 4) +
                Helper.roundToMultipleXLength(bsonData.length, 4);

        List<byte[]> fieldList = new ArrayList<byte[]>();
        fieldList.add(assembleHeader(messageLen, Operation.OP_DELETE.getCode()));

        fieldList.add(Helper.roundToMultipleX(bsonData, 4));

        return Helper.concatByteArray(fieldList);
    }

    public static byte[] buildInsertSnapshotRequest(DatabaseMessage DatabaseMessage) throws BaseException {
        Operation opCode = DatabaseMessage.getOperationCode();
        int messageLen = 0;

        messageLen = Helper.roundToMultipleXLength(MESSAGE_HEADER_LENGTH, 4);

        List<byte[]> fieldList = new ArrayList<byte[]>();
        fieldList.add(assembleHeader(messageLen, Operation.OP_INSERT_SNAPSHOT.getCode()));

        return Helper.concatByteArray(fieldList);
    }

    public static byte[] buildDisconnectRequest(DatabaseMessage DatabaseMessage) throws BaseException {
        Operation opCode = DatabaseMessage.getOperationCode();
        int messageLen = 0;

        messageLen = Helper.roundToMultipleXLength(MESSAGE_HEADER_LENGTH, 4);

        List<byte[]> fieldList = new ArrayList<byte[]>();
        fieldList.add(assembleHeader(messageLen, opCode.OP_DISCONNECT.getCode()));

        return Helper.concatByteArray(fieldList);
    }


    @SuppressWarnings("unused")
    private static byte[] assembleHeader(int messageLength, int operationCode) {
        ByteBuffer buf = ByteBuffer.allocate(MESSAGE_HEADER_LENGTH);
        if (Objects.equals(DatabaseConstants.SYSTEM_ENDIAN, DatabaseConstants.LITTLE_ENDIAN)) {
            buf.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            buf.order(ByteOrder.BIG_ENDIAN);
        }

        buf.putInt(messageLength);
        buf.putInt(operationCode);
        return buf.array();
    }

    public static DatabaseMessage msgExtractReply(byte[] inBytes)
            throws BaseException {
        List<byte[]> tmp = Helper.splitByteArray(inBytes, MESSAGE_HEADER_LENGTH);
        byte[] header = tmp.get(0);
        byte[] remaining = tmp.get(1);

        if (header.length != MESSAGE_HEADER_LENGTH || remaining == null) {
            return null;
        }

        DatabaseMessage DatabaseMessage = new DatabaseMessage();

        extractHeader(DatabaseMessage, header);

        tmp = Helper.splitByteArray(remaining, 4);
        byte[] returnCode = tmp.get(0);
        remaining = tmp.get(1);
        DatabaseMessage.setRc(Helper.byteToInt(returnCode));

        tmp = Helper.splitByteArray(remaining, 4);
        byte[] numReturn = tmp.get(0);
        remaining = tmp.get(1);
        DatabaseMessage.setNumReturn(Helper.byteToInt(numReturn));


        if (Operation.OP_QUERY == DatabaseMessage.getOperationCode()) {
            List<BSONObject> list = extractBSONObject(remaining);
            int nr = DatabaseMessage.getNumReturn();
            if (nr == list.size()) {
                for (int i = 0; i < nr; i++) {
                    BSONObject bsonObj = list.get(i);
                    System.out.println("query result " + i + " is " + bsonObj.toString());
                }
            } else {
                System.out.println("query result's size is not valid");
            }
        } else if (Operation.OP_DELETE == DatabaseMessage.getOperationCode()) {
            if (0 != DatabaseMessage.getRc()) {
                System.out.println("Delete error,  code=" + DatabaseMessage.getRc());
            }
        } else if (Operation.OP_INSERT == DatabaseMessage.getOperationCode()) {
            if (0 != DatabaseMessage.getRc()) {
                System.out.println("Insert error, code=" + DatabaseMessage.getRc());
            }
        } else if (Operation.OP_INSERT_SNAPSHOT == DatabaseMessage.getOperationCode()) {
            List<BSONObject> list = extractBSONObject(remaining);
            for (BSONObject bo : list) {
                int insertTimes = Integer.parseInt(bo.get("insertTimes").toString());
                DatabaseMessage.setInsertTimes(insertTimes);
                break;
            }

        }
        return DatabaseMessage;
    }

    private static List<BSONObject> extractBSONObject(byte[] inBytes)
            throws BaseException {
        int objLen;
        int objAllotLen;
        byte[] remaining = inBytes;
        List<BSONObject> objList = new ArrayList<BSONObject>();
        while (remaining != null) {
            objLen = getBSONObjectLength(remaining);
            if (objLen <= 0) {
                throw new BaseException("SDB_CLI_BSON_INV_LEN");
            }
            objAllotLen = Helper.roundToMultipleXLength(objLen, 4);

            List<byte[]> tmp = Helper.splitByteArray(remaining, objAllotLen);
            byte[] obj = tmp.get(0);
            remaining = tmp.get(1);

            byte[] bsonObj = Arrays.copyOfRange(obj, 0, objLen);
            objList.add(byteArrayToBSONObject(bsonObj));
        }

        return objList;
    }

    private static int getBSONObjectLength(byte[] inBytes) {
        byte[] tmp = new byte[4];

        tmp[0] = inBytes[0];
        tmp[1] = inBytes[1];
        tmp[2] = inBytes[2];
        tmp[3] = inBytes[3];

        return Helper.byteToInt(tmp);
    }

    private static void extractHeader(DatabaseMessage DatabaseMessage, byte[] header) {
        List<byte[]> tmp = Helper.splitByteArray(header, 4);
        byte[] msgLength = tmp.get(0);
        byte[] remaining = tmp.get(1);

        DatabaseMessage.setRequestLength(Helper.byteToInt(msgLength));

        tmp = Helper.splitByteArray(remaining, 4);
        byte[] opCode = tmp.get(0);
        remaining = tmp.get(1);

        DatabaseMessage.setOperationCode(Operation.getByValue(Helper.byteToInt(opCode)));
    }

    public static byte[] bsonObjectToByteArray(BSONObject obj) {
        BSONEncoder e = new BasicBSONEncoder();
        OutputBuffer buf = new BasicOutputBuffer();

        e.set(buf);
        e.putObject(obj);
        e.done();

        return buf.toByteArray();
    }

    @SuppressWarnings("unused")
    public static BSONObject byteArrayToBSONObject(byte[] inBytes) throws BaseException {
        if (inBytes == null || inBytes.length == 0) {
            return null;
        }

        BSONDecoder d = new BasicBSONDecoder();
        BSONCallback cb = new BasicBSONCallback();
        try {
            int s = d.decode(new ByteArrayInputStream(inBytes), cb);
            return  (BSONObject) cb.get();
        } catch (IOException e) {
            throw new BaseException("SDB_INVALID_ARG");
        }
    }

    public static BSONObject fromObject(Object object) throws BaseException {
        Gson gson = new Gson();
        String jString = gson.toJson(object);

        return fromJson(jString);
    }

    public static <T> T fromBson(BSONObject bObj, Class<T> classOfT) {
        bObj.removeField("_id");
        Gson gson = new Gson();
        Object object = gson.fromJson(bObj.toString(), (Type) classOfT);
        return Primitives.wrap(classOfT).cast(object);
    }

    public static BSONObject fromJson(String jsonString) throws BaseException {

        String fullString = "{\"bsonMap\":" + jsonString + "}";

        Gson gson = new Gson();
        ConvertHelpObject obj = gson.fromJson(fullString,
                ConvertHelpObject.class);

        LinkedHashMap<String, Object> bsonMap = obj.getBsonMap();

        BSONObject o1 = new BasicBSONObject();
        o1.putAll(bsonMap);

        return o1;
    }

    private static class ConvertHelpObject {
        private LinkedHashMap<String, Object> bsonMap;

        public LinkedHashMap<String, Object> getBsonMap() {
            return bsonMap;
        }

        @SuppressWarnings("unused")
        public void setBsonMap(LinkedHashMap<String, Object> bsonMap) {
            this.bsonMap = bsonMap;
        }
    }

    @SuppressWarnings("unused")
    public static byte[] appendInsertMsg(byte[] msg, BSONObject append) {
        List<byte[]> tmp = Helper.splitByteArray(msg, 4);
        byte[] msgLength = tmp.get(0);
        byte[] remaining = tmp.get(1);
        byte[] insertor = bsonObjectToByteArray(append);
        int length = Helper.byteToInt(msgLength);
        int messageLength = length
                + Helper.roundToMultipleXLength(insertor.length, 4);

        ByteBuffer buf = ByteBuffer.allocate(messageLength);
        if (Objects.equals(DatabaseConstants.SYSTEM_ENDIAN, DatabaseConstants.LITTLE_ENDIAN)) {
            buf.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            buf.order(ByteOrder.BIG_ENDIAN);
        }

        buf.putInt(messageLength);
        buf.put(remaining);
        buf.put(Helper.roundToMultipleX(insertor, 4));

        return buf.array();
    }

    private static String getMD5FromStr(String inStr) {
        MessageDigest md5 = null;
        try {
            md5 = MessageDigest.getInstance("MD5");
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
        char[] charArray = inStr.toCharArray();
        byte[] byteArray = new byte[charArray.length];

        for (int i = 0; i < charArray.length; i++) {
            byteArray[i] = (byte) charArray[i];
        }

        byte[] md5Bytes = md5.digest(byteArray);

        StringBuilder hexValue = new StringBuilder();

        for (byte md5Byte : md5Bytes) {
            int val = ((int) md5Byte) & 0xff;
            if (val < 16)
                hexValue.append("0");
            hexValue.append(Integer.toHexString(val));
        }

        return hexValue.toString();
    }
}