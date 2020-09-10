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
import com.rtsdk.eta.codec.GenericMsgFlags;

class GenericMsgImpl extends MsgImpl implements GenericMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = GenericMsgImpl.class.getCanonicalName() + ".GenericMsgImpl(GenericMsg other)";

	GenericMsgImpl()
	{
		super(DataTypes.GENERIC_MSG, null);
	}

	GenericMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.GENERIC_MSG, objManager);
	}
	
	GenericMsgImpl(GenericMsg other)
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

			if (other.attrib().dataType() != DataTypes.NO_DATA) {
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

		if (other.hasPermissionData()) {
			((com.rtsdk.eta.codec.RefreshMsg) _rsslMsg).permData(CodecFactory.createBuffer());
			permissionData(other.permissionData());
		}

		complete(other.complete());

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}
	
	@Override
	public GenericMsg clear()
	{
		msgClear();
		return this;
	}
	
	@Override
	public boolean hasSeqNum()
	{
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasSecondarySeqNum()
	{
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).checkHasSecondarySeqNum();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).seqNum();
	}

	@Override
	public long secondarySeqNum()
	{
		if (!hasSecondarySeqNum())
			throw ommIUExcept().message("Attempt to secondarySeqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).secondarySeqNum();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw ommIUExcept().message("Attempt to partNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).partNum();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		_permissionData = Utilities.copyFromPool( ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public boolean complete()
	{
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).checkMessageComplete();
	}

	@Override
	public boolean isProviderDriven() {
		return ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).checkIsProviderDriven();
	}

	@Override
	public GenericMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public GenericMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public GenericMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public GenericMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public GenericMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public GenericMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public GenericMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public GenericMsg seqNum(long seqNum)
	{
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public GenericMsg secondarySeqNum(long secondarySeqNum)
	{
		if (secondarySeqNum < 0 || secondarySeqNum > 4294967295L)
			throw ommOORExcept().message("Passed in seqNum is out of range. [0 - 4294967295]");
		
		((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).applyHasSecondarySeqNum();
		((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).secondarySeqNum(secondarySeqNum);
		return this;
	}

	@Override
	public GenericMsg partNum(int partNum)
	{
		msgPartNum(partNum);
		return this;
	}

	@Override
	public GenericMsg permissionData(ByteBuffer permissionData)
	{
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public GenericMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public GenericMsg payload(ComplexType data)
	{
		msgPayload(data);
		return this;
	}

	@Override
	public GenericMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}

	@Override
	public GenericMsg complete(boolean complete)
	{
		 int flags = 	((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).flags();	
		 if (complete)
			 flags |= GenericMsgFlags.MESSAGE_COMPLETE;
		 else
			 flags &= ~GenericMsgFlags.MESSAGE_COMPLETE;
		 
		 ((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).flags(flags);
		 
		return this;
	}

	@Override
	public GenericMsg providerDriven(boolean providerDriven)
	{
		int flags = 	((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).flags();
		if (providerDriven)
			flags |= GenericMsgFlags.PROVIDER_DRIVEN;
		else
			flags &= ~GenericMsgFlags.PROVIDER_DRIVEN;

		((com.rtsdk.eta.codec.GenericMsg)_rsslMsg).flags(flags);

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
		Utilities.addIndent(_toString, indent++).append("GenericMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													  .append(streamId())
													  .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													  .append(Utilities.rdmDomainAsString(domainType()))
													  .append("\"");	

		if (complete())
			Utilities.addIndent(_toString, indent, true).append("MessageComplete");
		
		if (hasSeqNum())
			Utilities.addIndent(_toString, indent, true).append("seqNum=\"")
														  .append(seqNum()).append("\"");

		if (hasSecondarySeqNum())
			Utilities.addIndent(_toString, indent, true).append("secondarySeqNum=\"")
			  											  .append(secondarySeqNum()).append("\"");

		if (hasPartNum())
			Utilities.addIndent(_toString, indent, true).append("partNum=\"")
			  											  .append(partNum()).append("\"");

		if (hasPermissionData())
		{
			Utilities.addIndent(_toString, indent, true).append("permissionData=\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}
		if (isProviderDriven())
			Utilities.addIndent(_toString, indent, true).append("ProviderDriven");
		
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
				
		Utilities.addIndent(_toString, indent, true).append("GenericMsgEnd\n");

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

		decodeAttribPayload();
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
	
	com.rtsdk.eta.codec.GenericMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.rtsdk.eta.codec.GenericMsg)(_rsslMsg) : null);
	}
}
