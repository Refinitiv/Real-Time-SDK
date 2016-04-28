package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Double;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Real;

class DoubleImpl implements Double
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
	public boolean equals(Double thatDouble)
	{
        return false;
	}

	@Override
	public double toDouble()
	{
        return 0;
	}

	@Override
	public Real toReal(int hint)
	{
        return null;
	}

    @Override
    public String toString()
    {
    	return null;
    }

    @Override
	public void value(double value)
	{
		
	}

	@Override
	public int value(String value)
	{
		return CodecReturnCodes.FAILURE;
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
	public int copy(Double destDouble) 
	{
		return CodecReturnCodes.FAILURE;
	}
}
