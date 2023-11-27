/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace LSEG.Ema.Access;

internal class Utilities
{
    private const int UnknownDT = -1;

    private const String LoginDomainString = "Login Domain";
    private const String DirectoryDomainString = "Directory Domain";
    private const String DictionaryDomainString = "Dictionary Domain";
    private const String MarketPriceDomainString = "MarketPrice Domain";
    private const String MarketByOrderDomainString = "MarketByOrder Domain";
    private const String MarketByPriceDomainString = "MarketByPrice Domain";
    private const String MarketMakerDomainString = "MarketMaker Domain";
    private const String SymbolListDomainString = "SymbolList Domain";
    private const String ServiceProviderStatusDomainString = "ServiceProviderStatus Domain";
    private const String HistoryDomainString = "History Domain";
    private const String HeadlineDomainString = "Headline Domain";
    private const String StoryDomainString = "Story Domain";
    private const String ReplayHeadlineDomainString = "ReplayHeadline Domain";
    private const String ReplayHistoryDomainString = "ReplayHistory Domain";
    private const String TransactionDomainString = "Transaction Domain";
    private const String YieldCurveDomainString = "YieldCurve Domain";
    private const String ContributionDomainString = "Contribution Domain";
    private const String ProviderAdminDomainString = "ProviderAdmin Domain";
    private const String AnalyticsDomainString = "Analytics Domain";
    private const String ReferenceDomainString = "Reference Domain";
    private const String NewsTextAnalyticsDomainString = "NewsTextAnalytics Domain";
    private const String SystemDomainString = "System Domain";

    internal static LSEG.Eta.Codec.CodecReturnCode RealignBuffer(EncodeIterator eIter, int newLength)
    {
        eIter.Buffer().Data(new ByteBuffer(newLength));

        return eIter.RealignBuffer(eIter.Buffer());
    }

    internal static StringBuilder AddIndent(StringBuilder temp, long indent, bool addLine)
    {
        if (addLine)
            temp.AppendLine();

        while (indent-- > 0)
            temp.Append("    ");

        return temp;
    }

    internal static StringBuilder AddIndent(StringBuilder temp, long indent)
    {
        while (indent-- > 0)
            temp.Append("    ");

        return temp;
    }

    internal static StringBuilder AsHexString(StringBuilder temp, EmaBuffer buffer)
    {
        for (int i = 0; i < buffer.Length; i++)
        {
            temp.AppendFormat(string.Format("0x{0:X2}", buffer[i]));
            if (i + 1 != buffer.Length)
            {
                temp.Append(' ');
            }
        }
        return temp;
    }

    internal static void ToRsslQos(uint rate, uint timeliness, Qos rsslQos)
    {
        rsslQos.IsDynamic = false;

        if (rate == OmmQos.Rates.TICK_BY_TICK)
            rsslQos.Rate(Eta.Codec.QosRates.TICK_BY_TICK);
        else if (rate == OmmQos.Rates.JUST_IN_TIME_CONFLATED)
            rsslQos.Rate(Eta.Codec.QosRates.JIT_CONFLATED);
        else
        {
            if (rate <= 65535)
            {
                rsslQos.Rate(Eta.Codec.QosRates.TIME_CONFLATED);
                rsslQos.RateInfo((int)rate);
            }
            else
                rsslQos.Rate(Eta.Codec.QosRates.JIT_CONFLATED);
        }

        if (timeliness == OmmQos.Timelinesses.REALTIME)
            rsslQos.Timeliness(Eta.Codec.QosTimeliness.REALTIME);
        else if (timeliness == OmmQos.Timelinesses.INEXACT_DELAYED)
            rsslQos.Timeliness(Eta.Codec.QosTimeliness.DELAYED_UNKNOWN);
        else
        {
            if (timeliness <= 65535)
            {
                rsslQos.Timeliness(Eta.Codec.QosTimeliness.DELAYED);
                rsslQos.TimeInfo((int)timeliness);
            }
            else
                rsslQos.Timeliness(Eta.Codec.QosTimeliness.DELAYED_UNKNOWN);
        }
    }

    internal static string RdmDomainAsString(int domain)
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

    internal static void Reallocate(Eta.Codec.EncodeIterator encodeIter, int newLength)
    {
        if ((encodeIter.Buffer() is not null)
            && (encodeIter.Buffer().Capacity >= newLength))
        {
            return;
        }

        Eta.Codec.Buffer bigBuffer = new();
        bigBuffer.Data(new ByteBuffer(newLength));

        encodeIter.SetBufferAndRWFVersion(bigBuffer, Eta.Codec.Codec.MajorVersion(),
                Eta.Codec.Codec.MinorVersion());
    }

    internal static int Convert_uint_int(uint value)
    {
        return value > int.MaxValue ? int.MaxValue : (int)value;
    }

    internal static int Convert_long_int(long value)
    {
        return value > int.MaxValue ? int.MaxValue : (int)value;
    }

    internal static int Convert_ulong_int(ulong value)
    {
        return value > int.MaxValue ? int.MaxValue : (int)value;
    }

    internal static long Convert_ulong_long(ulong value)
    {
        return value > long.MaxValue ? long.MaxValue : (long)value;
    }

    internal static uint Convert_ulong_uint(ulong value)
    {
        return value > uint.MaxValue ? uint.MaxValue : (uint)value;
    }
}
