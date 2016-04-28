package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldSetDefEntry;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;

class LocalFieldSetDefDbImpl extends FieldSetDefDbImpl implements LocalFieldSetDefDb
{
  
    LocalFieldSetDefDbImpl() 
    {
		super(0);
		
	}
		
    LocalFieldSetDefDbImpl(int maxID) 
    {
		super(maxID);
		
	}

	@Override
    public int decode(DecodeIterator iter)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encode(EncodeIterator iter)
	{
    	return CodecReturnCodes.FAILURE;
	}
    
    @Override
    public FieldSetDefEntry[][] entries()
    {
        return null;
    }

}
