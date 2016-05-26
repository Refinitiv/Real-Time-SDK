///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.valueadd.common.VaNode;

class DataImpl extends VaNode implements Data
{
	protected final static String EMPTY_STRING 			= "";
	protected final static String NOCODE_STRING 		= "NoCode";
	protected final static String BLANK_STRING 			= "(blank data)";
	protected final static String DEFAULTCODE_STRING 	= "Unknown DataCode value ";
	
	protected StringBuilder 											_toString = new StringBuilder();
	protected int 															_dataCode = DataCode.NO_CODE;
	protected ByteBuffer 												_asHex;
	protected com.thomsonreuters.upa.codec.DecodeIterator 	_rsslDecodeIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
	protected com.thomsonreuters.upa.codec.DecodeIterator 	_rsslNestedMsgDecodeIter;
	protected int 											_rsslMajVer = com.thomsonreuters.upa.codec.Codec.majorVersion();
	protected int 											_rsslMinVer = com.thomsonreuters.upa.codec.Codec.minorVersion();
	protected com.thomsonreuters.upa.codec.Buffer 			_rsslBuffer;
	protected EmaObjectManager _objManager;

	@Override
	public String codeAsString()
	{
		switch (code())
		{
			case DataCode.NO_CODE:
				return NOCODE_STRING;
			case DataCode.BLANK:
				return BLANK_STRING;
			default:
				return DEFAULTCODE_STRING + code();
		}
	}

	@Override
	public int dataType()
	{
		return DataTypes.NO_DATA;
	}
	
	@Override
	public int code()
	{
		return _dataCode;
	}
	
	public ByteBuffer asHex()
	{
		int rsslBufferDataLength = _rsslBuffer.length();
		if (_asHex == null || _asHex.capacity() < rsslBufferDataLength)
			_asHex = ByteBuffer.allocate(rsslBufferDataLength);
		else
			_asHex.clear();
		
		ByteBuffer rsslByteBuf = _rsslBuffer.data();
		int limit = rsslBufferDataLength + _rsslBuffer.position();
		for (int index = _rsslBuffer.position(); index < limit; ++index)
			_asHex.put(rsslByteBuf.get(index));
		
		_asHex.flip();
		return _asHex;
	}
	
	String toString(int indent)
	{
		return null;
	}

	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int errorCode) {}
	
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter) {}
	
	void decode(com.thomsonreuters.upa.codec.Msg rsslMsg, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary) {}
	
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary,	Object obj) {}
	
	int dataType(int rsslType, int majVer, int minVer, com.thomsonreuters.upa.codec.Buffer rsslBuffer)
	{
		int dType;

		if (com.thomsonreuters.upa.codec.DataTypes.MSG == rsslType)
		{
			if (_rsslNestedMsgDecodeIter == null)
				_rsslNestedMsgDecodeIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
			else
				_rsslNestedMsgDecodeIter.clear();

			int retCode = _rsslNestedMsgDecodeIter.setBufferAndRWFVersion(rsslBuffer, majVer, minVer);
			if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
				dType = DataTypes.ERROR;
			else
				dType = Utilities.toEmaMsgClass[_rsslNestedMsgDecodeIter.extractMsgClass()];
		}
		else
			dType = Utilities.toEmaDataType[rsslType];

		return dType;
	}
	
	DataImpl noDataInstance()
	{
   		DataImpl  retData = (DataImpl)_objManager._noDataPool.poll();
        if(retData == null)
        {
        	retData = (DataImpl)new NoDataImpl();
        	 _objManager._noDataPool .updatePool(retData);
        }
        
   		return retData;
	}
	
	DataImpl dataInstance(DataImpl data, int dType)
	{
		if (dType != data.dataType())
		{
			data.returnToPool();
			data = fromPool(dType);
		}
		
		return data;
	}

	DataImpl fromPool (int dType)
	{
		DataImpl retData;
		switch (dType)
		{
		case DataTypes.INT :
			retData = (DataImpl)_objManager._ommIntPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmIntImpl();
	        	_objManager._ommIntPool.updatePool(retData);
	        }
	        break;
		case DataTypes.UINT :
			retData = (DataImpl)_objManager._ommUIntPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmUIntImpl();
	        	_objManager._ommUIntPool.updatePool(retData);
	        }
			break;
		case DataTypes.FLOAT :
			retData = (DataImpl)_objManager._ommFloatPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmFloatImpl();
	        	_objManager._ommFloatPool.updatePool(retData);
	        }
			break;
		case DataTypes.DOUBLE :
			retData = (DataImpl)_objManager._ommDoublePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmDoubleImpl();
	        	_objManager._ommDoublePool.updatePool(retData);
	        }
			break;
		case DataTypes.BUFFER :
			retData = (DataImpl)_objManager._ommBufferPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmBufferImpl();
	        	_objManager._ommBufferPool.updatePool(retData);
	        }
			break;
		case DataTypes.ASCII :
			retData = (DataImpl)_objManager._ommAsciiPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmAsciiImpl();
	        	_objManager._ommAsciiPool.updatePool(retData);
	        }
			break;
		case DataTypes.UTF8 :
			retData = (DataImpl)_objManager._ommUtf8Pool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmUtf8Impl();
	        	_objManager._ommUtf8Pool.updatePool(retData);
	        }
			break;
		case DataTypes.RMTES :
			retData = (DataImpl)_objManager._ommRmtesPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmRmtesImpl();
	        	_objManager._ommRmtesPool.updatePool(retData);
	        }
			break;
		case DataTypes.REAL :
			retData = (DataImpl)_objManager._ommRealPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmRealImpl();
	        	_objManager._ommRealPool.updatePool(retData);
	        }
			break;
		case DataTypes.DATE :
			retData = (DataImpl)_objManager._ommDatePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmDateImpl();
	        	_objManager._ommDatePool.updatePool(retData);
	        }
			break;
		case DataTypes.TIME :
			retData = (DataImpl)_objManager._ommTimePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmTimeImpl();
	        	_objManager._ommTimePool.updatePool(retData);
	        }
			break;
		case DataTypes.DATETIME :
			retData = (DataImpl)_objManager._ommDateTimePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmDateTimeImpl();
	        	_objManager._ommDateTimePool.updatePool(retData);
	        }
			break;
		case DataTypes.QOS :
			retData = (DataImpl)_objManager._ommQosPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmQosImpl();
	        	_objManager._ommQosPool.updatePool(retData);
	        }
			break;
		case DataTypes.STATE :
			retData = (DataImpl)_objManager._ommStatePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmStateImpl();
	        	_objManager._ommStatePool.updatePool(retData);
	        }
			break;
		case DataTypes.ENUM :
			retData = (DataImpl)_objManager._ommEnumPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmEnumImpl();
	        	_objManager._ommEnumPool.updatePool(retData);
	        }
			break;
		case DataTypes.ARRAY :
			retData = (DataImpl)_objManager._ommArrayPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmArrayImpl(_objManager);
	        	_objManager._ommArrayPool.updatePool(retData);
	        }
			break;
		case DataTypes.FIELD_LIST :
			retData = (DataImpl)_objManager._fieldListPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new FieldListImpl(_objManager);
	        	_objManager._fieldListPool.updatePool(retData);
	        }
			break;
		case DataTypes.MAP :
			retData = (DataImpl)_objManager._mapPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new MapImpl(_objManager);
	        	_objManager._mapPool.updatePool(retData);
	        }
			break;
		case DataTypes.ELEMENT_LIST :
			retData = (DataImpl)_objManager._elementListPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new ElementListImpl(_objManager);
	        	_objManager._elementListPool.updatePool(retData);
	        }
			break;
		case DataTypes.FILTER_LIST :
			retData = (DataImpl)_objManager._filterListPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new FilterListImpl(_objManager);
	        	_objManager._filterListPool.updatePool(retData);
	        }
			break;
		case DataTypes.VECTOR :
			retData = (DataImpl)_objManager._vectorPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new VectorImpl(_objManager);
	        	_objManager._vectorPool.updatePool(retData);
	        }
			break;
		case DataTypes.SERIES :
			retData = (DataImpl)_objManager._seriesPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new SeriesImpl(_objManager);
	        	_objManager._seriesPool.updatePool(retData);
	        }
			break;
		case DataTypes.OPAQUE :
			retData = (DataImpl)_objManager._opaquePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmOpaqueImpl();
	        	_objManager._opaquePool.updatePool(retData);
	        }
			break;
		case DataTypes.ANSI_PAGE :
			retData = (DataImpl)_objManager._ansiPagePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmAnsiPageImpl();
	        	_objManager._ansiPagePool.updatePool(retData);
	        }
			break;
		case DataTypes.XML :
			retData = (DataImpl)_objManager._xmlPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmXmlImpl();
	        	_objManager._xmlPool.updatePool(retData);
	        }
			break;
		case DataTypes.REQ_MSG :
			retData = (DataImpl)_objManager._reqMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new ReqMsgImpl(_objManager);
	        	_objManager._reqMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.REFRESH_MSG :
			retData = (DataImpl)_objManager._refreshMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new RefreshMsgImpl(_objManager);
	        	_objManager._refreshMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.STATUS_MSG :
			retData = (DataImpl)_objManager._statusMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new StatusMsgImpl(_objManager);
	        	_objManager._statusMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.UPDATE_MSG :
			retData = (DataImpl)_objManager._updateMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new UpdateMsgImpl(_objManager);
	        	_objManager._updateMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.ACK_MSG :
			retData = (DataImpl)_objManager._ackMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new AckMsgImpl(_objManager);
	        	_objManager._ackMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.POST_MSG :
			retData = (DataImpl)_objManager._postMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new PostMsgImpl(_objManager);
	        	_objManager._postMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.GENERIC_MSG :
			retData = (DataImpl)_objManager._genericMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new GenericMsgImpl(_objManager);
	        	_objManager._genericMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.NO_DATA :
			 retData = (DataImpl)_objManager._noDataPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new NoDataImpl();
	        	 _objManager._noDataPool .updatePool(retData);
	        }
	        break;
		case DataTypes.ERROR :
		default:
			retData = (DataImpl)_objManager._ommErrorPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmErrorImpl();
	        	 _objManager._ommErrorPool .updatePool(retData);
	        }
	        break;
		}
		
		return retData;
	}

	Buffer encodedData()
	{
		switch (dataType())
        {
            case DataTypes.UPDATE_MSG :
            case DataTypes.REFRESH_MSG :
            case DataTypes.STATUS_MSG :
            case DataTypes.GENERIC_MSG :
            case DataTypes.POST_MSG :
            case DataTypes.ACK_MSG :
            case DataTypes.REQ_MSG:
            	return ((MsgImpl)this).encodedData();
            case DataTypes.FIELD_LIST:
            case DataTypes.ELEMENT_LIST:
            case DataTypes.FILTER_LIST:
            case DataTypes.MAP:
            case DataTypes.ARRAY:
            case DataTypes.VECTOR:
            case DataTypes.SERIES:
            	  return ((CollectionDataImpl)this).encodedData();
            case DataTypes.OPAQUE:
            case DataTypes.XML:
            case DataTypes.ANSI_PAGE:
            default :
            	return _rsslBuffer;
        }
	}
}