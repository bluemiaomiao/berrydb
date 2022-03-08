package io.github.berrydb.monitor.util;

import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public enum HashAlgorithm {
    KETAMA_HASH;

    public long hash(byte[] digest, int nTime) {
        long rv = ((long) (digest[3 + nTime * 4] & 0xFF) << 24)
                | ((long) (digest[2 + nTime * 4] & 0xFF) << 16)
                | ((long) (digest[1 + nTime * 4] & 0xFF) << 8)
                | (digest[nTime * 4] & 0xFF);
        return rv & 0xFFFFFFFFL;
    }

    public byte[] md5(String k) {
        MessageDigest md5;
        try {
            md5 = MessageDigest.getInstance("MD5");
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("MD5 not supported", e);
        }

        md5.reset();
        byte[] keyBytes = null;
        keyBytes = k.getBytes(StandardCharsets.UTF_8);

        md5.update(keyBytes);
        return md5.digest();
    }
}
