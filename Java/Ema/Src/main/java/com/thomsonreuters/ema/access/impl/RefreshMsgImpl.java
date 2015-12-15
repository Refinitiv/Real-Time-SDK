///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class RefreshMsgImpl extends MsgImpl implements RefreshMsg
{
	private OmmStateImpl 	_state;
	private OmmQosImpl 		_qos;
	private boolean 		_stateSet;
	private boolean 		_qosSet;
	
	public RefreshMsgImpl()
	{
		super(DataTypes.REFRESH_MSG);
	}

	@Override
	public RefreshMsg clear()
	{
		//TODO
		refreshMsgClear();
		return this;
	}

	@Override
	public boolean hasQos()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasQos(); 
	}
	
	@Override
	public boolean hasSeqNum()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public OmmState state()
	{
		if (_stateSet) 
			return _state;
		
		if (_state == null)
			_state = new OmmStateImpl();
		
		_state.decode(((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state());
		
		_stateSet = true;
		
		return _state;
	}

	@Override
	public OmmQos qos()
	{
		if (!hasQos())
			throw oommIUExcept().message("Attempt to qos() while it is NOT set.");
		
		if (_qosSet) 
			return _qos;
		
		if (_qos == null)
			_qos = new OmmQosImpl();
		
		_qos.decode(((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).qos());
		
		_qosSet = true;
		
		return _qos;
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw oommIUExcept().message("Attempt to seqNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).seqNum();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw oommIUExcept().message("Attempt to partNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).partNum();
	}

	@Override
	public ByteBuffer itemGroup()
	{
		GlobalPool.releaseByteBuffer(_itemGroup);
		_itemGroup = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).groupId());
		
		return _itemGroup;
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");

		GlobalPool.releaseByteBuffer(_permissionData);
		_permissionData = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).permData());
			
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw oommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw oommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean solicited()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkSolicited();
	}

	@Override
	public boolean doNotCache()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkDoNotCache();
	}

	@Override
	public boolean complete()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkRefreshComplete();
	}

	@Override
	public boolean clearCache()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkClearCache();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkPrivateStream();
	}
	
	@Override
	public RefreshMsg streamId(int streamId)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (streamId < -2147483648 || streamId > 2147483647)
			throw ommOORExcept().message("Passed in streamId is out of range. [(-2147483648) - 2147483647]");
				
		msgStreamId(streamId);
		return this;
	}

	@Override
	public RefreshMsg domainType(int domainType)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (domainType < 0 || domainType > 255)
			throw ommUDTExcept().message(domainType,"Passed in domainType is out of range. [0 - 255]");
	
		msgDomainType(domainType);
		return this;
	}

	@Override
	public RefreshMsg name(String name)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (name == null)
			throw ommOORExcept().message("Passed in name is null");
		
		msgName(name);
		return this;
	}

	@Override
	public RefreshMsg nameType(int nameType)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (nameType < 0 || nameType > 255)
			throw ommOORExcept().message("Passed in nameType is out of range. [0 - 255]");
		
		msgNameType(nameType);
		return this;
	}

	@Override
	public RefreshMsg serviceName(String serviceName)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (serviceName == null)
			throw ommOORExcept().message("Passed in serviceName is null");
		
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public RefreshMsg serviceId(int serviceId)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (serviceId < 0 || serviceId > 65535)
			throw ommOORExcept().message("Passed in serviceId is out of range. [0 - 65535]");
		
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public RefreshMsg id(int id)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (id < -2147483648 || id > 2147483647)
			throw ommOORExcept().message("Passed in id is out of range. [(-2147483648) - 2147483647]");
		
		msgId(id);
		return this;
	}

	@Override
	public RefreshMsg filter(long filter)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (filter < 0 || filter > 4294967296L)
			throw ommOORExcept().message("Passed in filter is out of range. [0 - 4294967296]");

		msgFilter(filter);
		return this;
	}

	@Override
	public RefreshMsg qos(int timeliness, int rate)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (!(timeliness >= 0 && timeliness <= 7))
			throw ommOORExcept().message("Passed timeliness in qos is out of range. [0 - 7]");
		 
		if (!(rate >= 0 && rate <= 15))
			throw ommOORExcept().message("Passed rate in qos is out of range. [0 - 15]");
		 
		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).applyHasQos();
		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).qos().rate(rate);
		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).qos().timeliness(timeliness);
		
		return this;
	}

	@Override
	public RefreshMsg state(int streamState, int dataState, int statusCode,	String statusText)
	{
		if (_initialEncoding)
			intialEncoding();
		
		//TODO emacpp has statusCode (actually has 266) more than 255 which is defined on wire  
		if (streamState < 0 || streamState > 31 || dataState < 0 || dataState > 7 || statusCode < 0 || statusCode > 255)
			throw ommOORExcept().message("Passed in streamState, dataState or statusCode is out of range. [0 - 31],[0 - 7],[0 - 255]");
		
		msgState(streamState, dataState, statusCode, statusText);
		return this;
	}
	
	@Override
	public RefreshMsg state(int streamState, int dataState)
	{
		if (_initialEncoding)
			intialEncoding();
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public RefreshMsg state(int streamState, int dataState, int statusCode)
	{
		if (_initialEncoding)
			intialEncoding();
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public RefreshMsg seqNum(long seqNum)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (seqNum < 0 || seqNum > 4294967296L)
			throw ommOORExcept().message("Passed in seqNum is out of range. [0 - 4294967296]");
		
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public RefreshMsg partNum(int partNum)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (partNum < 0 || partNum > 32767)
			throw ommOORExcept().message("Passed in partNum is out of range. [0 - 32767]");
		
		msgPartNum(partNum);
		return this;
	}

	@Override
	public RefreshMsg itemGroup(ByteBuffer itemGroup)
	{
		if (_initialEncoding)
			intialEncoding();
		
		//TODO or throw OmmException
		if (itemGroup == null)
			throw new NullPointerException("Passed in itemGroup must be non-null.");
		
		msgItemGroup(itemGroup);
		return this;
	}

	@Override
	public RefreshMsg permissionData(ByteBuffer permissionData)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (permissionData == null)
			throw new NullPointerException("Passed in permissionData must be non-null.");
		
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public RefreshMsg publisherId(long userId, long userAddress)
	{
		if (_initialEncoding)
			intialEncoding();
		
		msgPublisherId(userId, userAddress);
		return this;
	}

	@Override
	public RefreshMsg attrib(ComplexType data)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (data == null)
			throw new NullPointerException("Passed in attrib data must be non-null.");
		
		msgAttrib(data);
		return this;
	}

	@Override
	public RefreshMsg payload(ComplexType data)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (data == null)
			throw new NullPointerException("Passed in payload data must be non-null.");
		
		msgPayload(data);
		return this;
	}

	@Override
	public RefreshMsg extendedHeader(ByteBuffer buffer)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (buffer == null)
			throw new NullPointerException("Passed in extendedHeader buffer must be non-null.");
		
		if (_initialEncoding)
			intialEncoding();
		
		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).applyHasExtendedHdr();
		msgExtendedHeader(buffer);

		return this;
	}
	
	@Override
	public RefreshMsg solicited(boolean solicited)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (solicited)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).applySolicited();
	
		return this;
	}

	@Override
	public RefreshMsg doNotCache(boolean doNotCache)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (doNotCache)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).applyDoNotCache();
		
		return this;
	}

	@Override
	public RefreshMsg clearCache(boolean clearCache)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (clearCache)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).applyClearCache();
		
		return this;
	}

	@Override
	public RefreshMsg complete(boolean complete)
	{
		if (_initialEncoding)
			intialEncoding();
		
		msgComplete(complete);
		return this;
	}

	@Override
	public RefreshMsg privateStream(boolean privateStream)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (privateStream)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsgEncoded).applyPrivateStream();
		
		return this;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent++).append("RefreshMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													 .append(streamId())
													 .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													 .append(Utilities.rdmDomainAsString(domainType()))
													 .append("\"");			

		if (solicited())
			Utilities.addIndent(_toString, indent, true).append("solicited");

		if (complete())
			Utilities.addIndent(_toString, indent, true).append("RefreshComplete");

		if (privateStream())
			Utilities.addIndent(_toString, indent, true).append("privateStream");

		Utilities.addIndent(_toString, indent, true).append("state=\"")
													 .append(state().toString())
													 .append("\"");

		Utilities.addIndent(_toString, indent, true).append("itemGroup=\"");
		
		Utilities.asHexString(_toString, itemGroup()).append("\"");

		if (hasPermissionData())
		{
			Utilities.addIndent(_toString, indent, true).append("permissionData=\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}

		indent--;
		if (hasMsgKey())
		{
			indent++;
			if (hasName())
				Utilities.addIndent(_toString, indent, true).append("name=\"")
															 .append(name())
															 .append("\"");

			if (hasNameType())
				Utilities.addIndent(_toString, indent, true).append("nameType=\"")
															 .append(nameType())
															 .append("\"");

			if (hasServiceId())
				Utilities.addIndent(_toString, indent, true).append("serviceId=\"")
															 .append(serviceId())
															 .append("\"");

			if (hasServiceName())
				Utilities.addIndent(_toString, indent, true).append("serviceName=\"")
															 .append(serviceName())
															 .append("\"");

			if (hasFilter())
				Utilities.addIndent(_toString, indent, true).append("filter=\"")
															 .append(filter())
															 .append("\"");

			if (hasId())
				Utilities.addIndent(_toString, indent, true).append("id=\"")
															 .append(id())
															 .append("\"");

			indent--;

			if (hasAttrib())
			{
				indent++;
				Utilities.addIndent(_toString, indent, true).append("Attrib dataType=\"")
															 .append(DataType.asString(attribData().dataType()))
															 .append("\"\n");

				indent++;
				_toString.append(attribData().toString(indent));
				indent--;

				Utilities.addIndent(_toString, indent, true).append("AttribEnd");
				indent--;
			}
		}
			
		if (hasExtendedHeader())
		{
			indent++;
			Utilities.addIndent(_toString, indent, true).append("ExtendedHeader\n");

			indent++;
			Utilities.addIndent(_toString, indent);
			Utilities.asHexString(_toString, extendedHeader()).append("\"");
			indent--;

			Utilities.addIndent(_toString, indent, true).append("ExtendedHeaderEnd");
			indent--;
		}

		if (hasPayload())
		{
			indent++;
			Utilities.addIndent(_toString, indent, true).append("Payload dataType=\"")
														 .append(DataType.asString(payloadData().dataType()))
														 .append("\"\n");

			indent++;
			_toString.append(payloadData().toString(indent));
			indent--;

			Utilities.addIndent(_toString, indent).append("PayloadEnd");
			indent--;
		}
				
		Utilities.addIndent(_toString, indent, true).append("RefreshMsgEnd\n");

		return _toString.toString();
	}
	
	com.thomsonreuters.upa.codec.RefreshMsg rsslMsg()
	{
		return null;
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Msg rsslMsg, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary)
	{
		_rsslMsg = rsslMsg;

		_rsslBuffer = _rsslMsg.encodedMsgBuffer();
		
		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

		_stateSet = false;

		_qosSet = false;

		decodeAttribPayload();
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object obj)
	{
		_rsslNestedMsg.clear();

		_rsslMsg = _rsslNestedMsg;

		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

		_stateSet = false;

		_qosSet = false;

		_rsslDecodeIter.clear();

		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _errorCode, _rsslMinVer);
		if (CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}

		retCode = _rsslMsg.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case CodecReturnCodes.SUCCESS:
				_errorCode = ErrorCode.NO_ERROR;
				decodeAttribPayload();
				return;
			case CodecReturnCodes.ITERATOR_OVERRUN:
				_errorCode = ErrorCode.ITERATOR_OVERRUN;
				dataInstance(_attribDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(	rsslBuffer, _errorCode);
				return;
			case CodecReturnCodes.INCOMPLETE_DATA:
				_errorCode = ErrorCode.INCOMPLETE_DATA;
				dataInstance(_attribDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
				return;
			default:
				_errorCode = ErrorCode.UNKNOWN_ERROR;
				dataInstance(_attribDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
				return;
		}
	}
}