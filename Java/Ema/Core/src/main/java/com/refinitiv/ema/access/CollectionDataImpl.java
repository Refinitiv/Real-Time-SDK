///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.ComplexType;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

abstract class CollectionDataImpl extends DataImpl implements ComplexType
{
	protected final static int ENCODE_RSSL_BUFFER_INIT_SIZE = 4096;
	
	private OmmInvalidUsageExceptionImpl	_ommIUExcept;
	private OmmOutOfRangeExceptionImpl 	_ommOORExcept;
	protected com.refinitiv.eta.codec.DataDictionary _rsslDictionary;
	protected com.refinitiv.eta.codec.LocalFieldSetDefDb _rsslLocalFLSetDefDb;
	protected com.refinitiv.eta.codec.LocalElementSetDefDb _rsslLocalELSetDefDb;
	protected Object _rsslLocalSetDefDb;
	protected boolean _fillCollection;
	protected com.refinitiv.eta.codec.EncodeIterator _rsslEncodeIter;
	boolean _encodeComplete;
	protected int _errorCode = ErrorCode.NO_ERROR;
	protected StringBuilder _errorString;
	
	CollectionDataImpl(EmaObjectManager objManager)
	{
		if (objManager == null)
		{
			_encodeComplete = false;
			_rsslEncodeIter = com.refinitiv.eta.codec.CodecFactory.createEncodeIterator() ;
			_rsslBuffer = CodecFactory.createBuffer();
			_rsslBuffer.data(ByteBuffer.allocate(ENCODE_RSSL_BUFFER_INIT_SIZE));
		}
		else
			_objManager = objManager;
	}
	
	void clear()
	{
		_encodeComplete = false;
		
		_rsslEncodeIter.clear();
		ByteBuffer data = _rsslBuffer.data();
		if (data != null)
		{
			data.clear();
			_rsslBuffer.data(data);
		}
		else
			_rsslBuffer.clear();
	}

	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	OmmOutOfRangeExceptionImpl ommOORExcept()
	{
		if (_ommOORExcept == null)
			_ommOORExcept = new OmmOutOfRangeExceptionImpl();
		
		return _ommOORExcept;
	}
	
	StringBuilder errorString()
	{
		if (_errorString == null)
			_errorString = new StringBuilder();
		else
			_errorString.setLength(0);
		
		return _errorString;
	}
	
	abstract Buffer encodedData();
}