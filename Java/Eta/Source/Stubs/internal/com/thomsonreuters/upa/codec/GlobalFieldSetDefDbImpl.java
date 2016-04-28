package com.thomsonreuters.upa.codec;


import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldSetDef;
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.transport.Error;


class GlobalFieldSetDefDbImpl extends FieldSetDefDbImpl implements GlobalFieldSetDefDb
{
  
    GlobalFieldSetDefDbImpl() 
    {
		super(0);		
	}
	
    GlobalFieldSetDefDbImpl(int maxID) 
    {
		super(maxID);		
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
	public int addSetDef(FieldSetDef setDef, Error error) 
	{
		return CodecReturnCodes.FAILURE;
	}

}
