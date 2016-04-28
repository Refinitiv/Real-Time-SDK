package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Qos;

class QosImpl implements Qos
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
	public boolean equals(Qos thatQos)
	{
		return false;
	}

	@Override
	public boolean isBetter(Qos thatQos)
	{
        return false;
	}

	@Override
	public  boolean isInRange(Qos bestQos, Qos worstQos)
	{
       return false;
	}

	@Override
	public int timeliness(int timeliness)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int timeliness()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rate(int rate)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rate()
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public void dynamic(boolean dynamic)
	{
		
	}

	@Override
	public boolean isDynamic()
	{
		return false;
	}

	@Override
	public int timeInfo(int timeInfo)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int timeInfo()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rateInfo(int rateInfo)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rateInfo()
	{
		return CodecReturnCodes.FAILURE;
	}



	@Override
	public int copy(Qos destQos) 
	{
		return CodecReturnCodes.FAILURE;
	}
	
	
}
