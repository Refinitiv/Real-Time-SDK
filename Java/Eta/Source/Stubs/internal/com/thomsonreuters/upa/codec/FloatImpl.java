package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Real;

class FloatImpl implements Float
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
	public boolean equals(Float thatFloat)
	{
       return false;
	}

	@Override
	public float toFloat()
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
	public void value(float value)
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
	public void blank() 
	{
				
	}

	@Override
	public int copy(Float destFloat) 
	{
		return CodecReturnCodes.FAILURE;
	}
}
