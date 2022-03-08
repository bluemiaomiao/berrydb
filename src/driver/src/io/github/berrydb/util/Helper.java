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

import io.github.berrydb.core.DatabaseConstants;
import io.github.berrydb.excetion.BaseException;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

public class Helper {
    public static byte[] roundToMultipleX(byte[] byteArray, int multipler) {
        if (multipler == 0) {
            return byteArray;
        }

        int inLength = byteArray.length;
        int newLength = (inLength % multipler == 0) ? inLength : (inLength + multipler - inLength % multipler);

        ByteBuffer buf = ByteBuffer.allocate(newLength);
        buf.put(byteArray);

        return buf.array();
    }

    public static int roundToMultipleXLength(int inLength, int multipler) {
        if (multipler == 0) {
            return inLength;
        }

        return (inLength % multipler == 0) ? inLength : (inLength + multipler - inLength % multipler);
    }

    public static byte[] concatByteArray(List<byte[]> inByteArrayList)
            throws BaseException {
        if (inByteArrayList == null || inByteArrayList.size() == 0)
            return null;

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        try
        {
            for (byte[] bytes : inByteArrayList) {
                outputStream.write(bytes);
            }
            return outputStream.toByteArray();
        }
        catch(Exception e)
        {
            throw new BaseException("EDB_SYS");
        }
    }

    public static List<byte[]> splitByteArray(byte[] inByteArray, int length) {
        if (inByteArray == null) {
            return null;
        }

        List<byte[]> runtimes = new ArrayList<byte[]>();
        if (length >= inByteArray.length) {
            runtimes.add(inByteArray);
            runtimes.add(null);
            return runtimes;
        }

        byte[] firstPart = Arrays.copyOfRange(inByteArray, 0, length);
        byte[] secondPart = Arrays.copyOfRange(inByteArray, length, inByteArray.length);

        runtimes.add(firstPart);
        runtimes.add(secondPart);

        return runtimes;
    }

    @SuppressWarnings("unused")
    public static int byteToInt(byte[] byteArray) {
        if (byteArray == null || byteArray.length != 4) {
            throw new BaseException("SDB_CLI_CONV_ERROR");
        }

        ByteBuffer buffer = ByteBuffer.wrap(byteArray);
        if (Objects.equals(DatabaseConstants.SYSTEM_ENDIAN, DatabaseConstants.LITTLE_ENDIAN)) {
            buffer.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            buffer.order(ByteOrder.BIG_ENDIAN);
        }
        return buffer.getInt();
    }

    @SuppressWarnings("unused")
    public static long byteToLong(byte[] byteArray) {
        if (byteArray == null || byteArray.length != 8) {
            throw new BaseException("SDB_CLI_CONV_ERROR");
        }

        ByteBuffer buffer = ByteBuffer.wrap(byteArray);
        if (Objects.equals(DatabaseConstants.SYSTEM_ENDIAN, DatabaseConstants.LITTLE_ENDIAN)) {
            buffer.order(ByteOrder.LITTLE_ENDIAN);
        } else {
            buffer.order(ByteOrder.BIG_ENDIAN);
        }
        return buffer.getLong();
    }
}