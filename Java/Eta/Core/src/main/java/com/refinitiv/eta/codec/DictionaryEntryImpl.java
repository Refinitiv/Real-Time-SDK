package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EnumTypeTable;

class DictionaryEntryImpl implements DictionaryEntry
{
    final Buffer    _acronym = CodecFactory.createBuffer();
    final Buffer    _ddeAcronym = CodecFactory.createBuffer();
    int        _fid;
    int        _rippleToField;
    int        _fieldType;
    int        _length;
    int        _enumLength;
    int        _rwfType;
    int        _rwfLength;
    EnumTypeTable   _enumTypeTable;
	
    @Override
    public Buffer acronym()
    {
        return _acronym;
    }

    @Override
    public Buffer ddeAcronym()
    {
        return _ddeAcronym;
    }

    @Override
    public int fid()
    {
        return _fid;
    }

    @Override
    public int rippleToField()
    {
        return _rippleToField;
    }

    @Override
    public int fieldType()
    {
        return _fieldType;
    }

    @Override
    public int length()
    {
        return _length;
    }

    @Override
    public int enumLength()
    {
        return _enumLength;
    }

    @Override
    public int rwfType()
    {
        return _rwfType;
    }

    @Override
    public int rwfLength()
    {
        return _rwfLength;
    }

    @Override
    public EnumTypeTable enumTypeTable()
    {
        return _enumTypeTable;
    }
}
