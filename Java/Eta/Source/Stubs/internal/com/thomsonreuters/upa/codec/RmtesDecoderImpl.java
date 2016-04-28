package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.RmtesBuffer;
import com.thomsonreuters.upa.codec.RmtesCacheBuffer;
import com.thomsonreuters.upa.codec.RmtesDecoder;

class RmtesDecoderImpl implements RmtesDecoder
{

	@Override
	public int RMTESToUCS2(RmtesBuffer rmtesBuffer, RmtesCacheBuffer cacheBuffer) 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public boolean hasPartialRMTESUpdate(Buffer inBuffer)
	{
		return false;
	}

	@Override
	public int RMTESApplyToCache(Buffer inBuffer, RmtesCacheBuffer cacheBuffer) 
	{
		return CodecReturnCodes.FAILURE;
	}

  }
