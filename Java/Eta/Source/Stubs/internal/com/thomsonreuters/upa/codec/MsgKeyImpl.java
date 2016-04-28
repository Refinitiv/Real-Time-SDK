package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.MsgKey;

class MsgKeyImpl implements MsgKey
{
	
	@Override
    public void clear()
    {
     
    }
    
   

	@Override
    public boolean checkHasServiceId()
	{
		return false;
	}

	@Override
    public boolean checkHasName()
	{
		return false;
	}

	@Override
    public boolean checkHasNameType()
	{
		return false;
	}

	@Override
    public boolean checkHasFilter()
	{
		return false;
	}

	@Override
    public boolean checkHasIdentifier()
	{
		return false;
	}

	@Override
    public boolean checkHasAttrib()
	{
		return false;
	}

	@Override
    public void applyHasServiceId()
	{
		
	}

	@Override
    public void applyHasName()
	{
		
	}

	@Override
    public void applyHasNameType()
	{
		
	}

	@Override
    public void applyHasFilter()
	{
		
	}

	@Override
    public void applyHasIdentifier()
	{
		
	}

	@Override
    public void applyHasAttrib()
	{
		
	}
	
	@Override
    public void flags(int flags)
	{
	    
	}

    @Override
    public int flags()
	{
    	return CodecReturnCodes.FAILURE;
	}

	@Override
    public void serviceId(int serviceId)
	{
	    
	}

	@Override
    public int serviceId()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void nameType(int nameType)
	{
	  
	}

	@Override
    public int nameType()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void name(Buffer name)
	{
	  
	}

	@Override
    public Buffer name()
	{
		return null;
	}

	@Override
    public void filter(long filter)
	{
	    
	}

	@Override
    public long filter()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void identifier(int identifier)
	{
	    
	}

	@Override
    public int identifier()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void attribContainerType(int attribContainerType)
	{
	    
	}

	@Override
    public int attribContainerType()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
    public void encodedAttrib(Buffer encodedAttrib)
	{
	    
	}

	@Override
    public Buffer encodedAttrib()
	{
		return null;
	}
    
    @Override
    public boolean equals(MsgKey thatKey)
    {
    	return false;
    }
    
   
    @Override
    public int copy(MsgKey destKey)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int addFilterId(int filterId)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public boolean checkHasFilterId(int filterId)
    {
    	return false;
    }
}
