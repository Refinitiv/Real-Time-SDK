package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.EnumType;
import com.thomsonreuters.upa.codec.EnumTypeTable;

class EnumTypeTableImpl implements EnumTypeTable
{
		
	@Override
	public int maxValue() 
	{
		return CodecReturnCodes.FAILURE;
	}
	
	
	@Override
	public EnumType[] enumTypes() {
		return null;
	}
	
	@Override
	public int fidReferenceCount() 
	{
		return CodecReturnCodes.FAILURE;
	}
	

	@Override
	public int[] fidReferences() 
	{
		return null;
	}
	

}
