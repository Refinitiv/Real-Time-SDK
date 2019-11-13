///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.consumer.series300.example331__Directory__Streaming;

import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.OmmArrayEntry;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;

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
            default:
                break;
        }

        switch (msg.payload().dataType())
        {
            case DataTypes.MAP:
                decode(msg.payload().map());
                break;
            case DataTypes.FIELD_LIST:
                decode(msg.payload().fieldList());
                break;
            default:
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
                    case DataTypes.ARRAY:
                    {
                        boolean first = true;
                        for (OmmArrayEntry arrayEntry : elementEntry.array())
                        {
                            if (!first)
                                System.out.print(", ");
                            else
                                first = false;
                            switch (arrayEntry.loadType())
                            {
                                case DataTypes.ASCII:
                                    System.out.print(arrayEntry.ascii());
                                    break;
                                case DataTypes.UINT:
                                    System.out.print(arrayEntry.uintValue());
                                    break;
                                case DataTypes.QOS:
                                    System.out.print(arrayEntry.qos());
                                    break;
                                default:
                                    break;
                            }
                        }
                        System.out.println();
                    }
                        break;
                    case DataTypes.RMTES :
                        System.out.println(elementEntry.rmtes());
                        break;
                    case DataTypes.ERROR:
                        System.out.println(elementEntry.error().errorCode() + " (" + elementEntry.error().errorCodeAsString() + ")");
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
                        System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
                        break;
                    case DataTypes.ARRAY:
                        System.out.println(fieldEntry.array());
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

    void decode(Map map)
    {
        for (MapEntry mapEntry : map)
        {
            switch (mapEntry.loadType())
            {
                case DataTypes.FILTER_LIST:
                    decode(mapEntry.filterList());
                    break;
                default:
                    System.out.println();
                    break;
            }
        }
    }

    void decode(FilterList filterList)
    {
        for (FilterEntry filterEntry : filterList)
        {
            System.out.println("ID: " + filterEntry.filterId() + " Action = " + filterEntry.filterActionAsString() + " DataType: " + DataType.asString(filterEntry.loadType()) + " Value: ");

            switch (filterEntry.loadType())
            {
                case DataTypes.ELEMENT_LIST:
                    decode(filterEntry.elementList());
                    break;
                case DataTypes.MAP:
                    decode(filterEntry.map());
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

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();
            // APIQA: Use invalid serviceId for directoryHandle Reissue on
            // invalid handle
            Thread.sleep(600);
            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(999).filter(29), appClient, 1);
            System.out.println("DIR REQUEST HANDLE");
            System.out.println(directoryHandle);
            Thread.sleep(600);
            System.out.println("REISSUE DIRECTORY");
            consumer.reissue(reqMsg.clear().domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED").filter(3).interestAfterRefresh(true), directoryHandle);
            // END APIQA:

            long handle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);

            Thread.sleep(60000); // API calls onRefreshMsg(), onUpdateMsg() and
                                 // onStatusMsg()
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
