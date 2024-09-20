/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.util.ArrayDeque;
import java.util.Collection;
import java.util.Queue;

public class ItemInfoPool {

    private final Queue<ItemInfo> poolQueue;
    private final int maxSize;

    public ItemInfoPool(int capacity) {
        poolQueue = new ArrayDeque<>(capacity);
        this.maxSize = capacity;
    }

    public boolean returnToPool(ItemInfo instance) {
        if (poolQueue.size() < maxSize) {
            instance.clear();
            return poolQueue.add(instance);
        }
        return false;
    }

    public void returnToPool(Collection<ItemInfo> items) {
        if (poolQueue.size() + items.size() < maxSize) {
            poolQueue.addAll(items);
        } else if (poolQueue.size() < maxSize) {
            for (ItemInfo itemInfo : items) {
                if (!returnToPool(itemInfo)) {
                    break;
                }
            }
        }
    }

    public ItemInfo getFromPool() {
        if (poolQueue.size() > 0) {
            ItemInfo item = poolQueue.poll();
            item.clear();
            return item;
        }
        return null;
    }

}
