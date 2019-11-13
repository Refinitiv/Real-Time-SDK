///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.consumer.series400.example430__MarketPrice__TrepAuthentication;

import com.thomsonreuters.ema.access.Msg;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.domain.login.Login.LoginRefresh;
import com.thomsonreuters.ema.domain.login.Login.LoginReq;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Refresh Message\n");

        System.out.println(refreshMsg);
        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Update Message\n");

        System.out.println(updateMsg);
        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Status Message\n");

        System.out.println(statusMsg);
        System.out.println();
    }

    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event)
    {
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event)
    {
    }

    public void onAllMsg(Msg msg, OmmConsumerEvent event)
    {
    }
}

class AppLoginClient implements OmmConsumerClient
{
    public long handle = 0;
    public long ttReissue = 0;

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Login Refresh Message\n");

        System.out.println(refreshMsg);
        System.out.println();

        /* Get the handle from the event and save it for a future reissue */
        handle = event.handle();
        /* Get the time to reissue from the refresh and save it */
        LoginRefresh loginRefresh = EmaFactory.Domain.createLoginRefresh().message(refreshMsg);

        if (loginRefresh.hasAuthenticationTTReissue())
            ttReissue = loginRefresh.authenticationTTReissue();

    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Login Update Message\n");

        System.out.println(updateMsg);
        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Login Status Message\n");

        System.out.println(statusMsg);
        System.out.println();
    }

    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event)
    {
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event)
    {
    }

    public void onAllMsg(Msg msg, OmmConsumerEvent event)
    {
    }
}

public class Consumer
{
    private static String authenticationToken = "";
    private static String newauthenticationToken = "";
    private static String appId = "256";
    private static String authenticationExtended = "";

    public static void printHelp()
    {

        System.out.println("\nOptions:\n" + "  -?                            Shows this usage\n\n" + "  -authntoken <token>           Authentication token to use in login request [default = \"\"]\n"
                + "  -authnextended <name>         Extended authentication information to use in login request [default = \"\"]\n"
                + "  -appid <applicationId>        ApplicationId set as login Attribute [default = 256]\n" + "\n");
    }

    public static void printInvalidOption()
    {
        System.out.println("Detected a missing argument. Please verify command line options [-?]");
    }

    public static boolean init(String[] argv)
    {
        int count = argv.length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].compareTo("-?"))
            {
                printHelp();
                return false;
            }
            else if (0 == argv[idx].compareTo("-aid"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                appId = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareTo("-at"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                authenticationToken = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareTo("-newat"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                newauthenticationToken = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareTo("-ax"))
            {
                if (++idx >= count)
                {
                    printInvalidOption();
                    return false;
                }
                authenticationExtended = argv[idx];
                ++idx;
            }
            else
            {
                System.out.println("Unrecognized option. Please see command line help. [-?]");
                return false;
            }
        }

        return true;
    }

    private static void printActiveConfig()
    {
        System.out.println("Following options are selected:");

        System.out.println("appId = " + appId);
        System.out.println("Authentication Token = " + authenticationToken);
        System.out.println("New Authentication Token = " + newauthenticationToken);
        System.out.println("Authentication Extended = " + authenticationExtended);
    }

    public static void main(String[] args)
    {
        OmmConsumer consumer = null;
        try
        {
            if (!init(args))
                return;
            AppClient appClient = new AppClient();
            AppLoginClient appLoginClient = new AppLoginClient();
            LoginReq loginReq = EmaFactory.Domain.createLoginReq();

            printActiveConfig();

            OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();

            loginReq.clear().name(authenticationToken).nameType(EmaRdm.USER_AUTH_TOKEN).applicationId(appId);

            if (!authenticationExtended.isEmpty())
            {
                System.out.println("setting authnextended\n");
                loginReq.authenticationExtended(ByteBuffer.wrap(authenticationExtended.getBytes()));
            }

            config.addAdminMsg(loginReq.message());
            consumer = EmaFactory.createOmmConsumer(config, appLoginClient);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("TRI.N"), appClient);

            for (int i = 0; i < 120; i++)
            {
                if (appLoginClient.ttReissue != 0 && appLoginClient.ttReissue <= (System.currentTimeMillis() / 1000))
                {
                    loginReq.clear().name(authenticationToken).nameType(EmaRdm.USER_AUTH_TOKEN).applicationId(appId);

                    if (!authenticationExtended.isEmpty())
                        loginReq.authenticationExtended(ByteBuffer.wrap(authenticationExtended.getBytes()));

                    consumer.reissue(loginReq.message(), appLoginClient.handle);
                    appLoginClient.ttReissue = 0;
                }

                Thread.sleep(1000);
                if (i == 3)
                {
                    System.out.println("Pausing all item streams");
                    loginReq.clear().name(authenticationToken).nameType(EmaRdm.USER_AUTH_TOKEN).applicationId(appId).pause(true);
                    consumer.reissue(loginReq.message(), appLoginClient.handle);
                }
                if (i == 10)
                {
                    System.out.println("Resuming all item streams");
                    loginReq.clear().name(authenticationToken).nameType(EmaRdm.USER_AUTH_TOKEN).applicationId(appId).pause(false);
                    consumer.reissue(loginReq.message(), appLoginClient.handle);
                }
            }

        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
        finally
        {
            if (consumer != null)
                consumer.uninitialize();
        }
    }
}
//END APIQA
