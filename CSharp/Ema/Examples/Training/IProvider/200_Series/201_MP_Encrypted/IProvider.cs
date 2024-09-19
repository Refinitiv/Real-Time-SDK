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

using static LSEG.Ema.Access.EmaConfig;

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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Attrib(new ElementList().Complete()),
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

        fieldList.AddAscii(3, reqMsg.Name());
        fieldList.AddEnumValue(15, 840);
        fieldList.AddReal(21, 3900, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().ServiceName(reqMsg.ServiceName()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Solicited(true).Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().DomainType(reqMsg.DomainType())
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
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
            + "  -cert file containing the server certificate for encryption.\n"
            + "  -key file containing the server private key for encryption.\n"
            + "  -spTLSv1.2 Enable TLS 1.2 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
            + "  -spTLSv1.3 Enable TLS 1.3 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
            + "\n");
    }

    static bool ReadCommandlineArgs(string[] args, OmmIProviderConfig config)
    {
        try
        {
            int argsCount = 0;
            bool tls12 = false;
            bool tls13 = false;

            while (argsCount < args.Length)
            {
                if (args[argsCount].Equals("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-cert".Equals(args[argsCount]))
                {
                    config.ServerCertificate(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else if ("-key".Equals(args[argsCount]))
                {
                    config.ServerPrivateKey(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else if ("-spTLSv1.2".Equals(args[argsCount]))
                {
                    tls12 = true;
                    ++argsCount;
                }
                else if ("-spTLSv1.3".Equals(args[argsCount]))
                {
                    tls13 = true;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }

            // Set security protocol versions of TLS based on configured values, with default having TLS 1.2 and 1.3 enabled
            if ((tls12 && tls13) || (!tls12 && !tls13))
            {
                config.EncryptedProtocolFlags(EncryptedTLSProtocolFlags.TLSv1_2 | EncryptedTLSProtocolFlags.TLSv1_3);
            }
            else if (tls12)
            {
                config.EncryptedProtocolFlags(EncryptedTLSProtocolFlags.TLSv1_2);
            }
            else if (tls13)
            {
                config.EncryptedProtocolFlags(EncryptedTLSProtocolFlags.TLSv1_3);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"Exception: {e.Message}");
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
            FieldList fieldList = new FieldList();
            UpdateMsg updateMsg = new UpdateMsg();
            OmmIProviderConfig config = new OmmIProviderConfig().ProviderName("EncryptedProvider")
                .OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH);

            if (!ReadCommandlineArgs(args, config))
            {
                return;
            }

            provider = new OmmProvider(config, appClient);

            while (appClient.ItemHandle == 0)
            {
                provider.Dispatch(1000);
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                provider.Dispatch(1000);

                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(25, 3994 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(updateMsg.Clear().Payload(fieldList.Complete()), appClient.ItemHandle);

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
