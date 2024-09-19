/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{

    public static long? AckId;

    public AppClient() { }

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
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

    public void OnPostMsg(PostMsg postMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received PostMsg with id: " + postMsg.PostId());

        if (postMsg.SolicitAck())
        {
            AckMsg ackMsg = new AckMsg();

            if (postMsg.HasSeqNum)
            {
                ackMsg.SeqNum(postMsg.SeqNum());
            }
            if (postMsg.HasName)
            {
                ackMsg.Name(postMsg.Name());
            }
            if (postMsg.HasServiceId)
            {
                ackMsg.ServiceId(postMsg.ServiceId());
            }
            if (AckId is not null)
            {
                ackMsg.AckId((long)AckId);
            }
            else
            {
                ackMsg.AckId(postMsg.PostId());
            }
            try
            {
                ackMsg.DomainType(postMsg.DomainType());
                providerEvent.Provider.Submit(ackMsg, providerEvent.Handle);
            }
            catch (OmmException excp)
            {
                Console.WriteLine(excp.Message);
            }
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        ElementList elementList = new ElementList();
        elementList.AddUInt("SupportOMMPost", 1);

        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Attrib(elementList.Complete()),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().ServiceName(reqMsg.ServiceName()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Solicited(true).Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .DomainType(reqMsg.DomainType())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n"
            + "  -?\tShows this usage\n"
            + "  -ackId AckId sent in Ack messages.\n"
            + "\n");
    }

    static bool ReadCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if ("-?".Equals(args[argsCount]))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-ackId".Equals(args[argsCount]))
                {
                    AppClient.AckId = argsCount < (args.Length - 1) ? int.Parse(args[++argsCount]) : null;
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
        if (!ReadCommandlineArgs(args))
        {
            return;
        }

        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

            Thread.Sleep(120_000);
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
