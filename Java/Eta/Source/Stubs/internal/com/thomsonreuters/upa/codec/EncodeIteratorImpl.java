package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GlobalElementSetDefDb;
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.transport.TransportBuffer;

class EncodeIteratorImpl implements EncodeIterator
{
  
	
	@Override
	public void clear()
	{
		
	}
	
	@Override
	public int setBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion)
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
    public int setBufferAndRWFVersion(TransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion)
    {
		return CodecReturnCodes.FAILURE;
    }
	
  
	@Override
	public int realignBuffer(TransportBuffer newEncodeBuffer) 
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public int realignBuffer(Buffer newEncodeBuffer) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeNonRWFInit(Buffer buffer)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeNonRWFComplete(Buffer buffer, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}
	
	boolean isIteratorOverrun(int length)
	{
		return false;
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
	public int replaceStreamId(int streamId) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int replaceSeqNum(long seqNum) 
	{
		return CodecReturnCodes.FAILURE;
	}
	
	
	@Override
	public int replaceStreamState(int streamState) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int replaceDataState(int dataState) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int replaceStateCode(int stateCode) 
	{
		return CodecReturnCodes.FAILURE;
	}

	

	@Override
	public int replaceGroupId(Buffer groupId) 
	{
		return CodecReturnCodes.FAILURE;
	}

	
	@Override
	public int replacePostId(int postId) 
	{
		return CodecReturnCodes.FAILURE;
	}


	@Override
    public int setSolicitedFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int unsetSolicitedFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int setRefreshCompleteFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int unsetRefreshCompleteFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int setStreamingFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int unsetStreamingFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int setNoRefreshFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int unsetNoRefreshFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int setMsgKeyInUpdatesFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int unsetMsgKeyInUpdatesFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int setConfInfoInUpdatesFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
    public int unsetConfInfoInUpdatesFlag()
    {
		return CodecReturnCodes.FAILURE;
    }

	@Override
	public Buffer buffer()
	{
		return null;
	}
	
	@Override
	public TransportBuffer transportBuffer()
	{
		return null;
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
}
