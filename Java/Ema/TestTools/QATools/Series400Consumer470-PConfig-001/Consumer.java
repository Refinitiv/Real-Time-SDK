/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */
package com.refinitiv.ema.examples.training.consumer.series400.ex470_MP_WarmStandby;

import com.refinitiv.ema.access.*;

class AppClient implements OmmConsumerClient {

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event) {
        System.out.println(refreshMsg);
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {
        System.out.println(updateMsg);
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {
        System.out.println(statusMsg);
        if (statusMsg.state().statusCode() == OmmState.StatusCode.WSB_CHANGE_ACTIVE_COMPLETE) {
            System.out.println(event.warmStandbyChangeEventInfo());
        }
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {
    }

    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {
    }

    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {
    }
}

public class Consumer {
    // ── configurable defaults ─────────────────────────────────────────────────

    static String host1 = "localhost";
    static String port1 = "14002";
    static String host2 = "localhost";
    static String port2 = "14003";
    static String serviceName = "DIRECT_FEED";
    static String serviceName2 = "DIRECT_FEED";
    static String itemName = "SPOT";
    static String itemName2 = "TRI.N";
    static int channelType = 0;       // 0 = RSSL_SOCKET
    static int runtime = 60;      // seconds
    static int warmStandbyMode = 1;
    static boolean eventInfo = true;

    // ── usage text ────────────────────────────────────────────────────────────
    static void printHelp() {
        System.out.println(
                "Usage: Consumer [options]\n"
                + "\n"
                + "  -host1       <host>   Host for Channel_1 / Active server    (default: localhost)\n"
                + "  -port1       <port>   Port for Channel_1 / Active server    (default: 14002)\n"
                + "  -host2       <host>   Host for Channel_2 / Standby server   (default: localhost)\n"
                + "  -port2       <port>   Port for Channel_2 / Standby server   (default: 14003)\n"
                + "  -service     <name>   Service name to subscribe to          (default: DIRECT_FEED)\n"
                + "  -service2     <name>   Service name to subscribe to          (default: DIRECT_FEED)\n"
                + "  -item        <name>   Item name to subscribe to             (default: SPOT)\n"
                + "  -channelType <n>      Channel type: 0=SOCKET 2=HTTP 3=ENCRYPTED (default: 0)\n"
                + "  -runtime     <sec>    Run duration in seconds               (default: 60)\n"
                + "  -mode     <int>    	 Warm Stand by Mode(1 - login based, 2 -service based)  (default: 1)\n"
                + "  -info     <int>    	 Change event info  (default: true)\n"
                + "  -?                 Print this help message and exit\n"
        );
    }

    // ── argument parser ───────────────────────────────────────────────────────
    static void parseArgs(String[] args) {
        for (int i = 0; i < args.length; i++) {
            switch (args[i]) {
                case "-?":
                    printHelp();
                    System.exit(0);
                    break;

                case "-host1":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -host1");
                        printHelp();
                        System.exit(1);
                    }
                    host1 = args[++i];
                    break;

                case "-port1":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -port1");
                        printHelp();
                        System.exit(1);
                    }
                    port1 = args[++i];
                    break;

                case "-host2":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -host2");
                        printHelp();
                        System.exit(1);
                    }
                    host2 = args[++i];
                    break;

                case "-port2":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -port2");
                        printHelp();
                        System.exit(1);
                    }
                    port2 = args[++i];
                    break;

                case "-service":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -service");
                        printHelp();
                        System.exit(1);
                    }
                    serviceName = args[++i];
                    break;

                case "-service2":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -service");
                        printHelp();
                        System.exit(1);
                    }
                    serviceName2 = args[++i];
                    break;

                case "-item":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -item");
                        printHelp();
                        System.exit(1);
                    }
                    itemName = args[++i];
                    break;
                case "-item2":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -item");
                        printHelp();
                        System.exit(1);
                    }
                    itemName2 = args[++i];
                    break;

                case "-channelType":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -channelType");
                        printHelp();
                        System.exit(1);
                    }
                    try {
                        channelType = Integer.parseInt(args[++i]);
                    } catch (NumberFormatException e) {
                        System.out.println("Invalid integer for -channelType: " + args[i]);
                        printHelp();
                        System.exit(1);
                    }
                    break;

                case "-runtime":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -runtime");
                        printHelp();
                        System.exit(1);
                    }
                    try {
                        runtime = Integer.parseInt(args[++i]);
                    } catch (NumberFormatException e) {
                        System.out.println("Invalid integer for -runtime: " + args[i]);
                        printHelp();
                        System.exit(1);
                    }
                    break;

                case "-mode":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -runtime");
                        printHelp();
                        System.exit(1);
                    }
                    try {
                        warmStandbyMode = Integer.parseInt(args[++i]);
                    } catch (NumberFormatException e) {
                        System.out.println("Invalid integer for -mode: " + args[i]);
                        printHelp();
                        System.exit(1);
                    }
                    break;

                case "-info":
                    if (i + 1 >= args.length) {
                        System.out.println("Missing value for -runtime");
                        printHelp();
                        System.exit(1);
                    }
                    eventInfo = Boolean.parseBoolean(args[++i]);
                    break;

                default:
                    System.out.println("Unknown argument: " + args[i]);
                    printHelp();
                    System.exit(1);
            }
        }
    }

    // ── programmatic config ───────────────────────────────────────────────────
    static Map createProgramaticConfig() {
        Map configDb = EmaFactory.createMap();
        Map elementMap = EmaFactory.createMap();
        ElementList outerElementList = EmaFactory.createElementList();
        outerElementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_8"));

        ElementList innerElementList = EmaFactory.createElementList();
        innerElementList.add(EmaFactory.createElementEntry().ascii("WarmStandbyChannelSet", "WarmStandbyChannel_1"));
        innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_8", MapEntry.MapAction.ADD, innerElementList));

        outerElementList.add(EmaFactory.createElementEntry().map("ConsumerList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, outerElementList));
        outerElementList.clear();
        innerElementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().enumValue("ChannelType", channelType));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Host", host1));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", port1));
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().enumValue("ChannelType", channelType));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Host", host2));
        innerElementList.add(EmaFactory.createElementEntry().ascii("Port", port2));
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_2", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        outerElementList.add(EmaFactory.createElementEntry().map("ChannelList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, outerElementList));
        outerElementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("PerServiceNameSet", serviceName));
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Server_Info_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("PerServiceNameSet", serviceName2));
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Server_Info_2", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        outerElementList.add(EmaFactory.createElementEntry().map("WarmStandbyServerInfoList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyServerInfoGroup", MapEntry.MapAction.ADD, outerElementList));
        outerElementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().ascii("StartingActiveServer", "Server_Info_1"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("StandbyServerSet", "Server_Info_2"));
        innerElementList.add(EmaFactory.createElementEntry().enumValue("WarmStandbyMode", warmStandbyMode));
        /* 2 = service based, 1 = login based */
        elementMap.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyChannel_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        outerElementList.add(EmaFactory.createElementEntry().map("WarmStandbyList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyGroup", MapEntry.MapAction.ADD, outerElementList));
        outerElementList.clear();

        innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
        innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype.def"));
        elementMap.add(EmaFactory.createMapEntry().keyAscii("Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        outerElementList.add(EmaFactory.createElementEntry().map("DictionaryList", elementMap));
        elementMap.clear();

        configDb.add(EmaFactory.createMapEntry().keyAscii("DictionaryGroup", MapEntry.MapAction.ADD, outerElementList));
        outerElementList.clear();

        return configDb;
    }

    // ── main ──────────────────────────────────────────────────────────────────
    public static void main(String[] args) {
        parseArgs(args);

        OmmConsumer consumer = null;
        try {
            AppClient appClient = new AppClient();
            AppClient appClient2 = new AppClient();

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
                    .wsbChangeEventInfo(eventInfo).config(createProgramaticConfig()), appClient);

            consumer.registerClient(EmaFactory.createReqMsg().serviceName(serviceName).name(itemName), appClient, 0);
            if (warmStandbyMode == 2) {
                consumer.registerClient(EmaFactory.createReqMsg().serviceName(serviceName2).name(itemName), appClient2, 0);
            }

            final int printInterval = 5000;
            long nextPrintTime = System.currentTimeMillis() + printInterval;
            for (int i = 0; i < runtime; i++) {
                Thread.sleep(1000);
                long currentTime = System.currentTimeMillis();
                if (currentTime >= nextPrintTime) {
                    System.out.println(consumer.getWarmStandbyChannelInformation());
                    nextPrintTime = currentTime + printInterval;
                }
            }
        } catch (InterruptedException | OmmException excp) {
            System.out.println(excp.getMessage());
        } finally {
            if (consumer != null) {
                consumer.uninitialize();
            }
        }
    }
}
