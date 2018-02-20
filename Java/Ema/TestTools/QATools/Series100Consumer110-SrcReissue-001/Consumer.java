///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright Thomson Reuters 2015. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.consumer.series100.example110__MarketPrice__FileConfig;

import com.thomsonreuters.ema.access.Msg;

import java.util.Iterator;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.OmmArrayEntry;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        if (refreshMsg.hasMsgKey())
        {
            System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        }

        System.out.println("Item State: " + refreshMsg.state());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().fieldList());

        System.out.println(refreshMsg);

        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (updateMsg.hasMsgKey())
        {
            System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
        }

        if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
            decode(updateMsg.payload().fieldList());

        System.out.println(updateMsg);

        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        if (statusMsg.hasMsgKey())
        {
            System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
            System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));
        }

        if (statusMsg.hasState())
            System.out.println("Item State: " + statusMsg.state());

        System.out.println();
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent)
    {
    }

    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent)
    {
    }

    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent)
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
            case DataTypes.MAP:
                decode(msg.payload().map());
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
                            }
                        }
                        System.out.println();
                    }
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
        Iterator<FieldEntry> iter = fieldList.iterator();
        FieldEntry fieldEntry;
        while (iter.hasNext())
        {
            fieldEntry = iter.next();
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
                    case DataTypes.ERROR:
                        System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
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
        try
        {
            AppClient appClient = new AppClient();
            OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
            ReqMsg reqMsg = EmaFactory.createReqMsg();
            OmmConsumer consumer = EmaFactory.createOmmConsumer(config.addAdminMsg(reqMsg.clear().domainType(EmaRdm.MMT_DIRECTORY).filter(29).interestAfterRefresh(true).serviceId(999)));
            Integer closure = (Integer)1;
            long loginHandle = consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_LOGIN), appClient, closure);
            long itemHandle = consumer.registerClient(reqMsg.clear().serviceId(999).name("JPY="), appClient, closure);
            Thread.sleep(100);
            long dirHandle = consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_DIRECTORY).interestAfterRefresh(true).filter(29).serviceId(999), appClient, closure);
            System.out.println("REISSUE DIRECTORY");
            consumer.reissue(reqMsg.clear().domainType(EmaRdm.MMT_DIRECTORY).filter(29).interestAfterRefresh(true).serviceId(999), dirHandle);
            Thread.sleep(60000);
            consumer.uninitialize();
        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp);
        }
    }
}

//END APIQA
