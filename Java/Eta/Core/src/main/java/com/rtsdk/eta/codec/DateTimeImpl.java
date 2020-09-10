package com.rtsdk.eta.codec;

import static com.rtsdk.eta.codec.DateImpl.MONTHS_EN;
import java.util.Calendar;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class DateTimeImpl implements DateTime
{
    Date _date;
    Time _time;
    int _format;
    
    private Calendar _calendar;
    private Matcher matcher;
    
    // for value(String) method
    private String trimmedVal;
	
    // Date + time to milli
    private static final Pattern DATETIME_PATTERN_1 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+)");  // m/d/y h:m:s:milli
    private static final Pattern DATETIME_PATTERN_2 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");  // d my y h m s milli
    private static final Pattern DATETIME_PATTERN_3 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");// d month y h m s milli
    private static final Pattern DATETIME_PATTERN_4 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)"); //m/d/y h m s milli
    private static final Pattern DATETIME_PATTERN_5 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+)"); // d m y h:m:s:milli
    private static final Pattern DATETIME_PATTERN_6 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+)"); // d month year h:m:s:milli
    
    // date + time to nano
    private static final Pattern DATETIME_PATTERN_7 =
            Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");  // m/d/y h:m:s:milli:micro:nano
    private static final Pattern DATETIME_PATTERN_8 =
            Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");  // d my y h m s milli micro nano
    private static final Pattern DATETIME_PATTERN_9 =
            Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");// d month y h m s milli micro nano
    private static final Pattern DATETIME_PATTERN_10 =
            Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)"); //m/d/y h m s milli micro nano
    private static final Pattern DATETIME_PATTERN_11 =
            Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)"); // d m y h:m:s:milli:micro:nano
    private static final Pattern DATETIME_PATTERN_12 =
            Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)"); // d month year h:m:s:milli:micro:nano
    
    private static final Pattern DATETIME_PATTERN_13 =
            Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");  // m/d/y h:m:s:milli:micro
    private static final Pattern DATETIME_PATTERN_14 =
            Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");  // d my y h m s milli micro 
    private static final Pattern DATETIME_PATTERN_15 =
            Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");// d month y h m s milli micro 
    private static final Pattern DATETIME_PATTERN_16 =
            Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)"); //m/d/y h m s milli micro 
    private static final Pattern DATETIME_PATTERN_17 =
            Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)"); // d m y h:m:s:milli:micro
    private static final Pattern DATETIME_PATTERN_18 =
            Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)"); // d month year h:m:s:milli:micro
    
    private static final Pattern DATETIME_PATTERN_19 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+):(\\d+):(\\d+)");  // m/d/y h:m:s
    private static final Pattern DATETIME_PATTERN_20 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");  // d my y h m s 
    private static final Pattern DATETIME_PATTERN_21 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");// d month y h m s 
    private static final Pattern DATETIME_PATTERN_22 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)"); //m/d/y h m s 
    private static final Pattern DATETIME_PATTERN_23 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+)"); // d m y h:m:s
    private static final Pattern DATETIME_PATTERN_24 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+):(\\d+):(\\d+)"); // d month year h:m:s
    
    private static final Pattern DATETIME_PATTERN_25 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+):(\\d+)");  // m/d/y h:m
    private static final Pattern DATETIME_PATTERN_26 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");  // d my y h m  
    private static final Pattern DATETIME_PATTERN_27 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");// d month y h m 
    private static final Pattern DATETIME_PATTERN_28 = Pattern.compile("(\\d+)/(\\d+)/(\\d+)\\s(\\d+)\\s(\\d+)"); //m/d/y h m  
    private static final Pattern DATETIME_PATTERN_29 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+):(\\d+)"); // d m y h:m
    private static final Pattern DATETIME_PATTERN_30 = Pattern.compile("(\\d+)\\s(\\p{Alpha}+)\\s(\\d+)\\s(\\d+):(\\d+)"); // d month year h:m
 
    DateTimeImpl()
    {
        _date = new DateImpl();
        _time = new TimeImpl();
        _format = DateTimeStringFormatTypes.STR_DATETIME_RSSL;
    }

    @Override
    public void clear()
    {
        _date.clear();
        _time.clear();
        _format = DateTimeStringFormatTypes.STR_DATETIME_RSSL;
    }

    @Override
    public void blank()
    {
        _date.blank();
        _time.blank();
    }

    @Override
    public boolean isBlank()
    {
        return (_date.isBlank() & _time.isBlank());
    }

    @Override
    public boolean equals(DateTime thatDateTime)
    {
        return (thatDateTime != null && _time.equals(thatDateTime.time()) && _date.equals(thatDateTime.date()) && thatDateTime.format() == _format);
    }

    @Override
    public boolean isValid()
    {
        return _time.isValid() && _date.isValid();
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeDateTime(iter, this);
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.PrimitiveEncoder.encodeDateTime((EncodeIteratorImpl)iter, this);
    }    
    
    // match to minute
    private int matchDTToMin(String value)
    {
        int ret = CodecReturnCodes.SUCCESS;
        matcher = DATETIME_PATTERN_25.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(0);
            ret = _time.millisecond(0);
            ret = _time.microsecond(0);
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }

        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_26.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(0);
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_27.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(0);
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }

        matcher = DATETIME_PATTERN_28.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(0);
            ret = _time.millisecond(0);
            ret = _time.microsecond(0);
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }
        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_29.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(0);
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_30.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(0);
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        return CodecReturnCodes.INVALID_ARGUMENT;
    }    
    
    // match to second
    private int matchDTToSec(String value)
    {
        int ret = CodecReturnCodes.SUCCESS;
        matcher = DATETIME_PATTERN_19.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(0);
            ret = _time.microsecond(0);
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }

        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_20.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_21.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }

        matcher = DATETIME_PATTERN_22.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(0);
            ret = _time.microsecond(0);
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }
        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_23.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_24.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(0);
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        return CodecReturnCodes.INVALID_ARGUMENT;
    }
    
    // match to millisecond
    private int matchDTToMilli(String value)
    {
        int ret = CodecReturnCodes.SUCCESS;
        matcher = DATETIME_PATTERN_1.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.microsecond(0);
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }

        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_2.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_3.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }

        matcher = DATETIME_PATTERN_4.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.microsecond(0);
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }
        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_5.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_6.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(0);
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        return CodecReturnCodes.INVALID_ARGUMENT;
    }
    
    // matches to microsecond time
    private int matchDTToMicro(String value)
    {
        int ret = CodecReturnCodes.SUCCESS;
        matcher = DATETIME_PATTERN_13.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }

        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_14.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_15.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }

        matcher = DATETIME_PATTERN_16.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.nanosecond(0);

            return CodecReturnCodes.SUCCESS;
        }
        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_17.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_18.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(0);

                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        return CodecReturnCodes.INVALID_ARGUMENT;
    }
    
    // matches to nanosecond time
    private int matchDTToNano(String value)
    {
        int ret = CodecReturnCodes.SUCCESS;

        matcher = DATETIME_PATTERN_7.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.nanosecond(Integer.parseInt(matcher.group(9)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            return CodecReturnCodes.SUCCESS;
        }

        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_8.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(Integer.parseInt(matcher.group(9)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_9.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;

                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(Integer.parseInt(matcher.group(9)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }
        }

        matcher = DATETIME_PATTERN_10.matcher(trimmedVal);
        if (matcher.matches())
        {
            int a = Integer.parseInt(matcher.group(1));
            int b = Integer.parseInt(matcher.group(2));
            int c = Integer.parseInt(matcher.group(3));

            if (a > 255) // assume year here is greater than MAX UINT8
            {
                // 1974/04/14
                ret = _date.day(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else if (c < 100)// assume year here is less than 100, then add 1900
            {
                // 04/14/74
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c + 1900);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            else
            {
                // 04/14/1974
                ret = _date.day(b);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.month(a);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _date.year(c);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
            ret = _time.hour(Integer.parseInt(matcher.group(4)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.minute(Integer.parseInt(matcher.group(5)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.second(Integer.parseInt(matcher.group(6)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            ret = _time.nanosecond(Integer.parseInt(matcher.group(9)));
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            return CodecReturnCodes.SUCCESS;
        }
        if (Character.isDigit(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_11.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                int b = Integer.parseInt(matcher.group(2));
                int c = Integer.parseInt(matcher.group(3));

                if (a > 255) // assume year here is greater than MAX UINT8
                {
                    // 1974/04/14
                    ret = _date.day(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else if (c < 100)// assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    // 04/14/1974
                    ret = _date.day(b);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(Integer.parseInt(matcher.group(9)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }
        }
        else if (Character.isUpperCase(trimmedVal.charAt(3)) || Character.isLowerCase(trimmedVal.charAt(3)))
        {
            matcher = DATETIME_PATTERN_12.matcher(trimmedVal);
            if (matcher.matches())
            {
                int a = Integer.parseInt(matcher.group(1));
                String strMon = matcher.group(2);
                int c = Integer.parseInt(matcher.group(3));
                if (c < 100) // assume year here is less than 100, then add 1900
                {
                    // 04/14/74
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c + 1900);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }
                else
                {
                    ret = _date.day(a);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.month(translateMonth(strMon));
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                    ret = _date.year(c);
                    if (ret != CodecReturnCodes.SUCCESS)
                        return ret;
                }

                ret = _time.hour(Integer.parseInt(matcher.group(4)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.minute(Integer.parseInt(matcher.group(5)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.second(Integer.parseInt(matcher.group(6)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.millisecond(Integer.parseInt(matcher.group(7)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.microsecond(Integer.parseInt(matcher.group(8)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ret = _time.nanosecond(Integer.parseInt(matcher.group(9)));
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;

                return CodecReturnCodes.SUCCESS;
            }
            else
            {
                return CodecReturnCodes.INVALID_ARGUMENT;
            }
        }
        return CodecReturnCodes.INVALID_ARGUMENT;
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
            
			// Match ISO8601 DateTime.          
            if(trimmedVal.length() >= 13)
            {
            	int len = trimmedVal.length();
            	char separator = trimmedVal.charAt(10);
            	if( separator == 'T')
            	{ // DateTime in YYYY-MM-DDT<ISO8601 time formats>
	            	 ret = _date.value(trimmedVal.substring(0, 10));
	            	if (ret == CodecReturnCodes.SUCCESS)
	            	{
	            		ret = _time.value(trimmedVal.substring(11, len));
	            		if (ret == CodecReturnCodes.SUCCESS)
	            			return  CodecReturnCodes.SUCCESS;
	            		else
	            			return ret;
	            	}
	            	else
            			return ret;
	            }
            	else
            	{  // DateTime in YYYYMMDDT<ISO8601 time formats>
            		separator = trimmedVal.charAt(8);
                	if( separator == 'T')
                	{
    	            	ret = _date.value(trimmedVal.substring(0, 8));
		            	if (ret == CodecReturnCodes.SUCCESS)
		            	{
		            		ret = _time.value(trimmedVal.substring(9, len));
		            		if (ret == CodecReturnCodes.SUCCESS)
		            			return  CodecReturnCodes.SUCCESS;
		            		else
		            			return ret;
		            	}
		            	else
	            			return ret;
    	            }
            	}
            }
            
  
            	
              if (matchDTToNano(value) != CodecReturnCodes.SUCCESS)
                if (matchDTToMicro(value) != CodecReturnCodes.SUCCESS)
                    if (matchDTToMilli(value) != CodecReturnCodes.SUCCESS)
                        if (matchDTToSec(value) != CodecReturnCodes.SUCCESS)
                            if (matchDTToSec(value) != CodecReturnCodes.SUCCESS)
                                if (matchDTToMin(value) != CodecReturnCodes.SUCCESS)
                                    if (_time.value(value) != CodecReturnCodes.SUCCESS)
                                        return _date.value(value);

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
            if (MONTHS_EN[i].equalsIgnoreCase(monthStr))
            {
                month = i + 1;
                break;
            }
        }

        return month;
    }
    
    @Override
   public int format(int format)
   {
    	if(format >= DateTimeStringFormatTypes.STR_DATETIME_ISO8601 && format <= DateTimeStringFormatTypes.STR_DATETIME_RSSL)
	    {
	    	_format = format;
	    	_date.format(_format);
	    	_time.format(_format);
	    	return CodecReturnCodes.SUCCESS;
	    }
	    else
	    	return CodecReturnCodes.INVALID_ARGUMENT;
   }
    
    @Override
    public int format()
    {
        return _format;
    }

 
    @Override
    public String toString()
    {
        if (isBlank())
            return "";
        StringBuilder oBuffer = new StringBuilder();

        if (_date.isBlank())
            oBuffer.append("");
        else if (!_date.isValid())
        {
            oBuffer.append("Invalid dateTime");
        }
        else
        {
            /* normal date */
            /* put this into the same format as marketfeed uses where if any portion is blank, it is represented as spaces */
            if(_format == DateTimeStringFormatTypes.STR_DATETIME_RSSL)
            {
            	_date.format(DateTimeStringFormatTypes.STR_DATETIME_RSSL);
            	oBuffer.append(String.format("%s",_date.toString().trim()));
            }
            else if(_format == DateTimeStringFormatTypes.STR_DATETIME_ISO8601)
            {
            	_date.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
            	oBuffer.append(String.format("%s",_date.toString().trim()));

            }
            else
            {
            	oBuffer.append("Invalid DateTime format value ");
            	oBuffer.append(_format);
            	return oBuffer.toString();
            }  
        }
 
        if (!_time.isBlank())
        {
	        if(_format == DateTimeStringFormatTypes.STR_DATETIME_RSSL)
	        {
	        	_time.format(DateTimeStringFormatTypes.STR_DATETIME_RSSL);
	            if (!_date.isBlank())
	                oBuffer.append(String.format("%s%s", " ", _time.toString()));
	            else
	                oBuffer.append(String.format("%s", _time.toString()));
	        }
	        else if(_format == DateTimeStringFormatTypes.STR_DATETIME_ISO8601)
	        {
	        	_time.format(DateTimeStringFormatTypes.STR_DATETIME_ISO8601);
	            if (!_date.isBlank())
	                oBuffer.append(String.format("%s%s", "T", _time.toString()));
	            else
	                oBuffer.append(String.format("%s", _time.toString()));	
	        }
        }
        
        return oBuffer.toString();

    }

    @Override
    public Date date()
    {
        return _date;
    }

    @Override
    public Time time()
    {
        return _time;
    }

    @Override
    public void localTime()
    {
        _calendar = Calendar.getInstance();

        syncFromCalendar();
    }

    public int copy(DateTime destDateTime)
    {
        if (null == destDateTime)
            return CodecReturnCodes.INVALID_ARGUMENT;

        _date.copy(destDateTime.date());
        _time.copy(destDateTime.time());

        return CodecReturnCodes.SUCCESS;
    }    
   
    @Override
    public void gmtTime()
    {
        _calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));

        syncFromCalendar();
    }

    @Override
    public int day(int day)
    {
        return _date.day(day);
    }

    @Override
    public int day()
    {
        return _date.day();
    }

    @Override
    public int month(int month)
    {
        return _date.month(month);
    }

    @Override
    public int month()
    {
        return _date.month();
    }

    @Override
    public int year(int year)
    {
        return _date.year(year);
    }

    @Override
    public int year()
    {
        return _date.year();
    }

    @Override
    public int hour(int hour)
    {
        return _time.hour(hour);
    }

    @Override
    public int hour()
    {
        return _time.hour();
    }

    @Override
    public int minute(int minute)
    {
        return _time.minute(minute);
    }

    @Override
    public int minute()
    {
        return _time.minute();
    }

    @Override
    public int second(int second)
    {
        return _time.second(second);
    }

    @Override
    public int second()
    {
        return _time.second();
    }

    @Override
    public int millisecond(int millisecond)
    {
        return _time.millisecond(millisecond);
    }

    @Override
    public int millisecond()
    {
        return _time.millisecond();
    }
    
    @Override
    public int microsecond(int microsecond)
    {
        return _time.microsecond(microsecond);
    }

    @Override
    public int microsecond()
    {
        return _time.microsecond();
    }
    
    @Override
    public int nanosecond(int nanosecond)
    {
        return _time.nanosecond(nanosecond);
    }

    @Override
    public int nanosecond()
    {
        return _time.nanosecond();
    }

    @Override
    public int value(long value)
    {
        if (value < 0)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        if (_calendar == null)
        {
            _calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        }

        _calendar.setTimeInMillis(value);

        syncFromCalendar();

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public long millisSinceEpoch()
    {
        if (_calendar == null)
        {
            _calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        }

        syncToCalendar();

        return _calendar.getTimeInMillis();
    }

    // syncs up this date time from the calendar object
    private void syncFromCalendar()
    {
        _date.day(_calendar.get(Calendar.DAY_OF_MONTH));
        _date.month(_calendar.get(Calendar.MONTH) + 1);
        _date.year(_calendar.get(Calendar.YEAR));
        _time.hour(_calendar.get(Calendar.HOUR_OF_DAY));
        _time.minute(_calendar.get(Calendar.MINUTE));
        _time.second(_calendar.get(Calendar.SECOND));
        _time.millisecond(_calendar.get(Calendar.MILLISECOND));
        _time.microsecond(0);
        _time.nanosecond(0);
    }

    // syncs up this date time into the calendar object
    private void syncToCalendar()
    {
        _calendar.set(Calendar.DAY_OF_MONTH, _date.day());
        _calendar.set(Calendar.MONTH, _date.month() - 1);
        _calendar.set(Calendar.YEAR, _date.year());
        _calendar.set(Calendar.HOUR_OF_DAY, _time.hour());
        _calendar.set(Calendar.MINUTE, _time.minute());
        _calendar.set(Calendar.SECOND, _time.second());
        _calendar.set(Calendar.MILLISECOND, _time.millisecond());
    }

}
