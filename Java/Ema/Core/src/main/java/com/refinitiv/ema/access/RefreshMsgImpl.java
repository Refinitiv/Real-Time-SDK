///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

class RefreshMsgImpl extends MsgImpl implements RefreshMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = RefreshMsgImpl.class.getCanonicalName() + ".RefreshMsgImpl(RefreshMsg other)";
	private  byte[] defaultGroupId = { 0, 0};
	private OmmStateImpl 	_state = new OmmStateImpl();
	private OmmQosImpl 		_qos;
	private boolean 		_stateSet;
	private boolean 		_qosSet;
	
	RefreshMsgImpl()
	{
		super(DataTypes.REFRESH_MSG, null);
		initialEncoding();
	}

	RefreshMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.REFRESH_MSG, objManager);
	}
	
	RefreshMsgImpl(RefreshMsg other)
	{
		super((MsgImpl)other, CLONE_CONSTRUCTOR_NAME);

		if(other.hasMsgKey())
		{
			if (other.hasName())
				name(other.name());

			if (other.hasNameType())
				nameType(other.nameType());

			if (other.hasServiceId())
				serviceId(other.serviceId());

			if (other.hasId())
				id(other.id());

			if (other.hasFilter())
				filter(other.filter());

			if(other.attrib().dataType() != DataTypes.NO_DATA) {
				_rsslMsg.msgKey().encodedAttrib(CodecFactory.createBuffer());
				attrib(other.attrib().data());
				decodeAttribPayload();
			}
		}
		
		domainType(other.domainType());

		streamId(other.streamId());

		if (other.hasExtendedHeader()) {
			_rsslMsg.extendedHeader(CodecFactory.createBuffer());
			extendedHeader(other.extendedHeader());
		}
		
		if (other.hasQos())
			qos(other.qos().timeliness(), other.qos().rate());

		if (other.hasServiceName())
			serviceName(other.serviceName());

		if (other.hasSeqNum())
			seqNum(other.seqNum());

		if (other.hasPermissionData()) {
			((com.refinitiv.eta.codec.RefreshMsg) _rsslMsg).permData(CodecFactory.createBuffer());
			permissionData(other.permissionData());
		}
		
		if (other.hasPartNum())
			partNum(other.partNum());

		if (other.hasPublisherId())
			publisherId(other.publisherIdUserId(), other.publisherIdUserAddress());
		
		state(other.state().streamState(), other.state().dataState(), other.state().statusCode(), other.state().statusText());

		((com.refinitiv.eta.codec.RefreshMsg) _rsslMsg).groupId(CodecFactory.createBuffer());
		itemGroup(other.itemGroup());
		
		solicited(other.solicited());
		
		complete(other.complete());
		
		clearCache(other.clearCache());
		
		privateStream(other.privateStream());
		
		doNotCache(other.doNotCache());

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}
	
	@Override
	public RefreshMsg clear()
	{
		msgClear();
		initialEncoding();
		return this;
	}

	@Override
	public boolean hasQos()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkHasQos(); 
	}
	
	@Override
	public boolean hasSeqNum()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public OmmState state()
	{
		if (_stateSet) 	return _state;
		
		_state.decode(((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state());
		_stateSet = true;
		
		return _state;
	}

	@Override
	public OmmQos qos()
	{
		if (!hasQos())
			throw ommIUExcept().message("Attempt to qos() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		if (_qosSet) return _qos;
		
		if (_qos == null)
			_qos = new OmmQosImpl();
		
		_qos.decode(((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).qos());
		_qosSet = true;
		
		return _qos;
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).seqNum();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw ommIUExcept().message("Attempt to partNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).partNum();
	}

	@Override
	public ByteBuffer itemGroup()
	{
		_itemGroup = Utilities.copyFromPool( ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).groupId(), _itemGroup, _objManager);
		return _itemGroup;
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		_permissionData = Utilities.copyFromPool( ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean solicited()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkSolicited();
	}

	@Override
	public boolean doNotCache()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkDoNotCache();
	}

	@Override
	public boolean complete()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkRefreshComplete();
	}

	@Override
	public boolean clearCache()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkClearCache();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).checkPrivateStream();
	}
	
	@Override
	public RefreshMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public RefreshMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public RefreshMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public RefreshMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public RefreshMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public RefreshMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public RefreshMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public RefreshMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public RefreshMsg qos(int timeliness, int rate)
	{
		((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).applyHasQos();
		Utilities.toRsslQos(rate, timeliness, ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).qos());
		
		return this;
	}

	@Override
	public RefreshMsg state(int streamState, int dataState, int statusCode, String statusText)
	{
		if (CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().dataState(dataState) || 
				CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().code(statusCode) || 
				CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().text().data(statusText))
		{
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
										.append( streamState ).append( " / " )
										.append( dataState ).append( " / " )
										.append( statusCode ).append( " / " )
										.append( statusCode ).append( "'." ).toString();
			throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}

		return this;
	}
	
	@Override
	public RefreshMsg state(int streamState, int dataState)
	{
		state(streamState, dataState, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
		
		return this;
	}

	@Override
	public RefreshMsg state(int streamState, int dataState, int statusCode)
	{
		state(streamState, dataState, statusCode, DataImpl.EMPTY_STRING);
		
		return this;
	}

	@Override
	public RefreshMsg seqNum(long seqNum)
	{
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public RefreshMsg partNum(int partNum)
	{
		msgPartNum(partNum);
		return this;
	}

	@Override
	public RefreshMsg itemGroup(ByteBuffer itemGroup)
	{
		if (itemGroup == null)
			throw ommIUExcept().message("Passed in itemGroup is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		Utilities.copy(itemGroup, ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).groupId());
		return this;
	}

	@Override
	public RefreshMsg permissionData(ByteBuffer permissionData)
	{
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public RefreshMsg publisherId(long userId, long userAddress)
	{
		msgPublisherId(userId, userAddress);
		return this;
	}

	@Override
	public RefreshMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public RefreshMsg payload(ComplexType data)
	{
		msgPayload(data);
		return this;
	}

	@Override
	public RefreshMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}
	
	@Override
	public RefreshMsg solicited(boolean solicited)
	{
		if (solicited)
			((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).applySolicited();
	
		return this;
	}

	@Override
	public RefreshMsg doNotCache(boolean doNotCache)
	{
		if (doNotCache)
			((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).applyDoNotCache();
		
		return this;
	}

	@Override
	public RefreshMsg clearCache(boolean clearCache)
	{
		if (clearCache)
			((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).applyClearCache();
		
		return this;
	}

	@Override
	public RefreshMsg complete(boolean complete)
	{
		if (complete)
			((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).applyRefreshComplete();

		return this;
	}

	@Override
	public RefreshMsg privateStream(boolean privateStream)
	{
		if (privateStream)
			((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).applyPrivateStream();
		
		return this;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		if ( _objManager == null )
			return "\nDecoding of just encoded object in the same application is not supported\n";
		
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

				Utilities.addIndent(_toString, indent, false).append("AttribEnd");
				indent--;
			}
		}
			
		if (hasExtendedHeader())
		{
			indent++;
			Utilities.addIndent(_toString, indent, true).append("ExtendedHeader\n");

			indent++;
			Utilities.addIndent(_toString, indent);
			Utilities.asHexString(_toString, extendedHeader());
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
	
	com.refinitiv.eta.codec.RefreshMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.refinitiv.eta.codec.RefreshMsg)(_rsslMsg) : null);
	}

	@Override
	void decode(com.refinitiv.eta.codec.Msg rsslMsg, int majVer, int minVer,
			com.refinitiv.eta.codec.DataDictionary rsslDictionary)
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
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object obj)
	{
		_rsslNestedMsg.clear();

		_rsslMsg = _rsslNestedMsg;
		
		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

		_stateSet = false;

		_qosSet = false;

		_rsslDecodeIter.clear();

		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
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
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
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
	
	void initialEncoding()
	{
		 ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		 ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		 ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).state().code(com.refinitiv.eta.codec.StateCodes.NONE);

		 ((com.refinitiv.eta.codec.RefreshMsg)_rsslMsg).groupId().data( ByteBuffer.wrap(defaultGroupId));
	}
}
