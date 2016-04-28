package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EnumTypeTable;

class DictionaryEntryImpl implements DictionaryEntry
{
		
	@Override
	public Buffer acronym() 
	{
		return null;
	}

	@Override
	public Buffer ddeAcronym() 
	{
		return null;
	}

	@Override
	public int fid() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rippleToField() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int fieldType() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int length() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int enumLength() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rwfType() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int rwfLength() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public EnumTypeTable enumTypeTable() 
	{
		return null;
	}
}
