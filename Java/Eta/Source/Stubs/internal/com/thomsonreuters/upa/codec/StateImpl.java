package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.State;


class StateImpl implements State
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
	public int copy(State destState)
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
	public boolean isFinal()
	{
		return false;
	}

	@Override
	public int streamState(int streamState)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int streamState()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int dataState(int dataState)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int dataState()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int code(int code)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int code()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int text(Buffer text)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public Buffer text()
	{
		return null;
	}

	@Override
	public boolean equals(State thatState)
	{
		return false;
	}


}
