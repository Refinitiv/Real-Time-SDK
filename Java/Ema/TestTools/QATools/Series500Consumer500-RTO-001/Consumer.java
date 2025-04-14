///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex500_PreferredHost_FileCfg;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.eta.codec.CodecReturnCodes;

import java.nio.file.Files;
import java.nio.file.Paths;

class AppClient implements OmmConsumerClient
{
    private boolean updateCalled = false;

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());

        if (DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().fieldList());

        System.out.println("\nEvent channel info (refresh)\n" + event.channelInformation());
        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (!updateCalled)
        {
            updateCalled = true;
            System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

            if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
                decode(updateMsg.payload().fieldList());

            System.out.println("\nEvent channel info (update)\n" + event.channelInformation());
            System.out.println();
        }
        else {
            System.out.println("skipped printing updateMsg");
        }
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
            System.out.println("Item State: " +statusMsg.state());

        System.out.println("\nEvent channel info (status)\n" + event.channelInformation());
        System.out.println();
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

    void decode(FieldList fieldList)
    {
        for (FieldEntry fieldEntry : fieldList)
        {
            System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.code())
                System.out.println(" blank");
            else
                switch (fieldEntry.loadType())
                {
                    case DataTypes.REAL :
                        System.out.println(fieldEntry.real().asDouble());
                        break;
                    case DataTypes.DATE :
                        System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
                        break;
                    case DataTypes.TIME :
                        System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
                        break;
                    case DataTypes.DATETIME :
                        System.out.println(fieldEntry.dateTime().day() + " / " + fieldEntry.dateTime().month() + " / " +
                                fieldEntry.dateTime().year() + "." + fieldEntry.dateTime().hour() + ":" +
                                fieldEntry.dateTime().minute() + ":" + fieldEntry.dateTime().second() + ":" +
                                fieldEntry.dateTime().millisecond() + ":" + fieldEntry.dateTime().microsecond()+ ":" +
                                fieldEntry.dateTime().nanosecond());
                        break;
                    case DataTypes.INT :
                        System.out.println(fieldEntry.intValue());
                        break;
                    case DataTypes.UINT :
                        System.out.println(fieldEntry.uintValue());
                        break;
                    case DataTypes.ASCII :
                        System.out.println(fieldEntry.ascii());
                        break;
                    case DataTypes.ENUM :
                        System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
                        break;
                    case DataTypes.RMTES :
                        System.out.println(fieldEntry.rmtes());
                        break;
                    case DataTypes.ERROR :
                        System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
                        break;
                    default :
                        System.out.println();
                        break;
                }
        }
    }
}

public class Consumer
{
    private static final String DEFAULT_SOCKET_CONSUMER_NAME = "Consumer_9_1";
    private static final String DEFAULT_WEBSOCKET_CONSUMER_NAME = "Consumer_9_2";

    private static String userName;
    private static String password;
    private static String clientId;
    private static String clientSecret;
    private static String clientJwk;
    private static String audience;
    private static String itemName = "IBM.N";
    private static String serviceName = "ELEKTRON_DD";
    private static boolean connectWebSocket = false;

    static void printHelp()
    {
        System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -username machine ID to perform authorization with the\r\n"
                + "\ttoken service (mandatory for V1 password credentials).\n"
                + "  -password password to perform authorization with the token \r\n"
                + "\tservice (mandatory for V1 password credentials).\n"
                + "  -clientId client ID for application making the request to (mandatory) \r\n"
                + "  -clientSecret client secret for application making the request to (mandatory for V2 oAuth client credentials)\r\n"
                + "  -jwkFile file containing the private JWK encoded in JSON format. (mandatory for V2 client credentials grant with JWT)\n"
                + "  -audience Audience value for JWT (optional for V2 oAuth client credentials with JWT).\n"
                + "  -websocket Use the WebSocket transport protocol (optional) \r\n"
                + "\tRDP token service, also known as AppKey generated using an AppGenerator (mandatory).\n"
                + "  -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials(optional, only for V1 oAuth password credentials).\r\n"
                + "  -keyfile keystore file for encryption (mandatory).\n"
                + "  -keypasswd keystore password for encryption (mandatory).\n"
                + "  -tokenURL URL to perform authentication to get access and refresh tokens for V1 oAuth password credentials (optional).\n"
                + "  -tokenURLV1 URL to perform authentication to get access and refresh tokens for V1 oAuth password credentials (optional).\n"
                + "  -tokenURLV2 URL to perform authentication to get access and refresh tokens for V2 oAuth password credentials (optional).\n"
                + "  -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).\n"
                + "\nOptional parameters for establishing a connection and sending requests through a proxy server:\n"
                + "  -itemName Request item name (optional).\n"
                + "  -serviceName Service name (optional).\n"
                + "  -ph Proxy host name (optional).\n"
                + "  -pp Proxy port number (optional).\n"
                + "  -plogin User name on proxy server (optional).\n"
                + "  -ppasswd Password on proxy server (optional).\n"
                + "  -pdomain Proxy Domain (optional).\n"
                + "  -krbfile KRB File location and name. Needed for Negotiate/Kerberos \r\n"
                + "\tand Kerberos authentications (optional).\n"
                + "  -spTLSv1.2 Enable TLS 1.2 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
                + "  -spTLSv1.3 Enable TLS 1.3 security protocol. Default enables both TLS 1.2 and TLS 1.3 (optional). \n"
                + "\n");
    }

    static boolean readCommandlineArgs(String[] args, OmmConsumerConfig config)
    {
        try
        {
            int argsCount = 0;
            boolean tls12 = false;
            boolean tls13 = false;

            while (argsCount < args.length)
            {
                if (0 == args[argsCount].compareTo("-?"))
                {
                    printHelp();
                    return false;
                }
                else if ("-username".equals(args[argsCount]))
                {
                    userName = argsCount < (args.length-1) ? args[++argsCount] : null;
                    ++argsCount;
                    config.username(userName);
                }
                else if ("-password".equals(args[argsCount]))
                {
                    password = argsCount < (args.length-1) ? args[++argsCount] : null;
                    ++argsCount;
                    config.password(password);
                }
                else if ("-clientId".equals(args[argsCount]))
                {
                    clientId = argsCount < (args.length-1) ? args[++argsCount] : null;
                    config.clientId(clientId);
                    ++argsCount;
                }
                else if ("-clientSecret".equals(args[argsCount]))
                {
                    clientSecret = argsCount < (args.length-1) ? args[++argsCount] : null;
                    config.clientSecret(clientSecret);
                    ++argsCount;
                }
                else if ("-jwkFile".equals(args[argsCount]))
                {
                    String jwkFile = argsCount < (args.length-1) ? args[++argsCount] : null;
                    if(jwkFile != null)
                    {
                        try
                        {
                            // Get the full contents of the JWK file.
                            byte[] jwkBuffer = Files.readAllBytes(Paths.get(jwkFile));
                            clientJwk = new String(jwkBuffer);
                            config.clientJWK(clientJwk);
                        }
                        catch(Exception e)
                        {
                            System.err.println("Error loading JWK file: " + e.getMessage());
                            System.err.println();
                            System.out.println("Consumer exits...");
                            System.exit(CodecReturnCodes.FAILURE);
                        }
                    }
                    ++argsCount;
                }
                else if ("-audience".equals(args[argsCount]))
                {
                    audience = argsCount < (args.length-1) ? args[++argsCount] : null;
                    config.audience(audience);
                    ++argsCount;
                }
                else if ("-keyfile".equals(args[argsCount]))
                {
                    config.tunnelingKeyStoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-keypasswd".equals(args[argsCount]))
                {
                    config.tunnelingKeyStorePasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-itemName".equals(args[argsCount]))
                {
                    itemName = argsCount < (args.length-1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else if ("-serviceName".equals(args[argsCount]))
                {
                    serviceName = argsCount < (args.length-1) ? args[++argsCount] : null;
                    ++argsCount;
                }
                else if ("-ph".equals(args[argsCount]))
                {
                    config.tunnelingProxyHostName(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-pp".equals(args[argsCount]))
                {
                    config.tunnelingProxyPort(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-plogin".equals(args[argsCount]))
                {
                    config.tunnelingCredentialUserName(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-ppasswd".equals(args[argsCount]))
                {
                    config.tunnelingCredentialPasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-pdomain".equals(args[argsCount]))
                {
                    config.tunnelingCredentialDomain(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-krbfile".equals(args[argsCount]))
                {
                    config.tunnelingCredentialKRB5ConfigFile(argsCount < (args.length-1) ? args[++argsCount] : null);
                    ++argsCount;
                }
                else if ("-takeExclusiveSignOnControl".equals(args[argsCount]))
                {
                    String takeExclusiveSignOnControl = argsCount < (args.length-1) ? args[++argsCount] : null;

                    if(takeExclusiveSignOnControl != null)
                    {
                        if(takeExclusiveSignOnControl.equalsIgnoreCase("true"))
                            config.takeExclusiveSignOnControl(true);
                        else if (takeExclusiveSignOnControl.equalsIgnoreCase("false"))
                            config.takeExclusiveSignOnControl(false);
                    }

                    ++argsCount;
                }
                else if ("-websocket".equals(args[argsCount]))
                {
                    connectWebSocket = true;
                    ++argsCount;
                }
                else if ("-tokenURL".equals(args[argsCount]))
                {
                    if ( argsCount < (args.length-1) )
                    {
                        config.tokenServiceUrl( args[++argsCount] );
                    }
                    ++argsCount;
                }
                else if ("-tokenURLV1".equals(args[argsCount]))
                {
                    if ( argsCount < (args.length-1) )
                    {
                        config.tokenServiceUrlV1( args[++argsCount] );
                    }
                    ++argsCount;
                }
                else if ("-tokenURLV2".equals(args[argsCount]))
                {
                    if ( argsCount < (args.length-1) )
                    {
                        config.tokenServiceUrlV2( args[++argsCount] );
                    }
                    ++argsCount;
                }
                else if ("-serviceDiscoveryURL".equals(args[argsCount]))
                {
                    if ( argsCount < (args.length-1) )
                    {
                        config.serviceDiscoveryUrl( args[++argsCount] );
                    }
                    ++argsCount;
                }
                else if ("-spTLSv1.2".equals(args[argsCount]))
                {
                    tls12 = true;
                    ++argsCount;
                }
                else if ("-spTLSv1.3".equals(args[argsCount]))
                {
                    tls13 = true;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    printHelp();
                    return false;
                }
            }

            // Set security protocol versions of TLS based on configured values, with default having TLS 1.2 and 1.3 enabled
            if ((tls12 && tls13) || (!tls12 && !tls13))
            {
                config.tunnelingSecurityProtocol("TLS");
                config.tunnelingSecurityProtocolVersions(new String[] {"1.2", "1.3"});
            }
            else if (tls12)
            {
                config.tunnelingSecurityProtocol("TLS");
                config.tunnelingSecurityProtocolVersions(new String[]{"1.2"});
            }
            else if (tls13)
            {
                config.tunnelingSecurityProtocol("TLS");
                config.tunnelingSecurityProtocolVersions(new String[]{"1.3"});
            }

            if ( (userName == null || password == null || clientId == null) && (clientId == null || (clientSecret == null && clientJwk == null)))
            {
                System.out.println("Username, password, and clientId or clientId and clientSecret/jwkFile must be specified on the command line. Exiting...");
                printHelp();
                return false;
            }
        }
        catch (Exception e)
        {
            printHelp();
            return false;
        }

        return true;
    }

    public static void main(String[] args)
    {
        OmmConsumer consumer = null;
        try
        {
            OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();

            if (!readCommandlineArgs(args, config)) {
                return;
            }

            AppClient appClient = new AppClient();

            // The "Consumer_9_1" uses EncryptedProtocolType::RSSL_SOCKET while the "Consumer_9_2" uses EncryptedProtocolType::RSSL_WEBSOCKET predefined in EmaConfig.xml
            consumer  = EmaFactory.createOmmConsumer(config.consumerName(connectWebSocket ? DEFAULT_WEBSOCKET_CONSUMER_NAME : DEFAULT_SOCKET_CONSUMER_NAME));

            consumer.registerClient(EmaFactory.createReqMsg()
                    .serviceName(serviceName)
                    .name(itemName), appClient, 0);

            int printInterval = 5;
            ChannelInformation ci = EmaFactory.createChannelInformation();
            for (int i = 0; i < 600; i++) {
                Thread.sleep(1000); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

                if ((i % printInterval == 0)) {
                    consumer.channelInformation(ci);
                    if (ci.channelState() != ChannelInformation.ChannelState.INACTIVE) {
                        System.out.println("\nChannel information (consumer):\n\t" + ci);
                        System.out.println();
                    }
                }
            }
        }
        catch (InterruptedException | OmmException ex)
        {
            System.out.println(ex.getMessage());
        }
        finally
        {
            if (consumer != null) consumer.uninitialize();
        }
    }
}


