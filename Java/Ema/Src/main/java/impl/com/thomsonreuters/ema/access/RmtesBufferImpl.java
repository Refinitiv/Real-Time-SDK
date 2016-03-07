///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;

import com.thomsonreuters.ema.access.OmmRmtes;
import com.thomsonreuters.ema.access.RmtesBuffer;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.RmtesCacheBuffer;
import com.thomsonreuters.upa.codec.RmtesDecoder;

public class RmtesBufferImpl implements RmtesBuffer
{
	// TODO think about if backing will help performance by not trigger GC.
	private com.thomsonreuters.upa.codec.Buffer _rsslBuffer = CodecFactory.createBuffer();
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private RmtesDecoder _rsslDecoder = CodecFactory.createRmtesDecoder();
	private CharBuffer _cachedCharBuffer;
	// TODO fixme
	private RmtesCacheBuffer _rsslCacheBuffer = CodecFactory.createRmtesCacheBuffer(0);
	private CharBuffer _decodedCharBuffer;
	private com.thomsonreuters.upa.codec.RmtesBuffer _rsslDecodedBuffer = CodecFactory.createRmtesBuffer(0);
	private boolean _decodedUTF16BufferSet;
	private boolean _toStringSet;
	private boolean _applyToCache;
	private String _toString;

	RmtesBufferImpl()
	{
	}

	@Override
	public CharBuffer asUTF16()
	{
		if (!_decodedUTF16BufferSet)
		{
			if (!_applyToCache)
				apply();

			if (_rsslCacheBuffer.length() == 0)
			{
				if (_rsslDecodedBuffer.data() == null)
				{
					_decodedCharBuffer = CharBuffer.allocate(0);
					_rsslDecodedBuffer.data(_decodedCharBuffer);
					_rsslDecodedBuffer.allocatedLength(0);
					_rsslDecodedBuffer.length(0);
				} else
					_rsslDecodedBuffer.data().clear();

				_decodedUTF16BufferSet = true;
				return _rsslDecodedBuffer.data();
			}

			if (_rsslDecodedBuffer.data() == null || _rsslDecodedBuffer.allocatedLength() < _rsslCacheBuffer.length())
			{
				int preAllocatedLen = _rsslCacheBuffer.length() + 20;
				_decodedCharBuffer = CharBuffer.allocate(preAllocatedLen);
				_rsslDecodedBuffer.data(_decodedCharBuffer);
				_rsslDecodedBuffer.allocatedLength(preAllocatedLen);
			} else
				_rsslDecodedBuffer.data().clear();

			if (_rsslDecoder.RMTESToUCS2(_rsslDecodedBuffer, _rsslCacheBuffer) != CodecReturnCodes.SUCCESS)
				throw ommIUExcept().message("RMTES Parse error in asUTF16()");

			_rsslDecodedBuffer.data().limit(_rsslDecodedBuffer.length());
			_decodedUTF16BufferSet = true;
		}

		return _rsslDecodedBuffer.data();
	}

	@Override
	public String toString()
	{
		if (!_toStringSet)
		{
			if (!_applyToCache)
				apply();

			if (_rsslCacheBuffer.length() == 0)
			{
				_toString = DataImpl.EMPTY_STRING;
				_toStringSet = true;

				return _toString;
			}

			if (_rsslDecodedBuffer.data() == null || _rsslDecodedBuffer.allocatedLength() < _rsslCacheBuffer.length())
			{
				int preAllocatedLen = _rsslCacheBuffer.length() + 20;
				_decodedCharBuffer = CharBuffer.allocate(preAllocatedLen);
				_rsslDecodedBuffer.data(_decodedCharBuffer);
				_rsslDecodedBuffer.allocatedLength(preAllocatedLen);
			} else
				_rsslDecodedBuffer.data().clear();

			// TODO test me in UTF8 format
			if (_rsslDecoder.RMTESToUCS2(_rsslDecodedBuffer, _rsslCacheBuffer) != CodecReturnCodes.SUCCESS)
				throw ommIUExcept().message("RMTES Parse error in toString()");

			_rsslDecodedBuffer.data().limit(_rsslDecodedBuffer.length());
			_toString = _rsslDecodedBuffer.data().toString();

			_toStringSet = true;
		}

		return _toString;
	}

	@Override
	public RmtesBuffer clear()
	{
		_rsslBuffer.clear();

		if (_cachedCharBuffer != null)
			_cachedCharBuffer.clear();
		_rsslCacheBuffer.length(0);

		if (_decodedCharBuffer != null)
			_decodedCharBuffer.clear();
		_rsslDecodedBuffer.length(0);

		_decodedUTF16BufferSet = false;
		_toStringSet = false;
		_applyToCache = false;

		return this;
	}

	@Override
	public RmtesBuffer apply(RmtesBuffer source)
	{
		if (source == null)
			throw ommIUExcept().message("Source passed in is invalid in apply(RmtesBuffer source)");

		if (_applyToCache)
		{
			_decodedUTF16BufferSet = false;
			_toStringSet = false;
		}

		CharBuffer cacheCharBuffer = _rsslCacheBuffer.data();
		int cacheBufferPreAllocateLen = _rsslCacheBuffer.allocatedLength();

		if (((RmtesBufferImpl) source)._applyToCache)
		{
			int sourceBufferLen = ((RmtesBufferImpl) source)._rsslCacheBuffer.length();
			if (cacheCharBuffer == null || cacheBufferPreAllocateLen < sourceBufferLen)
			{
				_cachedCharBuffer = CharBuffer.allocate(sourceBufferLen);
				_rsslCacheBuffer.data(_cachedCharBuffer);
				_rsslCacheBuffer.allocatedLength(sourceBufferLen);
			}

			try
			{
				((RmtesBufferImpl) source)._rsslCacheBuffer.data().read(_rsslCacheBuffer.data());
				_rsslCacheBuffer.length(sourceBufferLen);
			} catch (IOException e)
			{
				throw ommIUExcept()
						.message("CharBuffer reading failed in apply(RmtesBuffer source)" + e.getLocalizedMessage());
			}

			_applyToCache = true;
		} else
		{
			Buffer rsslBuffer = ((RmtesBufferImpl) source)._rsslBuffer;

			if (_rsslDecoder.hasPartialRMTESUpdate(rsslBuffer))
			{
				if (cacheCharBuffer == null)
					throw ommIUExcept().message(
							"RmtesBuffer does not contain original RMTES encoded data in apply(RmtesBuffer source)");

				int sourceBufferLen = rsslBuffer.length() + _rsslCacheBuffer.length();
				if (cacheBufferPreAllocateLen < sourceBufferLen)
				{
					try
					{
						_cachedCharBuffer = CharBuffer.allocate(sourceBufferLen);
						cacheCharBuffer.read(_cachedCharBuffer);
						_rsslCacheBuffer.data(_cachedCharBuffer);
						_rsslCacheBuffer.length(cacheCharBuffer.length());
						_rsslCacheBuffer.allocatedLength(sourceBufferLen);
					} catch (IOException e)
					{
						throw ommIUExcept().message(
								"CharBuffer reading failed in apply(RmtesBuffer source)" + e.getLocalizedMessage());
					}
				}
			} else
			{
				int sourceBufferLen = rsslBuffer.length() + 20;

				if (cacheCharBuffer == null || cacheBufferPreAllocateLen < rsslBuffer.length())
				{
					_cachedCharBuffer = CharBuffer.allocate(sourceBufferLen);
					_rsslCacheBuffer.data(_cachedCharBuffer);
					_rsslCacheBuffer.allocatedLength(sourceBufferLen);
				}

				_rsslCacheBuffer.length(0);
			}

			if (_rsslDecoder.RMTESApplyToCache(rsslBuffer, _rsslCacheBuffer) != CodecReturnCodes.SUCCESS)
				throw ommIUExcept().message("RMTESApplyToCache() failed in apply(RmtesBuffer source)");

			_applyToCache = true;
		}

		return this;
	}

	@Override
	public RmtesBuffer apply(OmmRmtes source)
	{
		if (source == null || source.rmtes() == null)
			throw ommIUExcept().message("Source passed in is invalid in apply(OmmRmtes source)");

		return apply(source.rmtes());
	}

	boolean applyToCache()
	{
		return _applyToCache;
	}
	
	RmtesBuffer apply()
	{
		CharBuffer cacheCharBuffer = _rsslCacheBuffer.data();
		int cacheBufferPreAllocateLen = _rsslCacheBuffer.allocatedLength();

		if (_rsslDecoder.hasPartialRMTESUpdate(_rsslBuffer))
		{
			if (cacheCharBuffer == null)
				throw ommIUExcept().message("RmtesBuffer does not contain original RMTES encoded data in apply()");

			int sourceBufferLen = _rsslBuffer.length() + _rsslCacheBuffer.length();
			if (cacheBufferPreAllocateLen < sourceBufferLen)
			{
				try
				{
					_cachedCharBuffer = CharBuffer.allocate(sourceBufferLen);
					cacheCharBuffer.read(_cachedCharBuffer);
					_rsslCacheBuffer.data(_cachedCharBuffer);
					_rsslCacheBuffer.allocatedLength(sourceBufferLen);
					_rsslCacheBuffer.length(cacheCharBuffer.length());
				} catch (IOException e)
				{
					throw ommIUExcept().message("CharBuffer reading failed in apply()" + e.getLocalizedMessage());
				}
			}
		} else
		{
			int sourceBufferLen = _rsslBuffer.length() + 20;

			if (cacheCharBuffer == null || cacheBufferPreAllocateLen < _rsslBuffer.length())
			{
				_cachedCharBuffer = CharBuffer.allocate(sourceBufferLen);
				_rsslCacheBuffer.data(_cachedCharBuffer);
				_rsslCacheBuffer.allocatedLength(sourceBufferLen);
			}

			_rsslCacheBuffer.length(0);
		}

		if (_rsslDecoder.RMTESApplyToCache(_rsslBuffer, _rsslCacheBuffer) != CodecReturnCodes.SUCCESS)
			throw ommIUExcept().message("RMTESApplyToCache() failed in apply()");

		_applyToCache = true;

		return this;
	}

	com.thomsonreuters.upa.codec.Buffer rsslBuffer()
	{
		return _rsslBuffer;
	}

	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return _ommIUExcept;
	}

	// used only for JUNIT tests
	public void setRsslData(ByteBuffer buffer)
	{
		_rsslBuffer.data(buffer);
	}
}