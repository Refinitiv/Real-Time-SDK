package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementSetDef;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GlobalElementSetDefDb;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.transport.Error;


class GlobalElementSetDefDbImpl extends ElementSetDefDbImpl implements GlobalElementSetDefDb
{
   


	GlobalElementSetDefDbImpl() 
	{		
		super(0);
	}

	GlobalElementSetDefDbImpl(int maxID) 
	{		
		super(0);
	}


	@Override
    public int decode(DecodeIterator iter, int verbosity, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
  
    
    @Override
    public int encode(EncodeIterator iter, Int currentSetDef, int verbosity, Error error)
    {
    	return CodecReturnCodes.FAILURE;
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
        
    @Override
    public Buffer info_version()
    {
        return null;
    }
    
    @Override
    public void info_version(Buffer setInfo_version)
    {
        
    }
    
    @Override
    public int info_DictionaryID() 
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public void info_DictionaryID(int setInfo_DictionaryID) 
    {
        
    }



	@Override
	public int addSetDef(ElementSetDef setDef, Error error) 
	{
		return CodecReturnCodes.FAILURE;
	}
}
