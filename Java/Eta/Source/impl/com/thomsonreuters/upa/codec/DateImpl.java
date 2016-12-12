package com.thomsonreuters.upa.codec;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;

class DateImpl implements Date
{
    int _day;
    int _month;
    int _year;
	
    private String[] _months = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

    // for value(String) method
    private String trimmedVal;
    private Pattern datePattern1 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)");
    private Pattern datePattern2 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)");
    private Pattern datePattern3 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)");

    @Override
    public void clear()
    {
        _day = 0;
        _month = 0;
        _year = 0;
    }	
	
    @Override
    public int copy(Date destDate)
    {
        if (null == destDate)
            return CodecReturnCodes.INVALID_ARGUMENT;

        destDate.day(_day);
        destDate.month(_month);
        destDate.year(_year);

        return CodecReturnCodes.SUCCESS;
    }
	
    @Override
    public void blank()
    {
        clear();
    }
	
    @Override
    public boolean isBlank()
    {
        return (_day == 0 && _month == 0 && _year == 0) ? true : false;
    }
	
    @Override
    public boolean equals(Date thatDate)
    {
        return ((thatDate != null) &&
                (_day == ((DateImpl)thatDate)._day) &&
                (_month == ((DateImpl)thatDate)._month) &&
                (_year == ((DateImpl)thatDate)._year));
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeDate(iter, this);
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.PrimitiveEncoder.encodeDate((EncodeIteratorImpl)iter, this);
    }

    boolean isLeapYear()
    {
        if ((_year % 4 == 0) &&
           ((_year % 100 != 0) || (_year % 400 == 0)))
            return true;
        else
            return false;
    }
	
    public boolean isValid()
    {
        if (isBlank())
            return true;

        /* month or date or year of 0 is valid because marketfeed can send it */
        switch (_month)
        {
            case 0:
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                if (_day > 31)
                    return false;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                if (_day > 30)
                    return false;
                break;
            case 2:
                if (_day > 29)
                    return false;
                else if ((_day == 29) && !isLeapYear())
                    return false;
                break;
            default:
                return false;
        }

        return true;
    }
	
    @Override
    public String toString()
    {
        if (!isBlank())
        {
            if (isValid())
            {
                StringBuilder retStr = new StringBuilder();

                /* normal date */
                /* put this into the same format as marketfeed uses where if any portion is blank, it is represented as spaces */
                if (_day > 9)
                {
                    retStr.append(_day + " ");
                }
                else if (_day > 0)
                {
                    retStr.append(" " + _day + " ");
                }
                else
                {
                    retStr.append("   ");
                }

                if (_month > 0)
                {
                    retStr.append(_months[_month - 1] + " ");
                }
                else
                {
                    retStr.append("    ");
                }

                if (_year > 999)
                {
                    retStr.append(_year + " ");
                }
                else if (_year > 99)
                {
                    retStr.append(" " + _year + " ");
                }
                else if (_year > 9)
                {
                    retStr.append("  " + _year + " ");
                }
                else
                {
                    retStr.append("    ");
                }

                return retStr.toString();
            }
            else
            {
                return "Invalid date";
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
        try
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

            Matcher matcher = datePattern1.matcher(trimmedVal);

            if (matcher.matches() && matcher.groupCount() == 3)
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }

                return CodecReturnCodes.SUCCESS;
            }

            if (Character.isDigit(trimmedVal.charAt(3)))
            {
                matcher = datePattern2.matcher(trimmedVal);
                if (matcher.matches() && matcher.groupCount() == 3)
                {
                    int a = Integer.parseInt(matcher.group(1));
                    int b = Integer.parseInt(matcher.group(2));
                    int c = Integer.parseInt(matcher.group(3));

                    if (a > 255) // assume year here is greater than MAX UINT8
                    {
                        // 1974/04/14
                        ret = day(c);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = month(b);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = year(a);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                    }
                    else if (c < 100)// assume year here is less than 100, then add 1900
                    {
                        // 04/14/74
                        ret = day(b);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = month(a);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = year(c + 1900);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                    }
                    else
                    {
                        // 04/14/1974
                        ret = day(b);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = month(a);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = year(c);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                    }

                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    return CodecReturnCodes.INVALID_ARGUMENT;
                }
            }
            else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
            {
                matcher = datePattern3.matcher(trimmedVal);
                if (matcher.matches() && matcher.groupCount() == 3)
                {
                    int a = Integer.parseInt(matcher.group(1));
                    String strMon = matcher.group(2);
                    int c = Integer.parseInt(matcher.group(3));
                    if (c < 100) // assume year here is less than 100, then add 1900
                    {
                        // 04/14/74
                        ret = day(a);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = month(translateMonth(strMon));
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = year(c + 1900);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                    }
                    else
                    {
                        ret = day(a);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = month(translateMonth(strMon));
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                        ret = year(c);
                        if (ret != CodecReturnCodes.SUCCESS)
                            return ret;
                    }

                    return CodecReturnCodes.SUCCESS;
                }
                else
                {
                    return CodecReturnCodes.INVALID_ARGUMENT;
                }
            }
        }
        catch (Exception e)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
    private int translateMonth(String monthStr)
    {
        int i, month = 0;

        for (i = 0; i < 12; i++)
        {
            if (monthStr.equalsIgnoreCase(_months[i]))
            {
                month = i + 1;
                break;
            }
        }

        return month;
    }

    @Override
    public int day(int day)
    {
        if (day < 0 || day > 31)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _day = day;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int day()
    {
        return _day;
    }

    @Override
    public int month(int month)
    {
        if (month < 0 || month > 12)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _month = month;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int month()
    {
        return _month;
    }

    @Override
    public int year(int year)
    {
        if (year < 0 || year > 65535)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _year = year;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int year()
    {
        return _year;
    }

}
