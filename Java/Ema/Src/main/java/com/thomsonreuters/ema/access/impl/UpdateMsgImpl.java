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
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class UpdateMsgImpl extends MsgImpl implements UpdateMsg
{
	public UpdateMsgImpl()
	{
		super(DataTypes.UPDATE_MSG);
	}

	@Override
	public boolean hasSeqNum()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasConflated()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkHasConfInfo();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public int updateTypeNum()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).updateType();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw oommIUExcept().message("Attempt to seqNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).seqNum();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");

		GlobalPool.releaseByteBuffer(_permissionData);
		_permissionData = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).permData());
			
		return _permissionData;
	}

	@Override
	public int conflatedTime()
	{
		if (!hasConflated())
			throw oommIUExcept().message("Attempt to conflatedTime() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).conflationTime();
	}

	@Override
	public int conflatedCount()
	{
		if (!hasConflated())
			throw oommIUExcept().message("Attempt to conflatedCount() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).conflationCount();
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw oommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw oommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean doNotCache()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkDoNotCache();
	}

	@Override
	public boolean doNotConflate()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkDoNotConflate();
	}

	@Override
	public boolean doNotRipple()
	{
		return ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).checkDoNotRipple();
	}

	@Override
	public UpdateMsg clear()
	{
		updateMsgClear();
		return this;
	}

	@Override
	public UpdateMsg streamId(int streamId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg domainType(int domainType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg name(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg nameType(int nameType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg serviceName(String serviceName)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg serviceId(int serviceId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg id(int id)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg filter(long filter)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg updateTypeNum(int updateTypeNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg seqNum(long seqNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg permissionData(ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg conflated(int count, int time)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg publisherId(long userId, long userAddress)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg attrib(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg payload(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg extendedHeader(ByteBuffer buffer)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg doNotCache(boolean doNotCache)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg doNotConflate(boolean doNotConflate)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public UpdateMsg doNotRipple(boolean doNotRipple)
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
				
		Utilities.addIndent(_toString, indent, true).append("UpdateMsgEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Msg rsslMsg, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary)
	{
		_rsslMsg = rsslMsg; // created by upa

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