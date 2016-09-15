///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;

class Utilities
{
	private final static int UnknownDT = -1;

	private final static String LoginDomainString = "Login Domain";
	private final static String DirectoryDomainString = "Directory Domain";
	private final static String DictionaryDomainString = "Dictionary Domain";
	private final static String MarketPriceDomainString = "MarketPrice Domain";
	private final static String MarketByOrderDomainString = "MarketByOrder Domain";
	private final static String MarketByPriceDomainString = "MarketByPrice Domain";
	private final static String MarketMakerDomainString = "MarketMaker Domain";
	private final static String SymbolListDomainString = "SymbolList Domain";
	private final static String ServiceProviderStatusDomainString = "ServiceProviderStatus Domain";
	private final static String HistoryDomainString = "History Domain";
	private final static String HeadlineDomainString = "Headline Domain";
	private final static String StoryDomainString = "Story Domain";
	private final static String ReplayHeadlineDomainString = "ReplayHeadline Domain";
	private final static String ReplayHistoryDomainString = "ReplayHistory Domain";
	private final static String TransactionDomainString = "Transaction Domain";
	private final static String YieldCurveDomainString = "YieldCurve Domain";
	private final static String ContributionDomainString = "Contribution Domain";
	private final static String ProviderAdminDomainString = "ProviderAdmin Domain";
	private final static String AnalyticsDomainString = "Analytics Domain";
	private final static String ReferenceDomainString = "Reference Domain";
	private final static String NewsTextAnalyticsDomainString = "NewsTextAnalytics Domain";
	private final static String SystemDomainString = "System Domain";

	static int toEmaDataType[] = { UnknownDT, UnknownDT, UnknownDT, DataTypes.INT, DataTypes.UINT, DataTypes.FLOAT,
			DataTypes.DOUBLE, UnknownDT, DataTypes.REAL, DataTypes.DATE, DataTypes.TIME, DataTypes.DATETIME,
			DataTypes.QOS, DataTypes.STATE, DataTypes.ENUM, DataTypes.ARRAY, DataTypes.BUFFER, DataTypes.ASCII,
			DataTypes.UTF8, DataTypes.RMTES, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, DataTypes.NO_DATA, UnknownDT, DataTypes.OPAQUE, DataTypes.XML,
			DataTypes.FIELD_LIST, DataTypes.ELEMENT_LIST, DataTypes.ANSI_PAGE, DataTypes.FILTER_LIST, DataTypes.VECTOR,
			DataTypes.MAP, DataTypes.SERIES, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT };

	static int toEmaMsgClass[] = { UnknownDT, DataTypes.REQ_MSG, DataTypes.REFRESH_MSG, DataTypes.STATUS_MSG,
			DataTypes.UPDATE_MSG, UnknownDT, DataTypes.ACK_MSG, DataTypes.GENERIC_MSG, DataTypes.POST_MSG, UnknownDT,
			UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT };

	static int toRsslMsgClass[] = { MsgClasses.REQUEST, MsgClasses.REFRESH, MsgClasses.UPDATE, MsgClasses.STATUS,
			MsgClasses.POST, MsgClasses.ACK, MsgClasses.GENERIC, UnknownDT, UnknownDT, UnknownDT, UnknownDT, UnknownDT,
			UnknownDT, UnknownDT };

	static int toRsslDataType(int dType)
	{
		switch (dType)
		{
		case DataTypes.REQ_MSG:
		case DataTypes.REFRESH_MSG:
		case DataTypes.UPDATE_MSG:
		case DataTypes.STATUS_MSG:
		case DataTypes.POST_MSG:
		case DataTypes.ACK_MSG:
		case DataTypes.GENERIC_MSG:
			return com.thomsonreuters.upa.codec.DataTypes.MSG;
		case DataTypes.FIELD_LIST:
			return com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST;
		case DataTypes.ELEMENT_LIST:
			return com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST;
		case DataTypes.MAP:
			return com.thomsonreuters.upa.codec.DataTypes.MAP;
		case DataTypes.VECTOR:
			return com.thomsonreuters.upa.codec.DataTypes.VECTOR;
		case DataTypes.SERIES:
			return com.thomsonreuters.upa.codec.DataTypes.SERIES;
		case DataTypes.FILTER_LIST:
			return com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST;
		case DataTypes.OPAQUE:
			return com.thomsonreuters.upa.codec.DataTypes.OPAQUE;
		case DataTypes.XML:
			return com.thomsonreuters.upa.codec.DataTypes.XML;
		case DataTypes.ANSI_PAGE:
			return com.thomsonreuters.upa.codec.DataTypes.ANSI_PAGE;
		case DataTypes.ARRAY:
			return com.thomsonreuters.upa.codec.DataTypes.ARRAY;
		case DataTypes.INT:
			return com.thomsonreuters.upa.codec.DataTypes.INT;
		case DataTypes.UINT:
			return com.thomsonreuters.upa.codec.DataTypes.UINT;
		case DataTypes.REAL:
			return com.thomsonreuters.upa.codec.DataTypes.REAL;
		case DataTypes.FLOAT:
			return com.thomsonreuters.upa.codec.DataTypes.FLOAT;
		case DataTypes.DOUBLE:
			return com.thomsonreuters.upa.codec.DataTypes.DOUBLE;
		case DataTypes.DATE:
			return com.thomsonreuters.upa.codec.DataTypes.DATE;
		case DataTypes.TIME:
			return com.thomsonreuters.upa.codec.DataTypes.TIME;
		case DataTypes.DATETIME:
			return com.thomsonreuters.upa.codec.DataTypes.DATETIME;
		case DataTypes.QOS:
			return com.thomsonreuters.upa.codec.DataTypes.QOS;
		case DataTypes.STATE:
			return com.thomsonreuters.upa.codec.DataTypes.STATE;
		case DataTypes.ENUM:
			return com.thomsonreuters.upa.codec.DataTypes.ENUM;
		case DataTypes.BUFFER:
			return com.thomsonreuters.upa.codec.DataTypes.BUFFER;
		case DataTypes.ASCII:
			return com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING;
		case DataTypes.UTF8:
			return com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING;
		case DataTypes.RMTES:
			return com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING;
		case DataTypes.NO_DATA:
			return com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
		default:
			return com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
		}
	}

	static void toRsslQos(int rate, int timeliness, Qos rsslQos)
	{
		rsslQos.dynamic(false);

		if (rate == OmmQos.Rate.TICK_BY_TICK)
			rsslQos.rate(com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK);
		else if (rate == OmmQos.Rate.JUST_IN_TIME_CONFLATED)
			rsslQos.rate(com.thomsonreuters.upa.codec.QosRates.JIT_CONFLATED);
		else
		{
			if (rate <= 65535)
			{
				rsslQos.rate(com.thomsonreuters.upa.codec.QosRates.TIME_CONFLATED);
				rsslQos.rateInfo(rate);
			} else
				rsslQos.rate(com.thomsonreuters.upa.codec.QosRates.JIT_CONFLATED);
		}

		if (timeliness == OmmQos.Timeliness.REALTIME)
			rsslQos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.REALTIME);
		else if (timeliness == OmmQos.Timeliness.INEXACT_DELAYED)
			rsslQos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.DELAYED_UNKNOWN);
		else
		{
			if (timeliness <= 65535)
			{
				rsslQos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.DELAYED);
				rsslQos.timeInfo(timeliness);
			} else
				rsslQos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.DELAYED_UNKNOWN);
		}
	}

	static String rdmDomainAsString(int domain)
	{
		switch (domain)
		{
		case EmaRdm.MMT_LOGIN:
			return LoginDomainString;
		case EmaRdm.MMT_DIRECTORY:
			return DirectoryDomainString;
		case EmaRdm.MMT_DICTIONARY:
			return DictionaryDomainString;
		case EmaRdm.MMT_MARKET_PRICE:
			return MarketPriceDomainString;
		case EmaRdm.MMT_MARKET_BY_ORDER:
			return MarketByOrderDomainString;
		case EmaRdm.MMT_MARKET_BY_PRICE:
			return MarketByPriceDomainString;
		case EmaRdm.MMT_MARKET_MAKER:
			return MarketMakerDomainString;
		case EmaRdm.MMT_SYMBOL_LIST:
			return SymbolListDomainString;
		case EmaRdm.MMT_SERVICE_PROVIDER_STATUS:
			return ServiceProviderStatusDomainString;
		case EmaRdm.MMT_HISTORY:
			return HistoryDomainString;
		case EmaRdm.MMT_HEADLINE:
			return HeadlineDomainString;
		case EmaRdm.MMT_STORY:
			return StoryDomainString;
		case EmaRdm.MMT_REPLAYHEADLINE:
			return ReplayHeadlineDomainString;
		case EmaRdm.MMT_REPLAYSTORY:
			return ReplayHistoryDomainString;
		case EmaRdm.MMT_TRANSACTION:
			return TransactionDomainString;
		case EmaRdm.MMT_YIELD_CURVE:
			return YieldCurveDomainString;
		case EmaRdm.MMT_CONTRIBUTION:
			return ContributionDomainString;
		case EmaRdm.MMT_PROVIDER_ADMIN:
			return ProviderAdminDomainString;
		case EmaRdm.MMT_ANALYTICS:
			return AnalyticsDomainString;
		case EmaRdm.MMT_REFERENCE:
			return ReferenceDomainString;
		case EmaRdm.MMT_NEWS_TEXT_ANALYTICS:
			return NewsTextAnalyticsDomainString;
		case EmaRdm.MMT_SYSTEM:
			return SystemDomainString;
		default:
			String defaultStr = "Unknown RDM Domain. Value='" + domain + "'";
			return defaultStr;
		}
	}

	static StringBuilder addIndent(StringBuilder temp, long indent, boolean addLine)
	{
		if (addLine)
			temp.append("\n");

		while (indent-- > 0)
			temp.append("    ");

		return temp;
	}

	static StringBuilder addIndent(StringBuilder temp, long indent)
	{
		while (indent-- > 0)
			temp.append("    ");

		return temp;
	}

	static StringBuilder asHexString(StringBuilder temp, ByteBuffer buffer)
	{
		int limit = buffer.limit();
		for (int i = buffer.position(); i < limit; i++)
			temp.append(i + 1 == limit ? String.format("%02x", buffer.get(i))
					: String.format("%02x ", buffer.get(i)));

		return temp;
	}

	static ByteBuffer copyFromPool(Buffer srcBuffer, ByteBuffer destByteBuffer, EmaObjectManager objManager)
	{
		int srcBufferlength = srcBuffer.length();
		if (destByteBuffer == null)
			destByteBuffer =  objManager.acquireByteBuffer(srcBufferlength);
		else if (destByteBuffer != null && destByteBuffer.capacity() < srcBufferlength)
		{
			objManager.releaseByteBuffer(destByteBuffer);
			destByteBuffer =  objManager.acquireByteBuffer(srcBufferlength);
		}
		
		destByteBuffer.clear();
		
		ByteBuffer srcByteBuffer = srcBuffer.data();
		int limit = srcBuffer.position() + srcBuffer.length();	
		for (int index =  srcBuffer.position(); index < limit; ++index)
			destByteBuffer.put(srcByteBuffer.get(index));
		
		destByteBuffer.flip();
		
		return destByteBuffer;
	}
	
	static void  copy(Buffer srcBuffer, Buffer destBuffer)
	{
		ByteBuffer destByteBuffer = destBuffer.data();
		int srcBufferlength = srcBuffer.length();
		
		if ( destByteBuffer == null || destBuffer.capacity()  < srcBufferlength)
			destByteBuffer = ByteBuffer.allocate(srcBufferlength);
		else
			destByteBuffer.clear();

		ByteBuffer srcByteBuffer = srcBuffer.data();
		int limit = srcBuffer.position() + srcBuffer.length();	
		for (int index =  srcBuffer.position(); index < limit; ++index)
			destByteBuffer.put(srcByteBuffer.get(index));
		
		destByteBuffer.flip();
		
		destBuffer.data(destByteBuffer);
	}
	
	static void copy(ByteBuffer srcByteBuffer, Buffer destBuffer)
	{
		ByteBuffer destByteBuffer = destBuffer.data();
		int srcByteBufferlength = srcByteBuffer.limit() - srcByteBuffer.position();

		if (destByteBuffer == null || destByteBuffer.capacity() < srcByteBufferlength)
			destByteBuffer =  ByteBuffer.allocate(srcByteBufferlength);
		else
			destByteBuffer.clear();
		
		int limit = srcByteBuffer.limit();
		for (int index = srcByteBuffer.position(); index < limit; ++index)
			destByteBuffer.put(srcByteBuffer.get(index));
		
		destByteBuffer.flip();
		
		destBuffer.data(destByteBuffer);
	}
	 
	static Buffer realignBuffer(com.thomsonreuters.upa.codec.EncodeIterator eIter, int newLength)
	{
		Buffer bigBuffer = CodecFactory.createBuffer();
		bigBuffer.data(ByteBuffer.allocate(newLength));
		eIter.realignBuffer(bigBuffer);
		return bigBuffer;
	}
}