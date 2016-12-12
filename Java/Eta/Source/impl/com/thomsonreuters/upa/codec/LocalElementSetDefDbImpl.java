package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementSetDefEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.LocalElementSetDefDb;

class LocalElementSetDefDbImpl extends ElementSetDefDbImpl implements LocalElementSetDefDb
{
    /* Maximum local message scope set identifier */
    static final int MAX_LOCAL_ID = 15;

    ElementSetDefEntryImpl[][] _entries;
	
    LocalElementSetDefDbImpl()
    {
        super(MAX_LOCAL_ID);
        _entries = new ElementSetDefEntryImpl[MAX_LOCAL_ID + 1][255];

        for (int i = 0; i <= MAX_LOCAL_ID; ++i)
        {
            _definitions[i] = (ElementSetDefImpl)CodecFactory.createElementSetDef();
            _definitions[i]._setId = BLANK_ID;
            for (int j = 0; j < 255; j++)
            {
                _entries[i][j] = (ElementSetDefEntryImpl)CodecFactory.createElementSetDefEntry();
            }
        }
    }
	
    @Override
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
    }
	
    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeLocalElementSetDefDb(iter, this);
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeLocalElementSetDefDb(iter, this);
    }

    @Override
    public ElementSetDefEntry[][] entries()
    {
        return _entries;
    }
}
