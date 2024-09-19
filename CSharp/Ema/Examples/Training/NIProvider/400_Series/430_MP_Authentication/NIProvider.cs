/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using System.Text;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppLoginClient : IOmmProviderClient
{
    public long handle = 0;
    public long ttReissue = 0;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent e)
    {
        Console.WriteLine("Received Login Refresh Message\n");

        Console.WriteLine(refreshMsg);
        Console.WriteLine();

        /* Get the handle from the event and save it for a future reissue */
        handle = e.Handle;

        LoginRefresh loginRefresh = new LoginRefresh(refreshMsg);

        if (loginRefresh.HasAuthenticationTTReissue)
        {
            ttReissue = (long)loginRefresh.AuthenticationTTReissue();
        }

    }
    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmProviderEvent e)
    {
        Console.WriteLine("Received Login Update Message\n");

        Console.WriteLine(updateMsg);
        Console.WriteLine();
    }
    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent e)
    {
        Console.WriteLine("Received Login Status Message\n");

        Console.WriteLine(statusMsg);
        Console.WriteLine();
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
        try
        {
            AppLoginClient appLoginClient = new AppLoginClient();
            LoginReq loginReq = new LoginReq();

            if (!Init(args)) return;

            PrintActiveConfig();

            OmmNiProviderConfig config = new OmmNiProviderConfig();

            loginReq.Clear().Role(EmaRdm.LOGIN_ROLE_PROV).Name(authenticationToken).NameType(EmaRdm.USER_AUTH_TOKEN).ApplicationId(appId);
            if (authenticationExtended != "")
            {
                loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes(authenticationExtended)));
            }

            config.AddAdminMsg(loginReq.Message());

            provider = new OmmProvider(config, appLoginClient);

            long itemHandle = 5;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), itemHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                if (appLoginClient.ttReissue != 0 && appLoginClient.ttReissue <= DateTimeOffset.Now.ToUnixTimeSeconds())
                {
                    loginReq.Clear().Role(EmaRdm.LOGIN_ROLE_PROV).Name(authenticationToken).NameType(EmaRdm.USER_AUTH_TOKEN).ApplicationId(appId);
                    if (authenticationExtended != null && authenticationExtended.Length != 0)
                    {
                        loginReq.AuthenticationExtended(new EmaBuffer(Encoding.ASCII.GetBytes(authenticationExtended)));
                    }


                    provider.Reissue(loginReq.Message(), appLoginClient.handle);
                }
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), itemHandle);
                Thread.Sleep(1000);
            }
        }
        catch (Exception excp)
        {
            Console.WriteLine(excp.Message);
        }

        finally
        {
            provider?.Uninitialize();
        }
    }
}
