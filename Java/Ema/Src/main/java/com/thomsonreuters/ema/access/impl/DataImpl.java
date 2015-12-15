///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.valueadd.common.VaNode;

class DataImpl extends VaNode implements Data
{
	protected final static String NOCODE_STRING 		= "NoCode";
	protected final static String BLANK_STRING 			= "(blank data)";
	protected final static String DEFAULTCODE_STRING 	= "Unknown DataCode value ";
	
	protected StringBuilder 								_toString = new StringBuilder();
	protected int 											_dataCode = DataCode.NO_CODE;
	protected ByteBuffer 									_toHex;
	protected String 										_hexString;
	protected OmmOutOfRangeExceptionImpl 					_ommOORExcept;
	protected OmmInvalidUsageExceptionImpl					_oommIUExcept;
	protected OmmUnsupportedDomainTypeExceptionImpl 		_ommUDTExcept;
	protected com.thomsonreuters.upa.codec.DecodeIterator 	_rsslDecodeIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
	protected com.thomsonreuters.upa.codec.DecodeIterator 	_rsslNestedMsgDecodeIter;
	protected int 											_rsslMajVer = Codec.majorVersion();
	protected int 											_rsslMinVer = Codec.minorVersion();
	protected com.thomsonreuters.upa.codec.Buffer 			_rsslBuffer;
	
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
		//TODO pooling
		int rsslBufferDataLength = _rsslBuffer.length();
		if (_toHex == null || _toHex.capacity() < rsslBufferDataLength)
			_toHex = ByteBuffer.allocate(rsslBufferDataLength);
		else
			_toHex.clear();
		
		ByteBuffer rsslByteBuf = _rsslBuffer.data();
		int limit = rsslBufferDataLength + _rsslBuffer.position();
		for (int index = _rsslBuffer.position(); index < limit; ++index)
			_toHex.put(rsslByteBuf.get(index));
		
		return _toHex;
	}
	
	OmmOutOfRangeExceptionImpl ommOORExcept()
	{
		if (_ommOORExcept == null)
			_ommOORExcept = new OmmOutOfRangeExceptionImpl();
		
		return _ommOORExcept;
	}
	
	OmmInvalidUsageExceptionImpl oommIUExcept()
	{
		if (_oommIUExcept == null)
			_oommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _oommIUExcept;
	}
	
	OmmUnsupportedDomainTypeExceptionImpl ommUDTExcept()
	{
		if (_ommUDTExcept == null)
			_ommUDTExcept = new OmmUnsupportedDomainTypeExceptionImpl();
		
		return _ommUDTExcept;
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
				_rsslNestedMsgDecodeIter = CodecFactory.createDecodeIterator();
			else
				_rsslNestedMsgDecodeIter.clear();

			int retCode = _rsslNestedMsgDecodeIter.setBufferAndRWFVersion(rsslBuffer, majVer, minVer);
			if (CodecReturnCodes.SUCCESS != retCode)
				dType = DataTypes.ERROR;
			else
				dType = Utilities.toEmaMsgClass[_rsslNestedMsgDecodeIter.extractMsgClass()];
		}
		else
			dType = Utilities.toEmaDataType[rsslType];

		return dType;
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

	private DataImpl fromPool (int dType)
	{
		DataImpl retData;
		switch (dType)
		{
		case DataTypes.INT :
			retData = (DataImpl)GlobalPool._ommIntPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmIntImpl();
	        	GlobalPool._ommIntPool.updatePool(retData);
	        }
	        break;
		case DataTypes.UINT :
			retData = (DataImpl)GlobalPool._ommUIntPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmUIntImpl();
	        	GlobalPool._ommUIntPool.updatePool(retData);
	        }
			break;
		case DataTypes.FLOAT :
			retData = (DataImpl)GlobalPool._ommFloatPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmFloatImpl();
	        	GlobalPool._ommFloatPool.updatePool(retData);
	        }
			break;
		case DataTypes.DOUBLE :
			retData = (DataImpl)GlobalPool._ommDoublePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmDoubleImpl();
	        	GlobalPool._ommDoublePool.updatePool(retData);
	        }
			break;
		case DataTypes.BUFFER :
			retData = (DataImpl)GlobalPool._ommBufferPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmBufferImpl();
	        	GlobalPool._ommBufferPool.updatePool(retData);
	        }
			break;
		case DataTypes.ASCII :
			retData = (DataImpl)GlobalPool._ommAsciiPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmAsciiImpl();
	        	GlobalPool._ommAsciiPool.updatePool(retData);
	        }
			break;
		case DataTypes.UTF8 :
			retData = (DataImpl)GlobalPool._ommUtf8Pool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmUtf8Impl();
	        	GlobalPool._ommUtf8Pool.updatePool(retData);
	        }
			break;
		case DataTypes.RMTES :
			retData = (DataImpl)GlobalPool._ommRmtesPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmRmtesImpl();
	        	GlobalPool._ommRmtesPool.updatePool(retData);
	        }
			break;
		case DataTypes.REAL :
			retData = (DataImpl)GlobalPool._ommRealPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmRealImpl();
	        	GlobalPool._ommRealPool.updatePool(retData);
	        }
			break;
		case DataTypes.DATE :
			retData = (DataImpl)GlobalPool._ommDatePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmDateImpl();
	        	GlobalPool._ommDatePool.updatePool(retData);
	        }
			break;
		case DataTypes.TIME :
			retData = (DataImpl)GlobalPool._ommTimePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmTimeImpl();
	        	GlobalPool._ommTimePool.updatePool(retData);
	        }
			break;
		case DataTypes.DATETIME :
			retData = (DataImpl)GlobalPool._ommDateTimePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmDateTimeImpl();
	        	GlobalPool._ommDateTimePool.updatePool(retData);
	        }
			break;
		case DataTypes.QOS :
			retData = (DataImpl)GlobalPool._ommQosPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmQosImpl();
	        	GlobalPool._ommQosPool.updatePool(retData);
	        }
			break;
		case DataTypes.STATE :
			retData = (DataImpl)GlobalPool._ommStatePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmStateImpl();
	        	GlobalPool._ommStatePool.updatePool(retData);
	        }
			break;
		case DataTypes.ENUM :
			retData = (DataImpl)GlobalPool._ommEnumPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmEnumImpl();
	        	GlobalPool._ommEnumPool.updatePool(retData);
	        }
			break;
		case DataTypes.ARRAY :
			retData = (DataImpl)GlobalPool._ommArrayPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmArrayImpl();
	        	GlobalPool._ommArrayPool.updatePool(retData);
	        }
			break;
		case DataTypes.FIELD_LIST :
			retData = (DataImpl)GlobalPool._fieldListPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new FieldListImpl();
	        	GlobalPool._fieldListPool.updatePool(retData);
	        }
			break;
		case DataTypes.MAP :
			retData = (DataImpl)GlobalPool._mapPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new MapImpl();
	        	GlobalPool._mapPool.updatePool(retData);
	        }
			break;
		case DataTypes.ELEMENT_LIST :
			retData = (DataImpl)GlobalPool._elementListPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new ElementListImpl();
	        	GlobalPool._elementListPool.updatePool(retData);
	        }
			break;
		case DataTypes.FILTER_LIST :
			retData = (DataImpl)GlobalPool._filterListPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new FilterListImpl();
	        	GlobalPool._filterListPool.updatePool(retData);
	        }
			break;
		case DataTypes.VECTOR :
			retData = (DataImpl)GlobalPool._vectorPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new VectorImpl();
	        	GlobalPool._vectorPool.updatePool(retData);
	        }
			break;
		case DataTypes.SERIES :
			retData = (DataImpl)GlobalPool._seriesPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new SeriesImpl();
	        	GlobalPool._seriesPool.updatePool(retData);
	        }
			break;
		case DataTypes.OPAQUE :
			retData = (DataImpl)GlobalPool._opaquePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmOpaqueImpl();
	        	GlobalPool._opaquePool.updatePool(retData);
	        }
			break;
		case DataTypes.ANSI_PAGE :
			retData = (DataImpl)GlobalPool._ansiPagePool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmAnsiPageImpl();
	        	GlobalPool._ansiPagePool.updatePool(retData);
	        }
			break;
		case DataTypes.XML :
			retData = (DataImpl)GlobalPool._xmlPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmXmlImpl();
	        	GlobalPool._xmlPool.updatePool(retData);
	        }
			break;
		case DataTypes.REQ_MSG :
			retData = (DataImpl)GlobalPool._reqMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new ReqMsgImpl();
	        	GlobalPool._reqMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.REFRESH_MSG :
			retData = (DataImpl)GlobalPool._refreshMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new RefreshMsgImpl();
	        	GlobalPool._refreshMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.STATUS_MSG :
			retData = (DataImpl)GlobalPool._statusMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new StatusMsgImpl();
	        	GlobalPool._statusMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.UPDATE_MSG :
			retData = (DataImpl)GlobalPool._updateMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new UpdateMsgImpl();
	        	GlobalPool._updateMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.ACK_MSG :
			retData = (DataImpl)GlobalPool._ackMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new AckMsgImpl();
	        	GlobalPool._ackMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.POST_MSG :
			retData = (DataImpl)GlobalPool._postMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new PostMsgImpl();
	        	GlobalPool._postMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.GENERIC_MSG :
			retData = (DataImpl)GlobalPool._genericMsgPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new GenericMsgImpl();
	        	GlobalPool._genericMsgPool.updatePool(retData);
	        }
			break;
		case DataTypes.NO_DATA :
			 retData = (DataImpl)GlobalPool._noDataPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new NoDataImpl();
	        	 GlobalPool._noDataPool .updatePool(retData);
	        }
	        break;
		case DataTypes.ERROR :
		default:
			retData = (DataImpl)GlobalPool._ommErrorPool.poll();
	        if(retData == null)
	        {
	        	retData = (DataImpl)new OmmErrorImpl();
	        	 GlobalPool._ommErrorPool .updatePool(retData);
	        }
	        break;
		}
		
		return retData;
	}

	static ByteBuffer asByteBuffer(Buffer inBuffer)
	{
		int rsslBufferDataLength = inBuffer.length();
		ByteBuffer byteBuffer = GlobalPool.acquireByteBuffer(rsslBufferDataLength);
		
		ByteBuffer rsslByteBuf = inBuffer.data();
		int limit = rsslBufferDataLength + inBuffer.position();
		for (int index = inBuffer.position(); index < limit; ++index)
			byteBuffer.put(rsslByteBuf.get(index));
		
		return byteBuffer;
	}
}