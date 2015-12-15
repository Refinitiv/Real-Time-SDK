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
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class StatusMsgImpl extends MsgImpl implements StatusMsg
{
	private OmmStateImpl _state;
	private boolean 	 _stateSet;
	
	public StatusMsgImpl()
	{
		super(DataTypes.STATUS_MSG);
	}
	
	@Override
	public boolean hasState()
	{
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).checkHasState();
	}
	
	@Override
	public OmmState state()
	{
		if (!hasState())//TODO set as what is the default value or throw exception
			throw oommIUExcept().message("Attempt to state() while it is NOT set.");
		
		if (_stateSet) 
			return _state;
		
		_stateSet = true;
		
		if (_state == null)
			_state = new OmmStateImpl();
		
		if (hasState())
		{
			_state.decode(((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).state());
			return _state;
		}
		else //TODO set as what is the default value or throw exception
			throw oommIUExcept().message("Attempt to state() while it is NOT set.");

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
			throw oommIUExcept().message("Attempt to itemGroup() while it is NOT set.");
		
		GlobalPool.releaseByteBuffer(_itemGroup);
		_itemGroup = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).groupId());
		
		return _itemGroup;
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");

		GlobalPool.releaseByteBuffer(_permissionData);
		_permissionData = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).permData());
			
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw oommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw oommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.");
		
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
	public StatusMsg clear()
	{
		statusMsgClear();
		return this;
	}

	@Override
	public StatusMsg streamId(int streamId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg domainType(int domainType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg name(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg nameType(int nameType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg serviceName(String serviceName)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg serviceId(int serviceId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg id(int id)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg filter(long filter)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg state(int streamState, int dataState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg state(int streamState, int dataState, int statusCode)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg state(int streamState, int dataState, int statusCode,
			String statusText)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg itemGroup(ByteBuffer itemGroup)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg permissionData(ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg publisherId(long userId, long userAddress)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg attrib(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg payload(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg extendedHeader(ByteBuffer buffer)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg clearCache(boolean clearCache)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public StatusMsg privateStream(boolean privateStream)
	{
		// TODO Auto-generated method stub
		return null;
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