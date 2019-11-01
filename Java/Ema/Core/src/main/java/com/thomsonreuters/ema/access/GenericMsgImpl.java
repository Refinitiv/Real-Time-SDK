///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018-2019. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.GenericMsgFlags;

class GenericMsgImpl extends MsgImpl implements GenericMsg
{
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
		super(DataTypes.GENERIC_MSG, new EmaObjectManager());
		
		_objManager.initialize();
		
		MsgImpl.cloneBufferToMsg(this, (MsgImpl)other, "com.thomsonreuters.ema.access.GenericMsgImpl.GenericMsgImpl(GenericMsg other)");

		// Set the decoded values from the clone buffer to the encoder
		if(!hasMsgKey() && other.hasMsgKey())
			cloneMsgKey((MsgImpl)other, _rsslMsg.msgKey(), _rsslMsg.flags(), "com.thomsonreuters.ema.access.GenericMsgImpl.GenericMsgImpl(GenericMsg other)");

		if (hasMsgKey() || other.hasMsgKey())
		{
			if (hasName())
				name(name());

			if (hasNameType())
				nameType(nameType());

			if (hasServiceId())
				serviceId(serviceId());
			
			if (hasId())
				id(id());

			if (hasFilter())
				filter(filter());

			if (attrib().dataType() != DataTypes.NO_DATA)
				attrib(attrib().data());
		}

		domainType(domainType());

		if (hasExtendedHeader())
			extendedHeader(extendedHeader());

		payload(other.payload().data());
		
		decodeCloneAttribPayload((MsgImpl)other);
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
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasSecondarySeqNum()
	{
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).checkHasSecondarySeqNum();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).seqNum();
	}

	@Override
	public long secondarySeqNum()
	{
		if (!hasSecondarySeqNum())
			throw ommIUExcept().message("Attempt to secondarySeqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).secondarySeqNum();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw ommIUExcept().message("Attempt to partNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).partNum();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		_permissionData = Utilities.copyFromPool( ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public boolean complete()
	{
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).checkMessageComplete();
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
		
		((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasSecondarySeqNum();
		((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).secondarySeqNum(secondarySeqNum);
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
		 int flags = 	((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).flags();	
		 if (complete)
			 flags |= GenericMsgFlags.MESSAGE_COMPLETE;
		 else
			 flags &= ~GenericMsgFlags.MESSAGE_COMPLETE;
		 
		 ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).flags(flags);
		 
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
				
		Utilities.addIndent(_toString, indent, true).append("GenericMsgEnd\n");

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
	
	com.thomsonreuters.upa.codec.GenericMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.thomsonreuters.upa.codec.GenericMsg)(_rsslMsg) : null);
	}
}