/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using LSEG.Ema.Domain.Login;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppLoginClient : IOmmProviderClient
{

    public long handle = 0;
    public ulong ttReissue = 0;

    public bool IsConnectionUp { get; private set; }

    // END APIQA
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Login Refresh Message\n");

        Console.WriteLine(refreshMsg);
        Console.WriteLine();

        /* Get the handle from the event and save it for a future reissue */
        handle = providerEvent.Handle;

        LoginRefresh loginRefresh = new LoginRefresh().Message(refreshMsg);

        if (loginRefresh.HasAuthenticationTTReissue)
            ttReissue = loginRefresh.AuthenticationTTReissue();

        // APIQA:
        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN)
        {
            if (refreshMsg.State().DataState == OmmState.DataStates.OK)
                IsConnectionUp = true;
            else
                IsConnectionUp = false;
        }
        else
            IsConnectionUp = false;
        // END APIQA
    }

    public void onUpdateMsg(UpdateMsg updateMsg, IOmmProviderEvent _)
    {
        Console.WriteLine("Received Login Update Message\n");

        Console.WriteLine(updateMsg);
        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Login Status Message\n");

        Console.WriteLine(statusMsg);
        Console.WriteLine();
        // APIQA:
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
        // END APIQA
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
    private static string authenticationExtended = "";

    public static void PrintHelp()
    {

        Console.WriteLine("\nOptions:\n"
                          + "  -?                            Shows this usage\n\n"
                          + "  -at <token>           Authentication token to use in login request [default = \"\"]\n"
                          + "  -ax <name>         Extended authentication information to use in login request [default = \"\"]\n"
                          + "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n" + "\n");
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
            else if (argv[idx].Equals("-ax"))
            {
                if (++idx >= count)
                {
                    PrintInvalidOption();
                    return false;
                }
                authenticationExtended = argv[idx];
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
        Console.WriteLine("Authentication Extended = " + authenticationExtended);
    }

    public static void Main(string[] args)
    {
        OmmProvider? provider = null;

        // APIQA
        bool sendRefreshMsg = false;
        // END APIQA

        try
        {
            AppLoginClient appLoginClient = new AppLoginClient();
            LoginReq loginReq = new LoginReq();

            if (!Init(args))
                return;

            PrintActiveConfig();

            OmmNiProviderConfig config = new OmmNiProviderConfig();

            loginReq.Clear()
                .Role(EmaRdm.LOGIN_ROLE_PROV)
                .Name(authenticationToken)
                .NameType(EmaRdm.USER_AUTH_TOKEN)
                .ApplicationId(appId);

            if (!string.IsNullOrEmpty(authenticationExtended))
                loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes(authenticationExtended)));

            config.AddAdminMsg(loginReq.Message());

            provider = new OmmProvider(config, appLoginClient);

            long itemHandle = 5;

            FieldList fieldList = new FieldList();
            fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.Complete();

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(fieldList)
                            .Complete(true),
                            itemHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 300; i++)
            {
                // APIQA:
                if (appLoginClient.IsConnectionUp)
                {
                    // END APIQA
                    if (appLoginClient.ttReissue != 0
                        && appLoginClient.ttReissue <= (ulong)DateTimeOffset.Now.ToUnixTimeSeconds())
                    {
                        loginReq.Clear()
                            .Role(EmaRdm.LOGIN_ROLE_PROV)
                            .Name(authenticationToken)
                            .NameType(EmaRdm.USER_AUTH_TOKEN)
                            .ApplicationId(appId);

                        if (!string.IsNullOrEmpty(authenticationExtended))
                            loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes(authenticationExtended)));

                        provider.Reissue(loginReq.Message(), appLoginClient.handle);
                    }
                    // APIQA:
                    if (sendRefreshMsg)
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.Complete();

                        provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                                        .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                                        .Payload(fieldList)
                                        .Complete(true),
                                        itemHandle);

                        sendRefreshMsg = false;
                    }
                    else
                    {
                        // END APIQA
                        fieldList.Clear();
                        fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.Complete();

                        provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList), itemHandle);
                        // APIQA:
                    }
                }
                else
                {
                    sendRefreshMsg = true;
                    appLoginClient.ttReissue = 0;
                }
                // END APIQA
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
//END APIQA
