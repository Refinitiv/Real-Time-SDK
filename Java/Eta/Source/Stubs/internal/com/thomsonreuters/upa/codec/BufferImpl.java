package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;

class BufferImpl implements Buffer
{

    @Override
    public int data(ByteBuffer data)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int data(ByteBuffer data, int position, int length)
    {
    	return CodecReturnCodes.FAILURE;
    }

  
    @Override
    public int copy(ByteBuffer destBuffer)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int copy(byte[] destBuffer)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int copy(byte[] destBuffer, int destOffset)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int copy(Buffer destBuffer)
    {
    	return CodecReturnCodes.FAILURE;
    }

  
    @Override
    public int length()
    {
    	return -1;
    }

    @Override
    public int position()
    {
        return -1;
    }

    @Override
    public String toString()
    {
       return null;
    }

    @Override
    public int data(String str)
    {
    	return CodecReturnCodes.FAILURE;
    }

  
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
		return true;
	}
    
  

    @Override
    public ByteBuffer data()
    {
       return null;
    }

    String dataString()
    {
        return null;
    }

  
    @Override
    public boolean equals(Buffer buffer)
    {
       return false;
    }
    
    @Override
    public String toHexString()
    {
       return null;
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
    public int capacity()
    {
        return -1;
    }
}
