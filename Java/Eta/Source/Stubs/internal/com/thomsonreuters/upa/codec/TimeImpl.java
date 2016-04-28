package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Time;

class TimeImpl implements Time
{
  
    
	@Override
	public void clear()
	{
	
	}

	@Override
	public boolean isBlank()
	{
		return false;
	}

	@Override
	public boolean isValid()
	{
		return false;
	}
    
	@Override
    public int copy(Time destTime)
    {
		return CodecReturnCodes.FAILURE;
    }
	
	@Override
	public String toString()
	{
		return null;
	}
	
    @Override
    public int value(String value)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
	@Override
	public void blank()
	{
		
	}

    @Override
    public boolean equals(Time thatTime)
    {
    	return false;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int decode(DecodeIterator iter) 
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
}
