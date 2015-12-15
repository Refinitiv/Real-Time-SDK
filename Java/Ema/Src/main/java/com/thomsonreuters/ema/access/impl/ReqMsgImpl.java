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
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;

public class ReqMsgImpl extends MsgImpl implements ReqMsg
{
	private final static String TICKBYTICK_NAME = "TickByTick";
	private final static String JUSTINTIMECONFLATEDRATE_NAME = "JustInTimeConflatedrate";
	private final static String BESTCONFLATEDRATE_NAME = "BestConflatedRate";
	private final static String BESTRATE_NAME = "BestRate";
	private final static String UNKNOWNREQMSGQOSRATE_NAME = "Rate on ReqMsg. Value = ";
	private final static String REALTIME_NAME = "RealTime";
	private final static String BESTDELAYEDTIMELINESS_NAME = "BestDelayedTimeliness";
	private final static String BESTTIMELINESS_NAME = "BestTimeliness";
	private final static String UNKNOWNREQMSGQOSTIMELINESS_NAME = "QosTimeliness on ReqMsg. Value = ";
	
	public ReqMsgImpl()
	{
		super(DataTypes.REQ_MSG);
	}

	@Override
	public ReqMsg clear()
	{
		reqMsgClear();
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
	
	String toString(int indent)
	{
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
				
		Utilities.addIndent(_toString, indent, true).append("ReqMsgEnd\n");

		return _toString.toString();
	}
	
	@Override
	public boolean hasPriority()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkHasPriority(); 
	}

	@Override
	public boolean hasQos()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkHasQos(); 
	}
	
	@Override
	public boolean hasView()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkHasView();
	}

	@Override
	public boolean hasBatch()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkHasBatch();
	}

	@Override
	public int priorityClass()
	{
		if (!hasPriority())
		{
			String temp = "Attempt to priorityClass() while it is NOT set.";
			throw oommIUExcept().message(temp);
		}
		
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).priority().priorityClass();
	}

	@Override
	public int priorityCount()
	{
		if (!hasPriority())
		{
			String temp = "Attempt to priorityCount() while it is NOT set.";
			throw oommIUExcept().message(temp);
		}

		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).priority().count();
	}

	@Override
	public int qosTimeliness()
	{
		if (!hasQos())
		{
			String temp = "Attempt to qosTimeliness() while it is NOT set.";
			throw oommIUExcept().message(temp);
		}
		
		RequestMsg rsslMsg = ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg);
		int timeliness = Timeliness.BEST_TIMELINESS;
		int reqTimeliness = rsslMsg.qos().timeliness();
		int wReqTimeliness = rsslMsg.worstQos().timeliness();
		
		if (rsslMsg.checkHasWorstQos())
		{
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
			throw oommIUExcept().message(temp);
		}
		
		RequestMsg rsslMsg = ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg);
		int rate = Rate.BEST_RATE;
		int reqRate = rsslMsg.qos().rate();
		int wReqRate = rsslMsg.worstQos().rate();

		if (rsslMsg.checkHasWorstQos())
		{
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
			}
		}
		
		return rate;
	}

	@Override
	public boolean initialImage()
	{
		return !(((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkNoRefresh());
	}

	@Override
	public boolean interestAfterRefresh()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkStreaming();
	}

	@Override
	public boolean conflatedInUpdates()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkConfInfoInUpdates();
	}

	@Override
	public boolean pause()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkPause();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).checkPrivateStream();
	}
	
	@Override
	public ReqMsg streamId(int streamId)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (streamId < -2147483648 || streamId > 2147483647)
		{
			String temp = "Passed in streamId is out of range. [(-2147483648) - 2147483647]";
			throw ommOORExcept().message(temp);
		}
			
		msgStreamId(streamId);
		return this;
	}

	@Override
	public ReqMsg domainType(int domainType)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (domainType < 0 || domainType > 255)
		{
			String temp = "Passed in domainType is out of range. [0 - 255]";
			throw ommUDTExcept().message(domainType,temp);
		}
	
		msgDomainType(domainType);
		return this;
	}

	@Override
	public ReqMsg name(String name)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (name == null)
		{
			String temp = "Passed in name is null";
			throw ommOORExcept().message(temp);
		}
		
		msgName(name);
		return this;
	}

	@Override
	public ReqMsg nameType(int nameType)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (nameType < 0 || nameType > 255)
		{
			String temp = "Passed in nameType is out of range. [0 - 255]";
			throw ommOORExcept().message(temp);
		}
		
		msgNameType(nameType);
		return this;
	}

	@Override
	public ReqMsg serviceName(String serviceName)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (serviceName == null)
		{
			String temp = "Passed in serviceName is null";
			throw ommOORExcept().message(temp);
		}
		
		if (hasReqMsgServiceId())
		{
			String temp = "Attempt to set serviceName while service id is already set." ;
			throw ommOORExcept().message(temp);
		}
		
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public ReqMsg serviceId(int serviceId)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (serviceId < 0 || serviceId > 65535)
		{
			String temp = "Passed in serviceId is out of range. [0 - 65535]";
			throw ommOORExcept().message(temp);
		}
		
		if (hasServiceName())
		{
			String temp = "Attempt to set serviceId while service name is already set.";
			throw ommOORExcept().message(temp);
		}
		
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public ReqMsg id(int id)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (id < -2147483648 || id > 2147483647)
		{
			String temp = "Passed in id is out of range. [(-2147483648) - 2147483647]";
			throw ommOORExcept().message(temp);
		}
		
		msgId(id);
		return this;
	}

	@Override
	public ReqMsg filter(long filter)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (filter < 0 || filter > 4294967296L)
		{
			String temp = "Passed in filter is out of range. [0 - 4294967296]";
			throw ommOORExcept().message(temp);
		}
		
		msgFilter(filter);
		return this;
	}

	@Override
	public ReqMsg priority(int priorityClass, int priorityCount)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (priorityClass < 0 || priorityClass > 255)
		{
			String temp = "Passed priorityClass in priority is out of range. [0 - 255]";
			throw ommOORExcept().message(temp);
		}
		
		if (priorityCount < 0 || priorityCount > 65535)
		{
			String temp = "Passed priorityCount in priority is out of range. [0 - 65535]";
			throw ommOORExcept().message(temp);
		}
		
		if (_initialEncoding)
			intialEncoding();
		
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyHasPriority();
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).priority().priorityClass(priorityClass);
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).priority().count(priorityCount);

		return this;
	}

	@Override
	public ReqMsg qos(int timeliness, int rate)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (!(timeliness >= 0 && timeliness <= 7))
		{
			String temp = "Passed timeliness in qos is out of range. [0 - 7]";
			throw ommOORExcept().message(temp);
		}
		 
		if (!(rate >= 0 && rate <= 15))
		{
			String temp = "Passed rate in qos is out of range. [0 - 15]";
			throw ommOORExcept().message(temp);
		}
		
		if (_initialEncoding)
			intialEncoding();
		
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyHasQos();
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).qos().rate(Utilities.toRsslQosRate(rate));
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).qos().timeliness(Utilities.toRsslQosTimeliness(timeliness));
		
		return this;
	}

	@Override
	public ReqMsg attrib(ComplexType data)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (data == null)
		{
			String temp = "Passed in attrib is null";
			throw ommOORExcept().message(temp);
		}
		
		msgAttrib(data);
		return this;
	}

	@Override
	public ReqMsg payload(ComplexType data)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (data == null)
		{
			String temp = "Passed in payload is null";
			throw ommOORExcept().message(temp);
		}
		
		msgPayload(data);
		
		if (_rsslMsgEncoded.containerType() == com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)
			checkBatchView(_rsslMsgEncoded.encodedDataBody());
		
		return this;
	}

	@Override
	public ReqMsg extendedHeader(ByteBuffer buffer)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (buffer == null)
		{
			String temp = "Passed in extendedHeader is null";
			throw ommOORExcept().message(temp);
		}

		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyHasExtendedHdr();
		msgExtendedHeader(buffer);

		return this;
	}

	@Override
	public ReqMsg initialImage(boolean initialImage)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (!initialImage)
			((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyNoRefresh();
		
		return this;
	}

	@Override
	public ReqMsg interestAfterRefresh(boolean interestAfterRefresh)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (interestAfterRefresh)
		{
			((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyStreaming();
		}
		else
		{
			int rsslFlags = ((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).flags();
			rsslFlags = rsslFlags & ~RequestMsgFlags.STREAMING;
		   	((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).flags(rsslFlags);
		}
	
		return this;
	}

	@Override
	public ReqMsg pause(boolean pause)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (pause)
			((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyPause();
		
		return this;
	}

	@Override
	public ReqMsg conflatedInUpdates(boolean conflatedInUpdates)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (conflatedInUpdates)
			((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyConfInfoInUpdates();
		
		return this;
	}

	@Override
	public ReqMsg privateStream(boolean privateStream)
	{
		if (_initialEncoding)
			intialEncoding();
		
		if (privateStream)
			((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyPrivateStream();
		
		return this;
	}

	com.thomsonreuters.upa.codec.RequestMsg rsslMsg()
	{
		return (com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded;
	}

	void checkBatchView(Buffer rsslBuffer)
	{
		
	}
	
	boolean hasReqMsgServiceId()
	{
		return (_rsslMsgEncoded.msgKey() != null && _rsslMsgEncoded.msgKey().checkHasServiceId()) ? true : false;
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object obj)
	{
	}
}