package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.FieldSetDefEntry;

class FieldSetDefEntryImpl implements FieldSetDefEntry
{

	
	@Override
	public void clear()
	{

	}

	@Override
	public int fieldId() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void fieldId(int fieldId) 
	{
	   
	}

	@Override
	public int dataType() 
	{
		return DataTypes.UNKNOWN;
	}

	@Override
	public void dataType(int dataType) 
	{
	   
	}
}
