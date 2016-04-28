package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Real;


class RealImpl implements Real
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
	public int value(double value, int hint)
    {
    	return CodecReturnCodes.FAILURE;
    }

	@Override
	public int value(float value, int hint)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int value(long value, int hint)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public double toDouble()
    {
      return 0;
    }
    
	@Override
	public long toLong()
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public int hint()
	{
		return CodecReturnCodes.FAILURE;
	}

    @Override
	public boolean isBlank()
    {
    	return false;
    }
    
    @Override
    public boolean equals(Real thatReal)
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
	public int copy(Real destReal) 
	{
		return CodecReturnCodes.FAILURE;
	}

  
}
