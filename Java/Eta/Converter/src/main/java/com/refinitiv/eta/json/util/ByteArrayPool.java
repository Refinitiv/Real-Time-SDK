/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.util;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class ByteArrayPool {

    private Map<Integer, ObjectPool<byte[]>> arrayPools;
    private List<Integer> lengths;

    private int DEFAULT_ARRAY_SIZE = 4096;
    private int DEFAULT_NUM_OF_POOLS = 8;


    public ByteArrayPool() {
        arrayPools = new HashMap<>();
        arrayPools.put(0, new ObjectPool<>(true, () -> new byte[0]));
        lengths = new LinkedList<>();
        lengths.add(0);

        int currLen = DEFAULT_ARRAY_SIZE;
        for (int i = 0; i < DEFAULT_NUM_OF_POOLS; i++) {
            final int len = currLen;
            arrayPools.put(currLen, new ObjectPool<>(true, () -> new byte[len]));
            lengths.add(len);
            currLen = currLen + DEFAULT_ARRAY_SIZE;
        }
    }

    public byte[] poll(int length) {

        int n = length / DEFAULT_ARRAY_SIZE + 1;
        int newLen = DEFAULT_ARRAY_SIZE * n;
        ObjectPool<byte[]> pool = arrayPools.get(newLen);
        if (pool == null) {
            pool = new ObjectPool<>(true, () -> new byte[newLen]);
            arrayPools.put(newLen, pool);
        }

        return pool.get();
    }

    public void putBack(byte[] arr) {

        if (arr != null) {
            ObjectPool<byte[]> pool = arrayPools.get(arr.length);
            if (pool != null)
                pool.release(arr);
        }
    }

}
