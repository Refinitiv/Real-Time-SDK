package com.thomsonreuters.upa.codec;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Time;

class TimeImpl implements Time
{
    static final int BLANK_HOUR = 255;
    static final int BLANK_MINUTE = 255;
    static final int BLANK_SECOND = 255;
    static final int BLANK_MILLI = 65535;
    static final int BLANK_MICRO_NANO = 2047;

    int _hour;
    int _minute;
    int _second;
    int _millisecond;
    int _microsecond;
    int _nanosecond;
	
    // for value(String) method
    private String trimmedVal;
    private Matcher matcher;
    private Pattern timePattern1 = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\d+)");
    private Pattern timePattern2 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
    private Pattern timePattern3 = Pattern.compile("(\\d+):(\\d+):(\\d+)");
    private Pattern timePattern4 = Pattern.compile("(\\d+):(\\d+)");
    private Pattern timePattern5 = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
    private Pattern timePattern6 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
    private Pattern timePattern7 = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
    private Pattern timePattern8 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
    
    @Override
    public void clear()
    {
        _hour = 0;
        _minute = 0;
        _second = 0;
        _millisecond = 0;
        _microsecond = 0;
        _nanosecond = 0;
    }

    @Override
    public boolean isBlank()
    {
        return (_hour == BLANK_HOUR && _minute == BLANK_MINUTE && _second == BLANK_SECOND && 
                _millisecond == BLANK_MILLI  && _microsecond == BLANK_MICRO_NANO && _nanosecond == BLANK_MICRO_NANO) ? true : false;
    }

    @Override
    public boolean isValid()
    {
        if (isBlank())
            return true;

        /* month or date or year of 0 is valid because marketfeed can send it */
        if (_hour != BLANK_HOUR && (_hour < 0 || _hour > 23))
            return false;

        if (_minute != BLANK_MINUTE && (_minute < 0 || _minute > 59))
            return false;

        if (_second != BLANK_SECOND && (_second < 0 || _second > 60))
            return false;

        if (_millisecond != BLANK_MILLI && (_millisecond < 0 || _millisecond > 999))
            return false;

        if (_microsecond != BLANK_MICRO_NANO && (_microsecond < 0 || _microsecond > 999))
            return false;

        if (_nanosecond != BLANK_MICRO_NANO && (_nanosecond < 0 || _nanosecond > 999))
            return false;

        if (_nanosecond == BLANK_MICRO_NANO)
        {
            if (_microsecond == BLANK_MICRO_NANO)
            {
                if (_millisecond == BLANK_MILLI)
                {
                    if (_second == BLANK_SECOND)
                    {
                        if (_minute == BLANK_MINUTE)
                            return true;
                        else
                        {
                            if (_hour == BLANK_HOUR)
                                return false;
                            else
                                return true;
                        }
                    }
                    else
                    {
                        if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE))
                            return false;
                        else
                            return true;
                    }
                }
                else
                {
                    if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE) || (_second == BLANK_SECOND))
                        return false;
                    else
                        return true;
                }
            }
            else
            {
                if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE) || (_second == BLANK_SECOND) || (_millisecond == BLANK_MILLI))
                    return false;
                else
                    return true;
            }
        }
        else if ((_hour == BLANK_HOUR) || (_minute == BLANK_MINUTE) || (_second == BLANK_SECOND) ||
                (_millisecond == BLANK_MILLI) || (_microsecond == BLANK_MICRO_NANO))
            return false;

        return true;
    }

    @Override
    public int copy(Time destTime)
    {
        if (null == destTime)
            return CodecReturnCodes.INVALID_ARGUMENT;

        destTime.hour(_hour);
        destTime.minute(_minute);
        destTime.second(_second);
        destTime.millisecond(_millisecond);
        destTime.microsecond(_microsecond);
        destTime.nanosecond(_nanosecond);

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public String toString()
    {
        if (!isBlank())
        {
            if (isValid())
            {
                StringBuilder retStr = new StringBuilder();

                if (_hour > 9)
                {
                    retStr.append(_hour);
                }
                else
                {
                    retStr.append("0" + _hour);
                }

                if (_minute == BLANK_MINUTE)
                    return retStr.toString();
                if (_minute > 9)
                {
                    retStr.append(":" + _minute);
                }
                else
                {
                    retStr.append(":" + "0" + _minute);
                }

                if (_second == BLANK_SECOND)
                    return retStr.toString();
                if (_second > 9)
                {
                    retStr.append(":" + _second);
                }
                else
                {
                    retStr.append(":" + "0" + _second);
                }

                if (_millisecond == BLANK_MILLI)
                    return retStr.toString();
                if (_millisecond > 99)
                {
                    retStr.append(":" + _millisecond);
                }
                else if (_millisecond > 9)
                {
                    retStr.append(":" + "0" + _millisecond);
                }
                else
                {
                    retStr.append(":" + "00" + _millisecond);
                }

                if (_microsecond == BLANK_MICRO_NANO)
                    return retStr.toString();
                if (_microsecond > 99)
                {
                    retStr.append(":" + _microsecond);
                }
                else if (_microsecond > 9)
                {
                    retStr.append(":" + "0" + _microsecond);
                }
                else
                {
                    retStr.append(":" + "00" + _microsecond);
                }

                if (_nanosecond == BLANK_MICRO_NANO)
                    return retStr.toString();
                if (_nanosecond > 99)
                {
                    retStr.append(":" + _nanosecond);
                }
                else if (_nanosecond > 9)
                {
                    retStr.append(":" + "0" + _nanosecond);
                }
                else
                {
                    retStr.append(":" + "00" + _nanosecond);
                }

                return retStr.toString();
            }
            else
            {
                return "Invalid time";
            }
        }
        else
        {
            return "";
        }
    }
	
    @Override
    public int value(String value)
    {
        int ret = CodecReturnCodes.SUCCESS;

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
            // hh:mm:ss:lll:uuu:nnn
            matcher = timePattern7.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = millisecond(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = microsecond(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = nanosecond(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            // hh mm ss lll uuu nnn
            matcher = timePattern8.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = millisecond(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = microsecond(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = nanosecond(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            // hh:mm:ss:lll:uuu
            matcher = timePattern5.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = millisecond(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = microsecond(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            // hh mm ss lll uuu nnn
            matcher = timePattern6.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = millisecond(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = microsecond(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            matcher = timePattern1.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = millisecond(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            matcher = timePattern2.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = millisecond(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            matcher = timePattern3.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = second(Integer.parseInt(matcher.group(3)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            matcher = timePattern4.matcher(trimmedVal);
            if (matcher.matches())
            {
                ret = hour(Integer.parseInt(matcher.group(1)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = minute(Integer.parseInt(matcher.group(2)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }

            return CodecReturnCodes.INVALID_ARGUMENT;
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public void blank()
    {
        _hour = BLANK_HOUR;
        _minute = BLANK_MINUTE;
        _second = BLANK_SECOND;
        _millisecond = BLANK_MILLI;
        _microsecond = BLANK_MICRO_NANO;
        _nanosecond = BLANK_MICRO_NANO;
    }

    @Override
    public boolean equals(Time thatTime)
    {
        return ((thatTime != null) &&
                (_second == ((TimeImpl)thatTime)._second) &&
                (_minute == ((TimeImpl)thatTime)._minute) &&
                (_hour == ((TimeImpl)thatTime)._hour) &&
                (_millisecond == ((TimeImpl)thatTime)._millisecond) &&
                (_microsecond == ((TimeImpl)thatTime)._microsecond) &&
                (_nanosecond == ((TimeImpl)thatTime)._nanosecond));
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.PrimitiveEncoder.encodeTime((EncodeIteratorImpl)iter, this);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeTime(iter, this);
    }

    @Override
    public int hour(int hour)
    {
        if (hour != BLANK_HOUR && (hour < 0 || hour > 23))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _hour = hour;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int hour()
    {
        return _hour;
    }

    @Override
    public int minute(int minute)
    {
        if (minute != BLANK_MINUTE && (minute < 0 || minute > 59))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _minute = minute;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int minute()
    {
        return _minute;
    }

    @Override
    public int second(int second)
    {
        if (second != BLANK_SECOND && (second < 0 || second > 60))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _second = second;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int second()
    {
        return _second;
    }

    @Override
    public int millisecond(int millisecond)
    {
        if (millisecond != BLANK_MILLI && (millisecond < 0 || millisecond > 999))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _millisecond = millisecond;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int millisecond()
    {
        return _millisecond;
    }

    @Override
    public int microsecond(int microsecond)
    {
        if (microsecond != BLANK_MICRO_NANO && (microsecond < 0 || microsecond > 999))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _microsecond = microsecond;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int microsecond()
    {
        return _microsecond;
    }

    @Override
    public int nanosecond(int nanosecond)
    {
        if (nanosecond != BLANK_MICRO_NANO && (nanosecond < 0 || nanosecond > 999))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _nanosecond = nanosecond;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int nanosecond()
    {
        return _nanosecond;
    }
}
