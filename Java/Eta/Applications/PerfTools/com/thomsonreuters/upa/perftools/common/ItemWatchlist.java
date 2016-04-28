package com.thomsonreuters.upa.perftools.common;

import java.util.ArrayList;

/**
 * Item requests list. 
 */
public class ItemWatchlist
{
    private ArrayList<ItemInfo> _entryList;   // list of entries
    int _index;      // current index for cyclic access

    public ItemWatchlist(int count)
    {
        _entryList = new ArrayList<ItemInfo>(count); 
        _index = 0;
    }

    /**
     * Clears items in the watch list.
     */
    public void init()
    {
        clear();
    }

    /**
     * Clears items in the watch list.
     */
    public void clear()
    {
        for (ItemInfo itemInfo : _entryList)
        {
            itemInfo.clear();
        }
        _index = 0;
    }

    /**
     * Add item to the watch list.
     * 
     * @param itemInfo
     */
    public void add(ItemInfo itemInfo)
    {
        _entryList.add(itemInfo);
    }

    /**
     * Get next item in the list, moving index to the beginning if last item is
     * returned.
     * 
     * @return - next item in the list,  null if list is empty..
     */
    public ItemInfo getNext()
    {
        if (_entryList.isEmpty())
            return null;

        // get the current item
        ItemInfo item = _entryList.get(_index);

        // reset to beginning
        if (++_index == _entryList.size())
        	_index = 0;

        return item;
    }
    
    /**
     * Get first item in the list.
     * 
     * @return - first item in the list, null if list is empty.
     */
    public ItemInfo getFront()
    {
        if (_entryList.isEmpty())
            return null;

        return _entryList.get(0);
    }
    
    /**
     * Removes first item in the list and returning it.
     * 
     * @return - first item in the list that is removed, null if list is empty.
     */
    public ItemInfo removeFront()
    {
        if (_entryList.isEmpty())
            return null;

        return _entryList.remove(0);
    }
    
    /**
     * Number of items in the watch list.
     * 
     * @return Number of items in the watch list.
     */
    public int count()
    {
        return _entryList.size();
    }

}