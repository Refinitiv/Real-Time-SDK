///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.RmtesCacheBuffer;
import com.rtsdk.eta.codec.RmtesDecoder;

class RmtesBufferImpl implements RmtesBuffer
{
	private static final int RMTES_DECODE_BUFFER_INIT_SIZE = 256;
	
	private com.rtsdk.eta.codec.Buffer _rsslBuffer = CodecFactory.createBuffer();
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private RmtesDecoder _rsslDecoder = CodecFactory.createRmtesDecoder();
	private RmtesCacheBuffer _rsslRmtesCacheBuffer = CodecFactory.createRmtesCacheBuffer(0, null, 0);
	private com.rtsdk.eta.codec.RmtesBuffer _rsslRmtesBuffer = CodecFactory.createRmtesBuffer(0, null, 0);
	private boolean _decodedUTF16BufferSet;
	private boolean _decodedUTF8BufferSet;
	private boolean _toStringSet;
	private ByteBuffer _decodedUTF8ByteBuffer;
	private boolean _applyToCache;
	private String _toString;
	
	public RmtesBufferImpl() 
	{
		_rsslRmtesCacheBuffer.data(ByteBuffer.allocate(RMTES_DECODE_BUFFER_INIT_SIZE));
		_rsslRmtesCacheBuffer.allocatedLength(RMTES_DECODE_BUFFER_INIT_SIZE);
		
		_rsslRmtesBuffer.data(ByteBuffer.allocate(RMTES_DECODE_BUFFER_INIT_SIZE *2));
		_rsslRmtesBuffer.allocatedLength(RMTES_DECODE_BUFFER_INIT_SIZE *2);
	}

	@Override
	public ByteBuffer asUTF8()
	{
		if (!_decodedUTF8BufferSet)
		{
			toString();
			_decodedUTF8ByteBuffer = ByteBuffer.wrap(_toString.getBytes());
			_decodedUTF8BufferSet = true;
		}
		
		return _decodedUTF8ByteBuffer;
	}

	@Override
	public CharBuffer asUTF16()
	{
		if (!_decodedUTF16BufferSet)
		{
			decode();
			 _decodedUTF16BufferSet = true;
		}
		
		return _rsslRmtesBuffer.byteData().asCharBuffer();
	}

	@Override
	public String toString()
	{
		if (!_toStringSet)
		{
			decode();
			
			if (_rsslRmtesBuffer.length() == 0)
				_toString = DataImpl.EMPTY_STRING;
			else
				_toString =  new String ( _rsslRmtesBuffer.byteData().array(), 0,  _rsslRmtesBuffer.length(), Charset.forName("UTF-16BE"));

			_toStringSet = true;
		}
		
		return _toString;
	}

	@Override
	public RmtesBuffer clear()
	{
		_rsslBuffer.clear();
		_rsslRmtesCacheBuffer.clear();
		_rsslRmtesBuffer.clear();
		
		_decodedUTF16BufferSet = false;
		_decodedUTF8BufferSet = false;
		_toStringSet = false;
		_applyToCache = false;
		
		return this;
	}
	
	boolean applyToCache()
	{
		return _applyToCache;
	}

	@Override
	public RmtesBuffer apply(RmtesBuffer source)
	{
		if (source == null)
			throw ommIUExcept().message("Source passed in is invalid in apply(RmtesBuffer source)", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if (_applyToCache)
		{
			_decodedUTF16BufferSet = false;
			_decodedUTF8BufferSet = false;
			_toStringSet = false;
		}
		
		ByteBuffer cacheByteBuffer = _rsslRmtesCacheBuffer.byteData();
		int cacheBufferPreAllocateLen = _rsslRmtesCacheBuffer.allocatedLength();
		
		if (((RmtesBufferImpl)source)._applyToCache && !((RmtesBufferImpl)source)._toStringSet)
		{
			int sourceBufferLen = ((RmtesBufferImpl)source)._rsslRmtesCacheBuffer.length();
			if (cacheByteBuffer == null || cacheBufferPreAllocateLen < sourceBufferLen)
			{
				_rsslRmtesCacheBuffer.data(ByteBuffer.allocate(sourceBufferLen));
				_rsslRmtesCacheBuffer.allocatedLength(sourceBufferLen);
			}
			else 
				cacheByteBuffer.position(0);
			
			_rsslRmtesCacheBuffer.byteData().put(((RmtesBufferImpl)source)._rsslRmtesCacheBuffer.byteData());
			_rsslRmtesCacheBuffer.length(sourceBufferLen);
			
			_applyToCache = true;
		}
		else 
		{
			Buffer rsslBuffer = ((RmtesBufferImpl)source)._rsslBuffer;
			int origCacheDataLen = 	_rsslRmtesCacheBuffer.length();
			
			if (_rsslDecoder.hasPartialRMTESUpdate(rsslBuffer))
			{
				if (cacheByteBuffer == null)
					throw ommIUExcept().message("RmtesBuffer does not contain original RMTES encoded data in apply(RmtesBuffer source)", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
				
				int sourceBufferLen = rsslBuffer.length() + _rsslRmtesCacheBuffer.length();
				if ( cacheBufferPreAllocateLen < sourceBufferLen)
				{
						ByteBuffer newByteBuffer = ByteBuffer.allocate(sourceBufferLen);
						newByteBuffer.put(cacheByteBuffer);
						_rsslRmtesCacheBuffer.data(newByteBuffer);
						_rsslRmtesCacheBuffer.allocatedLength(sourceBufferLen);
						_rsslRmtesCacheBuffer.length(origCacheDataLen);
				}
			}
			else
			{
				int sourceBufferLen = rsslBuffer.length();
				
				if (cacheByteBuffer == null || cacheBufferPreAllocateLen < sourceBufferLen)
				{
					_rsslRmtesCacheBuffer.data(ByteBuffer.allocate(sourceBufferLen));
					_rsslRmtesCacheBuffer.allocatedLength(sourceBufferLen);
				}
				
				_rsslRmtesCacheBuffer.length(0);
			}
			
			int retCode;
			if ( (retCode = _rsslDecoder.RMTESApplyToCache(rsslBuffer, _rsslRmtesCacheBuffer)) != CodecReturnCodes.SUCCESS)
				throw ommIUExcept().message("rsslDecoder.RMTESApplyToCache() failed in apply(RmtesBuffer source)", retCode);

			_applyToCache = true;
		}
		
		return this;
	}

	@Override
	public RmtesBuffer apply(OmmRmtes source)
	{
		if (source == null || source.rmtes() == null)
			throw ommIUExcept().message("Source passed in is invalid in apply(OmmRmtes source)", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

		return apply(source.rmtes());
	}

	RmtesBuffer apply()
	{
		ByteBuffer cacheByteBuffer = _rsslRmtesCacheBuffer.byteData();
		int cacheBufferPreAllocateLen = _rsslRmtesCacheBuffer.allocatedLength();
		int origCacheDataLen = 	_rsslRmtesCacheBuffer.length();
		
		if (_rsslDecoder.hasPartialRMTESUpdate(_rsslBuffer))
		{
			if (cacheByteBuffer == null)
				throw ommIUExcept().message("RmtesBuffer does not contain original RMTES encoded data in apply()", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			
			int sourceBufferLen = _rsslBuffer.length() + _rsslRmtesCacheBuffer.length();
			if ( cacheBufferPreAllocateLen < sourceBufferLen)
			{
				ByteBuffer newByteBuffer = ByteBuffer.allocate(sourceBufferLen);
				newByteBuffer.put(cacheByteBuffer);
				_rsslRmtesCacheBuffer.data(newByteBuffer);
				_rsslRmtesCacheBuffer.allocatedLength(sourceBufferLen);
				_rsslRmtesCacheBuffer.length(origCacheDataLen);
			}
		}
		else
		{
			int sourceBufferLen = _rsslBuffer.length();
			
			if (cacheByteBuffer == null || cacheBufferPreAllocateLen < sourceBufferLen)
			{
				_rsslRmtesCacheBuffer.data(ByteBuffer.allocate(sourceBufferLen));
				_rsslRmtesCacheBuffer.allocatedLength(sourceBufferLen);
			}
			
			_rsslRmtesCacheBuffer.length(0);
		}
		
		int retCode;
		if ( (retCode = _rsslDecoder.RMTESApplyToCache(_rsslBuffer, _rsslRmtesCacheBuffer)) != CodecReturnCodes.SUCCESS)
			throw ommIUExcept().message("rsslDecoder.RMTESApplyToCache() failed in apply()", retCode);
		
		_applyToCache = true;
	
		return this;
	}
	
	private void decode()
	{
		if (!_applyToCache)
			apply();
		
		if (_rsslRmtesCacheBuffer.length() == 0)
		{
			if (_rsslRmtesBuffer.byteData() == null)
			{
				_rsslRmtesBuffer.data( ByteBuffer.allocate(RMTES_DECODE_BUFFER_INIT_SIZE));
				_rsslRmtesBuffer.allocatedLength(RMTES_DECODE_BUFFER_INIT_SIZE);
			}
			else 
				_rsslRmtesBuffer.clear();
		}
		
		if (_rsslRmtesBuffer.byteData() == null || _rsslRmtesBuffer.allocatedLength()  < _rsslRmtesCacheBuffer.length() *2  )
		{
			int preAllocatedLen = _rsslRmtesCacheBuffer.length()*2 + 20;
			_rsslRmtesBuffer.data(ByteBuffer.allocate(preAllocatedLen));
			_rsslRmtesBuffer.allocatedLength(preAllocatedLen);
		}
		else 
			_rsslRmtesBuffer.clear();
		
		int ret = _rsslDecoder.RMTESToUCS2(_rsslRmtesBuffer, _rsslRmtesCacheBuffer) ;
		 if (ret != CodecReturnCodes.SUCCESS)
         	throw ommIUExcept().message("rsslDecoder.RMTESToUCS2() failed in decode() : " + CodecReturnCodes.info(ret), ret);
		 
		 _rsslRmtesBuffer.byteData().limit(_rsslRmtesBuffer.length());
	}
	
	com.rtsdk.eta.codec.Buffer rsslBuffer()
	{
		return _rsslBuffer;
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	//used only for JUNIT tests
	public void setRsslData(ByteBuffer buffer)
	{
		 _rsslBuffer.data(buffer);
	}
}