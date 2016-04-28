package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Time;


class DateTimeImpl implements DateTime
{
   
    @Override
    public void clear()
    {
     
    }

    @Override
    public void blank()
    {
     
    }

    @Override
    public boolean isBlank()
    {
        return false;
    }

    @Override
    public boolean equals(DateTime thatDateTime)
    {
        return false;
    }

    @Override
    public boolean isValid()
    {
        return false;
    }

    @Override
    public int decode(DecodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
     
    @Override
    public int value(String value)
    {
    	return CodecReturnCodes.FAILURE;
    }

 
    @Override
    public String toString()
    {
       return null;

    }

    @Override
    public Date date()
    {
    	return null;
    }

    @Override
    public Time time()
    {
       return null;
    }

    @Override
    public void localTime()
    {
        
    }

  
    @Override
    public void gmtTime()
    {
        
    }

    @Override
    public int day(int day)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int day()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int month(int month)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int month()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int year(int year)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int year()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int hour(int hour)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int hour()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int minute(int minute)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int minute()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int second(int second)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int second()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int millisecond(int millisecond)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int millisecond()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int microsecond(int microsecond)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int microsecond()
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int nanosecond(int nanosecond)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int nanosecond()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int value(long value)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public long millisSinceEpoch()
    {
    	return CodecReturnCodes.FAILURE;
    }

	@Override
	public int copy(DateTime destDateTime) {
		return CodecReturnCodes.FAILURE;
	}

   	
}
