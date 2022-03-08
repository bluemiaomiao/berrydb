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

import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public enum HashAlgorithm {

    // 一致性哈希算法 -- Ketama算法(基于MD5)
    KETAMA_HASH;

    public long hash( byte[] digest, int nTime ) {
        long x = ( (long) (digest[3 + nTime*4] & 0xFF) << 24 )
                | ( (long) (digest[2 + nTime*4] & 0xFF ) << 16 )
                | ( (long) (digest[1 + nTime*4] & 0xFF) << 8 )
                | (digest[nTime * 4] & 0xFF);
        return x & 0xFFFFFFFFL;
    }

    public byte[] md5(String k) {
        MessageDigest md5;
        try {
            md5 = MessageDigest.getInstance("MD5");
        } catch( NoSuchAlgorithmException e ) {
            throw new RuntimeException("MD5 not supported", e);
        }

        md5.reset();
        byte[] keyBytes = null;
        keyBytes = k.getBytes(StandardCharsets.UTF_8);

        md5.update(keyBytes);
        return md5.digest();
    }
}