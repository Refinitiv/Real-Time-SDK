/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using LSEG.Eta.Common;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;

    #region APIQA
    //	public String OrderNr="100";
    public string OrderNr = "_CACHE_LIST.1093.8";
    #endregion APIQA

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_BY_ORDER:
                ProcessMarketByOrderRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    static void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketByOrderRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        #region APIQA
        List<string> itemNames = new ();

        try
        {
            foreach (string line in File.ReadLines("xmlKeyList"))
            {
                itemNames.Add(line.Trim());
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"File not found: {e.Message}");
        }
        #endregion APIQA

        #region APIQA
        //	entryData.add(new FieldEntry().realFromDouble(3427, 7.76, MagnitudeType.EXPONENT_NEG_2));
        //	entryData.add(new FieldEntry().realFromDouble(3429, 9600));
        //	entryData.add(new FieldEntry().enumValue(3428, 2));
        //	entryData.add(new FieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
        //	ByteBuffer buf = ByteBuffer.wrap(OrderNr.getBytes());
        //	map.add(new MapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryData));
        #endregion APIQA

        const int BatchSize = 20;
        foreach (var itemNameSet in itemNames.Chunk(BatchSize).Select((items, index) => (Items: items, Index: index)))
        {
            bool isComplete = itemNameSet.Index == (itemNames.Count - 1) / BatchSize;
            
            using (ClearableUtils<Map>.Reuse(out var map))
            {
                using (ClearableUtils<FieldList>.Reuse(out var mapSummaryData))
                {
                    mapSummaryData.AddEnumValue(15, 840);
                    mapSummaryData.AddEnumValue(53, 1);
                    mapSummaryData.AddEnumValue(3423, 1);
                    mapSummaryData.AddEnumValue(1709, 2);
                    map.SummaryData(mapSummaryData.Complete());
                }

                foreach (var itemName in itemNameSet.Items)
                {
                    using (ClearableUtils<FieldList>.Reuse(out var entryData))
                    {
                        EmaBuffer buf = new(Encoding.ASCII.GetBytes(itemName));
                        map.AddKeyBuffer(buf, MapAction.ADD, entryData.Complete());
                    }
                }

                providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
                    .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, 
                        isComplete switch { false => "Refresh Not Completed", true => "Refresh Completed" })
                    .Payload(map.Complete()).Complete(isComplete),
                    providerEvent.Handle);
            }
        }

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{
    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new ();
            OmmIProviderConfig config = new ();

            provider = new (config.Port("14002"), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }
            #region APIQA
            /*
            for (int i = 0; i < 60; i++)
            {
                using (ClearableUtils<FieldList>.Reuse(out var fieldList))
                {
                    fieldList.AddRealFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                    fieldList.AddRealFromDouble(3429, 9600);
                    fieldList.AddEnumValue(3428, 2);
                    fieldList.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));
                    using (ClearableUtils<Map>.Reuse(out var map))
                    {
                        map.AddKeyAscii(appClient.OrderNr, MapAction.ADD, fieldList.Complete());
                    }

                    provider.Submit( new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).Payload( map ), appClient.ItemHandle );

                    Thread.Sleep(1000);
                }
            }
            */
            Thread.Sleep(120_000);
            #endregion APIQA
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
