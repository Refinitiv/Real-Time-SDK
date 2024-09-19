/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import java.util.Objects;

/**
 * Contains information about a particular item being published.
 */
public class ItemInfo {
    private int _itemId;        // Item's Stream ID
    private ItemData _itemData;        // Holds information about the item's data. This data will be different depending on the domain of the item.
    private int _itemFlags;        // See ItemFlags struct
    private ItemAttributes _attributes;    // Attributes that uniquely identify this item
    private long _clientHandle;
    private long _itemHandle;    // Handle value of this item
    private boolean _active;        // True if not closed
    private MarketPriceItem _mpItem;

    /**
     * Instantiates a new item info.
     */
    public ItemInfo() {
        _itemData = new ItemData();
        _attributes = new ItemAttributes();
    }

    /**
     * Clears the item information.
     */
    public void clear() {
        _itemFlags = 0;
        _itemId = 0;
        _itemData.clear();
        _attributes.clear();
        _clientHandle = 0;
        _itemHandle = 0;
        _active = false;
        _mpItem = null;
    }

    /**
     * Item's Stream ID.
     *
     * @return the int
     */
    public int itemId() {
        return _itemId;
    }

    /**
     * Item id.
     *
     * @param itemId the item id
     */
    public void itemId(int itemId) {
        _itemId = itemId;
    }

    /**
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     *
     * @return the object
     */
    public Object itemData() {
        return _itemData;
    }

    /**
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     *
     * @param itemData the item data
     */
    public void itemData(ItemData itemData) {
        _itemData = itemData;
    }

    /**
     * Item flags.
     *
     * @return {@link ItemFlags}
     */
    public int itemFlags() {
        return _itemFlags;
    }

    /**
     * Item flags.
     *
     * @param itemFlags - {@link ItemFlags}
     */
    public void itemFlags(int itemFlags) {
        _itemFlags = itemFlags;
    }

    /**
     * Attributes that uniquely identify this item.
     *
     * @return {@link ItemAttributes}
     */
    public ItemAttributes attributes() {
        return _attributes;
    }

    public long itemHandle() {
        return _itemHandle;
    }

    public void itemHandle(long itemHandle) {
        this._itemHandle = itemHandle;
    }

    public long clientHandle() {
        return _clientHandle;
    }

    public void clientHandle(long clientHandle) {
        this._clientHandle = clientHandle;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ItemInfo itemInfo = (ItemInfo) o;
        return _clientHandle == itemInfo._clientHandle && _itemHandle == itemInfo._itemHandle;
    }

    @Override
    public int hashCode() {
        return Objects.hash(_clientHandle, _itemHandle);
    }

    public boolean active() {
        return _active;
    }

    public void active(boolean active) {
        this._active = active;
    }

    public MarketPriceItem marketPriceItem() {
        return _mpItem;
    }

    public void marketPriceItem(MarketPriceItem item) {
        _mpItem = item;
    }
}