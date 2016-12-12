package com.thomsonreuters.upa.valueadd.cache;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.valueadd.cache.CacheError;

class CacheErrorImpl implements CacheError
{
    int _errorId = CodecReturnCodes.SUCCESS;
    String _text = null;

    @Override
    public int errorId()
    {
        return _errorId;
    }

    @Override
    public void errorId(int errorId)
    {
        _errorId = errorId;
    }

    @Override
    public String text()
    {
        return _text;
    }

    @Override
    public void text(String text)
    {
        _text = text;
    }

    @Override
    public void clear()
    {
        _errorId = CodecReturnCodes.SUCCESS;
        _text = null;
    }

    public String toString()
    {
        return "Error" + "\n" + "\tErrorId: " + _errorId + "\n" + "\ttext: " + _text;
    }

}