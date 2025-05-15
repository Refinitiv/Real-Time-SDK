/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_DIRECTORY:
                ProcessDirectoryRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessDirectoryRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        OmmArray capablities = new OmmArray();
        capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
        capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);

        OmmArray dictionaryUsed = new OmmArray();
        dictionaryUsed.AddAscii("RWFFld");
        dictionaryUsed.AddAscii("RWFEnum");

        ElementList serviceInfoId = new ElementList();
        serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
        serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities.Complete());
        serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed.Complete());

        ElementList serviceStateId = new ElementList();
        serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);

        FilterList filterList = new FilterList();
        filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId.Complete());
        filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId.Complete());

        Map map = new Map();
        map.AddKeyUInt(2, MapAction.ADD, filterList.Complete());

        RefreshMsg refreshMsg = new RefreshMsg();

        //API QA
        try
        {
            Thread.Sleep(IProvider._delayDir);
            providerEvent.Provider.Submit(refreshMsg.DomainType(EmaRdm.MMT_DIRECTORY).ClearCache(true)
            .Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
            .Payload(map.Complete()).Solicited(true).Complete(true),
            providerEvent.Handle);
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

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
    //API QA
    public static String _portNumber = "14002";
    public static int _delayDir = 0;
    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -p port number.\n"
                + "  -delayDir Time to send directory response in milliseconds (default 0).\n"
                + "\n");
    }
    static bool ReadCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;
            int delaydir = 0;

            while (argsCount < args.Length)
            {
                if (0 == args[argsCount].CompareTo("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-p".Equals(args[argsCount]))
                {
                    IProvider._portNumber = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    Console.WriteLine("Port Number: " + IProvider._portNumber);
                    ++argsCount;
                }
                else if ("-delayDir".Equals(args[argsCount]))
                {
                    if (Int32.TryParse(args[++argsCount], out delaydir))
                    {
                        IProvider._delayDir = delaydir;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse delayDir '{args[argsCount]}'");
                        return false;
                    }
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

    //END APIQA
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            // API QA
            if (!ReadCommandlineArgs(args))
                return;
            //END API QA
            AppClient appClient = new AppClient();

            provider = new OmmProvider(new OmmIProviderConfig().
                    AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL).Port(IProvider._portNumber), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            FieldList fieldList = new FieldList();
            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);

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
