package io.github.berrydb.monitor.util;

import io.github.berrydb.monitor.network.ServerAddress;

import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class KetamaNodeLocatorTest {
    static Random ran = new Random();

    private static final Integer EXE_TIMES = 100000;
    private static final Integer NODE_COUNT = 5;
    private static final Integer VIRTUAL_NODE_COUNT = 160;

    public static void main(String[] args) {
        KetamaNodeLocatorTest test = new KetamaNodeLocatorTest();

        Map<ServerAddress, Integer> nodeRecord = new HashMap<ServerAddress, Integer>();

        List<ServerAddress> all = test.getServerAddress();
        List<String> allKeys = test.getAllString();

        assert all != null;
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

    private List<ServerAddress> getServerAddress() {
        try {
            List<ServerAddress> nodes = new ArrayList<ServerAddress>();

            ServerAddress addr = new ServerAddress("127.0.0.1", 2017);
            nodes.add(addr);

            addr = new ServerAddress("127.0.0.1", 2018);
            nodes.add(addr);

            addr = new ServerAddress("127.0.0.1", 2019);
            nodes.add(addr);

            addr = new ServerAddress("127.0.0.1", 2020);
            nodes.add(addr);

            addr = new ServerAddress("127.0.0.1", 2021);
            nodes.add(addr);

            return nodes;
        } catch (UnknownHostException e) {
            return null;
        }
    }
}
