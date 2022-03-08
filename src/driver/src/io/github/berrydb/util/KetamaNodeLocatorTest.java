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

import io.github.berrydb.network.ServerAddress;

import java.net.UnknownHostException;
import java.util.*;

public class KetamaNodeLocatorTest {
    static Random ran = new Random();

    private static final Integer EXE_TIMES = 100000;
    private static final Integer NODE_COUNT = 5;
    private static final Integer VIRTUAL_NODE_COUNT = 160;

    public static void main(String[] args) {
        KetamaNodeLocatorTest test = new KetamaNodeLocatorTest();

        Map<ServerAddress, Integer> nodeRecord = new HashMap<ServerAddress, Integer>();

        List<ServerAddress> all = test.getServerAddress(NODE_COUNT);
        List<String> allKeys = test.getAllString();
        KetamaNodeLocator locator = new KetamaNodeLocator(all, HashAlgorithm.KETAMA_HASH, VIRTUAL_NODE_COUNT);

        for (String key : allKeys) {
            ServerAddress sa = locator.getPrimary(key);
            nodeRecord.merge(sa, 1, Integer::sum);
        }

        System.out.println("Nodes count: " + NODE_COUNT + ", Keys count: " + EXE_TIMES + ", Normal percent : " + (float) 100 / NODE_COUNT + "%");
        System.out.println("------------------------------- boundary -------------------------------");
        for (Map.Entry<ServerAddress, Integer> entry : nodeRecord.entrySet()) {
            System.out.println("Node name:" + entry.getKey().getPort() + "-Times:" + entry.getValue() + " - Percent: " + (float) entry.getValue() / EXE_TIMES * 100 + "%");
        }
    }

    private List<String> getAllString() {
        List<String> allStrings = new ArrayList<String>(EXE_TIMES);
        for (int i = 0; i < EXE_TIMES; i++) {
            allStrings.add(generateRandomString(ran.nextInt(50)));
        }
        return allStrings;
    }

    private String generateRandomString(int length) {
        StringBuilder builder = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            builder.append((char) (ran.nextInt(95) + 32));
        }
        return builder.toString();
    }

    private List<ServerAddress> getServerAddress(int nodeCount) {
        try {
            List<ServerAddress> nodes = new ArrayList<ServerAddress>();
            ServerAddress sa = new ServerAddress("192.168.20.107", 48123);
            nodes.add(sa);
            sa = new ServerAddress("192.168.20.107", 48124);
            nodes.add(sa);
            sa = new ServerAddress("192.168.20.107", 48125);
            nodes.add(sa);
            sa = new ServerAddress("192.168.20.107", 48126);
            nodes.add(sa);
            sa = new ServerAddress("192.168.20.107", 48127);
            nodes.add(sa);
            return nodes;
        } catch (UnknownHostException e) {
            return null;
        }
    }
}