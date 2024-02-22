//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
package com.refinitiv.ema.examples.training.consumer.series100.ex110_MP_FileCfg;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.*;

class AppClient implements OmmConsumerClient
{
    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent)
    {

        if (refreshMsg.hasMsgKey())
            System.out.println("Item Name: " + refreshMsg.name() + "\nService Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "not set"));

        System.out.println("Item State: " + refreshMsg.state().toString());

        if (DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().fieldList());
        if (DataTypes.ELEMENT_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().elementList());
        if (DataTypes.MAP == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().map());
        if (DataTypes.OPAQUE == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().opaque());
        System.out.println("Received Refresh, SolicitedFlag=" + refreshMsg.solicited() + ", StreamID= " + refreshMsg.streamId() + "\n");
    }

    @Override
    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent)
    {

        if (updateMsg.hasMsgKey())
            System.out.println("Item Name: " + updateMsg.name() + "\nService Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "not set"));

        if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
            decode(updateMsg.payload().fieldList());
        if (DataTypes.ELEMENT_LIST == updateMsg.payload().dataType())
            decode(updateMsg.payload().elementList());
        if (DataTypes.MAP == updateMsg.payload().dataType())
            decode(updateMsg.payload().map());
        if (DataTypes.OPAQUE == updateMsg.payload().dataType())
            decode(updateMsg.payload().opaque());
    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent)
    {

        if (statusMsg.hasMsgKey())
            System.out.println("\nItem Name: " + statusMsg.name() + "\nService Name: " + statusMsg.serviceName());

        if (statusMsg.hasState())
            System.out.println("\nItem State: " + statusMsg.state().toString());
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent)
    {
    }

    @Override
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent)
    {
    }

    @Override
    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent)
    {
    }

    void decode(FieldList fieldList)
    {
        for (FieldEntry fe : fieldList)
        {
            System.out.println("Fid: " + fe.fieldId() + " Name = " + fe.name() + " DataType: " + fe.load().dataType() + " Value: ");

            if (fe.code() == Data.DataCode.BLANK)
                System.out.println(" blank");
            else
                switch (fe.loadType())
                {
                    case DataTypes.REAL:
                        System.out.println(fe.real().asDouble());
                        break;
                    case DataTypes.DATE:
                        System.out.println((long)fe.date().day() + " / " + (long)fe.date().month() + " / " + (long)fe.date().year());
                        break;
                    case DataTypes.TIME:
                        System.out.println((long)fe.time().hour() + ":" + (long)fe.time().minute() + ":" + (long)fe.time().second() + ":" + (long)fe.time().millisecond());
                        break;
                    case DataTypes.INT:
                        System.out.println(fe.intValue());
                        break;
                    case DataTypes.UINT:
                        System.out.println(fe.uintValue());
                        break;
                    case DataTypes.ASCII:
                        System.out.println(fe.ascii());
                        break;
                    case DataTypes.ERROR:
                        System.out.println(fe.error().errorCode() + "( " + fe.error().codeAsString() + " )");
                        break;
                    case DataTypes.RMTES :
                        System.out.println(fe.rmtes());
                        break;
                    case DataTypes.ENUM:
                        System.out.println(fe.enumValue());
                        break;
                    default:
                        System.out.println("");
                        ;
                        break;
                }
        }
    }

    void decode(ElementList elementList)
    {
        for (ElementEntry ee : elementList)
        {
            System.out.println("ElementEntry Data  " + ee.ascii());
        }
    }

    void decode(Map map)
    {
        if (map.summaryData().dataType() == DataTypes.FIELD_LIST)
        {
            System.out.println("Map Summary data:");
            decode(map.summaryData().fieldList());
        }

        for (MapEntry me : map)
        {
            if (me.key().dataType() == DataTypes.BUFFER)
                System.out.println("Action: " + me.mapActionAsString() + " key value: " + me.key().buffer());

            if (me.loadType() == DataTypes.FIELD_LIST)
            {
                System.out.println("Entry data:");
                decode(me.fieldList());
            }
        }
    }

    void decode(OmmOpaque ommOpaque)
    {
        System.out.println("OmmOpaque data: " + ommOpaque.asHex());
    }
}

public class Consumer
{

    public static void main(String[] args) throws InterruptedException
    {

        if (args.length != 1 && args.length != 4)
        {
            System.out.println("Error: Invalid number of arguments");
            System.out.println("Usage: -m 1-99(testcase 1 or 2 or 3 or 4 ...) -user userName");
            return;
        }

        try
        {
            AppClient client = new AppClient();
            AppClient client2 = new AppClient();
            OmmConsumerConfig cc = EmaFactory.createOmmConsumerConfig();
            OmmConsumerConfig cc1 = EmaFactory.createOmmConsumerConfig();

            String sName = new String("DIRECT_FEED");
            String loginuser1 = new String("user1");
            String loginuser2 = new String("user2");
            String YsName = new String("ATS201_1");

            if (args[2].equals("-user"))
            {
                cc.host("localhost:14002");
                cc1.host("localhost:14002");
                cc.applicationId("256");
                cc1.applicationId("256");
                loginuser1 = new String(args[3]);
                cc.username(loginuser1);
                cc1.username(loginuser2);
                sName = new String("DIRECT_FEED");
                System.out.println("serviceName use will be " + sName);
            }
            else
            {
                Thread.sleep(1000);
                cc.host("localhost:14002");
                sName = new String("DIRECT_FEED");
                System.out.println("serviceName use will be " + sName);
            }

            OmmConsumer consumer = EmaFactory.createOmmConsumer(cc);

            Integer closure1 = Integer.valueOf(1);
            Integer closure2 = Integer.valueOf(1);
            Integer closure3 = Integer.valueOf(1);
            Integer closure4 = Integer.valueOf(1);
            Integer closure5 = Integer.valueOf(1);
            Integer closure6 = Integer.valueOf(1);
            Integer closure7 = Integer.valueOf(1);
            int temp = 0;

            if (args[0].equalsIgnoreCase("-m"))
            {
                temp = Integer.valueOf(args[1]);
                if (temp < 0)
                    consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);
                else if (temp == 1)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);

                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure2);

                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);

                    Thread.sleep(600);
                    consumer.unregister(h2);
                }
                else if (temp == 2)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);

                    OmmArray ommArray2 = EmaFactory.createOmmArray();
                    ommArray2.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(22));

                    ElementList elementList2 = EmaFactory.createElementList();
                    elementList2.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList2.add(EmaFactory.createElementEntry().array(":ViewData", ommArray2));

                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList2), client, closure2);

                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);

                    Thread.sleep(600);
                }
                else if (temp == 3)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(6));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);

                    OmmArray ommArray2 = EmaFactory.createOmmArray();
                    ommArray2.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(32));

                    ElementList elementList2 = EmaFactory.createElementList();
                    elementList2.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList2.add(EmaFactory.createElementEntry().array(":ViewData", ommArray2));

                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList2), client, closure1);

                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);

                    OmmArray ommArray3 = EmaFactory.createOmmArray();
                    ommArray3.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(6));
                    ommArray3.add(EmaFactory.createOmmArrayEntry().intValue(11));
                    ommArray3.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList3 = EmaFactory.createElementList();
                    elementList3.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList3.add(EmaFactory.createElementEntry().array(":ViewData", ommArray3));

                    long h4 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList3), client, closure4);
                }
                else if (temp == 4)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(6));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);

                    OmmArray ommArray2 = EmaFactory.createOmmArray();
                    ommArray2.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(32));

                    ElementList elementList2 = EmaFactory.createElementList();
                    elementList2.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList2.add(EmaFactory.createElementEntry().array(":ViewData", ommArray2));

                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList2), client, closure2);

                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);

                    long h4 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").interestAfterRefresh(false), client, closure4);
                }
                else if (temp == 5)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);

                    Thread.sleep(600);
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").initialImage(false).payload(elementList), h1);
                }
                else if (temp == 6)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("CSCO.O").payload(elementList), client, closure1);

                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("SPOT"), client, closure3);

                    Thread.sleep(600);
                    System.out.println("PAUSE NOW");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("SPOT").pause(true).initialImage(false), h3);
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("CSCO.O").pause(true).initialImage(false).payload(elementList), h1);
                    Thread.sleep(1600);
                    System.out.println("RESUME  NOW");

                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("SPOT").pause(false).initialImage(false), h3);
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("CSCO.O").pause(false).initialImage(true), h1);
                }
                else if (temp == 7)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(4).add(EmaFactory.createOmmArrayEntry().ascii("Data"));
                    ommArray.add(EmaFactory.createOmmArrayEntry().ascii("Lata"));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 2));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TALK").domainType(200).payload(elementList), client, closure1);
                    Thread.sleep(1600);
                    System.out.println("Reissue with  full view after 1.6 seconds");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TALK").domainType(200), h1);
                }
                else if (temp == 8)
                {
                    System.out.println("Snap TRI.N full, view TRI.N(22,25), after 1.6 seocnds snap request withview TRI.N(6,25),  Final shoud be view with 22,25 only on h1");
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").interestAfterRefresh(false), client2, closure2);

                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);
                    Thread.sleep(3000);

                    OmmArray ommArray2 = EmaFactory.createOmmArray();
                    ommArray2.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(6));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList2 = EmaFactory.createElementList();
                    elementList2.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList2.add(EmaFactory.createElementEntry().array(":ViewData", ommArray2));

                    System.out.println("SEND REQUEST FOR TRI.N view 6,26");
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").interestAfterRefresh(false).payload(elementList2), client, closure3);
                }
                else if (temp == 9)
                {
                    System.out.println("SEND REQUEST FOR two TRI.N different Handles");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure2);
                    Thread.sleep(1600);
                    System.out.println("PAUSE NOW h1");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(true).initialImage(false), h1);
                    System.out.println("wait for some time");
                    Thread.sleep(1600);
                    System.out.println("PAUSE NOW h2");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(true).initialImage(false), h2);
                    System.out.println("STOP receive updates");
                    Thread.sleep(1600);
                    System.out.println("RESUME  NOW h1");

                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(false).initialImage(false), h1);
                    Thread.sleep(1600);
                    System.out.println("RESUME  NOW h2");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(false).initialImage(false), h2);
                }
                else if (temp == 10)
                {
                    System.out.println("SEND REQUEST FOR two TRI.N different Handles");

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure2);
                    Thread.sleep(1600);
                    System.out.println("PAUSE NOW h1");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(true).initialImage(false), h1);
                    System.out.println("wait for some time");
                    Thread.sleep(1600);
                    System.out.println("PAUSE NOW h2");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(true).initialImage(false), h2);
                    System.out.println("STOP receive updates");
                    Thread.sleep(1600);
                    System.out.println("RESUME  NOW h1");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(false).initialImage(true), h1);
                    Thread.sleep(1600);
                    System.out.println("RESUME  NOW h2");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").pause(false).initialImage(false), h2);
                }
                else if (temp == 11)
                {
                    System.out.println("SEND REQUEST FOR two TRI.N different Handles");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure2);
                    Thread.sleep(20000);
                }
                else if (temp == 12)
                {
                    System.out.println("SEND REQUEST FOR two TRI.N different Handles");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client2, closure2);
                    System.out.println("unregister handle h2 after 10 seconds");
                    Thread.sleep(10000);
                    System.out.println("unregister handle h2 after now");
                    consumer.unregister(h2);
                }
                else if (temp == 13)
                {
                    System.out.println("SEND REQUEST FOR RES-DS needs to run directConnect rsslProvider");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("RES-DS"), client, closure1);
                    Thread.sleep(1000);
                    System.out.println("SEND REQUEST FOR RES-DS as private");

                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("RES-DS").privateStream(true), client, closure2);
                }
                else if (temp == 16)
                {
                    System.out.println("SEND REQUEST FOR Different DOMAIN needs to do with live FEED ");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("AGG.V").domainType(7), client, closure2);
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("BBH.ITC").domainType(8), client, closure3);
                    long h4 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRBI.PQ").domainType(9), client, closure4);
                    long h5 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("_ADS_CACHE_LIST").domainType(10), client, closure5);
                    long h6 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(YsName).name("BASIC").domainType(22), client, closure6);
                    long h7 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("CSCO.O").interestAfterRefresh(false), client, closure7);
                }
                else if (temp == 17)
                {
                    System.out.println("SEND REQUEST FOR TRI.N, TRI.N , A.N live FEED ");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("A.N").interestAfterRefresh(true), client, closure2);
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure3);
                }
                else if (temp == 18)
                {
                    System.out.println("SEND REQUEST FOR SPOT live FEED with request for MSGKey in update flag set");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("SPOT"), client, closure1);
                }

                else if (temp == 19)
                {
                    System.out.println("SEND REQUEST FOR TRI.N, TRI.N , A.N live FEED ");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("A.N").interestAfterRefresh(true), client, closure2);
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure3);
                    Thread.sleep(5000);
                    System.out.println("Unreigster handle h1 should send priority change on network  ");
                    consumer.unregister(h1);
                }
                else if (temp == 20)
                {
                    System.out.println("SEND REQUEST FOR TRI.N, TRI.N , A.N live FEED  Reissue on TRI.N h1 with refresh request");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("A.N").interestAfterRefresh(true), client, closure2);
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure3);
                    Thread.sleep(5000);
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").initialImage(true), h1);
                    System.out.println("Reissue on handle h1 should refresh for h1  ");
                }
                else if (temp == 21)
                {
                    System.out.println("SEND REQUEST FOR TRI.N MarkteByPrice from sds direct connect use option -direct  Reissue on TRI.N h1 with refresh request");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").domainType(8), client, closure1);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").domainType(8), client, closure2);
                    Thread.sleep(1000);
                    System.out.println("REISSUE ONE H1 now");
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").initialImage(true), h1);
                }
                else if (temp == 22)
                {
                    System.out.println("SEND REQUEST FOR SPOT live FEED with request for MSGKey in update flag set channel config in EMACOnfig.xml should had  MsgKeyInUpdates  ");
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("SPOT"), client, closure1);
                }
                else if (temp == 23)
                {
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    System.out.println("wait");
                    Thread.sleep(6000);
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").initialImage(true).payload(elementList), h1);
                }
                else if (temp == 24)
                {
                    System.out.println("LOGIN REISSUE WITH PAUSE ALL and RESUMEALL ");
                    long login_handle = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("SPOT"), client, closure1);
                    System.out.println("wait 6 seconds");
                    Thread.sleep(6000);
                    consumer.reissue(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN).initialImage(false).pause(true), login_handle);
                    System.out.println("wait 20 seconds");
                    Thread.sleep(20000);
                    consumer.reissue(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN).initialImage(false).pause(false), login_handle);
                }
                else if (temp == 25)
                {
                    OmmConsumer consumer1 = EmaFactory.createOmmConsumer(cc1);
                    System.out.println("LOGIN Multiple handles with dacs enable ads loginuser1 has permission EFG don't  ");
                    long login_handle1 = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long login_handle2 = consumer1.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client2, closure3);
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("SPOT"), client, closure1);
                    long h2 = consumer1.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client2, closure4);
                }
                else if (temp == 26)
                {
                    OmmConsumer consumer1 = EmaFactory.createOmmConsumer(cc1);
                    System.out.println("LOGIN Multiple handles with dacs enable ads loginuser1 has permission EFG don't  ");
                    long login_handle1 = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long login_handle2 = consumer1.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client2, closure3);
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("CSCO.O"), client, closure1);
                    long h2 = consumer1.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client2, closure4);
                }
                else if (temp == 27)
                {
                    System.out.println("LOGIN handles unregister  unregister client will not clean login to ADS login will be clean when consumer is unregister ");
                    long login_handle1 = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure1);
                    Thread.sleep(2000);
                    System.out.println("UNREGISTER H1 which is using open for IBM.N");
                    consumer.unregister(h1);
                    System.out.println("UNREGISTER login handle which is using user1");
                    consumer.unregister(login_handle1);
                }
                else if (temp == 28)
                {
                    System.out.println("Two LOGIN handles  test item fanout");
                    long login_handle1 = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN).name(loginuser2), client, closure2);
                    long login_handle2 = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client, closure3);

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure1);
                }
                else if (temp == 29)
                {
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N"), client, closure1);
                    System.out.println("wait");
                    Thread.sleep(6000);
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));
                    consumer.reissue(EmaFactory.createReqMsg().serviceName(sName).name("TRI.NN").initialImage(true).payload(elementList), h1);
                }
                else if (temp == 30)
                {
                    long login_handle = consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), client);
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);
                    System.out.println("LOGIN REISSUE WITH PAUSE ALL and RESUMEALL ");
                    System.out.println("wait 2 seconds");
                    Thread.sleep(2000);
                    consumer.reissue(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN).initialImage(false).pause(true).name(loginuser1), login_handle);
                    System.out.println("wait 5 seconds");
                    Thread.sleep(5000);
                    consumer.reissue(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN).initialImage(false).pause(false).name(loginuser1), login_handle);
                }
                else if (temp == 31)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(-32768));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(-32767));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(-100));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(6));

                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));


                    OmmArray ommArray2 = EmaFactory.createOmmArray();
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(-32768));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().intValue(-100));

                    ElementList elementList2 = EmaFactory.createElementList();
                    elementList2.add(EmaFactory.createElementEntry().uintValue(":ViewType", 1));
                    elementList2.add(EmaFactory.createElementEntry().array(":ViewData", ommArray2));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList2), client, closure2);
                    long h2 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").interestAfterRefresh(false), client, closure4);
                    // Sleep 3 seconds
                    Thread.sleep(3000);
                    long h4 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList).interestAfterRefresh(false), client, closure1);
                    // Sleep 15 seconds
                    Thread.sleep(15000);
                    long h5 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList).interestAfterRefresh(false), client, closure1);

                }
                else if (temp == 32)
                {
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    OmmArray ommArray2 = EmaFactory.createOmmArray();
                    OmmArray ommArray3 = EmaFactory.createOmmArray();
                    OmmArray ommArray4 = EmaFactory.createOmmArray();
                    ommArray.add(EmaFactory.createOmmArrayEntry().ascii("TRDPRC_1"));
                    ommArray.add(EmaFactory.createOmmArrayEntry().ascii("BID"));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().ascii("ASK"));
                    ommArray2.add(EmaFactory.createOmmArrayEntry().ascii("VOL ACCUMULATED"));
                    ommArray3.add(EmaFactory.createOmmArrayEntry().ascii("BID"));
                    ommArray3.add(EmaFactory.createOmmArrayEntry().ascii("ASK_TIME"));
                    ommArray3.add(EmaFactory.createOmmArrayEntry().ascii("VOL ACCUMULATED"));
                    ommArray4.add(EmaFactory.createOmmArrayEntry().ascii("ASK_TIME"));
                    ommArray4.add(EmaFactory.createOmmArrayEntry().ascii("DIVPAYDATE"));
                    ommArray4.add(EmaFactory.createOmmArrayEntry().ascii("NETCHNG_1"));

                    ElementList elementList = EmaFactory.createElementList();
                    ElementList elementList1 = EmaFactory.createElementList();
                    ElementList elementList2 = EmaFactory.createElementList();
                    ElementList elementList3 = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(":ViewType", 2));
                    elementList.add(EmaFactory.createElementEntry().array(":ViewData", ommArray));
                    elementList1.add(EmaFactory.createElementEntry().uintValue(":ViewType", 2));
                    elementList1.add(EmaFactory.createElementEntry().array(":ViewData", ommArray2));
                    elementList2.add(EmaFactory.createElementEntry().uintValue(":ViewType", 2));
                    elementList2.add(EmaFactory.createElementEntry().array(":ViewData", ommArray3));
                    elementList3.add(EmaFactory.createElementEntry().uintValue(":ViewType", 2));
                    elementList3.add(EmaFactory.createElementEntry().array(":ViewData", ommArray4));

                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList), client, closure1);
                    Thread.sleep(3000);
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList1), client, closure3);
                    Thread.sleep(3000);
                    long h4 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").payload(elementList2), client, closure4);
                    Thread.sleep(3000);
                    long h5 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("TRI.N").interestAfterRefresh(false).payload(elementList3), client, closure5);
                }
                else if (temp == 99)
                {
                    System.out.println("Selected option 99");
                    OmmArray ommArray = EmaFactory.createOmmArray();
                    ommArray.fixedWidth(2).add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(2));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(3));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(4));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(6));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(7));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(8));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(9));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(10));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(11));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(12));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(13));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(14));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(15));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(16));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(18));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(19));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(21));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(22));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(23));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(24));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(25));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(26));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(27));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(28));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(29));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(30));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(31));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(32));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(33));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(34));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(35));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(36));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(37));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(38));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(39));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(40));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(42));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(43));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(53));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(56));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(58));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(60));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(61));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(71));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(75));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(76));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(77));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(78));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(79));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(90));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(91));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(100));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(104));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(105));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(110));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(111));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(117));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(118));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(131));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(178));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(198));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(259));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(293));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(296));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(340));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(350));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(351));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(372));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(373));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(374));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(375));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(376));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(377));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(378));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(379));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(728));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(869));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(998));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(999));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1000));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1001));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1002));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1003));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1021));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1023));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1025));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1041));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1042));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1043));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1044));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1055));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1056));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1067));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1075));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1076));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1080));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1379));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1383));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1392));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1404));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1465));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1501));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1642));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(1709));
                    ommArray.add(EmaFactory.createOmmArrayEntry().intValue(2326));
                    ElementList elementList = EmaFactory.createElementList();
                    elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
                    elementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, ommArray));
                    long h1 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N").payload(elementList), client, closure1);
                    Thread.sleep(600);
                }
                else
                {
                    long h3 = consumer.registerClient(EmaFactory.createReqMsg().serviceName(sName).name("IBM.N"), client, closure3);
                }
            }
            Thread.sleep(360000);
            consumer.uninitialize();
        }
        catch (OmmException excp)
        {
            System.out.println(excp.toString());
        }
    }
}
//END APIQA
