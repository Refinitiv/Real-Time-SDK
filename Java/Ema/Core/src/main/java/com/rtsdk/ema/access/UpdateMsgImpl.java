///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.nio.ByteBuffer;

import com.rtsdk.ema.access.DataType.DataTypes;
import com.rtsdk.ema.access.OmmError.ErrorCode;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;

class UpdateMsgImpl extends MsgImpl implements UpdateMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = UpdateMsgImpl.class.getCanonicalName() + ".UpdateMsgImpl(UpdateMsg other)";
	UpdateMsgImpl()
	{
		super(DataTypes.UPDATE_MSG, null);
	}
	
	UpdateMsgImpl(UpdateMsg other) {
		super((MsgImpl) other, CLONE_CONSTRUCTOR_NAME);

		if (other.hasMsgKey()) {
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

			if (other.attrib().dataType() != DataTypes.NO_DATA) {
				_rsslMsg.msgKey().encodedAttrib(CodecFactory.createBuffer());
				attrib(other.attrib().data());
				decodeAttribPayload();
			}
		}

		domainType(domainType());

		if (other.hasExtendedHeader()) {
			_rsslMsg.extendedHeader(CodecFactory.createBuffer());
			extendedHeader(other.extendedHeader());
		}

		if (other.hasServiceName())
			serviceName(other.serviceName());

		if (other.hasSeqNum())
			seqNum(other.seqNum());

		if (other.hasPermissionData()) {
			((com.rtsdk.eta.codec.UpdateMsg) _rsslMsg).permData(CodecFactory.createBuffer());
			permissionData(other.permissionData());
		}

		if (other.hasConflated())
			conflated(other.conflatedCount(), other.conflatedTime());

		if (other.hasPublisherId())
			publisherId(publisherIdUserId(), publisherIdUserAddress());

		updateTypeNum(other.updateTypeNum());

		doNotCache(other.doNotCache());

		doNotConflate(other.doNotConflate());

		doNotRipple(other.doNotRipple());

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}

	UpdateMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.UPDATE_MSG, objManager);
	}
	
	@Override
	public UpdateMsg clear()
	{
		msgClear();
		return this;
	}

	@Override
	public boolean hasSeqNum()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasConflated()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkHasConfInfo();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public int updateTypeNum()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).updateType();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).seqNum();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		_permissionData = Utilities.copyFromPool( ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public int conflatedTime()
	{
		if (!hasConflated())
			throw ommIUExcept().message("Attempt to conflatedTime() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).conflationTime();
	}

	@Override
	public int conflatedCount()
	{
		if (!hasConflated())
			throw ommIUExcept().message("Attempt to conflatedCount() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).conflationCount();
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean doNotCache()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkDoNotCache();
	}

	@Override
	public boolean doNotConflate()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkDoNotConflate();
	}

	@Override
	public boolean doNotRipple()
	{
		return ((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).checkDoNotRipple();
	}

	@Override
	public UpdateMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public UpdateMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public UpdateMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public UpdateMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public UpdateMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public UpdateMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public UpdateMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public UpdateMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public UpdateMsg updateTypeNum(int updateTypeNum)
	{
		if (updateTypeNum < 0 || updateTypeNum > 255)
			throw ommOORExcept().message("Passed in updateTypeNum is out of range.  [0 - 255]");
			
		((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).updateType(updateTypeNum);
		return this;
	}

	@Override
	public UpdateMsg seqNum(long seqNum)
	{
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public UpdateMsg permissionData(ByteBuffer permissionData)
	{
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public UpdateMsg conflated(int count, int time)
	{
		if (count < 0 || count > 32767 || time < 0 || time > 65535)
			throw ommOORExcept().message("Passed in count or time is out of range. [0 - 32767] [0 - 65535]");
			
		((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).applyHasConfInfo();
		((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).conflationCount(count);
		((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).conflationTime(time);
			
		return this;
	}

	@Override
	public UpdateMsg publisherId(long userId, long userAddress)
	{
		msgPublisherId(userId, userAddress);
		return this;
	}

	@Override
	public UpdateMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public UpdateMsg payload(ComplexType data)
	{
		msgPayload(data);
		return this;
	}

	@Override
	public UpdateMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}

	@Override
	public UpdateMsg doNotCache(boolean doNotCache)
	{
		if (doNotCache)
			((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).applyDoNotCache();
	
		return this;
	}

	@Override
	public UpdateMsg doNotConflate(boolean doNotConflate)
	{
		if (doNotConflate)
			((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).applyDoNotConflate();
			
		return this;
	}

	@Override
	public UpdateMsg doNotRipple(boolean doNotRipple)
	{
		if (doNotRipple)
			((com.rtsdk.eta.codec.UpdateMsg)_rsslMsg).applyDoNotRipple();
			
		return this;
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	com.rtsdk.eta.codec.UpdateMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.rtsdk.eta.codec.UpdateMsg)(_rsslMsg) : null);
	}
	
	String toString(int indent)
	{
		if ( _objManager == null )
			return "\nDecoding of just encoded object in the same application is not supported\n";
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent++).append("UpdateMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													 .append(streamId())
													 .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													 .append(Utilities.rdmDomainAsString(domainType()))
													 .append("\"");
		Utilities.addIndent(_toString, indent, true).append("updateTypeNum=\"")
		 											 .append(updateTypeNum())
		 											 .append("\"");

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
				
		Utilities.addIndent(_toString, indent, true).append("UpdateMsgEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(com.rtsdk.eta.codec.Msg rsslMsg, int majVer, int minVer,
			com.rtsdk.eta.codec.DataDictionary rsslDictionary)
	{
		_rsslMsg = rsslMsg;

		_rsslBuffer = _rsslMsg.encodedMsgBuffer();
				
		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

		decodeAttribPayload();
	}
	
	public boolean rsslBuffer() {
		return _rsslBuffer.length() != 0; 
	}

	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.rtsdk.eta.codec.DataDictionary rsslDictionary, Object obj)
	{
		_rsslNestedMsg.clear();

		_rsslMsg = _rsslNestedMsg;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

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
