package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldSetDefEntry;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;

class LocalFieldSetDefDbImpl extends FieldSetDefDbImpl implements LocalFieldSetDefDb
{
    static final int MAX_LOCAL_ID = 15;
    /* 15 defs * 255 entries per def = 3825 -- * 3 bytes per field list def (2 byte fid, 1 byte type) = ~12000 */
    FieldSetDefEntryImpl[][] _entries = new FieldSetDefEntryImpl[MAX_LOCAL_ID + 1][255];
	
    LocalFieldSetDefDbImpl()
    {
        super(MAX_LOCAL_ID);
        for (int i = 0; i <= MAX_LOCAL_ID; ++i)
        {
            for (int j = 0; j < 255; j++)
            {
                _entries[i][j] = (FieldSetDefEntryImpl)CodecFactory.createFieldSetDefEntry();
            }
        }

        maxSetId = 0;
    }
    
    public void clear()
    {
        for (int i = 0; i <= MAX_LOCAL_ID; ++i)
        {
            _definitions[i]._setId = BLANK_ID;
            for (int j = 0; j < 255; j++)
            {
                _entries[i][j].clear();
            }
        }

        maxSetId = 0;
    }
		
    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeLocalFieldSetDefDb(iter, this);
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeLocalFieldSetDefDb(iter, this);
    }

    @Override
    public FieldSetDefEntry[][] entries()
    {
        return _entries;
    }

}
