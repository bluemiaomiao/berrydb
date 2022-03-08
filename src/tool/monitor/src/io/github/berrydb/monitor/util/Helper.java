package io.github.berrydb.monitor.util;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import io.github.berrydb.monitor.exception.BaseException;
import io.github.berrydb.monitor.core.DatabaseConstants;

public class Helper {
    public static byte[] roundToMultipleX(byte[] byteArray, int multipler) {
        if (multipler == 0)
            return byteArray;

        int inLength = byteArray.length;
        int newLength = inLength % multipler == 0 ? inLength : inLength
                + multipler - inLength % multipler;
        ByteBuffer buf = ByteBuffer.allocate(newLength);
        buf.put(byteArray);

        return buf.array();
    }

    public static int roundToMultipleXLength(int inLength, int multipler) {
        if (multipler == 0)
            return inLength;

        return inLength % multipler == 0 ? inLength : inLength + multipler
                - inLength % multipler;
    }

    public static byte[] concatByteArray(List<byte[]> inByteArrayList)
            throws BaseException {
        if (inByteArrayList == null || inByteArrayList.size() == 0)
            return null;

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        try {
            for (int i = 0; i < inByteArrayList.size(); i++) {
                outputStream.write(inByteArrayList.get(i));
            }
            return outputStream.toByteArray();
        } catch (Exception e) {
            throw new BaseException("DB_SYS");
        }
    }

    public static List<byte[]> splitByteArray(byte[] inByteArray, int length) {
        if (inByteArray == null) {
            return null;
        }

        List<byte[]> rtnList = new ArrayList<byte[]>();
        if (length >= inByteArray.length) {
            rtnList.add(inByteArray);
            rtnList.add(null);
            return rtnList;
        }

        byte[] firstPart = Arrays.copyOfRange(inByteArray, 0, length);
        byte[] seconPart = Arrays.copyOfRange(inByteArray, length,
                inByteArray.length);

        rtnList.add(firstPart);
        rtnList.add(seconPart);

        return rtnList;
    }

    @SuppressWarnings("unused")
    public static int byteToInt(byte[] byteArray) {
        if (byteArray == null || byteArray.length != 4) {
            throw new BaseException("SDB_CLI_CONV_ERROR");
        }

        ByteBuffer bb = ByteBuffer.wrap(byteArray);
        if (DatabaseConstants.SYSTEM_ENDIAN == DatabaseConstants.LITTLE_ENDIAN) {
            bb.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            bb.order(ByteOrder.BIG_ENDIAN);
        }
        return bb.getInt();
    }

    @SuppressWarnings("unused")
    public static long byteToLong(byte[] byteArray) {
        if (byteArray == null || byteArray.length != 8) {
            throw new BaseException("SDB_CLI_CONV_ERROR");
        }

        ByteBuffer bb = ByteBuffer.wrap(byteArray);
        if (DatabaseConstants.SYSTEM_ENDIAN == DatabaseConstants.LITTLE_ENDIAN) {
            bb.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            bb.order(ByteOrder.BIG_ENDIAN);
        }
        return bb.getLong();
    }

}
