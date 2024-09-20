///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.Codec;

import java.nio.ByteBuffer;

class AckMsgImpl extends MsgImpl implements AckMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = AckMsgImpl.class.getCanonicalName() + ".AckMsgImpl(AckMsg  other)";
	private final static String ACCESSDENIED_STRING = "AccessDenied";
	private final static String DENIEDBYSOURCE_STRING = "DeniedBySource";
	private final static String SOURCEDOWN_STRING = "SourceDown";
	private final static String SOURCEUNKNOWN_STRING = "SourceUnknown";
	private final static String NORESOURCES_STRING = "NoResources";
	private final static String NORESPONSE_STRING = "NoResponse";
	private final static String SYMBOLUNKNOWN_STRING = "SymbolUnknown";
	private final static String NOTOPEN_STRING = "NotOpen";
	private final static String GATEWAYDOWN_STRING = "GatewayDown";
	private final static String NONE_STRING = "None";
	private final static String INVALIDCONTENT_STRING = "InvalidContent";
	private final static String UNKNOWNNACKCODE_STRING = "Unknown NackCode value ";

	private String _text;
	private boolean _textSet;
	
	AckMsgImpl()
	{
		super(DataTypes.ACK_MSG,  null);
	}

	AckMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.ACK_MSG, objManager);
	}
	
	AckMsgImpl(AckMsg other)
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
		
		if (other.hasText())
			text(other.text());

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}
	
	@Override
	public String nackCodeAsString()
	{
		switch (nackCode())
		{
		case NackCode.NONE:
			return NONE_STRING;
		case NackCode.ACCESS_DENIED:
			return ACCESSDENIED_STRING;
		case NackCode.DENIED_BY_SOURCE:
			return DENIEDBYSOURCE_STRING;
		case NackCode.SOURCE_DOWN:
			return SOURCEDOWN_STRING;
		case NackCode.SOURCE_UNKNOWN:
			return SOURCEUNKNOWN_STRING;
		case NackCode.NO_RESOURCES:
			return NORESOURCES_STRING;
		case NackCode.NO_RESPONSE:
			return NORESPONSE_STRING;
		case NackCode.GATEWAY_DOWN:
			return GATEWAYDOWN_STRING;
		case NackCode.SYMBOL_UNKNOWN:
			return SYMBOLUNKNOWN_STRING;
		case NackCode.NOT_OPEN:
			return NOTOPEN_STRING;
		case NackCode.INVALID_CONTENT:
			return INVALIDCONTENT_STRING;
		default:
			return (UNKNOWNNACKCODE_STRING + nackCode());
		}
	}

	@Override
	public boolean hasSeqNum()
	{
		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasNackCode()
	{
		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).checkHasNakCode();
	}

	@Override
	public boolean hasText()
	{
		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).checkHasText();
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).seqNum();
	}

	@Override
	public long ackId()
	{
		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).ackId();
	}

	@Override
	public int nackCode()
	{
		if (!hasNackCode())
			throw ommIUExcept().message("Attempt to nackCode() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).nakCode();
	}

	@Override
	public String text()
	{
		if (!hasText())
			throw ommIUExcept().message("Attempt to text() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		if (_textSet)  return _text;
		
		if (((com.refinitiv.eta.codec.AckMsg) _rsslMsg).text().length() == 0)
			_text = DataImpl.EMPTY_STRING;
		else
			_text = ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).text().toString();

		_textSet = true;
		
		return _text;
	}

	@Override
	public boolean privateStream()
	{
		return ((com.refinitiv.eta.codec.AckMsg) _rsslMsg).checkPrivateStream();
	}

	@Override
	public AckMsg clear()
	{
		msgClear();
		return this;
	}

	@Override
	public AckMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public AckMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public AckMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public AckMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public AckMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public AckMsg serviceId(int serviceId)
	{	
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public AckMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public AckMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public AckMsg seqNum(long seqNum)
	{
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public AckMsg ackId(long ackId)
	{
		if (ackId < 0 || ackId > 4294967295L)
			throw ommOORExcept().message("Passed in actId is out of range. [0 - 4294967295]");

		((com.refinitiv.eta.codec.AckMsg)_rsslMsg).ackId(ackId);
		return this;
	}

	@Override
	public AckMsg nackCode(int nackCode)
	{
		if (nackCode < 0 || nackCode > 255)
			throw ommOORExcept().message("Passed in nackCode is out of range. [0 - 255]");

		((com.refinitiv.eta.codec.AckMsg)_rsslMsg).applyHasNakCode();
		((com.refinitiv.eta.codec.AckMsg)_rsslMsg).nakCode(nackCode);
		return this;
	}

	@Override
	public AckMsg text(String text)
	{
		if (text == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		((com.refinitiv.eta.codec.AckMsg)_rsslMsg).applyHasText();
		((com.refinitiv.eta.codec.AckMsg)_rsslMsg).text().data(text);
		
		return this;
	}

	@Override
	public AckMsg attrib(ComplexType attrib)
	{
		msgAttrib(attrib);
		return this;
	}

	@Override
	public AckMsg payload(ComplexType payload)
	{
		msgPayload(payload);
		return this;
	}

	@Override
	public AckMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}

	@Override
	public AckMsg privateStream(boolean privateStream)
	{
		if (privateStream)
			((com.refinitiv.eta.codec.AckMsg)_rsslMsg).applyPrivateStream();
		
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

		AckMsg ackMsg  = new AckMsgImpl(_objManager);

		((MsgImpl) ackMsg).decode(((DataImpl)this).encodedData(), Codec.majorVersion(), Codec.minorVersion(), ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);
		if (_errorCode != ErrorCode.NO_ERROR)
		{
			return "\nFailed to decode AckMsg with error: " + ((MsgImpl) ackMsg).errorString() + "\n";
		}

		return ackMsg.toString();
	}

	String toString(int indent)
	{
		if ( _objManager == null )
			return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";
		
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent++).append("AckMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"").append(streamId()).append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"").append(Utilities.rdmDomainAsString(domainType())).append("\"");
		Utilities.addIndent(_toString, indent, true).append("ackId=\"").append(ackId()).append("\"");

		if (hasSeqNum())
			Utilities.addIndent(_toString, indent, true).append("seqNum=\"").append(seqNum()).append("\"");

		if (hasNackCode())
			Utilities.addIndent(_toString, indent, true).append("nackCode=\"").append(nackCodeAsString()).append("\"");

		if (hasText())
			Utilities.addIndent(_toString, indent, true).append("text=\"").append(text()).append("\"");

		indent--;
		if (hasMsgKey())
		{
			indent++;
			if (hasName())
				Utilities.addIndent(_toString, indent, true).append("name=\"").append(name()).append("\"");

			if (hasNameType())
				Utilities.addIndent(_toString, indent, true).append("nameType=\"").append(nameType()).append("\"");

			if (hasServiceId())
				Utilities.addIndent(_toString, indent, true).append("serviceId=\"").append(serviceId()).append("\"");

			if (hasServiceName())
				Utilities.addIndent(_toString, indent, true).append("serviceName=\"").append(serviceName())
						.append("\"");

			if (hasFilter())
				Utilities.addIndent(_toString, indent, true).append("filter=\"").append(filter()).append("\"");

			if (hasId())
				Utilities.addIndent(_toString, indent, true).append("id=\"").append(id()).append("\"");

			indent--;

			if (hasAttrib())
			{
				indent++;
				Utilities.addIndent(_toString, indent, true).append("Attrib dataType=\"")
						.append(DataType.asString(attribData().dataType())).append("\"\n");

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
					.append(DataType.asString(payloadData().dataType())).append("\"\n");

			indent++;
			_toString.append(payloadData().toString(indent));
			indent--;

			Utilities.addIndent(_toString, indent).append("PayloadEnd");
			indent--;
		}

		Utilities.addIndent(_toString, indent, true).append("AckMsgEnd\n");

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
		_textSet = false;

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
		_textSet = false;

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
			dataInstance(_payloadDecoded, DataTypes.ERROR).decode(rsslBuffer, _errorCode);
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
