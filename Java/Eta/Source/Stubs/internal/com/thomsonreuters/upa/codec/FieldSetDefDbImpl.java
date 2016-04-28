package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.FieldSetDef;
import com.thomsonreuters.upa.codec.FieldSetDefDb;

class FieldSetDefDbImpl implements FieldSetDefDb
{
  
	FieldSetDefDbImpl(int maxID)
	{
		
	}
	
    @Override
    public void clear()
    {
      
    }
    
    @Override
    public FieldSetDef[] definitions() 
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
