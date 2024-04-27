/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Collections.Generic;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppClient : IOmmProviderClient
{
    public bool IsConnectionUp { get; private set; }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + providerEvent.Handle + " Closure: " + providerEvent.Closure);

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN)
        {
            if (refreshMsg.State().DataState == OmmState.DataStates.OK)
                IsConnectionUp = true;
            else
                IsConnectionUp = false;
        }
        else
            IsConnectionUp = false;
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Status. Item Handle: " + providerEvent.Handle + " Closure: " + providerEvent.Closure);

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
        {
            Console.WriteLine("Item State: " + statusMsg.State());
            if (statusMsg.State().StreamState == OmmState.StreamStates.OPEN)
            {
                if (statusMsg.State().DataState == OmmState.DataStates.OK)
                    IsConnectionUp = true;
                else
                {
                    IsConnectionUp = false;
                }
            }
            else
                IsConnectionUp = false;
        }
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
    }

    public void OnAllMsg(Msg msg, IOmmProviderEvent providerEvent)
    {
    }
}

public class NIProvider
{
    private static string authenticationToken = "";
    private static string appId = "256";

    public static void PrintHelp()
    {

        Console.WriteLine("\nOptions:\n"
                          + "  -?                            Shows this usage\n\n"
                          + "  -at <token>           Authentication token to use in login request [default = \"\"]\n"
                          + "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n"
                          + "\n");
    }

    public static void PrintInvalidOption()
    {
        Console.WriteLine("Detected a missing argument. Please verify command line options [-?]");
    }

    public static bool Init(string[] argv)
    {
        int count = argv.Length;
        int idx = 0;

        while (idx < count)
        {
            if (argv[idx].Equals("-?"))
            {
                PrintHelp();
                return false;
            }
            else if (argv[idx].Equals("-aid"))
            {
                if (++idx >= count)
                {
                    PrintInvalidOption();
                    return false;
                }
                appId = argv[idx];
                ++idx;
            }
            else if (argv[idx].Equals("-at"))
            {
                if (++idx >= count)
                {
                    PrintInvalidOption();
                    return false;
                }
                authenticationToken = argv[idx];
                ++idx;
            }
            else
            {
                Console.WriteLine("Unrecognized option. Please see command line help. [-?]");
                return false;
            }
        }

        return true;
    }

    private static void PrintActiveConfig()
    {
        Console.WriteLine("Following options are selected:");

        Console.WriteLine("appId = " + appId);
        Console.WriteLine("Authentication Token = " + authenticationToken);
    }

    public static void Main(string[] args)
    {
        if (!Init(args))
            return;

        AppClient appClient = new AppClient();
        bool sendRefreshMsg = false;
        OmmProvider? provider = null;

        PrintActiveConfig();

        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            ElementList elementList = new ElementList();
            elementList.AddAscii(EmaRdm.ENAME_APP_ID, appId);
            provider = new OmmProvider(config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH)
                                       .UserName(authenticationToken)
                                       .ApplicationId(appId));

            RequestMsg reqMsg = new RequestMsg();

            long loginHandle = provider.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), appClient);

            provider.Dispatch(1_000_000);

            long itemHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.Complete();

            provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("TRI.N")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(fieldList).Complete(true),
                            itemHandle);

            provider.Dispatch(1_000_000);

            for (int i = 0; i < 60; i++)
            {
                if (appClient.IsConnectionUp)
                {
                    if (sendRefreshMsg)
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 14700 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.Complete();

                        provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("TRI.N")
                                        .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                                        .Payload(fieldList).Complete(true),
                                        itemHandle);

                        sendRefreshMsg = false;
                    }
                    else
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.Complete();

                        provider.Submit(new UpdateMsg().ServiceName("TEST_NI_PUB").Name("TRI.N").Payload(fieldList), itemHandle);
                    }
                }
                else
                {
                    sendRefreshMsg = true;
                }
                provider.Dispatch(1_000_000);
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
//END APIQA
