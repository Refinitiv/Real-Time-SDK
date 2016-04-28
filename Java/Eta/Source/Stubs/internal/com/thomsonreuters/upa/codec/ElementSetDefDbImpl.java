package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.ElementSetDef;
import com.thomsonreuters.upa.codec.ElementSetDefDb;

class ElementSetDefDbImpl implements ElementSetDefDb
{
  
	ElementSetDefDbImpl(int maxID)
	{
		
	}
	
    @Override
    public void clear()
    {
        
    }

    @Override
    public ElementSetDef[] definitions() 
    {
        return null;
    }

    @Override
    public int maxSetId() 
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void maxSetId(int setMaxSetId) 
    {
        
    }
    
}
