package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;

class EnumImpl implements Enum
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
	public int toInt()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
	public int value(int value)
    {
    	return CodecReturnCodes.FAILURE;
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
	public boolean equals(Enum thatEnum) 
	{
		return false;
	}

	@Override
	public int copy(Enum destEnum) 
	{
		return CodecReturnCodes.FAILURE;
	}
}
