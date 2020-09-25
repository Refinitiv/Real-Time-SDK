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

class PostMsgImpl extends MsgImpl implements PostMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = PostMsgImpl.class.getCanonicalName() + ".PostMsgImpl(PostMsg other)";
	private final static String CREATE_NAME = "Create";
	private final static String DELETE_NAME = " | Delete";
	private final static String MODIFY_NAME = " | ModifyPermission";
	private final static String PREAPPEND_STRING = "PostUserRights are: ";
	
	private StringBuilder _postUserRightsString;
	
	PostMsgImpl()
	{
		super(DataTypes.POST_MSG, null);
	}

	PostMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.POST_MSG, objManager);
	}
	
	PostMsgImpl(PostMsg other)
	{
		super((MsgImpl)other, CLONE_CONSTRUCTOR_NAME);

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

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}

	@Override
	public String postUserRightsAsString()
	{
		if (_postUserRightsString == null)
			_postUserRightsString = new StringBuilder();
		else 
			_postUserRightsString.setLength(0);
		
		int postUserRights = postUserRights();
		_postUserRightsString.append(PREAPPEND_STRING);
		
		if ((postUserRights & PostUserRights.CREATE) > 0)
			_postUserRightsString.append(CREATE_NAME);

		if ((postUserRights & PostUserRights.DELETE) > 0)
			_postUserRightsString.append(DELETE_NAME);

		if ((postUserRights & PostUserRights.MODIFY_PERMISSION) > 0)
			_postUserRightsString.append(MODIFY_NAME);

		return _postUserRightsString.toString();
	}

	@Override
	public boolean hasSeqNum()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPostId()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkHasPostId();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPostUserRights()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkHasPostUserRights();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).seqNum();
	}

	@Override
	public long postId()
	{
		if (!hasPostId())
			throw ommIUExcept().message("Attempt to postId() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).postId();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw ommIUExcept().message("Attempt to partNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).partNum();
	}

	@Override
	public int postUserRights()
	{
		if (!hasPostUserRights())
			throw ommIUExcept().message("Attempt to postUserRights() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).postUserRights();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		_permissionData = Utilities.copyFromPool( ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean solicitAck()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkAck();
	}

	@Override
	public boolean complete()
	{
		return ((com.refinitiv.eta.codec.PostMsg)_rsslMsg).checkPostComplete();
	}

	@Override
	public PostMsg clear()
	{
		msgClear();
		return this;
	}
	
	@Override
	public PostMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public PostMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public PostMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public PostMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public PostMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public PostMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public PostMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public PostMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public PostMsg seqNum(long seqNum)
	{
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public PostMsg postId(long postId)
	{
		if (postId < 0 || postId > 4294967295L)
			throw ommOORExcept().message("Passed in postId is out of range. [0 - 4294967295]");
		
		((com.refinitiv.eta.codec.PostMsg)_rsslMsg).applyHasPostId();
		((com.refinitiv.eta.codec.PostMsg)_rsslMsg).postId(postId);

		return this;
	}

	@Override
	public PostMsg partNum(int partNum)
	{
		msgPartNum(partNum);
		return this;
	}

	@Override
	public PostMsg postUserRights(int postUserRights)
	{
		if (postUserRights < 0 || postUserRights > 32767)
			throw ommOORExcept().message("Passed in postUserRights is out of range. [0 - 32767]");
		
		((com.refinitiv.eta.codec.PostMsg)_rsslMsg).applyHasPostUserRights();
		((com.refinitiv.eta.codec.PostMsg)_rsslMsg).postUserRights(postUserRights);

		return this;
	}

	@Override
	public PostMsg permissionData(ByteBuffer permissionData)
	{
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public PostMsg publisherId(long userId, long userAddress)
	{
		msgPublisherId(userId, userAddress);
		return this;
	}

	@Override
	public PostMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public PostMsg payload(ComplexType data)
	{
		msgPayload(data);
		return this;
	}

	@Override
	public PostMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}

	@Override
	public PostMsg solicitAck(boolean ack)
	{
		if (ack)
			((com.refinitiv.eta.codec.PostMsg)_rsslMsg).applyAck();
	
		return this;
	}

	@Override
	public PostMsg complete(boolean complete)
	{
		if (complete)
			((com.refinitiv.eta.codec.PostMsg)_rsslMsg).applyPostComplete();

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
		Utilities.addIndent(_toString, indent++).append("PostMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													  .append(streamId())
													  .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													  .append(Utilities.rdmDomainAsString(domainType()))
													  .append("\"");	

		if (solicitAck())
			Utilities.addIndent(_toString, indent, true).append("Ack Requested");

		if (complete())
			Utilities.addIndent(_toString, indent, true).append("MessageComplete");
		
		if (hasSeqNum())
			Utilities.addIndent(_toString, indent, true).append("seqNum=\"")
														  .append(seqNum()).append("\"");

		if (hasPartNum())
			Utilities.addIndent(_toString, indent, true).append("partNum=\"")
			  											  .append(partNum()).append("\"");

		if (hasPostId())
			Utilities.addIndent(_toString, indent, true).append("postId=\"")
			  											  .append(postId()).append("\"");

		if (hasPostUserRights())
			Utilities.addIndent(_toString, indent, true).append("postUserRights=\"")
			  											  .append(postUserRights()).append("\"");
		
		if (hasPermissionData())
		{
			Utilities.addIndent(_toString, indent, true).append("permissionData=\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}
		
		Utilities.addIndent(_toString, indent, true).append("publisherIdUserId=\"")
													  .append(publisherIdUserId()).append("\"");
		Utilities.addIndent(_toString, indent, true).append("publisherIdUserAddress=\"")
		  											  .append(publisherIdUserAddress()).append("\"");
		
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
				
		Utilities.addIndent(_toString, indent, true).append("PostMsgEnd\n");

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

	com.refinitiv.eta.codec.PostMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.refinitiv.eta.codec.PostMsg)(_rsslMsg) : null);
	}
}
