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
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class PostMsgImpl extends MsgImpl implements PostMsg
{
	private final static String CREATE_NAME = "Create";
	private final static String DELETE_NAME = " | Delete";
	private final static String MODIFY_NAME = " | ModifyPermission";
	private final static String PREAPPEND_STRING = "PostUserRights are: ";
	
	private StringBuilder _postUserRightsString;
	
	public PostMsgImpl()
	{
		super(DataTypes.POST_MSG);
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
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPostId()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkHasPostId();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPostUserRights()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkHasPostUserRights();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw oommIUExcept().message("Attempt to seqNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).seqNum();
	}

	@Override
	public long postId()
	{
		if (!hasPostId())
			throw oommIUExcept().message("Attempt to postId() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).postId();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw oommIUExcept().message("Attempt to partNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).partNum();
	}

	@Override
	public int postUserRights()
	{
		if (!hasPostUserRights())
			throw oommIUExcept().message("Attempt to postUserRights() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).postUserRights();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		GlobalPool.releaseByteBuffer(_permissionData);
		_permissionData = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).permData());
			
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean solicitAck()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkAck();
	}

	@Override
	public boolean complete()
	{
		return ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).checkPostComplete();
	}

	@Override
	public PostMsg clear()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg streamId(int streamId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg domainType(int domainType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg name(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg nameType(int nameType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg serviceName(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg serviceId(int serviceId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg id(int id)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg filter(long filter)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg seqNum(long seqNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg postId(long postId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg partNum(int partNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg postUserRights(int postUserRights)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg permissionData(ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg publisherId(long UserId, long UserAddress)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg attrib(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg payload(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg extendedHeader(ByteBuffer buffer)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg solicitAck(boolean ack)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public PostMsg complete(boolean complete)
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
				
		Utilities.addIndent(_toString, indent, true).append("PostMsgEnd\n");

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