///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.Codec;

class StatusMsgImpl extends MsgImpl implements StatusMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = StatusMsgImpl.class.getCanonicalName() + ".StatusMsgImpl(StatusMsg other)";
	private OmmStateImpl _state = new OmmStateImpl();
	private boolean 	 _stateSet;
	
	StatusMsgImpl()
	{
		super(DataTypes.STATUS_MSG, null);
	}

	StatusMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.STATUS_MSG, objManager);
	}
	
	StatusMsgImpl(StatusMsg other)
	{
		super((MsgImpl)other, CLONE_CONSTRUCTOR_NAME);

		if (other.hasMsgKey())
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

		if (other.hasExtendedHeader()) {
			_rsslMsg.extendedHeader(CodecFactory.createBuffer());
			extendedHeader(other.extendedHeader());
		}

		if (other.hasServiceName())
			serviceName(other.serviceName());

		if (other.hasPermissionData()) {
			((com.refinitiv.eta.codec.RefreshMsg) _rsslMsg).permData(CodecFactory.createBuffer());
			permissionData(other.permissionData());
		}

		if (other.hasItemGroup()) {
			((com.refinitiv.eta.codec.RefreshMsg) _rsslMsg).groupId(CodecFactory.createBuffer());
			itemGroup(other.itemGroup());
		}

		if(other.hasState())
			state(other.state().streamState(), other.state().dataState(), other.state().statusCode(), other.state().statusText());

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}
	
	@Override
	public StatusMsg clear()
	{
		msgClear();
		return this;
	}
	
	@Override
	public boolean hasState()
	{
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).checkHasState();
	}
	
	@Override
	public OmmState state()
	{
		if (!hasState())
			throw ommIUExcept().message("Attempt to state() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		if (_stateSet) return _state;
		
		_state.decode(((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).state());
		_stateSet = true;
		
		return _state;
	}

	@Override
	public boolean hasItemGroup()
	{
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).checkHasGroupId();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public ByteBuffer itemGroup()
	{
		if (!hasItemGroup())
			throw ommIUExcept().message("Attempt to itemGroup() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		_itemGroup = Utilities.copyFromPool( ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).groupId(), _itemGroup, _objManager);
		return _itemGroup;
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		_permissionData = Utilities.copyFromPool( ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean clearCache()
	{
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).checkClearCache();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).checkPrivateStream();
	}

	@Override
	public StatusMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public StatusMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public StatusMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public StatusMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public StatusMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public StatusMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public StatusMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public StatusMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public StatusMsg state(int streamState, int dataState)
	{
		state(streamState, dataState, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
		
		return this;
	}

	@Override
	public StatusMsg state(int streamState, int dataState, int statusCode)
	{
		state(streamState, dataState, statusCode, DataImpl.EMPTY_STRING);
		
		return this;
	}

	@Override
	public StatusMsg state(int streamState, int dataState, int statusCode,
			String statusText)
	{
		((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).applyHasState();
		
		if (CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).state().streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).state().dataState(dataState) || 
				CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).state().code(statusCode) || 
				CodecReturnCodes.SUCCESS != ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).state().text().data(statusText))
		{
			((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).flags( ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).flags() & ~com.refinitiv.eta.codec.StatusMsgFlags.HAS_STATE );
					
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
										.append( streamState ).append( " / " )
										.append( dataState ).append( " / " )
										.append( statusCode ).append( " / " )
										.append( statusCode ).append( "'." ).toString();
			throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		
		((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).state().text().data(statusText);

		return this;
	}

	@Override
	public StatusMsg itemGroup(ByteBuffer itemGroup)
	{
		if (itemGroup == null)
			throw ommIUExcept().message("Passed in itemGroup is null", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).applyHasGroupId();
		Utilities.copy(itemGroup, ((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).groupId());

		return this;
	}

	@Override
	public StatusMsg permissionData(ByteBuffer permissionData)
	{
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public StatusMsg publisherId(long userId, long userAddress)
	{
		msgPublisherId(userId, userAddress);
		return this;
	}

	@Override
	public StatusMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public StatusMsg payload(ComplexType data)
	{
		msgPayload(data);
		return this;
	}

	@Override
	public StatusMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}

	@Override
	public StatusMsg clearCache(boolean clearCache)
	{
		if (clearCache)
			((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).applyClearCache();
			
		return this;
	}

	@Override
	public StatusMsg privateStream(boolean privateStream)
	{
		if (privateStream)
			((com.refinitiv.eta.codec.StatusMsg)_rsslMsg).applyPrivateStream();
			
		return this;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}

	@Override
	public String toString (DataDictionary dictionary)
	{
		if (!dictionary.isFieldDictionaryLoaded() || !dictionary.isEnumTypeDefLoaded())
			return "\nDictionary is not loaded.\n";

		if (_objManager == null)
		{
			_objManager = new EmaObjectManager();
			_objManager.initialize(((DataImpl)this).dataType());
		}

		StatusMsg statusMsg = new StatusMsgImpl(_objManager);

		((MsgImpl) statusMsg).decode(((DataImpl)this).encodedData(), Codec.majorVersion(), Codec.minorVersion(), ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);
		if (_errorCode != ErrorCode.NO_ERROR)
		{
			return "\nFailed to decode StatusMsg with error: " + ((MsgImpl) statusMsg).errorString() + "\n";
		}

		return statusMsg.toString();
	}

	com.refinitiv.eta.codec.StatusMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.refinitiv.eta.codec.StatusMsg)(_rsslMsg) : null);
	}
	
	String toString(int indent)
	{
		if ( _objManager == null )
			return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";
		
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent++).append("StatusMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													 .append(streamId())
													 .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													 .append(Utilities.rdmDomainAsString(domainType()))
													 .append("\"");

		if (privateStream())
			Utilities.addIndent(_toString, indent, true).append("privateStream");

		if (hasState())
			Utilities.addIndent(_toString, indent, true).append("state=\"")
													 .append(state().toString())
													 .append("\"");

		if (hasItemGroup())
		{
			Utilities.addIndent(_toString, indent, true).append("itemGroup=\"");
			Utilities.asHexString(_toString, itemGroup()).append("\"");
		}
		
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

			Utilities.addIndent(_toString, indent, true).append("PayloadEnd");
			indent--;
		}
				
		Utilities.addIndent(_toString, indent, true).append("StatusMsgEnd\n");

		return _toString.toString();
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
