package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.FieldSetDef;
import com.thomsonreuters.upa.codec.FieldSetDefEntry;

class FieldSetDefImpl implements FieldSetDef
{
    long               _setId;
    int                _count;
    FieldSetDefEntry[] _entries;
    
    /**
     * Instantiates a new field set def impl.
     */
    public FieldSetDefImpl()
    {
        clear();
    }
    
    @Override
    public void clear()
    {
        _setId = 0;
        _count = 0;
        _entries = null;
    }

    @Override
    public long setId()
    {
        return _setId;
    }

    @Override
    public void setId(long setId)
    {
        assert (setId >= 0 && setId <= 32767) : "setId is out of range (0-32767)"; // 0x8000 (u15rb)

        _setId = setId;
    }

    @Override
    public int count()
    {
        return _count;
    }

    @Override
    public void count(int count)
    {
        assert (count >= 0 && count <= 65535) : "count is out of range (0-65535)"; // uint16
        _count = count;
    }

    @Override
    public FieldSetDefEntry[] entries()
    {
        return _entries;
    }

    @Override
    public void entries(FieldSetDefEntry[] entries)
    {
        assert (entries != null) : "entries must be non-null";

        _entries = entries;
    }

    @Override
    public void copy(FieldSetDef destSetDef)
    {
        FieldSetDefImpl newFieldSetDef = (FieldSetDefImpl)destSetDef;
        newFieldSetDef._count = _count;

        newFieldSetDef.entries(_entries);

        newFieldSetDef.setId(_setId);
        destSetDef = newFieldSetDef;
    }
    
    /**
     * Allocate entries.
     *
     * @param count the count
     * @return the int
     */
    public int allocateEntries(int count)
    {
        if (_entries[0] != null)
            return CodecReturnCodes.FAILURE;

        for (int i = 0; i < count; i++)
        {
            _entries[i] = CodecFactory.createFieldSetDefEntry();
        }

        return CodecReturnCodes.SUCCESS;
    }
}
