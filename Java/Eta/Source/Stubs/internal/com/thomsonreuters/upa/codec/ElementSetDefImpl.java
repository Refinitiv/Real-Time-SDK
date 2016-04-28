package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.ElementSetDef;
import com.thomsonreuters.upa.codec.ElementSetDefEntry;

class ElementSetDefImpl implements ElementSetDef
{
		
	@Override
	public void clear()
	{
		}

	@Override
	public int setId() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void setId(int setId) 
	{
	    
	}

	@Override
	public int count() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void count(int count) 
	{
	   
	}

	@Override
	public ElementSetDefEntry[] entries() 
	{
		return null;
	}

	@Override
	public void entries(ElementSetDefEntry entries[]) 
	{
	    
	}
	
	@Override
    public void copy(ElementSetDef elementSetDef) 
	{
        
    }
}
