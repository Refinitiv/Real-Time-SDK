package com.thomsonreuters.upa.codec;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.RmtesCacheBuffer;

class RmtesCacheBufferImpl implements RmtesCacheBuffer
{
  
	RmtesCacheBufferImpl(int x)
	{
		
	}
	
    RmtesCacheBufferImpl(int dataLength, ByteBuffer byteData, int allocLength)
    {

    }

    @Override
    public int length()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int allocatedLength()
    {
    	return CodecReturnCodes.FAILURE;
    }

    /* Deprecated to leverage ByteBuffer instead of CharBuffer */
    @Deprecated
    @Override
    public CharBuffer data()
    {
        return null;
    }

    @Override
    public void length(int x)
    {
       
    }

    @Override
    public void allocatedLength(int x)
    {
       
    }

    /* Deprecated to leverage ByteBuffer instead of CharBuffer */
    @Deprecated
    @Override
    public void data(CharBuffer x)
    {
        
    }

	@Override
	public void clear()
	{
				
	}
	
    @Override
    public ByteBuffer byteData()
    {
        return null;
    }

    @Override
    public void data(ByteBuffer x)
    {

    }

}
