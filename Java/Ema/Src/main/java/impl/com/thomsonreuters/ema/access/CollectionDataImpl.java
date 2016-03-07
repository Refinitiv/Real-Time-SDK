///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;

abstract class CollectionDataImpl extends DataImpl implements ComplexType
{
	protected final static int ENCODE_RSSL_BUFFER_INIT_SIZE = 4096;

	private OmmInvalidUsageExceptionImpl	_ommIUExcept;
	private OmmOutOfRangeExceptionImpl 	_ommOORExcept;
	protected com.thomsonreuters.upa.codec.DataDictionary _rsslDictionary;
	protected com.thomsonreuters.upa.codec.LocalFieldSetDefDb _rsslLocalFLSetDefDb;
	protected com.thomsonreuters.upa.codec.LocalElementSetDefDb _rsslLocalELSetDefDb;
	protected Object _rsslLocalSetDefDb;
	protected boolean _fillCollection;
	protected com.thomsonreuters.upa.codec.EncodeIterator _rsslEncodeIter;
	boolean _encodeComplete;
	protected int _errorCode = ErrorCode.NO_ERROR;
	protected StringBuilder _errorString;
	
	CollectionDataImpl(boolean decoding)
	{
		if (!decoding)
		{
			_encodeComplete = false;
			_rsslEncodeIter = com.thomsonreuters.upa.codec.CodecFactory.createEncodeIterator() ;
			_rsslBuffer = CodecFactory.createBuffer();
			_rsslBuffer.data(ByteBuffer.allocateDirect(ENCODE_RSSL_BUFFER_INIT_SIZE));
		}
	}
	
	void clear()
	{
		_encodeComplete = false;
	}

	DataImpl dataInstance(int dType)
	{
		DataImpl retData;
		switch (dType)
		{
		case DataTypes.INT :
	        	retData = (DataImpl)new OmmIntImpl();
	        break;
		case DataTypes.UINT :
	        	retData = (DataImpl)new OmmUIntImpl();
			break;
		case DataTypes.FLOAT :
	        	retData = (DataImpl)new OmmFloatImpl();
			break;
		case DataTypes.DOUBLE :
	        	retData = (DataImpl)new OmmDoubleImpl();
			break;
		case DataTypes.BUFFER :
	        	retData = (DataImpl)new OmmBufferImpl();
			break;
		case DataTypes.ASCII :
	        	retData = (DataImpl)new OmmAsciiImpl();
			break;
		case DataTypes.UTF8 :
	        	retData = (DataImpl)new OmmUtf8Impl();
			break;
		case DataTypes.RMTES :
	        	retData = (DataImpl)new OmmRmtesImpl();
			break;
		case DataTypes.REAL :
	        	retData = (DataImpl)new OmmRealImpl();
			break;
		case DataTypes.DATE :
	        	retData = (DataImpl)new OmmDateImpl();
			break;
		case DataTypes.TIME :
	        	retData = (DataImpl)new OmmTimeImpl();
			break;
		case DataTypes.DATETIME :
	        	retData = (DataImpl)new OmmDateTimeImpl();
			break;
		case DataTypes.QOS :
	        	retData = (DataImpl)new OmmQosImpl();
			break;
		case DataTypes.STATE :
	        	retData = (DataImpl)new OmmStateImpl();
			break;
		case DataTypes.ENUM :
	        	retData = (DataImpl)new OmmEnumImpl();
			break;
		case DataTypes.ARRAY :
	        	retData = (DataImpl)new OmmArrayImpl(true);
			break;
		case DataTypes.FIELD_LIST :
	        	retData = (DataImpl)new FieldListImpl(true);
			break;
		case DataTypes.MAP :
	        	retData = (DataImpl)new MapImpl(true);
			break;
		case DataTypes.ELEMENT_LIST :
	        	retData = (DataImpl)new ElementListImpl(true);
			break;
		case DataTypes.FILTER_LIST :
	        	retData = (DataImpl)new FilterListImpl(true);
			break;
		case DataTypes.VECTOR :
	        	retData = (DataImpl)new VectorImpl(true);
			break;
		case DataTypes.SERIES :
	        	retData = (DataImpl)new SeriesImpl(true);
			break;
		case DataTypes.OPAQUE :
	        	retData = (DataImpl)new OmmOpaqueImpl();
			break;
		case DataTypes.ANSI_PAGE :
	        	retData = (DataImpl)new OmmAnsiPageImpl();
			break;
		case DataTypes.XML :
	        	retData = (DataImpl)new OmmXmlImpl();
			break;
		case DataTypes.REQ_MSG :
	        	retData = (DataImpl)new ReqMsgImpl(true);
			break;
		case DataTypes.REFRESH_MSG :
	        	retData = (DataImpl)new RefreshMsgImpl(true);
			break;
		case DataTypes.STATUS_MSG :
	        	retData = (DataImpl)new StatusMsgImpl(true);
			break;
		case DataTypes.UPDATE_MSG :
	        	retData = (DataImpl)new UpdateMsgImpl(true);
			break;
		case DataTypes.ACK_MSG :
	        	retData = (DataImpl)new AckMsgImpl(true);
			break;
		case DataTypes.POST_MSG :
	        	retData = (DataImpl)new PostMsgImpl(true);
			break;
		case DataTypes.GENERIC_MSG :
	        	retData = (DataImpl)new GenericMsgImpl(true);
			break;
		case DataTypes.NO_DATA :
	        	retData = (DataImpl)new NoDataImpl();
	        break;
		case DataTypes.ERROR :
		default:
	        	retData = (DataImpl)new OmmErrorImpl();
	        break;
		}
		
		return retData;
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