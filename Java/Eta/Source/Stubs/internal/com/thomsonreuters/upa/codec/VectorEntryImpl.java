package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.VectorEntry;

class VectorEntryImpl implements VectorEntry
{
	
    @Override
    public void clear()
    {
      
    }

    @Override
    public int encode(EncodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encodeInit(EncodeIterator iter, int maxEncodingSize)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int decode(DecodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public boolean checkHasPermData()
    {
        return false;
    }

    @Override
    public void applyHasPermData()
    {
        
    }

    @Override
    public int flags() 
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void flags(int flags) 
    {
       
    }

    @Override
    public int action() 
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void action(int action) 
    {
       
    }

    @Override
    public long index()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void index(long index)
    {
        
    }

    @Override
    public Buffer permData() 
    {
        return null;
    }

    @Override
    public void permData(Buffer permData) 
    {
    	
    }

    @Override
    public Buffer encodedData() 
    {
        return null;
    }

    @Override
    public void encodedData(Buffer encodedData) 
    {
        
    }
}
