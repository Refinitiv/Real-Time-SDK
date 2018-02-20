package com.thomsonreuters.ansi;

public final class UpdateType implements Cloneable
{
    public short    row;            /* row on which the update appears */
    public short    upd_beg;        /* beginning column of update */
    public short    upd_end;        /* ending column of update */

    public Object clone()
    {
        UpdateType update = new UpdateType();
        update.row = row;
        update.upd_beg = upd_beg;
        update.upd_end = upd_end;
        return update;
    }
}

