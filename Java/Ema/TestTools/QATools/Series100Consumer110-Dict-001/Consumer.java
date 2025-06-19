/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.consumer.series100.ex110_MP_FileCfg;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmArrayEntry;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.Series;
import com.refinitiv.ema.access.SeriesEntry;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

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
            case DataTypes.SERIES:
                decode(msg.payload().series());
                break;
            case DataTypes.FIELD_LIST:
                decode(msg.payload().fieldList());
                break;
            case DataTypes.ELEMENT_LIST:
                decode(msg.payload().elementList());
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
                    case DataTypes.ARRAY:
                        System.out.println();
                        decode(elementEntry.array());
                        break;
                    default:
                        System.out.println();
                        break;
                }
        }
    }

    void decode(OmmArray array)
    {
        for (OmmArrayEntry arrayEntry : array)
        {
            System.out.print(" DataType: " + DataType.asString(arrayEntry.load().dataType()) + " Value: ");

            if (Data.DataCode.BLANK == arrayEntry.code())
                System.out.println(" blank");
            else
                switch (arrayEntry.loadType())
                {
                    case DataTypes.REAL:
                        System.out.println(arrayEntry.real().asDouble());
                        break;
                    case DataTypes.DATE:
                        System.out.println(arrayEntry.date().day() + " / " + arrayEntry.date().month() + " / " + arrayEntry.date().year());
                        break;
                    case DataTypes.TIME:
                        System.out.println(arrayEntry.time().hour() + ":" + arrayEntry.time().minute() + ":" + arrayEntry.time().second() + ":" + arrayEntry.time().millisecond());
                        break;
                    case DataTypes.INT:
                        System.out.println(arrayEntry.intValue());
                        break;
                    case DataTypes.UINT:
                        System.out.println(arrayEntry.uintValue());
                        break;
                    case DataTypes.ASCII:
                        System.out.println(arrayEntry.ascii());
                        break;
                    case DataTypes.ENUM:
                        System.out.println(arrayEntry.enumValue());
                        break;
                    case DataTypes.RMTES :
                        System.out.println(arrayEntry.rmtes());
                        break;
                    case DataTypes.ERROR:
                        System.out.println(arrayEntry.error().errorCode() + " (" + arrayEntry.error().errorCodeAsString() + ")");
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
                    case DataTypes.RMTES:
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

    void decode(Series series)
    {
        switch (series.summaryData().dataType())
        {
            case DataTypes.ELEMENT_LIST:
                decode(series.summaryData().elementList());
                break;
        }

        for (SeriesEntry seriesEntry : series)
        {
            System.out.println(" DataType: " + DataType.asString(seriesEntry.loadType()) + " Value: ");

            switch (seriesEntry.loadType())
            {
                case DataTypes.ELEMENT_LIST:
                    decode(seriesEntry.elementList());
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
            config.addAdminMsg(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).filter(EmaRdm.DICTIONARY_NORMAL).interestAfterRefresh(false).name("RWFFld"));
            config.addAdminMsg(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).filter(EmaRdm.DICTIONARY_NORMAL).interestAfterRefresh(false).name("RWFEnum"));
            config.username("user");
            OmmConsumer consumer = EmaFactory.createOmmConsumer(config);
            long handle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("JPY="), appClient);
            Thread.sleep(300000); // API calls onRefreshMsg(), onUpdateMsg() and
                                  // onStatusMsg()
            consumer.uninitialize();
        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
    }
}

//END APIQA
