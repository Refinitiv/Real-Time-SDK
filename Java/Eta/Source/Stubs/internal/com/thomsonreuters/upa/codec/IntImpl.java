package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;

class IntImpl implements Int
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
    public void value(int value)
    {
     
    }
    
	@Override
	public void value(long value)
	{
		
	}
	
    @Override
	public long toLong()
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
	public boolean equals(Int thatInt) 
	{
		return false;
	}


	@Override
	public int copy(Int destInt) 
	{
		return CodecReturnCodes.FAILURE;
	}
}
