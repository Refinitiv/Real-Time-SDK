package com.refinitiv.eta.codec;

class FloatImpl implements Float
{
    float _floatValue;
    boolean _isBlank;

    // for value(String) method
    private String trimmedVal;
	
    @Override
    public void clear()
    {
        _floatValue = 0;
    }

    public void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
	
    public int copy(Float destFloat)
    {
        if (null == destFloat)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((FloatImpl)destFloat)._floatValue = _floatValue;
        ((FloatImpl)destFloat)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }
   
    @Override
    public boolean equals(Float thatFloat)
    {
        return ((thatFloat != null) &&
                (_floatValue == thatFloat.toFloat()) &&
                (_isBlank == thatFloat.isBlank()));
    }

    @Override
    public float toFloat()
    {
        return _floatValue;
    }

    @Override
    public Real toReal(int hint)
    {
        assert (hint >= RealHints.EXPONENT_14 && hint <= RealHints.MAX_DIVISOR) : "hint is out of range. Refer to RealHints";

        RealImpl real = (RealImpl)CodecFactory.createReal();

        if (_floatValue != 0)
        {
            real._hint = hint;
            if (_floatValue > 0)
                real._value = (long)(_floatValue * RealImpl.powHintsExp[hint] + 0.5);
            else
                real._value = (long)(_floatValue * RealImpl.powHintsExp[hint] - 0.5);
            real._isBlank = false;
        }
        else
        {
            real.blank();
        }

        return real;
    }

    @Override
    public String toString()
    {
        return java.lang.Float.toString(_floatValue);
    }

    @Override
    public void value(float value)
    {
        _floatValue = value;
        _isBlank = false;
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
            value(java.lang.Float.parseFloat(trimmedVal));
            return CodecReturnCodes.SUCCESS;
        }
        catch (NumberFormatException e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeFloat((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeFloat(iter, this);
    }
}
