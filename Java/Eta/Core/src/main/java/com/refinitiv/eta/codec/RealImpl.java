package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RealHints;
import java.lang.Double;

class RealImpl implements Real
{
    static final byte BLANK_REAL = 0x20;
    static final double powHintsExp[] = { 100000000000000.0, 10000000000000.0, 1000000000000.0, 100000000000.0, 10000000000.0,
                                          1000000000.0, 100000000.0, 10000000.0, 1000000.0, 100000.0, 10000.0, 1000.0, 100.0,
                                          10.0, 1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001, 1, 2, 4, 8, 16, 32, 64, 128, 256 };
    static final String zeroDisplayStrings[] = { "0.0E-14", "0.0E-13", "0.0E-12", "0.0E-11", "0.0E-10", "0.0E-9", "0.0E-8", "0.0E-7",
                                                 "0.0E-6", "0.0E-5", "0.0E-4", "0.000", "0.00", "0.0", "0", "0", "0", "0", "0", "0",
                                                 "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0" };

    boolean     _isBlank;
    int         _hint;
    long        _value;
	
    // for toString() method so don't have to re-convert if set by string
    private String _stringVal;
	
    // for value(String) method
    private String trimmedVal;
    static private String infinity = "Inf";
    static private String negInfinity = "-Inf";
    static private String notANumber = "NaN";
    private final int MAX_STRLEN = 20;
    UInt valueUInt = CodecFactory.createUInt();
    UInt tempValue = CodecFactory.createUInt();
    Int trailzerovalue = CodecFactory.createInt();
    Int trailzerocount = CodecFactory.createInt();
    Int foundDigit = CodecFactory.createInt();
    Int nextDigit = CodecFactory.createInt();
    UInt denominator = CodecFactory.createUInt();
    Int expdiff = CodecFactory.createInt();
    UInt numerator = CodecFactory.createUInt();

    @Override
    public void clear()
    {
        _isBlank = false;
        _hint = RealHints.EXPONENT0;
        _value = 0;
        _stringVal = null;
    }

    @Override
    public void blank()
    {
        _isBlank = true;
        _hint = RealHints.EXPONENT_14;
        _value = 0;
        _stringVal = null;
    }

    public int copy(Real destReal)
    {
        if (null == destReal)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((RealImpl)destReal)._value = _value;
        ((RealImpl)destReal)._hint = _hint;
        ((RealImpl)destReal)._isBlank = _isBlank;

        ((RealImpl)destReal)._stringVal = null;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int value(double value, int hint)
    {
        if (!(hint >= RealHints.EXPONENT_14 && hint <= RealHints.NOT_A_NUMBER && hint != 31)) // 31 is 'reserved'
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        if (java.lang.Double.isNaN(value))
        {
            _value = 0;
            _hint = RealHints.NOT_A_NUMBER;
        }
        else if (value == java.lang.Double.POSITIVE_INFINITY)
        {
            _value = 0;
            _hint = RealHints.INFINITY;
        }
        else if (value == java.lang.Double.NEGATIVE_INFINITY)
        {
            _value = 0;
            _hint = RealHints.NEG_INFINITY;
        }
        else
        {
            _hint = hint;

            double res;
            if (value > 0)
                res = value * powHintsExp[hint] + 0.5;
            else
                res = value * powHintsExp[hint] - 0.5;

            if (res < Long.MIN_VALUE || Long.MAX_VALUE < res) {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

            // Checks the corner cases.
            // Uses direct assignment value Long.MAX_VALUE (64 bits) and prohibits the conversation from double value (53 bits).
            if (Long.MAX_VALUE == res)
                _value = Long.MAX_VALUE;
            else if (Long.MIN_VALUE == res)
                _value = Long.MIN_VALUE;
            else
                _value = (long)res;
        }
        _isBlank = false;
        _stringVal = null;
        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int value(float value, int hint)
    {
        if (!(hint >= RealHints.EXPONENT_14 && hint <= RealHints.NOT_A_NUMBER && hint != 31)) // 31 is 'reserved'
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        if (java.lang.Float.isNaN(value))
        {
            _value = 0;
            _hint = RealHints.NOT_A_NUMBER;
        }
        else if (value == java.lang.Float.POSITIVE_INFINITY)
        {
            _value = 0;
            _hint = RealHints.INFINITY;
        }
        else if (value == java.lang.Float.NEGATIVE_INFINITY)
        {
            _value = 0;
            _hint = RealHints.NEG_INFINITY;
        }
        else
        {
            _hint = hint;

            float res;
            if (value > 0)
                res = (float)(value * powHintsExp[hint]) + 0.5f;
            else
                res = (float)(value * powHintsExp[hint]) - 0.5f;

            if (res < Long.MIN_VALUE || Long.MAX_VALUE < res) {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

            // Checks the corner cases.
            // Uses direct assignment value Long.MAX_VALUE (64 bits) and prohibits the conversation from float value (24 bits).
            if (Long.MAX_VALUE == res)
                _value = Long.MAX_VALUE;
            else if (Long.MIN_VALUE == res)
                _value = Long.MIN_VALUE;
            else
                _value = (long)res;
        }
        _isBlank = false;
        _stringVal = null;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int value(long value, int hint)
    {
        if (!(hint >= RealHints.EXPONENT_14 && hint <= RealHints.NOT_A_NUMBER && hint != 31 && hint != 32)) // 31 is 'reserved'
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _value = value;
        _hint = hint;
        _isBlank = false;
        _stringVal = null;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public double toDouble()
    {
        if (_isBlank == true)
            return 0;
        switch (_hint)
        {
            case RealHints.NOT_A_NUMBER:
                return java.lang.Double.NaN;
            case RealHints.INFINITY:
                return java.lang.Double.POSITIVE_INFINITY;
            case RealHints.NEG_INFINITY:
                return java.lang.Double.NEGATIVE_INFINITY;
            default:
                return _value / powHintsExp[_hint];
        }
    }

    @Override
    public long toLong()
    {
        return _value;
    }

    @Override
    public int hint()
    {
        return _hint;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
    
    @Override
    public boolean equals(Real thatReal)
    {
        if (thatReal != null)
        {
            if ((_value == thatReal.toLong()) &&
                    (_hint == thatReal.hint()) &&
                    (_isBlank == thatReal.isBlank()))
                return true;
            else
                return ((toDouble() == thatReal.toDouble()) && (_isBlank == thatReal.isBlank()));
        }
        else
            return false;
    }

    boolean isHintBlank()
    {
        return (_hint & BLANK_REAL) > 0 ? true : false;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.PrimitiveEncoder.encodeReal((EncodeIteratorImpl)iter, this);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeReal(iter, this);
    }

    @Override
    public String toString()
    {
        if (_stringVal == null)
        {
            switch (_hint)
            {
                case RealHints.INFINITY:
                {
                    _stringVal = infinity;
                    return "Inf";
                }
                case RealHints.NEG_INFINITY:
                {
                    _stringVal = negInfinity;
                    return "-Inf";
                }
                case RealHints.NOT_A_NUMBER:
                {
                    _stringVal = notANumber;
                    return "NaN";
                }
                default:
                    break;
            }

            if (_value != 0)
            {
                return Double.toString(toDouble());
            }
            else
            {
                return zeroDisplayStrings[_hint];
            }
        }
        else
        {
            return _stringVal;
        }
    }

    @Override
    public int value(String value)
    {
        if (value == null)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        int maxStringLen = MAX_STRLEN;
        boolean isNeg = false;
        int valIdx = 0;
        _stringVal = null;

        valueUInt.clear();
        tempValue.clear();
        trailzerovalue.clear();
        trailzerocount.clear();
        foundDigit.clear();
        nextDigit.clear();
        denominator.clear();
        expdiff.clear();
        numerator.clear();

        trimmedVal = value.trim();

        if (valIdx == trimmedVal.length())
        {
            blank();
            _stringVal = trimmedVal;
            return CodecReturnCodes.SUCCESS;
        }

        if (trimmedVal.equalsIgnoreCase(infinity))
        {
            _isBlank = false;
            _value = 0;
            _stringVal = infinity;
            _hint = RealHints.INFINITY;
            return CodecReturnCodes.SUCCESS;
        }
        else if (trimmedVal.equalsIgnoreCase(negInfinity))
        {
            _isBlank = false;
            _value = 0;
            _stringVal = negInfinity;
            _hint = RealHints.NEG_INFINITY;
            return CodecReturnCodes.SUCCESS;
        }
        else if (trimmedVal.equalsIgnoreCase(notANumber))
        {
            _isBlank = false;
            _value = 0;
            _stringVal = notANumber;
            _hint = RealHints.NOT_A_NUMBER;
            return CodecReturnCodes.SUCCESS;
        }

        // check if blank
        if (trimmedVal.charAt(valIdx) == '+')
        {
            boolean isAllZeroes = true;
            for (int i = valIdx + 1; i < trimmedVal.length(); i++)
            {
                if (trimmedVal.charAt(i) != '0' && trimmedVal.charAt(i) != '.')
                {
                    isAllZeroes = false;
                    break;
                }
            }
            if (isAllZeroes)
            {
                blank();
                _stringVal = trimmedVal;
                return CodecReturnCodes.SUCCESS;
            }
        }

        // check if negative
        if (trimmedVal.charAt(valIdx) == '-')
        {
            isNeg = true;
            maxStringLen++;
            valIdx++;
        }
        else if (trimmedVal.charAt(valIdx) == '+')
        {
            valIdx++;
        }

        valIdx = rwf_atonumber_end_trailzero(trimmedVal, valIdx, valueUInt, foundDigit, trailzerovalue, trailzerocount, nextDigit, tempValue);

        if (valIdx == trimmedVal.length())
        {
            // number must be no bigger than max string length
            if (trimmedVal.length() <= maxStringLen)
            {
                value(!isNeg ? valueUInt.toLong() : -valueUInt.toLong(), RealHints.EXPONENT0);
                _isBlank = false;
            }
            else
            {
                // error
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
            _stringVal = trimmedVal;
            return CodecReturnCodes.SUCCESS;
        }

        /* Check for decimal value */
        if (trimmedVal.charAt(valIdx) == '.')
        {
            /* It is a decimal value */
            int startdec = ++valIdx;
            int exponent;
            maxStringLen++;

            if (trimmedVal.length() > maxStringLen)
            {
                // error
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

            valIdx = rwf_atonumber_end(trimmedVal, valIdx, valueUInt, foundDigit, nextDigit, tempValue);

            exponent = valIdx - startdec;

            if (exponent == 0)
            {
                // error
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

            value(!isNeg ? valueUInt.toLong() : -valueUInt.toLong(), RealHints.EXPONENT0 - exponent);
            if (trimmedVal.charAt(0) != '+')
            {
                _isBlank = false;
            }
        }
        else if (trimmedVal.charAt(valIdx) == ' ')
        {
            valIdx++;
            maxStringLen++;

            /* Check for another digit. Then it might be a fraction. */
            if ((trimmedVal.charAt(valIdx) >= '0' && trimmedVal.charAt(valIdx) <= '9'))
            {
                tempValue.clear();
                denominator.clear();

                valIdx = rwf_atonumber_end(trimmedVal, valIdx, numerator, foundDigit, nextDigit, tempValue);

                /* Verify fraction */
                if (trimmedVal.charAt(valIdx) != '/')
                {
                    // error
                    return CodecReturnCodes.INVALID_ARGUMENT;
                }

                maxStringLen++;
                if (trimmedVal.length() >= maxStringLen)
                {
                    // error
                    return CodecReturnCodes.INVALID_ARGUMENT;
                }

                valIdx++;
                valIdx = rwf_atonumber_end(trimmedVal, valIdx, denominator, foundDigit, nextDigit, tempValue);

                int hint = rwf_SetFractionHint((int)denominator.toLong());
                if (hint == 0)
                {
                    // error
                    return CodecReturnCodes.INVALID_ARGUMENT;
                }

                valueUInt.value((valueUInt.toLong() * denominator.toLong()) + numerator.toLong());
                value(!isNeg ? valueUInt.toLong() : -valueUInt.toLong(), hint);
                _isBlank = false;
            }
            else if (valIdx == trimmedVal.length())
            {
                value(!isNeg ? valueUInt.toLong() : -valueUInt.toLong(), RealHints.EXPONENT0);
                _isBlank = false;
            }
            else
            {
                // error
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        else if (trimmedVal.charAt(valIdx) == '/')
        {
            tempValue.clear();
            denominator.clear();

            valIdx++;
            valIdx = rwf_atonumber_end(trimmedVal, valIdx, denominator, foundDigit, nextDigit, tempValue);

            int hint = rwf_SetFractionHint((int)denominator.toLong());
            if (hint == 0)
            {
                // error
                return CodecReturnCodes.INVALID_ARGUMENT;
            }

            /* value stays as value */
            value(!isNeg ? valueUInt.toLong() : -valueUInt.toLong(), hint);
            _isBlank = false;
        }
        else
        {
            // error
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int rwf_atonumber_end_trailzero(String str, int index, UInt result, Int foundDigit, Int trailzerovalue, Int trailzerocount,
                                            Int nextDigit, UInt tempValue)
    {
        while ((index < str.length()) && (str.charAt(index) >= '0' && str.charAt(index) <= '9'))
        {
            tempValue.value(result.toLong() * 10);
            nextDigit.value((str.charAt(index) - 0x30));
            if (str.charAt(index) == '0')
            {
                if (trailzerocount.toLong() == 0)
                    trailzerovalue.value(result.toLong());
                trailzerocount.value(trailzerocount.toLong() + 1);
            }
            else
                trailzerocount.value(0);
            foundDigit.value(1);
            result.value(tempValue.toLong() + nextDigit.toLong());
            index++;
        }

        return index;
    }

    private int rwf_atonumber_end(String str, int index, UInt result, Int foundDigit, Int nextDigit, UInt tempValue)
    {
        while ((index < str.length()) && (str.charAt(index) >= '0' && str.charAt(index) <= '9'))
        {
            foundDigit.value(1);
            tempValue.value(result.toLong() * 10);
            nextDigit.value((str.charAt(index) - 0x30));
            result.value(tempValue.toLong() + nextDigit.toLong());
            index++;
        }

        return index;
    }

    private int rwf_SetFractionHint(int denom)
    {
        int retval = 0;
        switch (denom)
        {
            case 1:
                retval = RealHints.FRACTION_1;
                break;
            case 2:
                retval = RealHints.FRACTION_2;
                break;
            case 4:
                retval = RealHints.FRACTION_4;
                break;
            case 8:
                retval = RealHints.FRACTION_8;
                break;
            case 16:
                retval = RealHints.FRACTION_16;
                break;
            case 32:
                retval = RealHints.FRACTION_32;
                break;
            case 64:
                retval = RealHints.FRACTION_64;
                break;
            case 128:
                retval = RealHints.FRACTION_128;
                break;
            case 256:
                retval = RealHints.FRACTION_256;
                break;
            default:
                break;
        }
        return retval;
    }
}
