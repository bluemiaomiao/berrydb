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

import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

public class KetamaNodeLocator {
    private final TreeMap<Long, ServerAddress> ketamaNodes;
    private final HashAlgorithm hashAlgorithm;
    private int numReps = 160;

    public KetamaNodeLocator(List<ServerAddress> nodeList, HashAlgorithm algo, int nodeCopies) {
        this.hashAlgorithm = algo;
        this.ketamaNodes = new TreeMap<>();
        this.numReps = nodeCopies;

        for(ServerAddress node : nodeList) {
            for(int x = 0; x < this.numReps / 4; x++) {
                byte[] digest = this.hashAlgorithm.md5(node.getHost() + ":" + node.getPort());
                for(int y = 0; y < 4; y++) {
                    long m = this.hashAlgorithm.hash(digest, y);
                    this.ketamaNodes.put(m, node);
                }
            }
        }
    }

    public ServerAddress getPrimary(final String s) {
        byte[] digest = this.hashAlgorithm.md5(s);
        return this.getNodeForKey(this.hashAlgorithm.hash(digest, 0));
    }

    public ServerAddress getNodeForKey(long hash) {
        final ServerAddress addr;
        Long key = hash;
        if(this.ketamaNodes.isEmpty()) {
            return null;
        }

        if(!this.ketamaNodes.containsKey(key)) {
            SortedMap<Long, ServerAddress> tailMap = this.ketamaNodes.tailMap(key);
            if(tailMap.isEmpty()) {
                key = this.ketamaNodes.firstKey();
            } else {
                key = tailMap.firstKey();
            }
        }

        addr = this.ketamaNodes.get(key);
        return addr;
    }
}