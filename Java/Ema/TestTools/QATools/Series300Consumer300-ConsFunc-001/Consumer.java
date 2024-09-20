///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.consumer.series300.ex300_MP_Close;

import com.refinitiv.ema.access.Msg;

import java.util.Iterator;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementList;
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
        if (refreshMsg.hasMsgKey())
            System.out.println("Item Name: " + refreshMsg.name() + " Service Name: " + refreshMsg.serviceName());

        System.out.println("Item State: " + refreshMsg.state());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
            decode(refreshMsg.payload().fieldList());

        System.out.println();
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        if (updateMsg.hasMsgKey())
            System.out.println("Item Name: " + updateMsg.name() + " Service Name: " + updateMsg.serviceName());

        if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
            decode(updateMsg.payload().fieldList());

        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        if (statusMsg.hasMsgKey())
            System.out.println("Item Name: " + statusMsg.name() + " Service Name: " + statusMsg.serviceName());

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
                    case DataTypes.RMTES :
                        System.out.println(fieldEntry.rmtes());
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
}

public class Consumer
{
    private static String authenticationToken = "";
    private static String appId = "256";

    public static void printHelp()
    {

        System.out.println("\nOptions:\n" + "  -?                            Shows this usage\n\n" + "  -at <token>           Authentication token to use in login request [default = \"\"]\n"
                + "  -aid <applicationId>        ApplicationId set as login Attribute [default = 256]\n" + "\n");
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
    }

    public static void main(String[] args)
    {
        printActiveConfig();
        if (!init(args))
            return;
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();

            ReqMsg reqMsg = EmaFactory.createReqMsg();
            ElementList elementList = EmaFactory.createElementList();
            elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, appId));
            elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, "127.0.0.1/net"));
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1));

            consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().operationModel(OmmConsumerConfig.OperationModel.USER_DISPATCH)
                    .addAdminMsg(reqMsg.domainType(EmaRdm.MMT_LOGIN).name(authenticationToken).nameType(EmaRdm.USER_NAME).attrib(elementList))
                    .addAdminMsg(reqMsg.clear().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER)));

            consumer.registerClient(reqMsg.clear().serviceName("NI_PUB").name("TRI.N"), appClient, null);

            long startTime = System.currentTimeMillis();
            while (startTime + 60000 > System.currentTimeMillis())
                consumer.dispatch(10); // calls to onRefreshMsg(),
                                       // onUpdateMsg(), or onStatusMsg()
                                       // execute on this thread
        }
        catch (OmmException excp)
        {
            System.out.println(excp);
        }
        finally
        {
            if (consumer != null)
                consumer.uninitialize();
        }
    }
}

//END APIQA
