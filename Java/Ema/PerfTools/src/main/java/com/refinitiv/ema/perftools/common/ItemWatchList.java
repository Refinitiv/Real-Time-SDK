/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Set;

/**
 * Item requests list.
 */
public class ItemWatchList {
    int index;      // current index for cyclic access
    private ArrayList<ItemInfo> entryList;   // list of entries

    /**
     * Instantiates a new item watchlist.
     *
     * @param count the count
     */
    public ItemWatchList(int count) {
        entryList = new ArrayList<>(count);
        index = 0;
    }

    /**
     * Clears items in the watch list.
     */
    public void init() {
        clear();
    }

    /**
     * Clears items in the watch list.
     */
    public void clear() {
        for (ItemInfo itemInfo : entryList) {
            itemInfo.clear();
        }
        index = 0;
    }

    /**
     * Add item to the watch list.
     *
     * @param itemInfo the item info
     */
    public void add(ItemInfo itemInfo) {
        entryList.add(itemInfo);
    }

    /**
     * Get next item in the list, moving index to the beginning if last item is
     * returned.
     *
     * @return - next item in the list,  null if list is empty..
     */
    public ItemInfo getNext() {
        if (entryList.isEmpty()) {
            return null;
        }

        if (index >= entryList.size()) {
            index = 0;
        }

        // get the current item
        ItemInfo item = entryList.get(index++);

        return item;
    }

    /**
     * Get first item in the list.
     *
     * @return - first item in the list, null if list is empty.
     */
    public ItemInfo getFront() {
        if (entryList.isEmpty()) {
            return null;
        }

        return entryList.get(0);
    }

    /**
     * Removes first item in the list and returning it.
     *
     * @return - first item in the list that is removed, null if list is empty.
     */
    public ItemInfo removeFront() {
        if (entryList.isEmpty()) {
            return null;
        }

        return entryList.remove(0);
    }

    /**
     * Removes item which is equivalent to specified item.
     *
     * @param itemInfo which should be removed.
     * @return true if item has successfully been deleted.
     */
    public boolean removeItem(ItemInfo itemInfo) {
        int index = entryList.indexOf(itemInfo);
        if (index == -1) {
            return false;
        }
        if (index < this.index) {
            --this.index;
        }
        return itemInfo.equals(entryList.remove(index));
    }

    /**
     * Removes items which are related to specified clients.
     *
     * @param clientIds client items which should be removed.
     */
    public void removeItemsForClients(Set<Long> clientIds) {
        ListIterator<ItemInfo> entryIterator = this.entryList.listIterator();
        while (entryIterator.hasNext()) {
            if (entryIterator.nextIndex() < index) {
                index--;
            }
            ItemInfo itemInfo = entryIterator.next();
            if (clientIds.contains(itemInfo.clientHandle())) {
                entryIterator.remove();
            }
        }
    }

    /**
     * Remove items which are equivalent to specified items.
     *
     * @param items which should be removed.
     */
    public void removeItems(Set<ItemInfo> items) {
        ListIterator<ItemInfo> entryIterator = this.entryList.listIterator();
        while (entryIterator.hasNext()) {
            if (entryIterator.nextIndex() < index) {
                index--;
            }
            ItemInfo itemInfo = entryIterator.next();
            if (items.contains(itemInfo)) {
                entryIterator.remove();
            }
        }
    }

    /**
     * Number of items in the watch list.
     *
     * @return Number of items in the watch list.
     */
    public int count() {
        return entryList.size();
    }

    /**
     * Check item watchlist for emptiness.
     *
     * @return true if ItemWatchlist is empty. False if at least one item is available.
     */
    public boolean isEmpty() {
        return count() == 0;
    }
}