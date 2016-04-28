package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;

class DateImpl implements Date
{

	@Override
	public void clear()
	{
	
	}
	
	
	@Override
	public int copy(Date destDate)
	{
		return CodecReturnCodes.FAILURE;
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
	public boolean equals(Date thatDate)
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
	public boolean isValid() 
	{
		return false;
	}
}
