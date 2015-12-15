///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;

abstract class ComplexTypeImpl extends DataImpl implements ComplexType
{
	protected final static String EMPTY_STRING = "";
	
	protected int _errorCode = ErrorCode.NO_ERROR;
	protected com.thomsonreuters.upa.codec.DataDictionary _rsslDictionary;
	protected com.thomsonreuters.upa.codec.DictionaryEntry _rsslDictionaryEntry;
	protected com.thomsonreuters.upa.codec.LocalFieldSetDefDb _rsslLocalFLSetDefDb;
	protected com.thomsonreuters.upa.codec.LocalElementSetDefDb _rsslLocalELSetDefDb;
	protected Object _rsslLocalSetDefDb;
	protected com.thomsonreuters.upa.codec.DecodeIterator _rsslDecodeIterForCollection;
	protected boolean _fillCollection;
	protected boolean _decodingStarted;
	protected boolean _encodingStarted;
	protected boolean _atEnd;
	protected DataImpl[] _loadPool;
	protected DataImpl _load;  
	
	
	DataImpl load()
	{
		return _load;
	}
	
	void decodeInitialize()
	{
		if (_load == null)
		{
			_loadPool = new DataImpl[34];
			
			_loadPool[DataTypes.REQ_MSG] = new ReqMsgImpl();
			_loadPool[DataTypes.REFRESH_MSG] = new RefreshMsgImpl();
			_loadPool[DataTypes.UPDATE_MSG] = new UpdateMsgImpl();
			_loadPool[DataTypes.STATUS_MSG] = new StatusMsgImpl();
			_loadPool[DataTypes.POST_MSG] = new PostMsgImpl();
			_loadPool[DataTypes.ACK_MSG] = new AckMsgImpl();
			_loadPool[DataTypes.GENERIC_MSG] = new GenericMsgImpl();
			_loadPool[DataTypes.FIELD_LIST] = new FieldListImpl();
			_loadPool[DataTypes.ELEMENT_LIST] = new ElementListImpl();
			_loadPool[DataTypes.MAP] = new MapImpl();
			_loadPool[DataTypes.VECTOR] = new VectorImpl();
			_loadPool[DataTypes.SERIES] = new SeriesImpl();
			_loadPool[DataTypes.FILTER_LIST] = new FilterListImpl();
			_loadPool[DataTypes.OPAQUE] = new OmmOpaqueImpl();
			_loadPool[DataTypes.XML] = new OmmXmlImpl();
			_loadPool[DataTypes.ANSI_PAGE] = new OmmAnsiPageImpl();
			_loadPool[DataTypes.ARRAY] = new OmmArrayImpl();
			_loadPool[DataTypes.INT] = new OmmIntImpl();
			_loadPool[DataTypes.UINT] = new OmmUIntImpl();
			_loadPool[DataTypes.REAL] = new OmmRealImpl();
			_loadPool[DataTypes.FLOAT] = new OmmFloatImpl();
			_loadPool[DataTypes.DOUBLE] = new OmmDoubleImpl();
			_loadPool[DataTypes.DATE] = new OmmDateImpl();
			_loadPool[DataTypes.TIME] = new OmmTimeImpl();
			_loadPool[DataTypes.DATETIME] = new OmmDateTimeImpl();
			_loadPool[DataTypes.QOS] = new OmmQosImpl();
			_loadPool[DataTypes.STATE] = new OmmStateImpl();
			_loadPool[DataTypes.ENUM] = new OmmEnumImpl();
			_loadPool[DataTypes.BUFFER] = new OmmBufferImpl();
			_loadPool[DataTypes.ASCII] = new OmmAsciiImpl();
			_loadPool[DataTypes.UTF8] = new OmmUtf8Impl();
			_loadPool[DataTypes.RMTES] = new OmmRmtesImpl();
			_loadPool[DataTypes.ERROR] = new OmmErrorImpl();
			_loadPool[DataTypes.NO_DATA] = new NoDataImpl();
		}
	}
	
	DataImpl dataInstance (int dType)
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
	        	retData = (DataImpl)new OmmArrayImpl();
			break;
		case DataTypes.FIELD_LIST :
	        	retData = (DataImpl)new FieldListImpl();
			break;
		case DataTypes.MAP :
	        	retData = (DataImpl)new MapImpl();
			break;
		case DataTypes.ELEMENT_LIST :
	        	retData = (DataImpl)new ElementListImpl();
			break;
		case DataTypes.FILTER_LIST :
	        	retData = (DataImpl)new FilterListImpl();
			break;
		case DataTypes.VECTOR :
	        	retData = (DataImpl)new VectorImpl();
			break;
		case DataTypes.SERIES :
	        	retData = (DataImpl)new SeriesImpl();
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
	        	retData = (DataImpl)new ReqMsgImpl();
			break;
		case DataTypes.REFRESH_MSG :
	        	retData = (DataImpl)new RefreshMsgImpl();
			break;
		case DataTypes.STATUS_MSG :
	        	retData = (DataImpl)new StatusMsgImpl();
			break;
		case DataTypes.UPDATE_MSG :
	        	retData = (DataImpl)new UpdateMsgImpl();
			break;
		case DataTypes.ACK_MSG :
	        	retData = (DataImpl)new AckMsgImpl();
			break;
		case DataTypes.POST_MSG :
	        	retData = (DataImpl)new PostMsgImpl();
			break;
		case DataTypes.GENERIC_MSG :
	        	retData = (DataImpl)new GenericMsgImpl();
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
	
	//TODO
	ByteBuffer dataBuffer()
	{
		return null;
	}
}