package com.thomsonreuters.upa.valueadd.cache;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.valueadd.cache.CacheError;


class CacheErrorImpl implements CacheError
{
	
	
    @Override
    public int errorId()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void errorId(int errorId)
    {
    	
    }
    
    @Override
    public String text()
    {
    	return null;
    }

    @Override
    public void text(String text)
    {
    	
    }

    @Override
    public void clear()
    {
    	
    }
    
    public String toString()
	{
		return null;
	}
}