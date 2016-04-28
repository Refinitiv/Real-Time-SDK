package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementSetDefEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.LocalElementSetDefDb;

class LocalElementSetDefDbImpl extends ElementSetDefDbImpl implements LocalElementSetDefDb
{
	LocalElementSetDefDbImpl() 
	{
		super(0);
		
	}
	
	LocalElementSetDefDbImpl(int maxID) 
	{
		super(maxID);
		
	}

	@Override
	public void clear()
	{
		
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
	public ElementSetDefEntry[][] entries() 
	{
		return null;
	}
}
