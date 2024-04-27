/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public Dictionary<long, List<long>> ItemHandles = new();

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("channel info for request message event\n\t" + providerEvent.ChannelInformation());
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        // sanity checking
        if (!ItemHandles.ContainsKey(providerEvent.ClientHandle))
        {
            Console.WriteLine("did not find client " + providerEvent.ClientHandle + " in ItemHandles");
            return;
        }

        if (reqMsg.DomainType() == EmaRdm.MMT_LOGIN)
        {
            // removing client
            Console.WriteLine("removing client " + providerEvent.ClientHandle);
            ItemHandles.Remove(providerEvent.ClientHandle);
        }
        else if (reqMsg.DomainType() == EmaRdm.MMT_MARKET_PRICE)
        {
            // removing item
            Console.WriteLine("removing item " + providerEvent.Handle + " from client " + providerEvent.ClientHandle);
            List<long> tmp = ItemHandles[(long)providerEvent.ClientHandle];
            if (tmp is null)
            {
                Console.WriteLine("client " + providerEvent.ClientHandle + " had no items");
                return;
            }
            tmp.Remove((long)providerEvent.Handle);
            ItemHandles[(long)providerEvent.ClientHandle] = tmp;
        }
        Console.WriteLine("channel info for close event:\n\t" + providerEvent.ChannelInformation());
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);

        if (ItemHandles.ContainsKey((long)providerEvent.ClientHandle))
        {
            Console.WriteLine("map already contains an element with handle" + providerEvent.ClientHandle);
        }
        else
        {
            ItemHandles[(long)(providerEvent.ClientHandle)] = new List<long>();
            Console.WriteLine("added client " + providerEvent.ClientHandle);
        }
        Console.WriteLine("channel info for login event:\n\t" + providerEvent.ChannelInformation());
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg()
            .Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        List<long> handles = ItemHandles[providerEvent.ClientHandle];
        if (handles is null)
        {
            Console.WriteLine("did not find client in ItemHandles for processMarketPriceRequest");
            return;
        }

        handles.Add(providerEvent.Handle);
        ItemHandles[providerEvent.ClientHandle] = handles;

        Console.WriteLine("added item " + providerEvent.Handle + " to client " + providerEvent.ClientHandle);
        Console.WriteLine("channel info for market price request event:\n\t" + providerEvent.ChannelInformation());
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg()
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        int clientCount = 0;

        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();

            OmmIProviderConfig config = new OmmIProviderConfig("EmaConfig.xml");

            provider = new OmmProvider(config.OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH).Port("14002"), appClient);

            List<ChannelInformation> ci = new List<ChannelInformation>();

            while (appClient.ItemHandles.Count == 0)
            {
                provider.Dispatch(10_000);
            }

            DateTime startTime = DateTime.Now;
            DateTime nextPublishTime = startTime + TimeSpan.FromMilliseconds(1000);
            int i = 0;

            while (startTime + TimeSpan.FromMilliseconds(60_000) > DateTime.Now)
            {
                fieldList.Clear();
                ++i;
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.Complete();

                foreach (List<long> handles in appClient.ItemHandles.Values)
                {
                    foreach (long handle in handles)
                    {
                        try
                        {
                            provider.Submit(new UpdateMsg().Payload(fieldList), handle);
                        }

                        catch (OmmInvalidUsageException)
                        {
                            Console.WriteLine("attempted to send message to invalid handle " + handle);
                        }
                    }
                }

                while (true)
                {
                    TimeSpan dispatchTime = nextPublishTime - DateTime.Now;
                    if (dispatchTime > TimeSpan.Zero)
                    {
                        provider.Dispatch((int)dispatchTime.TotalMilliseconds);
                    }
                    else
                    {
                        nextPublishTime += TimeSpan.FromMilliseconds(1000);
                        break;
                    }
                }

                if (appClient.ItemHandles.Count != clientCount)
                {
                    clientCount = appClient.ItemHandles.Count;
                    provider.ConnectedClientChannelInfo(ci);

                    Console.WriteLine(ci.Count + " connected clients");
                    foreach (ChannelInformation clientInfo in ci)
                    {
                        Console.WriteLine("client: " + clientInfo);
                    }
                }
            }
            Thread.Sleep(1000);

            if (appClient.ItemHandles.Count > 0)
            {
                Console.WriteLine(ci.Count + " remaining connected clients after main loop");
                provider.ConnectedClientChannelInfo(ci);

                foreach (ChannelInformation clientInfo in ci)
                {
                    Console.WriteLine("client: " + clientInfo);
                }
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
