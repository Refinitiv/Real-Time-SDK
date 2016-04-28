package com.thomsonreuters.upa.codec;

import java.math.BigInteger;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.UInt;

class UIntImpl implements UInt
{
   



    @Override
    public void clear()
    {
    	
    }

	   
    
	void blank()
	{
		
	}

	@Override
	public boolean isBlank()
	{
		return false;
	}
    
	@Override
    public long toLong()
    {
        return CodecReturnCodes.FAILURE;
    }
           
    @Override
	public BigInteger toBigInteger()
    {
    	return null;
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
    public int value(BigInteger value)
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
	public boolean equals(UInt thatUInt) 
	{
		return false;
	}



	@Override
	public int copy(UInt destUInt) 
	{
		return CodecReturnCodes.FAILURE;
	}
}
