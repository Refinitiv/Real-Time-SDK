/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using System;
using System.Text;
using System.Threading;

public class NIProvider
{
    public static void Main()
    {
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            using OmmProvider provider = new OmmProvider(config.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL).UserName("user"));

            long sourceDirectoryHandle = 1;
            long aaoHandle = 5;

            provider.Submit(
                new DirectoryRefreshMsg()
                    .ClearCache(true)
                    .Filter(DirectoryFilters.SERVICE_INFO_FILTER | DirectoryFilters.SERVICE_STATE_FILTER)
                    .Complete(true)
                    .ServiceList(sl => sl
                        .Add(new DirectoryService()
                            .ServiceId(1)
                            .Action(DirectoryMapAction.ADD)
                            .Info(i => i
                                .Action(DirectoryFilterAction.SET)
                                .ServiceName("TEST_NI_PUB")
                                .CapabilitiesList(cl => cl
                                    .Add(EmaRdm.MMT_MARKET_PRICE)
                                    .Add(EmaRdm.MMT_MARKET_BY_ORDER))
                                .DictionariesUsedList(dul => dul
                                    .Add("RWFFld")
                                    .Add("RWFEnum")))
                            .State(s => s
                                .Action(DirectoryFilterAction.SET)
                                .IsServiceUp(true)))),
                sourceDirectoryHandle);

            FieldList summary = new FieldList();
            FieldList entryLoad = new FieldList();

            summary.AddEnumValue(15, 840);
            summary.AddEnumValue(53, 1);
            summary.AddEnumValue(3423, 1);
            summary.AddEnumValue(1709, 2);

            Map map = new Map();

            map.SummaryData(summary.Complete());

            entryLoad.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            entryLoad.AddRealFromDouble(3429, 9600);
            entryLoad.AddEnumValue(3428, 2);
            entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

            map.AddKeyAscii("100", MapAction.ADD, entryLoad.Complete());

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("TEST_NI_PUB").Name("AAO.V")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(map.Complete()).Complete(true), aaoHandle);

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

                Thread.Sleep(1000);
            }
        }
        catch (Exception excp)
        {
            Console.WriteLine(excp.Message);
        }
    }
}
