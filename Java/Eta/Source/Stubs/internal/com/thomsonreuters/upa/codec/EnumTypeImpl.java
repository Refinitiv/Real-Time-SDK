package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.EnumType;

class EnumTypeImpl implements EnumType
{
	
	@Override
	public int value() 
	{
		return CodecReturnCodes.FAILURE;
	}
	@Override
	public Buffer display() 
	{
		return null;
	}

	@Override
	public Buffer meaning() 
	{
		return null;
	}

}
