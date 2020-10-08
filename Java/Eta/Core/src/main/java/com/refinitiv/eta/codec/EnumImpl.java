package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Enum;

class EnumImpl implements Enum
{
    int _enumValue;
    boolean _isBlank;
    
    // for value(String) method
    private String trimmedVal;

    EnumImpl()
    {
    }

    @Override
    public void clear()
    {
        _enumValue = 0;
    }

    void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
	
    public int copy(Enum destEnum)
    {
        if (null == destEnum)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((EnumImpl)destEnum)._enumValue = _enumValue;
        ((EnumImpl)destEnum)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int toInt()
    {
        return _enumValue;
    }

    @Override
    public int value(int value)
    {
        if (!(value >= 0 && value <= 65535))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _enumValue = value;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeEnum(iter, this);
    }
	
    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeEnum((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }
	
    @Override
    public String toString()
    {
        return Integer.toString(_enumValue);
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
            return value(Integer.parseInt(trimmedVal));
        }
        catch (NumberFormatException e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public boolean equals(Enum thatEnum)
    {
        return ((thatEnum != null) &&
                (thatEnum.toInt() == _enumValue) &&
                (thatEnum.isBlank() == _isBlank));
    }
}
