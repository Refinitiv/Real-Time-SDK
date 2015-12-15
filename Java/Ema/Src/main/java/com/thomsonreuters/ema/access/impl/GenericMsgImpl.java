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
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class GenericMsgImpl extends MsgImpl implements GenericMsg
{
	public GenericMsgImpl()
	{
		super(DataTypes.GENERIC_MSG);
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
			throw oommIUExcept().message("Attempt to seqNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).seqNum();
	}

	@Override
	public long secondarySeqNum()
	{
		if (!hasSecondarySeqNum())
			throw oommIUExcept().message("Attempt to secondarySeqNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).secondarySeqNum();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw oommIUExcept().message("Attempt to partNum() while it is NOT set.");
		
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).partNum();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		GlobalPool.releaseByteBuffer(_permissionData);
		_permissionData = DataImpl.asByteBuffer(((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).permData());
			
		return _permissionData;
	}

	@Override
	public boolean complete()
	{
		return ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).checkMessageComplete();
	}

	@Override
	public GenericMsg clear()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg streamId(int streamId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg domainType(int domainType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg name(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg nameType(int nameType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg serviceId(int serviceId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg id(int id)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg filter(long filter)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg seqNum(long seqNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg secondarySeqNum(long secondarySeqNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg partNum(int partNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg permissionData(ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg attrib(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg payload(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg extendedHeader(ByteBuffer buffer)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GenericMsg complete(boolean complete)
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