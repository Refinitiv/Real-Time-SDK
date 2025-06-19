/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Text;
using System.Threading;

public class NIProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            provider = new OmmProvider(config.AdminControlDirectory(OmmNiProviderConfig.AdminControlMode.USER_CONTROL).UserName("user"));

            long sourceDirectoryHandle = 1;
            long aaoHandle = 5;

            OmmArray capablities = new OmmArray();
            capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
            capablities.AddUInt(EmaRdm.MMT_MARKET_BY_ORDER);

            OmmArray dictionaryUsed = new OmmArray();
            dictionaryUsed.AddAscii("RWFFld");
            dictionaryUsed.AddAscii("RWFEnum");

            ElementList serviceInfoId = new ElementList();

            serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "TEST_NI_PUB");
            serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities.Complete());
            serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed.Complete());

            ElementList serviceStateId = new ElementList();
            serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);

            FilterList filterList = new FilterList();
            filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId.Complete());
            filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId.Complete());

            Map map = new Map();
            map.AddKeyUInt(1, MapAction.ADD, filterList.Complete());

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_DIRECTORY).ClearCache(true).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                .Payload(map.Complete()).Complete(true), sourceDirectoryHandle);

            FieldList summary = new FieldList();
            FieldList entryLoad = new FieldList();

            summary.AddEnumValue(15, 840);
            summary.AddEnumValue(53, 1);
            summary.AddEnumValue(3423, 1);
            summary.AddEnumValue(1709, 2);

            map.Clear();

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
        finally
        {
            provider?.Uninitialize();
        }
    }
}
