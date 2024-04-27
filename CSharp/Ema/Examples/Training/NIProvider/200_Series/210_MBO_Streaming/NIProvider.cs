/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.NIProvider;

public class NIProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();
            Map map = new Map();
            FieldList summary = new FieldList();
            FieldList entryLoad = new FieldList();
            long aaoHandle = 5;
            long aggHandle = 6;

            provider = new OmmProvider(config.UserName("user"));
            summary.AddEnumValue(15, 840);
            summary.AddEnumValue(53, 1);
            summary.AddEnumValue(3423, 1);
            summary.AddEnumValue(1709, 2);

            map.SummaryData(summary.Complete());

            entryLoad.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            entryLoad.AddRealFromDouble(3429, 9600);
            entryLoad.AddEnumValue(3428, 2);
            entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

            map.AddKeyAscii("100", MapAction.ADD, entryLoad.Complete());

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("TEST_NI_PUB").Name("AAO.V")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(map.Complete()).Complete(true),
                aaoHandle);

            summary.Clear();

            summary.AddEnumValue(15, 840);
            summary.AddEnumValue(53, 1);
            summary.AddEnumValue(3423, 1);
            summary.AddEnumValue(1709, 2);

            map.Clear();

            map.SummaryData(summary.Complete());

            entryLoad.Clear();

            entryLoad.AddRealFromDouble(3427, 9.92, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            entryLoad.AddRealFromDouble(3429, 1200);
            entryLoad.AddEnumValue(3428, 2);
            entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

            map.AddKeyAscii("222", MapAction.ADD, entryLoad.Complete());

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("TEST_NI_PUB").Name("AGG.V")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(map.Complete()).Complete(true),
                aggHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                entryLoad.Clear();

                entryLoad.AddRealFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                entryLoad.AddRealFromDouble(3429, 9600);
                entryLoad.AddEnumValue(3428, 2);
                entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                map.Clear();

                map.AddKeyAscii("100", MapAction.UPDATE, entryLoad.Complete());

                provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("TEST_NI_PUB").Name("AAO.V")
                    .Payload(map.Complete()), aaoHandle);

                entryLoad.Clear();

                entryLoad.AddRealFromDouble(3427, 9.92 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                entryLoad.AddRealFromDouble(3429, 1200);
                entryLoad.AddEnumValue(3428, 2);
                entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                map.Clear();

                map.AddKeyAscii("222", MapAction.UPDATE, entryLoad.Complete());

                provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("TEST_NI_PUB").Name("AGG.V")
                    .Payload(map.Complete()), aggHandle);

                Thread.Sleep(1000);
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
