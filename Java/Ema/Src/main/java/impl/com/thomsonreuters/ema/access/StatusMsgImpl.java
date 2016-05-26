///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class StatusMsgImpl extends MsgImpl implements StatusMsg
{
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
	
	@Override
	public StatusMsg clear()
	{
		msgClear();
		return this;
	}
	
	@Override
	public boolean hasState()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkHasState();
	}
	
	@Override
	public OmmState state()
	{
		if (!hasState())
			throw ommIUExcept().message("Attempt to state() while it is NOT set.");
		
		if (_stateSet) return _state;
		
		_state.decode(((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state());
		_stateSet = true;
		
		return _state;
	}

	@Override
	public boolean hasItemGroup()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkHasGroupId();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public ByteBuffer itemGroup()
	{
		if (!hasItemGroup())
			throw ommIUExcept().message("Attempt to itemGroup() while it is NOT set.");
		
		_itemGroup = Utilities.copyFromPool( ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).groupId(), _itemGroup, _objManager);
		return _itemGroup;
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.");

		_permissionData = Utilities.copyFromPool( ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean clearCache()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkClearCache();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkPrivateStream();
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
		if (hasServiceName())
			throw ommIUExcept().message("Attempt to set serviceId while service name is already set.");
		
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
		((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasState();
		
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state().streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state().dataState(dataState) || 
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state().code(statusCode) || 
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state().text().data(statusText))
		{
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).flags( ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).flags() & ~com.thomsonreuters.upa.codec.StatusMsgFlags.HAS_STATE );
					
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
										.append( streamState ).append( " / " )
										.append( dataState ).append( " / " )
										.append( statusCode ).append( " / " )
										.append( statusCode ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state().text().data(statusText);

		return this;
	}

	@Override
	public StatusMsg itemGroup(ByteBuffer itemGroup)
	{
		if (itemGroup == null)
			throw ommIUExcept().message("Passed in itemGroup is null");
		
		((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasGroupId();
		Utilities.copy(itemGroup, ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).groupId());

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
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyClearCache();
			
		return this;
	}

	@Override
	public StatusMsg privateStream(boolean privateStream)
	{
		if (privateStream)
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyPrivateStream();
			
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

			Utilities.addIndent(_toString, indent, true).append("PayloadEnd");
			indent--;
		}
				
		Utilities.addIndent(_toString, indent, true).append("StatusMsgEnd\n");

		return _toString.toString();
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