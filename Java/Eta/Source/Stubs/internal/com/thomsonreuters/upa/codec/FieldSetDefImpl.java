package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.FieldSetDef;
import com.thomsonreuters.upa.codec.FieldSetDefEntry;

class FieldSetDefImpl implements FieldSetDef
{
  
    @Override
    public void clear()
    {
      
    }

    @Override
    public long setId() 
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void setId(long setId) 
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
    public FieldSetDefEntry[] entries() 
    {
        return null;
    }

    @Override
    public void entries(FieldSetDefEntry[] entries) 
    {
    	
    }

    @Override
    public void copy(FieldSetDef destSetDef) {
      
    }
    
   
}
