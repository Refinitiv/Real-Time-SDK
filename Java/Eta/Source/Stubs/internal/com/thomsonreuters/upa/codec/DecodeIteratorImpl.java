package com.thomsonreuters.upa.codec;


import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.GlobalElementSetDefDb;
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.transport.TransportBuffer;


class DecodeIteratorImpl implements DecodeIterator
{

	@Override
	public void clear() 
	{
				
	}

	@Override
	public int setBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion,int rwfMinorVersion) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int setBufferAndRWFVersion(TransportBuffer buffer,int rwfMajorVersion, int rwfMinorVersion) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int setGlobalFieldSetDefDb(GlobalFieldSetDefDb setDefDb) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int setGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int finishDecodeEntries() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int majorVersion() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int minorVersion() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int extractMsgClass() 
	{		
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int extractDomainType() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int extractStreamId() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int extractSeqNum() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int extractGroupId(Buffer groupId) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int extractPostId() 
	{
		return CodecReturnCodes.FAILURE;
	}
}
