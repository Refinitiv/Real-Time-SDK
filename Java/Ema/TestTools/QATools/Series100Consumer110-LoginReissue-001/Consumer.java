///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.rtsdk.ema.examples.training.consumer.series100.example110__MarketPrice__FileConfig;

import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.access.Data;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.DataType.DataTypes;
import com.rtsdk.ema.access.ElementEntry;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.Vector;
import com.rtsdk.ema.access.VectorEntry;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerConfig;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());

        decode(refreshMsg);

        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

        decode(updateMsg);

        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
            System.out.println("Item State: " + statusMsg.state());

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

    void decode(Msg msg)
    {
        switch (msg.attrib().dataType())
        {
            case DataTypes.ELEMENT_LIST:
                decode(msg.attrib().elementList());
                break;
        }

        switch (msg.payload().dataType())
        {
            case DataTypes.ELEMENT_LIST:
                decode(msg.payload().elementList());
                break;
            case DataTypes.FIELD_LIST:
                decode(msg.payload().fieldList());
                break;
        }
    }

    void decode(ElementList elementList)
    {
        for (ElementEntry elementEntry : elementList)
        {
            System.out.print(" Name = " + elementEntry.name() + " DataType: " + DataType.asString(elementEntry.load().dataType()) + " Value: ");

            if (Data.DataCode.BLANK == elementEntry.code())
                System.out.println(" blank");
            else
                switch (elementEntry.loadType())
                {
                    case DataTypes.REAL:
                        System.out.println(elementEntry.real().asDouble());
                        break;
                    case DataTypes.DATE:
                        System.out.println(elementEntry.date().day() + " / " + elementEntry.date().month() + " / " + elementEntry.date().year());
                        break;
                    case DataTypes.TIME:
                        System.out.println(elementEntry.time().hour() + ":" + elementEntry.time().minute() + ":" + elementEntry.time().second() + ":" + elementEntry.time().millisecond());
                        break;
                    case DataTypes.INT:
                        System.out.println(elementEntry.intValue());
                        break;
                    case DataTypes.UINT:
                        System.out.println(elementEntry.uintValue());
                        break;
                    case DataTypes.ASCII:
                        System.out.println(elementEntry.ascii());
                        break;
                    case DataTypes.ENUM:
                        System.out.println(elementEntry.enumValue());
                        break;
                    case DataTypes.RMTES :
                        System.out.println(elementEntry.rmtes());
                        break;
                    case DataTypes.ERROR:
                        System.out.println(elementEntry.error().errorCode() + " (" + elementEntry.error().errorCodeAsString() + ")");
                        break;
                    case DataTypes.VECTOR:
                        decode(elementEntry.vector());
                        break;
                    default:
                        System.out.println();
                        break;
                }
        }
    }

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
                    case DataTypes.REAL:
                        System.out.println(fieldEntry.real().asDouble());
                        break;
                    case DataTypes.DATE:
                        System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
                        break;
                    case DataTypes.TIME:
                        System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
                        break;
                    case DataTypes.INT:
                        System.out.println(fieldEntry.intValue());
                        break;
                    case DataTypes.UINT:
                        System.out.println(fieldEntry.uintValue());
                        break;
                    case DataTypes.ASCII:
                        System.out.println(fieldEntry.ascii());
                        break;
                    case DataTypes.ENUM:
                        System.out.println(fieldEntry.enumValue());
                        break;
                    case DataTypes.RMTES :
                        System.out.println(fieldEntry.rmtes());
                        break;
                    case DataTypes.ERROR:
                        System.out.println(fieldEntry.error().errorCode() + " (" + fieldEntry.error().errorCodeAsString() + ")");
                        break;
                    default:
                        System.out.println();
                        break;
                }
        }
    }

    void decode(Vector vector)
    {
        switch (vector.summaryData().dataType())
        {
            case DataTypes.ELEMENT_LIST:
                decode(vector.summaryData().elementList());
                break;
        }

        for (VectorEntry vectorEntry : vector)
        {
            System.out.print("Position: " + vectorEntry.position() + " Action = " + vectorEntry.vectorActionAsString() + " DataType: " + DataType.asString(vectorEntry.loadType()) + " Value: ");

            switch (vectorEntry.loadType())
            {
                case DataTypes.ELEMENT_LIST:
                    decode(vectorEntry.elementList());
                    break;
                default:
                    System.out.println();
                    break;
            }
        }
    }
}

public class Consumer
{
    public static void main(String[] args)
    {
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();

            ElementList elementList = EmaFactory.createElementList();

            elementList.add(EmaFactory.createElementEntry().uintValue("SingleOpen", 1));
            elementList.add(EmaFactory.createElementEntry().uintValue("AllowSuspectData", 1));
            elementList.add(EmaFactory.createElementEntry().uintValue("ProvidePermissionExpressions", 0));
            elementList.add(EmaFactory.createElementEntry().uintValue("ProvidePermissionProfile", 0));
            elementList.add(EmaFactory.createElementEntry().uintValue("DownloadConnectionConfig", 1));
            elementList.add(EmaFactory.createElementEntry().ascii("InstanceId", "2"));
            elementList.add(EmaFactory.createElementEntry().ascii("ApplicationId", "256"));
            elementList.add(EmaFactory.createElementEntry().ascii("ApplicationName", "Test Application"));
            elementList.add(EmaFactory.createElementEntry().ascii("Password", "secrete"));
            elementList.add(EmaFactory.createElementEntry().ascii("Position", "127.0.0.1"));
            elementList.add(EmaFactory.createElementEntry().uintValue("Role", 0));
            elementList.add(EmaFactory.createElementEntry().uintValue("SupportProviderDictionaryDownload", 0));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            config.addAdminMsg(reqMsg.domainType(EmaRdm.MMT_LOGIN).name("apiqa").nameType(3).attrib(elementList));

            consumer = EmaFactory.createOmmConsumer(config.host("localhost:14002"));

            Integer closure = (Integer)1;
            Integer closure2 = (Integer)2;

            long loginHandle = consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_LOGIN), appClient, closure);

            long handle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("CSCO.O"), appClient, closure2);

            Thread.sleep(1000);

            System.out.println("Login Reissue with different name from apiqa to apiqa2");

            consumer.reissue(reqMsg.clear().initialImage(true).name("apiqa2").domainType(EmaRdm.MMT_LOGIN).attrib(elementList).serviceName("DIRECT_FEED").nameType(3), loginHandle);

            System.out.println("Login Reissue done with name apiqa2");

            Thread.sleep(60000); // API calls onRefreshMsg(), onUpdateMsg() and
                                 // onStatusMsg()

            consumer.uninitialize();
        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println("Got OmmException from the main() function.");
            System.out.println(excp.getMessage());
            consumer.uninitialize();
        }
    }
}
//END APIQA
