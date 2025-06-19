/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

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
    int _format = DateTimeStringFormatTypes.STR_DATETIME_RSSL;
	
    // for value(String) method
    private String trimmedVal;
    private Matcher matcher;
    private static final Pattern TIME_PATTERN_1 = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\d+)");
    private static final Pattern TIME_PATTERN_2 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
    private static final Pattern TIME_PATTERN_3 = Pattern.compile("(\\d+):(\\d+):(\\d+)");
    private static final Pattern TIME_PATTERN_4 = Pattern.compile("(\\d+):(\\d+)");
    private static final Pattern TIME_PATTERN_5 = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
    private static final Pattern TIME_PATTERN_6 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
    private static final Pattern TIME_PATTERN_7 = Pattern.compile("(\\d+):(\\d+):(\\d+):(\\d+):(\\d+):(\\d+)");
    private static final Pattern TIME_PATTERN_8 = Pattern.compile("(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");
    private static final Pattern TIME_PATTERN_9 = Pattern.compile("(\\d+):(\\d+):(\\d+)\\.(\\d+)"); // ISO8601 hh:mm:ss.nnnnnnnnn e.g. 08:37:48.009216350
    private static final Pattern TIME_PATTERN_10 = Pattern.compile("(\\d+):(\\d+):(\\d+),(\\d+)"); // ISO8601 hh:mm:ss.nnnnnnnnn e.g. 08:37:48,009216350
   
    @Override
    public void clear()
    {
        _hour = 0;
        _minute = 0;
        _second = 0;
        _millisecond = 0;
        _microsecond = 0;
        _nanosecond = 0;
        _format = DateTimeStringFormatTypes.STR_DATETIME_RSSL;
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
        destTime.format(_format);
        
        return CodecReturnCodes.SUCCESS;
    }
    
    @Override
   public int format(int format)
   {
    	if(format >= DateTimeStringFormatTypes.STR_DATETIME_ISO8601 && format <= DateTimeStringFormatTypes.STR_DATETIME_RSSL)
	    {
	    	_format = format;
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

    // Converts Time to string in ISO8601 'HH:MM:SS.nnnnnnnnn' format & trims trailing 0s (e.g. '12:15:35.5006619' --Trimmed trail zeros after nano).
    public String toStringIso8601()
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
        retStr.append(String.format(":%02d", _minute));
 
        if (_second == BLANK_SECOND)
            return retStr.toString();
        retStr.append(String.format(":%02d", _second));


        if (_millisecond == BLANK_MILLI)
            return retStr.toString();
        retStr.append(String.format(".%03d", _millisecond));
  
        if (_microsecond != BLANK_MICRO_NANO)
        {
            retStr.append(String.format("%03d", _microsecond));
	
	        if (_nanosecond != BLANK_MICRO_NANO)
	        {
	        	retStr.append(String.format("%03d", _nanosecond));
	        }    	
        }
        
        // Trim trailing zeros
     	String tempVal = retStr.toString();
    	int i = tempVal.length();
    	for (;  i >=1; --i)
    	{
    		if(Character.isDigit(tempVal.charAt(i -1)) &&  tempVal.charAt(i -1) != '0')
    			break;
    		else if(tempVal.charAt(i -1) == '.')
    		{
    			i--;
    			break;
    		}
    	}
    	if(i < tempVal.length())
    		return retStr.toString().substring(0, i);
    	return retStr.toString();
    }
    
    // Converts Time to string in "hour:minute:second:milli" format (e.g. 15:24:54:627).
    public String toStringRssl()
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
    
    @Override
    public String toString()
    {
        if (!isBlank())
        {
            if (isValid())
            {
            	if(_format == DateTimeStringFormatTypes.STR_DATETIME_RSSL)
            		return toStringRssl();
            	else if(_format == DateTimeStringFormatTypes.STR_DATETIME_ISO8601)
            		return toStringIso8601();
            	else
            		return "Invalid Format";
            }
            else
                return "Invalid time";
        }
        else
        {
            return "";
        }
        
    }
	
    // Converts the ISO8601 time string fractional seconds represented after comma/decimal into milli, micro and nano seconds.
    // Expects only the portion after comma/decimal of the ISO8601 time string ['HH:MM:SS,nnnnnnnnn'].
    // E.g from the time string '10:15:55,678009700' Or '101555,678009700', the portion '678009700' after the comma 
    // is converted to 678 milli, 9 micro and 700 nano seconds.
    public int iso8601FractionalStingTimeToTime (String isoFractionalTime)
    {
    	int ret = CodecReturnCodes.SUCCESS;
		int len = isoFractionalTime.length();
		int placeValue = 100;
		int tempVal = 0;
		int i = 0;
		for (;  i < len && i < 3; i++)
		{
		  if(Character.isDigit(isoFractionalTime.charAt(i)))
		  {
			tempVal = tempVal +  Character.getNumericValue(isoFractionalTime.charAt(i)) * placeValue;
			placeValue = placeValue / 10;
		  }
		}
		 ret = millisecond(tempVal );
		 if (ret != CodecReturnCodes.SUCCESS)
             return ret;
		
		tempVal = 0;
		placeValue = 100;
		for (;  i < len && i < 6; i++)
		{
		  if(Character.isDigit(isoFractionalTime.charAt(i)))
		  {
			tempVal = tempVal +  Character.getNumericValue(isoFractionalTime.charAt(i)) * placeValue;
			placeValue = placeValue / 10;
		  }
		}
		ret = microsecond(tempVal);
		if (ret != CodecReturnCodes.SUCCESS)
	        return ret;   		
 
		tempVal = 0;
		placeValue = 100;
		for (;  i < len &&  i < 9; i++)
		{
		  if(Character.isDigit(isoFractionalTime.charAt(i)))
		  {
			tempVal = tempVal +  Character.getNumericValue(isoFractionalTime.charAt(i)) * placeValue;
			placeValue = placeValue / 10;
		  }
		}
		ret = nanosecond(tempVal);
		if (ret != CodecReturnCodes.SUCCESS)
	        return ret;   		 
   	
    	return CodecReturnCodes.SUCCESS;
    }

    @Deprecated
    public int valueOld(String value)
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
        	// Trim trailing non digits
        	String tempVal = trimmedVal;
        	int len = trimmedVal.length();
        	int i = 0;
        	for (;  i < len; ++i)
        	{
        		if(Character.isDigit(tempVal.charAt(i)) == false)
        		{
        			if(tempVal.charAt(i) != ':' && tempVal.charAt(i) != '.'  && tempVal.charAt(i) != ','  && tempVal.charAt(i) != ' ')
        				break;
        		}
        	}
        	if(i > 0)
        		trimmedVal = tempVal.substring(0, i);
        	else
        		return CodecReturnCodes.INVALID_ARGUMENT;

        	// ISO8601 hh:mm:ss.nnnnnnnnn e.g. 08:37:48.009216350 
        	matcher = TIME_PATTERN_9.matcher(trimmedVal);
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
                
        		return iso8601FractionalStingTimeToTime(matcher.group(4));
        	}
        	
        	// ISO8601 hh:mm:ss.nnnnnnnnn e.g. 08:37:48,009216350
        	matcher = TIME_PATTERN_10.matcher(trimmedVal);
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
                
        		return iso8601FractionalStingTimeToTime(matcher.group(4));
        	}        	
            // hh:mm:ss:lll:uuu:nnn
            matcher = TIME_PATTERN_7.matcher(trimmedVal);
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
            matcher = TIME_PATTERN_8.matcher(trimmedVal);
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
            matcher = TIME_PATTERN_5.matcher(trimmedVal);
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
            matcher = TIME_PATTERN_6.matcher(trimmedVal);
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

            matcher = TIME_PATTERN_1.matcher(trimmedVal);
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

            matcher = TIME_PATTERN_2.matcher(trimmedVal);
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

            matcher = TIME_PATTERN_3.matcher(trimmedVal);
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

            matcher = TIME_PATTERN_4.matcher(trimmedVal);
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
    public int value(String data) {
        int tmpInt = 0;
        int min = 0;
        int sec = 0;
        int hour = 0;
        int milli = 0;
        int micro = 0;
        int nano = 0;
        int numberCount = 0;
        int position;
        int end;
        int placeValue = 0;
        int i;

        if (data == null || data.isEmpty()) {
            this._hour = 255;
            this._minute = 255;
            this._second = 255;
            this._millisecond = 65535;
            this._microsecond = 2047;
            this._nanosecond = 2047;

            return CodecReturnCodes.BLANK_DATA;
        }

        position = 0;
        end = data.length();

        clear();

        /* Check for ISO case of hh:mm:ss first*/
        while (position < end && Character.isDigit(data.charAt(position))) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
            ++numberCount;
            ++position;
        }

	    /** If there are 2 characters in the first number, this is minimally hh:mm.
            It can also be:
                hh:mm:ss
                hh:mm:ss:mmm
                hh:mm:ss:mmm:mmm
                hh:mm:ss:mmm:mmm:nnn
                hh:mm:ss.nnnnnnnnn
                hh:mm:ss,nnnnnnnnn
         */
        if (numberCount == 2 && data.charAt(position) == ':') {
            hour = tmpInt;
            tmpInt = 0;
            numberCount = 0;

            position++;
            if (position >= end) {
                return CodecReturnCodes.INVALID_DATA;
            }

            while (position < end && Character.isDigit(data.charAt(position))) {
                tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
                ++numberCount;
                ++position;
            }
            min = tmpInt;

            if (numberCount != 2) {
                return CodecReturnCodes.INVALID_DATA;
            }

            /* Check to see if we're at the end or if there's a time zone(which will be dropped) */
            if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                this._hour = hour;
                this._minute = min;
                return CodecReturnCodes.SUCCESS;
            }

            if (data.charAt(position) != ':') {
                return CodecReturnCodes.INVALID_DATA;
            }

            position++;
            tmpInt = 0;
            numberCount = 0;

            while (position < end && Character.isDigit(data.charAt(position))) {
                tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
                ++numberCount;
                ++position;
            }
            sec = tmpInt;

            if (numberCount != 2) {
                return CodecReturnCodes.INVALID_DATA;
            }

            /* Check to see if we're at the end or if there's a time zone(which will be dropped) */
            if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                this._hour = hour;
                this._minute = min;
                this._second = sec;
                return CodecReturnCodes.SUCCESS;
            }

            tmpInt = 0;
            numberCount = 0;
            if (data.charAt(position) == ':') {
                ++position;

                while (position < end && Character.isDigit(data.charAt(position))) {
                    tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
                    ++numberCount;
                    ++position;
                }

                milli = tmpInt;

                if (numberCount != 3) {
                    return CodecReturnCodes.INVALID_DATA;
                }

                if (position >= end) {
                    this._hour = hour;
                    this._minute = min;
                    this._second = sec;
                    this._millisecond = milli;
                    return CodecReturnCodes.SUCCESS;
                }

                tmpInt = 0;
                numberCount = 0;
                if (data.charAt(position) != ':') {
                    return CodecReturnCodes.INVALID_DATA;
                }

                position++;
                while (position < end && Character.isDigit(data.charAt(position))) {
                    tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
                    ++numberCount;
                    ++position;
                }
                micro = tmpInt;

                if (numberCount != 3) {
                    return CodecReturnCodes.INVALID_DATA;
                }

                if (position >= end) {
                    this._hour = hour;
                    this._minute = min;
                    this._second = sec;
                    this._millisecond = milli;
                    this._microsecond = micro;
                    return CodecReturnCodes.SUCCESS;
                }

                tmpInt = 0;
                numberCount = 0;
                if (data.charAt(position) != ':') {
                    return CodecReturnCodes.INVALID_DATA;
                }

                position++;
                while (position < end && Character.isDigit(data.charAt(position))) {
                    tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
                    ++numberCount;
                    ++position;
                }

                nano = tmpInt;

                if (numberCount != 3) {
                    return CodecReturnCodes.INVALID_DATA;
                } else {
                    this._hour = hour;
                    this._minute = min;
                    this._second = sec;
                    this._millisecond = milli;
                    this._microsecond = micro;
                    this._nanosecond = nano;
                    return CodecReturnCodes.SUCCESS;
                }
            } else if (data.charAt(position) == '.' || data.charAt(position) == ',') {
                ++position;
                if (position == end) {
                    return CodecReturnCodes.INVALID_DATA;
                }

                this._hour = hour;
                this._minute = min;
                this._second = sec;

                placeValue = 100;
                for (i = 0; i < 3; ++i) {
                    if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                        return CodecReturnCodes.SUCCESS;
                    } else if (Character.isDigit(data.charAt(position))) {
                        this._millisecond = this._millisecond + Character.getNumericValue(data.charAt(position)) * placeValue;
                        placeValue = placeValue / 10;
                    } else {
                        clear();
                        return CodecReturnCodes.INVALID_DATA;
                    }
                    position++;
                }

                placeValue = 100;
                for (i = 0; i < 3; ++i) {
                    if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                        return CodecReturnCodes.SUCCESS;
                    } else if (Character.isDigit(data.charAt(position))) {
                        this._microsecond = this._microsecond + Character.getNumericValue(data.charAt(position)) * placeValue;
                        placeValue = placeValue / 10;
                    } else {
                        clear();
                        return CodecReturnCodes.INVALID_DATA;
                    }
                    position++;
                }

                placeValue = 100;
                for (i = 0; i < 3; ++i) {
                    if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                        return CodecReturnCodes.SUCCESS;
                    } else if (Character.isDigit(data.charAt(position))) {
                        this._nanosecond = this._nanosecond + Character.getNumericValue(data.charAt(position)) * placeValue;
                        placeValue = placeValue / 10;
                    } else {
                        clear();
                        return CodecReturnCodes.INVALID_DATA;
                    }
                    position++;
                }

                /* We've parsed all available data, and are ignoring any time zone specification */
                return CodecReturnCodes.SUCCESS;
            } else {
                return CodecReturnCodes.INVALID_DATA;
            }
        } else if (numberCount == 6 || numberCount == 4) {
            /**
             *  This covers:
             *  hhmm
             *  hhmmss
             *  hhmmss.nnnnnnnnn
             *  hhmmss,nnnnnnnnn
             */

            /* reset tmp to the beginning */
            position = 0;
            /* Since we've verified that this is exactly 6 digits, we can just parse hhmmss */
            for (i = 0; i < 2; ++i) {
                this._hour = this._hour * 10 + Character.getNumericValue(data.charAt(position));
                position++;
            }

            for (i = 0; i < 2; ++i) {
                this._minute = this._minute * 10 + Character.getNumericValue(data.charAt(position));
                position++;
            }

            if (numberCount == 6) {
                for (i = 0; i < 2; ++i) {
                    this._second = this._second * 10 + Character.getNumericValue(data.charAt(position));
                    position++;
                }

                /* Ignoring ISO time zone formatting */
                if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                    return CodecReturnCodes.SUCCESS;
                } else if (data.charAt(position) == '.' || data.charAt(position) == ',') {
                    ++position;
                    if (position == end) {
                        return CodecReturnCodes.INVALID_DATA;
                    }

                    placeValue = 100;
                    for (i = 0; i < 3; ++i) {
                        if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                            return CodecReturnCodes.SUCCESS;
                        } else if (Character.isDigit(position)) {
                            this._millisecond = this._millisecond + Character.getNumericValue(data.charAt(position)) * placeValue;
                            placeValue = placeValue / 10;
                        } else {
                            clear();
                            return CodecReturnCodes.INVALID_DATA;
                        }
                        position++;
                    }

                    placeValue = 100;
                    for (i = 0; i < 3; ++i) {
                        if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                            return CodecReturnCodes.SUCCESS;
                        } else if (Character.isDigit(data.charAt(position))) {
                            this._microsecond = this._microsecond + Character.getNumericValue(data.charAt(position)) * placeValue;
                            placeValue = placeValue / 10;
                        } else {
                            clear();
                            return CodecReturnCodes.INVALID_DATA;
                        }
                        position++;
                    }

                    placeValue = 100;
                    for (i = 0; i < 3; ++i) {
                        if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                            return CodecReturnCodes.SUCCESS;
                        } else if (Character.isDigit(data.charAt(position))) {
                            this._nanosecond = this._nanosecond + Character.getNumericValue(data.charAt(position)) * placeValue;
                            placeValue = placeValue / 10;
                        } else {
                            clear();
                            return CodecReturnCodes.INVALID_DATA;
                        }
                        position++;
                    }

                    /* We've parsed all available data, and are ignoring any time zone specification */
                    return CodecReturnCodes.SUCCESS;
                } else {
                    return CodecReturnCodes.INVALID_DATA;
                }
            } else if (position >= end || data.charAt(position) == 'Z' || data.charAt(position) == '+' || data.charAt(position) == '-') {
                return CodecReturnCodes.SUCCESS;
            } else {
                return CodecReturnCodes.INVALID_DATA;
            }
        }

        /* Catchall for remaining alternate formats */
        position = 0;

        while (position < end && Character.isSpaceChar(data.charAt(position))) {
            position++; /* skip whitespace */
        }

        /* if all whitespaces init to blank */
        if (position >= end) {
            this._hour = 255;
            this._minute = 255;
            this._second = 255;
            this._millisecond = 65535;
            this._microsecond = 2047;
            this._nanosecond = 2047;
            return CodecReturnCodes.BLANK_DATA;
        }

        for (tmpInt = 0; position < end && Character.isDigit(data.charAt(position)); position++) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
        }
        this._hour = tmpInt;

        while (position < end && ((data.charAt(position) == ':') || Character.isSpaceChar(data.charAt(position)))) {
            position++;
        }

        if (position >= end || !Character.isDigit(data.charAt(position))) {
            return CodecReturnCodes.INVALID_DATA;
        }

        for (tmpInt = 0; position < end && Character.isDigit(data.charAt(position)); position++) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
            this._minute = tmpInt;
        }

        while (position < end && (data.charAt(position) == ':' || Character.isSpaceChar(data.charAt(position)))) {
            position++;
        }

        for (tmpInt = 0; position < end && Character.isDigit(data.charAt(position)); position++) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
            this._second = tmpInt;
        }

        while (position < end && (data.charAt(position) == ':' || Character.isSpaceChar(data.charAt(position)))) {
            position++;
        }

        for (tmpInt = 0; position < end && Character.isDigit(data.charAt(position)); position++) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
            this._millisecond = tmpInt;
        }

        while (position < end && (data.charAt(position) == ':' || Character.isSpaceChar(data.charAt(position)))) {
            position++;
        }

        for (tmpInt = 0; position < end && Character.isDigit(data.charAt(position)); position++) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
            this._microsecond = tmpInt;
        }

        while (position < end && (data.charAt(position) == ':' || Character.isSpaceChar(data.charAt(position)))) {
            position++;
        }

        for (tmpInt = 0; position < end && Character.isDigit(data.charAt(position)); position++) {
            tmpInt = tmpInt * 10 + Character.getNumericValue(data.charAt(position));
            this._nanosecond = tmpInt;
        }
        return CodecReturnCodes.SUCCESS;
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
                (_nanosecond == ((TimeImpl)thatTime)._nanosecond) &&
                (_format == ((TimeImpl)thatTime)._format));
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
