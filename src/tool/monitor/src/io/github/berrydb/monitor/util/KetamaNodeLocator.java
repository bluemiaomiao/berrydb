package io.github.berrydb.monitor.util;

import io.github.berrydb.monitor.network.ServerAddress;

import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

public class KetamaNodeLocator {
    private TreeMap<Long, ServerAddress> ketamaNodes = null;
    private HashAlgorithm hashAlgo = null;
    private int numreps = 160;

    public KetamaNodeLocator(List<ServerAddress> nodeList, HashAlgorithm alg, int nodeCopies) {
        this.hashAlgo = alg;
        this.ketamaNodes = new TreeMap<Long, ServerAddress>();

        this.numreps = nodeCopies;

        for (ServerAddress node : nodeList) {
            for (int i = 0; i < this.numreps / 4; i++) {
                byte[] digest = this.hashAlgo.md5(node.getHost() + ":" + node.getPort() + i);
                for (int h = 0; h < 4; h++) {
                    long m = this.hashAlgo.hash(digest, h);
                    this.ketamaNodes.put(m, node);
                }
            }
        }
    }

    public ServerAddress getPrimary(final String str) {
        byte[] digest = this.hashAlgo.md5(str);
        return getNodeForKey(this.hashAlgo.hash(digest, 0));
    }

    ServerAddress getNodeForKey(long hash) {
        final ServerAddress rv;
        Long key = hash;
        if (!ketamaNodes.containsKey(key)) {
            SortedMap<Long, ServerAddress> tailMap = this.ketamaNodes.tailMap(key);
            if (tailMap.isEmpty()) {
                key = this.ketamaNodes.firstKey();
            } else {
                key = tailMap.firstKey();
            }
        }

        rv = this.ketamaNodes.get(key);
        return rv;
    }
}
