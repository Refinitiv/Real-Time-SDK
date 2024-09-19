/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using System;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

public class NIProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            provider = new OmmProvider(config.Host("localhost:14003").UserName("user"));

            long itemHandle = 5;

            FieldList mapSummaryData = new FieldList();
            mapSummaryData.AddEnumValue(15, 840);
            mapSummaryData.AddEnumValue(53, 1);
            mapSummaryData.AddEnumValue(3423, 1);
            mapSummaryData.AddEnumValue(1709, 2);
            mapSummaryData.Complete();

            FieldList mapKeyAscii = new FieldList();
            mapKeyAscii.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            mapKeyAscii.AddRealFromDouble(3429, 9600);
            mapKeyAscii.AddEnumValue(3428, 2);
            mapKeyAscii.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));
            mapKeyAscii.Complete();

            Map map = new Map();
            map.SummaryData(mapSummaryData);
            map.AddKeyAscii("100", MapAction.ADD, mapKeyAscii);
            map.Complete();

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AAO.V")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(map).Complete(true), itemHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                mapKeyAscii = new FieldList();
                mapKeyAscii.AddRealFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                mapKeyAscii.AddRealFromDouble(3429, 9600);
                mapKeyAscii.AddEnumValue(3428, 2);
                mapKeyAscii.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));
                mapKeyAscii.Complete();

                map = new Map();
                map.AddKeyAscii("100", MapAction.ADD, mapKeyAscii);
                map.Complete();

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("AAO.V").DomainType(EmaRdm.MMT_MARKET_BY_ORDER).Payload(map),
                    itemHandle);
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
