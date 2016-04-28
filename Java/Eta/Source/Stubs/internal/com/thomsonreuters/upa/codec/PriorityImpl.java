package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.Priority;

class PriorityImpl implements Priority
{

		
	@Override
	public void clear()
	{
		
	}

	@Override
	public void priorityClass(int priorityClass)
	{
        
	}

	@Override
	public int priorityClass()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void count(int count)
	{
       
	}

	@Override
	public int count()
	{
		return CodecReturnCodes.FAILURE;
	}
}
