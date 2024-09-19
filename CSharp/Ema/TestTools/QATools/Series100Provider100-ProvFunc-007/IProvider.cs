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

//APIQA
class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    //APIQA
    public static int NUMOFITEMSINSYMBOLLIST = 100; // Number of items in symbolList

    public static void PrintHelp(bool reflect)
    {
        if (!reflect)
        {
            Console.WriteLine("\nOptions:\n"
                + "  -?\tShows this usage\n\n"
                + "  -c  \tSend the number of items in symbolList [default = 100]\n"
                + "\n");

            System.Environment.Exit(-1);
        }
        else
        {
            Console.WriteLine("\n  Options will be used:\n"
                + "  -c \t " + NUMOFITEMSINSYMBOLLIST + "\n\n");
        }
    }

    public bool ReadCommandlineArgs(string[] argv)
    {
        int count = argv.Length;
        int idx = 0;

        while (idx < count)
        {
            if (argv[idx].Equals("-?"))
            {
                PrintHelp(false);
                return false;
            }
            else if (argv[idx].Equals("-c"))
            {
                if (++idx >= count)
                {
                    PrintHelp(false);
                    return false;
                }
                if (int.TryParse(argv[idx], out int num))
                {
                    NUMOFITEMSINSYMBOLLIST = num;
                    ++idx;
                }
                else
                {
                    Console.WriteLine($"Could not parse as an int value: {argv[idx]}");
                    return false;
                }
            }
            else
            {
                PrintHelp(false);
                return false;
            }
        }
        PrintHelp(true);
        return true;
    }

    //ENA APIQA
    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            //APIQA
            case EmaRdm.MMT_SYMBOL_LIST:
                ProcessSymbolListRequest(reqMsg, providerEvent);
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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name())
            .NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
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

    //APIQA
    void ProcessSymbolListRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        Map mapEnc = new Map();
        FieldList fieldList1 = new FieldList();
        string a;

        for (int i = 0; i < NUMOFITEMSINSYMBOLLIST; ++i)
        {
            a = "A" + i;

            mapEnc.AddKeyAscii(a, MapAction.ADD, fieldList1.Complete());
        }

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(mapEnc.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }
    // END APIQA

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{

    public static void Main(string[] args)
    {

        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();

            //APIQA
            //FieldList fieldList = new FieldList();
            if (!appClient.ReadCommandlineArgs(args))
            {
                return;
            }

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                Thread.Sleep(1000);
            }
            // END APIQA
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
