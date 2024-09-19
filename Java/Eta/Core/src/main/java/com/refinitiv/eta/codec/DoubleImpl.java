/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

class DoubleImpl implements Double
{
    double _doubleValue;
    boolean _isBlank;
	
    // for value(String) method
    private String trimmedVal;

    @Override
    public void clear()
    {
        _doubleValue = 0;
    }

    @Override
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
    
    public int copy(Double destDouble)
    {
        if (null == destDouble)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((DoubleImpl)destDouble)._doubleValue = _doubleValue;
        ((DoubleImpl)destDouble)._isBlank = _isBlank;

        return CodecReturnCodes.SUCCESS;
    }
	
    @Override
    public boolean equals(Double thatDouble)
    {
        return ((thatDouble != null) &&
                (_doubleValue == thatDouble.toDouble()) &&
                (_isBlank == thatDouble.isBlank()));
    }

    @Override
    public double toDouble()
    {
        return _doubleValue;
    }

    @Override
    public Real toReal(int hint)
    {
        assert (hint >= RealHints.EXPONENT_14 && hint <= RealHints.MAX_DIVISOR) : "hint is out of range. Refer to RealHints";

        RealImpl real = (RealImpl)CodecFactory.createReal();

        if (_doubleValue != 0)
        {
            real._hint = hint;
            if (_doubleValue > 0)
                real._value = (long)(_doubleValue * RealImpl.powHintsExp[hint] + 0.5);
            else
                real._value = (long)(_doubleValue * RealImpl.powHintsExp[hint] - 0.5);
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
        return java.lang.Double.toString(_doubleValue);
    }

    @Override
    public void value(double value)
    {
        _doubleValue = value;
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
            value(java.lang.Double.parseDouble(trimmedVal));
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
            return Encoders.PrimitiveEncoder.encodeDouble((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeDouble(iter, this);
    }
}
