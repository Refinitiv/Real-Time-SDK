/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

internal class AppClient : IOmmConsumerClient
{
    
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        if (refreshMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + refreshMsg.Name() + "\nService Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "not set"));

        Console.WriteLine("Item State: " + refreshMsg.State().ToString());

        if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());
        if (DataTypes.ELEMENT_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().ElementList());
        if (DataTypes.MAP == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().Map());
        if (DataTypes.OPAQUE == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().OmmOpaque());
        Console.WriteLine("Received Refresh, SolicitedFlag=" + refreshMsg.Solicited() + ", StreamID= " + refreshMsg.StreamId() + "\n");
    }

    
    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {

        if (updateMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + updateMsg.Name() + "\nService Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "not set"));

        if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());
        if (DataTypes.ELEMENT_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().ElementList());
        if (DataTypes.MAP == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().Map());
        if (DataTypes.OPAQUE == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().OmmOpaque());
    }

    
    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {

        if (statusMsg.HasMsgKey)
            Console.WriteLine(statusMsg.HasName?("\nItem Name: " + statusMsg.Name()):"" + "\nService Name: " + statusMsg.ServiceName());

        if (statusMsg.HasState)
            Console.WriteLine("\nItem State: " + statusMsg.State().ToString());
    }

    static void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fe in fieldList)
        {
            Console.WriteLine("Fid: " + fe.FieldId + " Name = " + fe.Name + " DataType: " + fe.Load!.DataType + " Value: ");

            if (fe.Code == Data.DataCode.BLANK)
                Console.WriteLine(" blank");
            else
                switch (fe.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fe.OmmRealValue().AsDouble());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine((long)fe.OmmDateValue().Day + " / " + (long)fe.OmmDateValue().Month + " / " + (long)fe.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine((long)fe.OmmTimeValue().Hour + ":" + (long)fe.OmmTimeValue().Minute + ":" + (long)fe.OmmTimeValue().Second + ":" + (long)fe.OmmTimeValue().Millisecond);
                        break;
                    case DataTypes.INT:
                        Console.WriteLine(fe.OmmIntValue());
                        break;
                    case DataTypes.UINT:
                        Console.WriteLine(fe.OmmUIntValue());
                        break;
                    case DataTypes.ASCII:
                        Console.WriteLine(fe.OmmAsciiValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine(fe.OmmErrorValue().ErrorCode + "( " + fe.OmmErrorValue().CodeAsString() + " )");
                        break;
                    case DataTypes.RMTES :
                        Console.WriteLine(fe.OmmRmtesValue());
                        break;
                    case DataTypes.ENUM:
                        Console.WriteLine(fe.OmmEnumValue());
                        break;
                    default:
                        Console.WriteLine("");
                        ;
                        break;
                }
        }
    }

    static void Decode(ElementList elementList)
    {
        foreach (ElementEntry ee in elementList)
        {
            Console.WriteLine("ElementEntry Data  " + ee.OmmAsciiValue());
        }
    }

    static void Decode(Map map)
    {
        if (map.SummaryData().DataType == DataTypes.FIELD_LIST)
        {
            Console.WriteLine("Map Summary data:");
            Decode(map.SummaryData().FieldList());
        }

        foreach (MapEntry me in map)
        {
            if (me.Key.DataType == DataTypes.BUFFER)
                Console.WriteLine("Action: " + me.MapActionAsString() + " key value: " + me.Key.Buffer());

            if (me.LoadType == DataTypes.FIELD_LIST)
            {
                Console.WriteLine("Entry data:");
                Decode(me.FieldList());
            }
        }
    }

    static void Decode(OmmOpaque ommOpaque)
    {
        Console.WriteLine("OmmOpaque data: " + ommOpaque.AsHex());
    }
}

public class Consumer
{

    public static void Main(string[] args)
    {
        if (args.Length != 1 && args.Length != 4)
        {
            Console.WriteLine("Error: Invalid number of arguments");
            Console.WriteLine("Usage: -m 1-99(testcase 1 or 2 or 3 or 4 ...) -user userName");
            return;
        }

        try
        {
            AppClient client = new();
            AppClient client2 = new();
            OmmConsumerConfig cc = new();
            OmmConsumerConfig cc1 = new();

            string sName = new("DIRECT_FEED");
            string loginuser1 = new("user1");
            string loginuser2 = new("user2");
            string YsName = new("ATS201_1");

            if (args[2].Equals("-user"))
            {
                cc.Host("localhost:14002");
                cc1.Host("localhost:14002");
                cc.ApplicationId("256");
                cc1.ApplicationId("256");
                loginuser1 = new string(args[3]);
                cc.UserName(loginuser1);
                cc1.UserName(loginuser2);
                sName = new string("DIRECT_FEED");
                Console.WriteLine("serviceName use will be " + sName);
            }
            else
            {
                Thread.Sleep(1000);
                cc.Host("localhost:14002");
                sName = new string("DIRECT_FEED");
                Console.WriteLine("serviceName use will be " + sName);
            }

            OmmConsumer consumer = new(cc);

            int closure1 = 1;
            int closure2 = 1;
            int closure3 = 1;
            int closure4 = 1;
            int closure5 = 1;
            int closure6 = 1;
            int closure7 = 1;
            int temp = 0;

            if (string.Equals(args[0], "-m", StringComparison.InvariantCultureIgnoreCase))
            {
                temp = int.Parse(args[1]);
                if (temp < 0)
                    consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);
                else if (temp == 1)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);

                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure2);

                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);

                    Thread.Sleep(600);
                    consumer.Unregister(h2);
                }
                else if (temp == 2)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);

                    OmmArray ommArray2 = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray2.AddInt(25);
                    ommArray2.AddInt(22);

                    ElementList elementList2 = new();
                    elementList2.AddUInt(":ViewType", 1);
                    elementList2.AddArray(":ViewData", ommArray2.Complete());

                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList2.Complete()), client, closure2);

                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);

                    Thread.Sleep(600);
                }
                else if (temp == 3)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(6);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);

                    OmmArray ommArray2 = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray2.AddInt(25);
                    ommArray2.AddInt(32);

                    ElementList elementList2 = new();
                    elementList2.AddUInt(":ViewType", 1);
                    elementList2.AddArray(":ViewData", ommArray2.Complete());

                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList2.Complete()), client, closure1);

                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);

                    OmmArray ommArray3 = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray3.AddInt(6);
                    ommArray3.AddInt(11);
                    ommArray3.AddInt(25);

                    ElementList elementList3 = new();
                    elementList3.AddUInt(":ViewType", 1);
                    elementList3.AddArray(":ViewData", ommArray3.Complete());

                    long h4 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList3.Complete()), client, closure4);
                }
                else if (temp == 4)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(6);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);

                    OmmArray ommArray2 = new OmmArray()
                    {
                        FixedWidth = 2
                    }.AddInt(25)
                     .AddInt(32);

                    ElementList elementList2 = new();
                    elementList2.AddUInt(":ViewType", 1);
                    elementList2.AddArray(":ViewData", ommArray2.Complete());

                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList2.Complete()), client, closure2);

                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);

                    long h4 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").InterestAfterRefresh(false), client, closure4);
                }
                else if (temp == 5)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());
					
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);

                    Thread.Sleep(600);
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").InitialImage(false).Payload(elementList.Complete()), h1);
                }
                else if (temp == 6)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("CSCO.O").Payload(elementList.Complete()), client, closure1);

                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("SPOT"), client, closure3);

                    Thread.Sleep(600);
                    Console.WriteLine("PAUSE NOW");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("SPOT").Pause(true).InitialImage(false), h3);
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("CSCO.O").Pause(true).InitialImage(false).Payload(elementList.Complete()), h1);
                    Thread.Sleep(1600);
                    Console.WriteLine("RESUME  NOW");

                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("SPOT").Pause(false).InitialImage(false), h3);
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("CSCO.O").Pause(false).InitialImage(true), h1);
                }
                else if (temp == 7)
                {
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 4
                    };
                    ommArray.AddAscii("Data");
                    ommArray.AddAscii("Lata");

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 2);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TALK").DomainType(200).Payload(elementList.Complete()), client, closure1);
                    Thread.Sleep(1600);
                    Console.WriteLine("Reissue with  full view after 1.6 seconds");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TALK").DomainType(200), h1);
                }
                else if (temp == 8)
                {
                    Console.WriteLine("Snap TRI.N full, view TRI.N(22,25), after 1.6 seocnds snap request withview TRI.N(6,25),  Final shoud be view with 22,25 only on h1");
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").InterestAfterRefresh(false), client2, closure2);

                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);
                    Thread.Sleep(3000);

                    OmmArray ommArray2 = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray2.AddInt(6);
                    ommArray2.AddInt(25);

                    ElementList elementList2 = new();
                    elementList2.AddUInt(":ViewType", 1);
                    elementList2.AddArray(":ViewData", ommArray2.Complete());

                    Console.WriteLine("SEND REQUEST FOR TRI.N view 6,26");
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").InterestAfterRefresh(false).Payload(elementList2.Complete()), client, closure3);
                }
                else if (temp == 9)
                {
                    Console.WriteLine("SEND REQUEST FOR two TRI.N different Handles");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure2);
                    Thread.Sleep(1600);
                    Console.WriteLine("PAUSE NOW h1");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(true).InitialImage(false), h1);
                    Console.WriteLine("wait for some time");
                    Thread.Sleep(1600);
                    Console.WriteLine("PAUSE NOW h2");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(true).InitialImage(false), h2);
                    Console.WriteLine("STOP receive updates");
                    Thread.Sleep(1600);
                    Console.WriteLine("RESUME  NOW h1");

                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(false).InitialImage(false), h1);
                    Thread.Sleep(1600);
                    Console.WriteLine("RESUME  NOW h2");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(false).InitialImage(false), h2);
                }
                else if (temp == 10)
                {
                    Console.WriteLine("SEND REQUEST FOR two TRI.N different Handles");

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure2);
                    Thread.Sleep(1600);
                    Console.WriteLine("PAUSE NOW h1");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(true).InitialImage(false), h1);
                    Console.WriteLine("wait for some time");
                    Thread.Sleep(1600);
                    Console.WriteLine("PAUSE NOW h2");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(true).InitialImage(false), h2);
                    Console.WriteLine("STOP receive updates");
                    Thread.Sleep(1600);
                    Console.WriteLine("RESUME  NOW h1");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(false).InitialImage(true), h1);
                    Thread.Sleep(1600);
                    Console.WriteLine("RESUME  NOW h2");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").Pause(false).InitialImage(false), h2);
                }
                else if (temp == 11)
                {
                    Console.WriteLine("SEND REQUEST FOR two TRI.N different Handles");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure2);
                    Thread.Sleep(20000);
                }
                else if (temp == 12)
                {
                    Console.WriteLine("SEND REQUEST FOR two TRI.N different Handles");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client2, closure2);
                    Console.WriteLine("unregister handle h2 after 10 seconds");
                    Thread.Sleep(10000);
                    Console.WriteLine("unregister handle h2 after now");
                    consumer.Unregister(h2);
                }
                else if (temp == 13)
                {
                    Console.WriteLine("SEND REQUEST FOR RES-DS needs to run directConnect rsslProvider");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("RES-DS"), client, closure1);
                    Thread.Sleep(1000);
                    Console.WriteLine("SEND REQUEST FOR RES-DS as private");

                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("RES-DS").PrivateStream(true), client, closure2);
                }
                else if (temp == 16)
                {
                    Console.WriteLine("SEND REQUEST FOR Different DOMAIN needs to do with live FEED ");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("AGG.V").DomainType(7), client, closure2);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("BBH.ITC").DomainType(8), client, closure3);
                    long h4 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRBI.PQ").DomainType(9), client, closure4);
                    long h5 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("_ADS_CACHE_LIST").DomainType(10), client, closure5);
                    long h6 = consumer.RegisterClient(new RequestMsg().ServiceName(YsName).Name("BASIC").DomainType(22), client, closure6);
                    long h7 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("CSCO.O").InterestAfterRefresh(false), client, closure7);
                }
                else if (temp == 17)
                {
                    Console.WriteLine("SEND REQUEST FOR TRI.N, TRI.N , A.N live FEED ");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("A.N").InterestAfterRefresh(true), client, closure2);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure3);
                }
                else if (temp == 18)
                {
                    Console.WriteLine("SEND REQUEST FOR SPOT live FEED with request for MSGKey in update flag set");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("SPOT"), client, closure1);
                }

                else if (temp == 19)
                {
                    Console.WriteLine("SEND REQUEST FOR TRI.N, TRI.N , A.N live FEED ");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("A.N").InterestAfterRefresh(true), client, closure2);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure3);
                    Thread.Sleep(5000);
                    Console.WriteLine("Unreigster handle h1 should send priority change On network  ");
                    consumer.Unregister(h1);
                }
                else if (temp == 20)
                {
                    Console.WriteLine("SEND REQUEST FOR TRI.N, TRI.N , A.N live FEED  Reissue On TRI.N h1 with refresh request");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("A.N").InterestAfterRefresh(true), client, closure2);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure3);
                    Thread.Sleep(5000);
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").InitialImage(true), h1);
                    Console.WriteLine("Reissue On handle h1 should refresh for h1  ");
                }
                else if (temp == 21)
                {
                    Console.WriteLine("SEND REQUEST FOR TRI.N MarkteByPrice from sds direct connect use option -direct  Reissue On TRI.N h1 with refresh request");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").DomainType(8), client, closure1);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").DomainType(8), client, closure2);
                    Thread.Sleep(1000);
                    Console.WriteLine("REISSUE ONE H1 now");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").InitialImage(true), h1);
                }
                else if (temp == 22)
                {
                    Console.WriteLine("SEND REQUEST FOR SPOT live FEED with request for MSGKey in update flag set channel config in EMACOnfig.xml should had  MsgKeyInUpdates  ");
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("SPOT"), client, closure1);
                }
                else if (temp == 23)
                {
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    Console.WriteLine("wait");
                    Thread.Sleep(6000);
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);
                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.N").InitialImage(true).Payload(elementList.Complete()), h1);
                }
                else if (temp == 24)
                {
                    Console.WriteLine("LOGIN REISSUE WITH PAUSE ALL and RESUMEALL ");
                    long login_handle = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("SPOT"), client, closure1);
                    Console.WriteLine("wait 6 seconds");
                    Thread.Sleep(6000);
                    consumer.Reissue(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN).InitialImage(false).Pause(true), login_handle);
                    Console.WriteLine("wait 20 seconds");
                    Thread.Sleep(20000);
                    consumer.Reissue(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN).InitialImage(false).Pause(false), login_handle);
                }
                else if (temp == 25)
                {
                    OmmConsumer consumer1 = new(cc1);
                    Console.WriteLine("LOGIN Multiple handles with dacs enable ads loginuser1 has permission EFG don't  ");
                    long login_handle1 = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long login_handle2 = consumer1.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client2, closure3);
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("SPOT"), client, closure1);
                    long h2 = consumer1.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client2, closure4);
                }
                else if (temp == 26)
                {
                    OmmConsumer consumer1 = new(cc1);
                    Console.WriteLine("LOGIN Multiple handles with dacs enable ads loginuser1 has permission EFG don't  ");
                    long login_handle1 = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long login_handle2 = consumer1.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client2, closure3);
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("CSCO.O"), client, closure1);
                    long h2 = consumer1.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client2, closure4);
                }
                else if (temp == 27)
                {
                    Console.WriteLine("LOGIN handles unregister  unregister client will not clean login to ADS login will be clean when consumer is unregister ");
                    long login_handle1 = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client, closure2);
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure1);
                    Thread.Sleep(2000);
                    Console.WriteLine("UNREGISTER H1 which is using open for IBM.N");
                    consumer.Unregister(h1);
                    Console.WriteLine("UNREGISTER login handle which is using user1");
                    consumer.Unregister(login_handle1);
                }
                else if (temp == 28)
                {
                    Console.WriteLine("Two LOGIN handles  test item fanout");
                    long login_handle1 = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN).Name(loginuser2), client, closure2);
                    long login_handle2 = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client, closure3);

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure1);
                }
                else if (temp == 29)
                {
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure1);
                    Console.WriteLine("wait");
                    Thread.Sleep(6000);
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);
                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("TRI.NN").InitialImage(true).Payload(elementList.Complete()), h1);
                }
                else if (temp == 30)
                {
                    long login_handle = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), client);
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(25);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);
                    Console.WriteLine("LOGIN REISSUE WITH PAUSE ALL and RESUMEALL ");
                    Console.WriteLine("wait 2 seconds");
                    Thread.Sleep(2000);
                    consumer.Reissue(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN).InitialImage(false).Pause(true).Name(loginuser1), login_handle);
                    Console.WriteLine("wait 5 seconds");
                    Thread.Sleep(5000);
                    consumer.Reissue(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN).InitialImage(false).Pause(false).Name(loginuser1), login_handle);
                }
                else if (temp == 31)
                {
                    OmmArray ommArray = new();
                    ommArray.AddInt(22);
                    ommArray.AddInt(-32768);
                    ommArray.AddInt(-32767);
                    ommArray.AddInt(-100);
                    ommArray.AddInt(6);

                    ElementList elementList = new();
                    elementList.AddUInt(":ViewType", 1);
                    elementList.AddArray(":ViewData", ommArray.Complete());


                    OmmArray ommArray2 = new();
                    ommArray2.AddInt(25);
                    ommArray2.AddInt(22);
                    ommArray2.AddInt(-32768);
                    ommArray2.AddInt(-100);

                    ElementList elementList2 = new();
                    elementList2.AddUInt(":ViewType", 1);
                    elementList2.AddArray(":ViewData", ommArray2.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList2.Complete()), client, closure2);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").InterestAfterRefresh(false), client, closure4);
                    // Sleep 3 seconds
                    Thread.Sleep(3000);
                    long h4 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()).InterestAfterRefresh(false), client, closure1);
                    // Sleep 15 seconds
                    Thread.Sleep(15000);
                    long h5 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()).InterestAfterRefresh(false), client, closure1);

                }
                else if (temp == 32)
                {
                    OmmArray ommArray = new();
                    OmmArray ommArray2 = new();
                    OmmArray ommArray3 = new();
                    OmmArray ommArray4 = new();
                    ommArray.AddAscii("TRDPRC_1");
                    ommArray.AddAscii("BID");
                    ommArray2.AddAscii("ASK");
                    ommArray2.AddAscii("VOL ACCUMULATED");
                    ommArray3.AddAscii("BID");
                    ommArray3.AddAscii("ASK_TIME");
                    ommArray3.AddAscii("VOL ACCUMULATED");
                    ommArray4.AddAscii("ASK_TIME");
                    ommArray4.AddAscii("DIVPAYDATE");
                    ommArray4.AddAscii("NETCHNG_1");

                    ElementList elementList = new();
                    ElementList elementList1 = new();
                    ElementList elementList2 = new();
                    ElementList elementList3 = new();
                    elementList.AddUInt(":ViewType", 2);
                    elementList.AddArray(":ViewData", ommArray.Complete());
                    elementList1.AddUInt(":ViewType", 2);
                    elementList1.AddArray(":ViewData", ommArray2.Complete());
                    elementList2.AddUInt(":ViewType", 2);
                    elementList2.AddArray(":ViewData", ommArray3.Complete());
                    elementList3.AddUInt(":ViewType", 2);
                    elementList3.AddArray(":ViewData", ommArray4.Complete());

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList.Complete()), client, closure1);
                    Thread.Sleep(3000);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList1.Complete()), client, closure3);
                    Thread.Sleep(3000);
                    long h4 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").Payload(elementList2.Complete()), client, closure4);
                    Thread.Sleep(3000);
                    long h5 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N").InterestAfterRefresh(false).Payload(elementList3.Complete()), client, closure5);
                }
                else if (temp == 33)
                {
                    var array = new OmmArray()
                       .AddAscii("TRI.N")
                       .AddAscii("IBM.N")
                       .Complete();

                    var batch = new ElementList()
                        .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array)
                        .Complete();

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Payload(batch), client, closure1);
                    Thread.Sleep(3000);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("TRI.N"), client, closure2);
                    Thread.Sleep(3000);
                }
                else if (temp == 34)
                {
                    var array = new OmmArray()
                       .AddAscii("TRI.N")
                       .AddAscii("IBM.N")
                       .Complete();

                    var batch = new ElementList()
                        .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array)
                        .Complete();

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Payload(batch), client, closure1);
                    Thread.Sleep(3000);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("LSEG.O"), client, closure2);
                }
                else if (temp == 35)
                {
                    var array = new OmmArray()
                       .AddAscii("TRI.N")
                       .AddAscii("IBM.N")
                       .Complete();

                    var batch = new ElementList()
                        .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array)
                        .Complete();

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Payload(batch), client, closure1);
                    Thread.Sleep(3000);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("LSEG.O").InterestAfterRefresh(false), client, closure2);
                }
                else if (temp == 36)
                {
                    OmmArray ommArray = new();
                    ommArray.AddAscii("TRDPRC_1");
                    ommArray.AddAscii("BID");
                    ommArray.Complete();

                    OmmArray array = new OmmArray()
                       .AddAscii("TRI.N")
                       .AddAscii("IBM.N")
                       .Complete();

                    ElementList batch = new ElementList()
                        .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array)
                        .AddUInt(EmaRdm.ENAME_VIEW_TYPE, 2)
                        .AddArray(EmaRdm.ENAME_VIEW_DATA, ommArray)
                        .Complete();

                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Payload(batch), client, closure2);
                    Thread.Sleep(3000);
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("GOOG.O").InterestAfterRefresh(false), client, closure3);
                }
                else if (temp == 37)
                {
                    OmmArray ommArray = new();
                    OmmArray ommArray2 = new();

                    ommArray.AddAscii("ASK");
                    ommArray.AddAscii("BID");
                    ommArray.Complete();

                    ElementList elementList = new();
                    ElementList elementList1 = new();

                    OmmArray array = new OmmArray()
                        .AddAscii("TRI.N")
                        .AddAscii("IBM.N")
                        .Complete();

                    ElementList batch = new ElementList()
                        .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array)
                        .AddUInt(EmaRdm.ENAME_VIEW_TYPE, 2)
                        .AddArray(EmaRdm.ENAME_VIEW_DATA, ommArray)
                        .Complete();

                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Payload(batch), client, closure1);
                    Thread.Sleep(3000);
                    long h2 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N").InterestAfterRefresh(false), client, closure3);
                    Thread.Sleep(20000);
                    Console.WriteLine("Reissue");
                    consumer.Reissue(new RequestMsg().ServiceName(sName).Name("IBM.N").InitialImage(false).StreamId(28), h2);
                }
                else if (temp == 99)
                {
                    Console.WriteLine("Selected option 99");
                    OmmArray ommArray = new()
                    {
                        FixedWidth = 2
                    };
                    ommArray.AddInt(22);
                    ommArray.AddInt(2);
                    ommArray.AddInt(3);
                    ommArray.AddInt(4);
                    ommArray.AddInt(6);
                    ommArray.AddInt(7);
                    ommArray.AddInt(8);
                    ommArray.AddInt(9);
                    ommArray.AddInt(10);
                    ommArray.AddInt(11);
                    ommArray.AddInt(12);
                    ommArray.AddInt(13);
                    ommArray.AddInt(14);
                    ommArray.AddInt(15);
                    ommArray.AddInt(16);
                    ommArray.AddInt(18);
                    ommArray.AddInt(19);
                    ommArray.AddInt(21);
                    ommArray.AddInt(22);
                    ommArray.AddInt(23);
                    ommArray.AddInt(24);
                    ommArray.AddInt(25);
                    ommArray.AddInt(26);
                    ommArray.AddInt(27);
                    ommArray.AddInt(28);
                    ommArray.AddInt(29);
                    ommArray.AddInt(30);
                    ommArray.AddInt(31);
                    ommArray.AddInt(32);
                    ommArray.AddInt(33);
                    ommArray.AddInt(34);
                    ommArray.AddInt(35);
                    ommArray.AddInt(36);
                    ommArray.AddInt(37);
                    ommArray.AddInt(38);
                    ommArray.AddInt(39);
                    ommArray.AddInt(40);
                    ommArray.AddInt(42);
                    ommArray.AddInt(43);
                    ommArray.AddInt(53);
                    ommArray.AddInt(56);
                    ommArray.AddInt(58);
                    ommArray.AddInt(60);
                    ommArray.AddInt(61);
                    ommArray.AddInt(71);
                    ommArray.AddInt(75);
                    ommArray.AddInt(76);
                    ommArray.AddInt(77);
                    ommArray.AddInt(78);
                    ommArray.AddInt(79);
                    ommArray.AddInt(90);
                    ommArray.AddInt(91);
                    ommArray.AddInt(100);
                    ommArray.AddInt(104);
                    ommArray.AddInt(105);
                    ommArray.AddInt(110);
                    ommArray.AddInt(111);
                    ommArray.AddInt(117);
                    ommArray.AddInt(118);
                    ommArray.AddInt(131);
                    ommArray.AddInt(178);
                    ommArray.AddInt(198);
                    ommArray.AddInt(259);
                    ommArray.AddInt(293);
                    ommArray.AddInt(296);
                    ommArray.AddInt(340);
                    ommArray.AddInt(350);
                    ommArray.AddInt(351);
                    ommArray.AddInt(372);
                    ommArray.AddInt(373);
                    ommArray.AddInt(374);
                    ommArray.AddInt(375);
                    ommArray.AddInt(376);
                    ommArray.AddInt(377);
                    ommArray.AddInt(378);
                    ommArray.AddInt(379);
                    ommArray.AddInt(728);
                    ommArray.AddInt(869);
                    ommArray.AddInt(998);
                    ommArray.AddInt(999);
                    ommArray.AddInt(1000);
                    ommArray.AddInt(1001);
                    ommArray.AddInt(1002);
                    ommArray.AddInt(1003);
                    ommArray.AddInt(1021);
                    ommArray.AddInt(1023);
                    ommArray.AddInt(1025);
                    ommArray.AddInt(1041);
                    ommArray.AddInt(1042);
                    ommArray.AddInt(1043);
                    ommArray.AddInt(1044);
                    ommArray.AddInt(1055);
                    ommArray.AddInt(1056);
                    ommArray.AddInt(1067);
                    ommArray.AddInt(1075);
                    ommArray.AddInt(1076);
                    ommArray.AddInt(1080);
                    ommArray.AddInt(1379);
                    ommArray.AddInt(1383);
                    ommArray.AddInt(1392);
                    ommArray.AddInt(1404);
                    ommArray.AddInt(1465);
                    ommArray.AddInt(1501);
                    ommArray.AddInt(1642);
                    ommArray.AddInt(1709);
                    ommArray.AddInt(2326);
                    ElementList elementList = new();
                    elementList.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1);
                    elementList.AddArray(EmaRdm.ENAME_VIEW_DATA, ommArray.Complete());
                    long h1 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N").Payload(elementList.Complete()), client, closure1);
                    Thread.Sleep(600);
                }
                else
                {
                    long h3 = consumer.RegisterClient(new RequestMsg().ServiceName(sName).Name("IBM.N"), client, closure3);
                }
            }
            Thread.Sleep(360000);
            consumer.Uninitialize();
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }
}
//END APIQA
