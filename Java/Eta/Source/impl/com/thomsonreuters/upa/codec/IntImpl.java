package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;

class IntImpl implements Int
{
    long _longValue;
    boolean _isBlank;
    
    // for value(String) method
    private String trimmedVal;

    IntImpl()
    {
    }
    
    @Override
    public void clear()
    {
        _longValue = 0;
    }
    
    void blank()
    {
        clear();
        _isBlank = true;
    }
	
    public int copy(Int destInt)
    {
        if (null == destInt)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((IntImpl)destInt)._longValue = _longValue;
        ((IntImpl)destInt)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }

    @Override
    public void value(int value)
    {
        _longValue = value;
        _isBlank = false;
    }

    @Override
    public void value(long value)
    {
        _longValue = value;
        _isBlank = false;
    }

    @Override
    public long toLong()
    {
        return _longValue;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeInt((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }
    
    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeInt(iter, this);
    }

    @Override
    public String toString()
    {
        return Long.toString(_longValue);
    }
    
    @Override
    public int value(String value)
    {
        if (value == null)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        trimmedVal = value.trim();
        if (trimmedVal.length() == 0)
        {
            // blank
            blank();
            return CodecReturnCodes.SUCCESS;
        }

        try
        {
            value(Long.parseLong(trimmedVal));
            return CodecReturnCodes.SUCCESS;
        }
        catch (NumberFormatException e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public boolean equals(Int thatInt)
    {
        return ((thatInt != null) &&
                (_longValue == thatInt.toLong()) &&
                (_isBlank == thatInt.isBlank()));
    }
}
