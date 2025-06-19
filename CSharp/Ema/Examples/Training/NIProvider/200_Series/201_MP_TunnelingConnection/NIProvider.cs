/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.NIProvider;

public class NIProvider
{
    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n"
            + "  -?\tShows this usage\n"
            + "    if the application will attempt to make an encrypted\n"
            + "    connection, ChannelType must need to be set to ChannelType::RSSL_ENCRYPTED\n"
            + "    in EMA configuration file.\n"
            + "  -ph Proxy host name.\n"
            + "  -pp Proxy port number.\n"
            + "  -plogin User name on proxy server.\n"
            + "  -ppasswd Password on proxy server.\n"
            + "\n");
    }

    static bool ReadCommandlineArgs(string[] args, OmmNiProviderConfig config)
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
                else if ("-ph".Equals(args[argsCount]))
                {
                    config.ProxyHost(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-pp".Equals(args[argsCount]))
                {
                    config.ProxyPort(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else if ("-plogin".Equals(args[argsCount]))
                {
                    config.ProxyUserName(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else if ("-ppasswd".Equals(args[argsCount]))
                {
                    config.ProxyPassword(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
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
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            if (!ReadCommandlineArgs(args, config))
                return;

            provider = new OmmProvider(config.UserName("user").ProviderName("Provider_4"));

            long ibmHandle = 5;
            long triHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(fieldList.Complete()).Complete(true), ibmHandle);

            fieldList.Clear();

            fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(fieldList.Complete()).Complete(true), triHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N").Payload(fieldList.Complete()), ibmHandle);

                fieldList.Clear();
                fieldList.AddReal(22, 4100 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 21 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), triHandle);
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
