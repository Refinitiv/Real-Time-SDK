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
        if (ItemHandles.ContainsKey(providerEvent.ClientHandle) == false)
        {
            Console.WriteLine("did not find client " + providerEvent.ClientHandle + " in ItemHandles");
            return;
        }

        if (reqMsg.DomainType() == EmaRdm.MMT_LOGIN)
        {	// removing client
            Console.WriteLine("removing client " + providerEvent.ClientHandle);
            ItemHandles.Remove(providerEvent.ClientHandle);
        }
        else if (reqMsg.DomainType() == EmaRdm.MMT_MARKET_PRICE)
        {	// removing item
            Console.WriteLine("removing item " + providerEvent.Handle + " from client " + providerEvent.ClientHandle);
            List<long> tmp = ItemHandles[(long)providerEvent.ClientHandle];
            if (tmp == null)
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

        if (ItemHandles.ContainsKey((long)providerEvent.ClientHandle) == true)
        {
            Console.WriteLine("map already contains an element with handle" + providerEvent.ClientHandle);
        }
        else
        {
            ItemHandles[(long)providerEvent.ClientHandle] = new List<long>();
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

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
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
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

//APIQA
class AppErrorClient : IOmmProviderErrorClient
{
    public void OnInvalidHandle(long handle, string text)
    {
        Console.WriteLine("onInvalidHandle callback function" + "\nInvalid handle: " + handle + "\nError text: " + text);
    }

    public void OnInvalidUsage(string text, int errorCode)
    {
        Console.WriteLine("onInvalidUsage callback function" + "\nError text: " + text + " , Error code: " + errorCode);
    }
}
//END API QA

public class IProvider
{
    static int maxOutputBuffers = 2000;
    static int guaranteedOutputBuffers = 2000;
    static int highWaterMark = 1000;
    static int serverNumPoolBuffers = 3000;
    static int compressionThreshold = 40;

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n"
            + "  -?\tShows this usage\n"
            + "  -maxOutputBuffers : value of maxOutputBuffer to modify.\r\n"
            + "  -guaranteedOutputBuffers : value of guaranteedOutputBuffers to modify.\n"
            + "  -highWaterMark : value of highWaterMark to modify.\r\n"
            + "  -serverNumPoolBuffers : value of serverNumPoolBuffer to modify.\r\n"
            + "  -compressionThreshold : value of compressionThreshold to modify.\n"
            + "\n");
    }

    static bool ReadCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if (args[argsCount].Equals("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-maxOutputBuffers".Equals(args[argsCount]))
                {
                    maxOutputBuffers = argsCount < (args.Length - 1) ? int.Parse(args[++argsCount]) : maxOutputBuffers;
                    ++argsCount;
                }
                else if ("-guaranteedOutputBuffers".Equals(args[argsCount]))
                {
                    guaranteedOutputBuffers = argsCount < (args.Length - 1) ? int.Parse(args[++argsCount]) : guaranteedOutputBuffers;
                    ++argsCount;
                }
                else if ("-highWaterMark".Equals(args[argsCount]))
                {
                    highWaterMark = argsCount < (args.Length - 1) ? int.Parse(args[++argsCount]) : highWaterMark;
                    ++argsCount;
                }
                else if ("-serverNumPoolBuffers".Equals(args[argsCount]))
                {
                    serverNumPoolBuffers = argsCount < (args.Length - 1) ? int.Parse(args[++argsCount]) : serverNumPoolBuffers;
                    ++argsCount;
                }
                else if ("-compressionThreshold".Equals(args[argsCount]))
                {
                    compressionThreshold = argsCount < (args.Length - 1) ? int.Parse(args[++argsCount]) : compressionThreshold;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }

        }
        catch (Exception)
        {
            PrintHelp();
            return false;
        }

        return true;
    }

    public static void Main(string[] args)
    {
        OmmProvider? provider = null;

        try
        {
            AppClient appClient = new AppClient();
            if (!ReadCommandlineArgs(args))
            {
                return;
            }

            AppErrorClient appErrorClient = new AppErrorClient();
            FieldList fieldList = new FieldList();

            OmmIProviderConfig config = new OmmIProviderConfig("EmaConfig.xml");

            provider = new OmmProvider(config.OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH).Port("14002"), appClient, appErrorClient);

            List<ChannelInformation> ci = new List<ChannelInformation>();

            while (appClient.ItemHandles.Count == 0)
            {
                provider.Dispatch(10_000);
            }

            DateTime startTime = DateTime.Now;
            DateTime nextPublishTime = startTime + TimeSpan.FromSeconds(1);
            int i = 0;

            for (int j = 0; j < 60; j++)
            {
                fieldList.Clear();
                ++i;
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                foreach (List<long> handles in appClient.ItemHandles.Values)
                {
                    foreach (long handle in handles)
                    {
                        if (j == 1)
                        {
                            Console.WriteLine("Modify maxOutputBuffers to " + maxOutputBuffers);
                            Console.WriteLine("Modify guaranteedOutputBuffers to " + guaranteedOutputBuffers);
                            Console.WriteLine("Modify highWaterMark to " + highWaterMark);
                            Console.WriteLine("Modify serverNumPoolBuffers to " + serverNumPoolBuffers);
                            Console.WriteLine("Modify compressionThreshold to " + compressionThreshold);

                            provider.ModifyIOCtl(IOCtlCode.MAX_NUM_BUFFERS, maxOutputBuffers, handle); // maxNumBuffer
                            provider.ModifyIOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, guaranteedOutputBuffers, handle); //guaranteedOutputBuffers
                            provider.ModifyIOCtl(IOCtlCode.HIGH_WATER_MARK, highWaterMark, handle); //highWaterMark
                            // RTSDK-7911 todo: no such code is implemented yet provider.ModifyIOCtl(8, serverNumPoolBuffers, handle); //serverNumPoolBuffers
                            provider.ModifyIOCtl(IOCtlCode.COMPRESSION_THRESHOLD, compressionThreshold, handle); //compressionThreshold

                        }
                        provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), handle);
                        Console.WriteLine("----Sent Update message index : " + j);
                        Thread.Sleep(1000);
                    }
                }
                while (true)
                {
                    TimeSpan dispatchTime = nextPublishTime - DateTime.Now;
                    if (dispatchTime > TimeSpan.Zero)
                    {
                        provider.Dispatch((int)dispatchTime.TotalMilliseconds * 1000);
                    }
                    else
                    {
                        nextPublishTime += TimeSpan.FromSeconds(1);
                        break;
                    }
                }

                //if (appClient.ItemHandles.size() != clientCount) {
                if (appClient.ItemHandles.Count > 0)
                {
                    //clientCount = appClient.ItemHandles.size();
                    provider.ConnectedClientChannelInfo(ci);
                    Console.WriteLine(ci.Count + " connected clients");
                    //for ( ChannelInformation K : ci)
                    //Console.WriteLine("client: " + K);
                    foreach (ChannelInformation channelInfo in ci)
                    {
                        Console.WriteLine("client: " + channelInfo);
                        Console.WriteLine("Test getMaxOutputBuffers() : " + channelInfo.MaxOutputBuffers);
                        Console.WriteLine("Test getGuaranteedOutputBuffers() : " + channelInfo.GuaranteedOutputBuffers);
                        Console.WriteLine("Test getCompressionThreshold() : " + channelInfo.CompressionThreshold);
                    }
                }
            }

            if (appClient.ItemHandles.Count > 0)
            {
                Console.WriteLine(ci.Count + " remaining connected clients after main loop");
                provider.ConnectedClientChannelInfo(ci);
                //for ( ChannelInformation K : ci) {
                //Console.WriteLine("client: " + K);
                foreach (ChannelInformation channelInfo in ci)
                {
                    Console.WriteLine("client: " + channelInfo);
                    Console.WriteLine("Test getMaxOutputBuffers() : " + channelInfo.MaxOutputBuffers);
                    Console.WriteLine("Test getGuaranteedOutputBuffers() : " + channelInfo.GuaranteedOutputBuffers);
                    Console.WriteLine("Test getCompressionThreshold() : " + channelInfo.CompressionThreshold);
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
