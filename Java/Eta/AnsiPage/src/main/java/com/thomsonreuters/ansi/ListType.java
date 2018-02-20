package com.thomsonreuters.ansi;

public final class ListType implements Cloneable
{
    public short index; /* num of entries in upd_list */
    public UpdateType upd_list[];

    /**
     * Instantiates a new list type.
     */
    public ListType()
    {
        upd_list = new UpdateType[256];
        for (int i = 0; i < 256; i++)
            upd_list[i] = new UpdateType();
    }

    private ListType(short i)
    {
        upd_list = new UpdateType[256];
        index = i;
    }

    public Object clone()
    {
        ListType newList = new ListType(index);
        for (int i = 0; i < 256; i++)
        {
            newList.upd_list[i] = (UpdateType)upd_list[i].clone();
        }
        return newList;
    }
}
