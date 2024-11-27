///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.*;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

class ReqMsgImpl extends MsgImpl implements ReqMsg
{
	private final static String CLONE_CONSTRUCTOR_NAME = ReqMsgImpl.class.getCanonicalName() + ".ReqMsgImpl(ReqMsg other)";
	private final static String TICKBYTICK_NAME = "TickByTick";
	private final static String JUSTINTIMECONFLATEDRATE_NAME = "JustInTimeConflatedrate";
	private final static String BESTCONFLATEDRATE_NAME = "BestConflatedRate";
	private final static String BESTRATE_NAME = "BestRate";
	private final static String UNKNOWNREQMSGQOSRATE_NAME = "Rate on ReqMsg. Value = ";
	private final static String REALTIME_NAME = "RealTime";
	private final static String BESTDELAYEDTIMELINESS_NAME = "BestDelayedTimeliness";
	private final static String BESTTIMELINESS_NAME = "BestTimeliness";
	private final static String UNKNOWNREQMSGQOSTIMELINESS_NAME = "QosTimeliness on ReqMsg. Value = ";
	private final static String VIEW_DATA_STRING = ":ViewData";
	private final static String ITEM_LIST_STRING = ":ItemList";
	
	private OmmQosImpl _qos;
	private com.refinitiv.eta.codec.ElementList _rsslElementList;
	private com.refinitiv.eta.codec.Array _rsslArray;
	private com.refinitiv.eta.codec.ElementEntry _rsslElementEntry;
	private com.refinitiv.eta.codec.ArrayEntry _rsslArrayEntry;
	private com.refinitiv.eta.codec.Buffer _rsslItemBuffer;
    private List<String> _batchItemList;
    private String _serviceListName;
    
	
    ReqMsgImpl()
	{
		super(DataTypes.REQ_MSG, null);
		initialEncoding();
	}

    ReqMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.REQ_MSG, objManager);
	}
    
    ReqMsgImpl(ReqMsg other)
	{
		super((MsgImpl) other, CLONE_CONSTRUCTOR_NAME);

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

		if (other.payload().dataType() != DataTypes.NO_DATA) {
			_rsslMsg.encodedDataBody(CodecFactory.createBuffer());
			payload(other.payload().data());
		}
	}

    @Override
	public ReqMsg clear()
	{
		msgClear();
		initialEncoding();
		_serviceListName = null;
		return this;
	}

	@Override
	public String rateAsString()
	{
		switch (qosRate())
		{
		case Rate.TICK_BY_TICK :
			return TICKBYTICK_NAME;
		case Rate.JIT_CONFLATED :
			return JUSTINTIMECONFLATEDRATE_NAME;
		case Rate.BEST_CONFLATED_RATE :
			return BESTCONFLATEDRATE_NAME;
		case Rate.BEST_RATE :
			return BESTRATE_NAME;
		default :
			return (UNKNOWNREQMSGQOSRATE_NAME + qosRate());
		}
	}

	@Override
	public String timelinessAsString()
	{
		switch (qosTimeliness())
		{
		case Timeliness.REALTIME :
			return REALTIME_NAME;
		case Timeliness.BEST_DELAYED_TIMELINESS :
			return BESTDELAYEDTIMELINESS_NAME;
		case Timeliness.BEST_TIMELINESS :
			return BESTTIMELINESS_NAME;
		default :
			return (UNKNOWNREQMSGQOSTIMELINESS_NAME + qosTimeliness());
		}
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

		ReqMsg reqMsg = new ReqMsgImpl(_objManager);

		((MsgImpl) reqMsg).decode(((DataImpl)this).encodedData(), Codec.majorVersion(), Codec.minorVersion(), ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);
		if (_errorCode != ErrorCode.NO_ERROR)
		{
			return "\nFailed to decode ReqMsg with error: " + ((MsgImpl) reqMsg).errorString() + "\n";
		}

		return reqMsg.toString();
	}

	String toString(int indent)
	{
		if ( _objManager == null )
			return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";
		
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent++).append("ReqMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													 .append(streamId())
													 .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													 .append(Utilities.rdmDomainAsString(domainType()))
													 .append("\"");		
		
		if (privateStream())
			Utilities.addIndent(_toString, indent, true).append("privateStream");
		
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
			
			if (hasServiceListName())
				Utilities.addIndent(_toString, indent, true).append("hasServiceListName=\"")
															 .append(serviceListName())
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

			Utilities.addIndent(_toString, indent, true).append("PayloadEnd");
			indent--;
		}
				
		Utilities.addIndent(_toString, indent, true).append("ReqMsgEnd\n");

		return _toString.toString();
	}
	
	@Override
	public boolean hasPriority()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkHasPriority(); 
	}

	@Override
	public boolean hasQos()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkHasQos(); 
	}
	
	@Override
	public boolean hasView()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkHasView();
	}

	@Override
	public boolean hasBatch()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkHasBatch();
	}
	
	@Override
	public boolean hasServiceListName() {

		return _serviceListName != null ? true : false;
	}

	@Override
	public int priorityClass()
	{
		if (!hasPriority())
		{
			String temp = "Attempt to priorityClass() while it is NOT set.";
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).priority().priorityClass();
	}

	@Override
	public int priorityCount()
	{
		if (!hasPriority())
		{
			String temp = "Attempt to priorityCount() while it is NOT set.";
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).priority().count();
	}

	@Override
	public int qosTimeliness()
	{
		if (!hasQos())
		{
			String temp = "Attempt to qosTimeliness() while it is NOT set.";
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		com.refinitiv.eta.codec.RequestMsg rsslMsg = ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg);
		int timeliness = Timeliness.BEST_TIMELINESS;
		int reqTimeliness = rsslMsg.qos().timeliness();
		
		if (rsslMsg.checkHasWorstQos())
		{
			int wReqTimeliness = rsslMsg.worstQos().timeliness();
			
			if (reqTimeliness == wReqTimeliness)
			{
				switch (reqTimeliness)
				{
				case QosTimeliness.REALTIME:
					timeliness = Timeliness.REALTIME;
					break;
				case QosTimeliness.DELAYED_UNKNOWN:
					timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
					break;
				case QosTimeliness.DELAYED:
					if (rsslMsg.qos().timeInfo() == rsslMsg.worstQos().timeInfo())
						timeliness = rsslMsg.qos().timeInfo();
					else
						timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
					break;
				default:
					break;
				}
			}
			else
			{
				if (reqTimeliness == QosTimeliness.REALTIME)
					timeliness = Timeliness.BEST_TIMELINESS;
				else
					timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
	 		}
		}
		else
		{
			switch (reqTimeliness)
			{
				case QosTimeliness.REALTIME:
					timeliness = Timeliness.REALTIME;
					break;
				case QosTimeliness.DELAYED_UNKNOWN:
					timeliness = Timeliness.BEST_DELAYED_TIMELINESS;
					break;
				case QosTimeliness.DELAYED:
						timeliness = rsslMsg.qos().timeInfo();
					break;
				default:
					break;
			}
		}

		return timeliness;
	}

	@Override
	public int qosRate()
	{
		if (!hasQos())
		{
			String temp = "Attempt to qosRate() while it is NOT set.";
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		com.refinitiv.eta.codec.RequestMsg rsslMsg = ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg);
		int rate = Rate.BEST_RATE;
		int reqRate = rsslMsg.qos().rate();

		if (rsslMsg.checkHasWorstQos())
		{
			int wReqRate = rsslMsg.worstQos().rate();
			
			if (reqRate == wReqRate)
			{
				switch (reqRate)
				{
				case QosRates.TICK_BY_TICK:
					rate = Rate.TICK_BY_TICK;
					break;
				case QosRates.JIT_CONFLATED:
					rate = Rate.JIT_CONFLATED;
					break;
				case QosRates.TIME_CONFLATED:
					if (rsslMsg.qos().rateInfo() == rsslMsg.worstQos().rateInfo())
						rate = rsslMsg.qos().rateInfo();
					else
						rate = Rate.BEST_CONFLATED_RATE;
					break;
				default:
					break;
				}
			}
			else
			{
				if (reqRate == QosRates.TICK_BY_TICK)
					rate = Rate.BEST_RATE;
				else
					rate = Rate.BEST_CONFLATED_RATE;
			}
		}
		else
		{
			switch (reqRate)
			{
			case QosRates.TICK_BY_TICK:
				rate = Rate.TICK_BY_TICK;
				break;
			case QosRates.JIT_CONFLATED:
				rate = Rate.JIT_CONFLATED;
				break;
			case QosRates.TIME_CONFLATED:
				rate = rsslMsg.qos().rateInfo();
				break;
			default:
				break;
			}
		}
		
		return rate;
	}

	@Override
	public boolean initialImage()
	{
		return !(((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkNoRefresh());
	}

	@Override
	public boolean interestAfterRefresh()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkStreaming();
	}

	@Override
	public boolean conflatedInUpdates()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkConfInfoInUpdates();
	}

	@Override
	public boolean pause()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkPause();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).checkPrivateStream();
	}
	
	@Override
	public ReqMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public ReqMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public ReqMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public ReqMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public ReqMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}
	
	@Override
	public ReqMsg serviceListName(String serviceListName)
	{
		if (serviceListName == null)
			throw ommIUExcept().message("Passed in serviceListName is null.", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if(_serviceNameSet)
		{
			throw ommIUExcept().message("Service name is already set for this ReqMsg.", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		
		if(_rsslMsg.msgKey().checkHasServiceId())
		{
			throw ommIUExcept().message("Service Id is already set for this ReqMsg.", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		
		_serviceListName = serviceListName;
			
		return this;
	}

	@Override
	public ReqMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public ReqMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public ReqMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public ReqMsg priority(int priorityClass, int priorityCount)
	{
		if (priorityClass < 0 || priorityClass > 255)
			throw ommOORExcept().message("Passed priorityClass in priority is out of range. [0 - 255].");
		
		if (priorityCount < 0 || priorityCount > 65535)
			throw ommOORExcept().message("Passed priorityCount in priority is out of range. [0 - 65535].");
		
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyHasPriority();
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).priority().priorityClass(priorityClass);
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).priority().count(priorityCount);

		return this;
	}

	@Override
	public ReqMsg qos(int timeliness, int rate)
	{
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyHasQos();
		Qos rsslQos = ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).qos();
		rsslQos.clear();
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyHasWorstQos();
		Qos rsslWQos = ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).worstQos();
		rsslWQos.clear();
		
		boolean hasQosRange = false;
		
		switch (rate)
		{
		case Rate.TICK_BY_TICK :
			rsslQos.rate(com.refinitiv.eta.codec.QosRates.TICK_BY_TICK);
			rsslWQos.rate(com.refinitiv.eta.codec.QosRates.TICK_BY_TICK);
			break;
		case Rate.JIT_CONFLATED :
			rsslQos.rate(com.refinitiv.eta.codec.QosRates.JIT_CONFLATED);
			rsslWQos.rate(com.refinitiv.eta.codec.QosRates.JIT_CONFLATED);
			break;
		case Rate.BEST_CONFLATED_RATE :
			rsslQos.rate(com.refinitiv.eta.codec.QosRates.TIME_CONFLATED);
			rsslQos.rateInfo (1);
			
			hasQosRange = true;
			rsslWQos.rate(com.refinitiv.eta.codec.QosRates.JIT_CONFLATED);
			break;
		case Rate.BEST_RATE :
			rsslQos.rate(com.refinitiv.eta.codec.QosRates.TICK_BY_TICK);

			hasQosRange = true;
			rsslWQos.rate( com.refinitiv.eta.codec.QosRates.JIT_CONFLATED);
			break;
		default :
			if ( rate <= 65535 )
			{
				rsslQos.rate(com.refinitiv.eta.codec.QosRates.TIME_CONFLATED);
				rsslQos.rateInfo(rate);
				
				rsslWQos.rate(com.refinitiv.eta.codec.QosRates.TIME_CONFLATED);
				rsslWQos.rateInfo(rate);
			}
			else
			{
				rsslQos.rate(com.refinitiv.eta.codec.QosRates.JIT_CONFLATED);
				rsslWQos.rate(com.refinitiv.eta.codec.QosRates.JIT_CONFLATED);
			}
			break;
		}

		switch (timeliness)
		{
		case Timeliness.REALTIME :
			rsslQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.REALTIME);
			rsslWQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.REALTIME);
			break;
		case Timeliness.BEST_DELAYED_TIMELINESS :
			rsslQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED);
			rsslQos.timeInfo(1);
			
			hasQosRange = true;
			rsslWQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED);
			rsslWQos.timeInfo(65535);
			break;
		case Timeliness.BEST_TIMELINESS :
			rsslQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.REALTIME);

			hasQosRange = true;
			rsslWQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED);
			rsslWQos.timeInfo(65535);
			break;
		default :
			if ( timeliness <= 65535 )
			{
				rsslQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED);
				rsslQos.timeInfo(timeliness);
				
				rsslWQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED);
				rsslWQos.timeInfo(timeliness);
			}
			else
			{
				rsslQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED_UNKNOWN);
				rsslWQos.timeliness(com.refinitiv.eta.codec.QosTimeliness.DELAYED_UNKNOWN);
			}
			break;
		}
		
		if ( hasQosRange )
		{
			rsslQos.dynamic(true);
			rsslWQos.dynamic(true);
		}
		else
		{
			int flags = ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).flags();
			flags &= ~com.refinitiv.eta.codec.RequestMsgFlags.HAS_WORST_QOS;
			((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).flags(flags);
		}
		
		return this;
	}

	@Override
	public ReqMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public ReqMsg payload(ComplexType data)
	{
		msgPayload(data);
		if (_rsslMsg.containerType() == com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)
			checkBatchView(_rsslMsg.encodedDataBody());
		
		return this;
	}

	@Override
	public ReqMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}

	@Override
	public ReqMsg initialImage(boolean initialImage)
	{
		if (!initialImage)
			((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyNoRefresh();
		
		return this;
	}

	@Override
	public ReqMsg interestAfterRefresh(boolean interestAfterRefresh)
	{
		int rsslFlags = ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).flags();
		if (interestAfterRefresh)
			rsslFlags |= com.refinitiv.eta.codec.RequestMsgFlags.STREAMING;
		else
			rsslFlags &= ~com.refinitiv.eta.codec.RequestMsgFlags.STREAMING;
		
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).flags(rsslFlags);
	
		return this;
	}

	@Override
	public ReqMsg pause(boolean pause)
	{
		if (pause)
			((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyPause();
		
		return this;
	}

	@Override
	public ReqMsg conflatedInUpdates(boolean conflatedInUpdates)
	{
		if (conflatedInUpdates)
			((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyConfInfoInUpdates();
		
		return this;
	}

	@Override
	public ReqMsg privateStream(boolean privateStream)
	{
		if (privateStream)
			((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyPrivateStream();
		
		return this;
	}

	com.refinitiv.eta.codec.RequestMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.refinitiv.eta.codec.RequestMsg)(_rsslMsg) : null);
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
		
		qosInt();
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
				qosInt();
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
	
	void qosInt()
	{
		if (_qos == null)
			_qos = new OmmQosImpl();
		else
			_qos.clear();
		
		_qos.decode( ((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).qos() );
	}
	
	void checkBatchView(Buffer rsslBuffer)
	{
		if (_rsslElementList == null)
		{
			_rsslElementList = CodecFactory.createElementList();
			_rsslElementEntry = CodecFactory.createElementEntry();
		}
		else
		{
			_rsslElementList.clear();
			_rsslElementEntry.clear();
		}

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if ( retCode !=CodecReturnCodes.SUCCESS)
		{
			String temp =  "ReqMsgImpl.checkBatchView() failed to set iterator version in ReqMsgImpl.payload(). Internal error "  + CodecReturnCodes.toString(retCode);
			throw ommIUExcept().message( temp, retCode );
		}

		retCode = _rsslElementList.decode(_rsslDecodeIter, null);
		if ( retCode != CodecReturnCodes.SUCCESS )
		{
				String temp =  "ReqMsgImpl.checkBatchView() failed to decode ElementList in ReqMsgImpl.payload(). Internal error "  + CodecReturnCodes.toString(retCode);
				throw ommIUExcept().message( temp, retCode );
		}
	
		while ( true )
		{
			retCode = _rsslElementEntry.decode(_rsslDecodeIter);
			switch ( retCode )
			{
				case CodecReturnCodes.END_OF_CONTAINER :
						return;
				case CodecReturnCodes.SUCCESS :
					if ( _rsslElementEntry.name().length() == 9 && _rsslElementEntry.name().toString().compareTo( VIEW_DATA_STRING ) == 0)
					{
						((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyHasView();
					}
					else if ( _rsslElementEntry.name().length() == 9 &&	_rsslElementEntry.name().toString().compareTo(ITEM_LIST_STRING) == 0) 
					{
						if ( _batchItemList == null )
							_batchItemList = new ArrayList<String>();
						else
							_batchItemList.clear();
	
						if ( _rsslElementEntry.dataType() == com.refinitiv.eta.codec.DataTypes.ARRAY )
						{
							if (_rsslArray == null)
							{
								_rsslArray = CodecFactory.createArray();
								_rsslArrayEntry = CodecFactory.createArrayEntry();
							}
							else
							{
								_rsslArray.clear();
								_rsslArrayEntry.clear();
							}
	
							if ( _rsslArray.decode(_rsslDecodeIter) >= CodecReturnCodes.SUCCESS  )
							{
								if ( _rsslArray.primitiveType() == com.refinitiv.eta.codec.DataTypes.ASCII_STRING )
								{
									if (_rsslItemBuffer == null)
										_rsslItemBuffer = CodecFactory.createBuffer();
									else
										_rsslItemBuffer.clear();
								
									while ( _rsslArrayEntry.decode(_rsslDecodeIter) != CodecReturnCodes.END_OF_CONTAINER )
									{
										if (_rsslItemBuffer.decode(_rsslDecodeIter) == CodecReturnCodes.SUCCESS )
											_batchItemList.add(_rsslItemBuffer.toString());
									}
									
									if ( _batchItemList.size() > 0 )
										((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyHasBatch();
								}
							}
						}
				}
				break;
			case CodecReturnCodes.INCOMPLETE_DATA :
			case CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
			default :
				{
					String temp =  "ReqMsgImpl.checkBatchView() failed to decode ElementEntry. Internal error "  + CodecReturnCodes.toString(retCode);
					throw ommIUExcept().message( temp, retCode );
				}
			}
		}
	}
	
	void initialEncoding()
	{
		((com.refinitiv.eta.codec.RequestMsg)_rsslMsg).applyStreaming();
	}
		
	List<String> batchItemList()
	{
		return _batchItemList;
	}

	@Override
	public String serviceListName() 
	{
		return _serviceListName;
	}
}
