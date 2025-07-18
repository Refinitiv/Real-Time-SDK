/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series300.ex331_Directory_Streaming;

import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.FilterList;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.OmmArrayEntry;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
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
            System.out.println("Item State: " +statusMsg.state());
        
        System.out.println();
    }
    
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event) {}
    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event){}
    public void onAllMsg(Msg msg, OmmConsumerEvent event){}

    void decode(Msg msg)
    {
        switch(msg.attrib().dataType())
        {
        case DataTypes.ELEMENT_LIST:
            decode(msg.attrib().elementList());
            break;
        default:
            break;
        }

        switch(msg.payload().dataType())
        {
        case  DataTypes.MAP:
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
                case DataTypes.REAL :
                    System.out.println(elementEntry.real().asDouble());
                    break;
                case DataTypes.DATE :
                    System.out.println(elementEntry.date().day() + " / " + elementEntry.date().month() + " / " + elementEntry.date().year());
                    break;
                case DataTypes.TIME :
                    System.out.println(elementEntry.time().hour() + ":" + elementEntry.time().minute() + ":" + elementEntry.time().second() + ":" + elementEntry.time().millisecond());
                    break;
                case DataTypes.INT :
                    System.out.println(elementEntry.intValue());
                    break;
                case DataTypes.UINT :
                    System.out.println(elementEntry.uintValue());
                    break;
                case DataTypes.ASCII :
                    System.out.println(elementEntry.ascii());
                    break;
                case DataTypes.ENUM :
                    System.out.println(elementEntry.enumValue());
                    break;
                case DataTypes.ARRAY :
                    {
                        boolean first = true;
                        for(OmmArrayEntry arrayEntry : elementEntry.array())
                        {
                            if ( !first )
                                System.out.print(", ");
                            else
                                first = false;
                            switch(arrayEntry.loadType())
                            {
                            case DataTypes.ASCII :
                                System.out.print(arrayEntry.ascii());
                                break;
                            case DataTypes.UINT :
                                System.out.print(arrayEntry.uintValue());
                                break;
                            case DataTypes.QOS :
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
                case DataTypes.ERROR :
                    System.out.println(elementEntry.error().errorCode() +" (" + elementEntry.error().errorCodeAsString() + ")");
                    break;
                default :
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
                case DataTypes.REAL :
                    System.out.println(fieldEntry.real().asDouble());
                    break;
                case DataTypes.DATE :
                    System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
                    break;
                case DataTypes.TIME :
                    System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
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
                case DataTypes.ARRAY :
                    System.out.println(fieldEntry.array());
                    break;
                case DataTypes.RMTES :
                    System.out.println(fieldEntry.rmtes());
                    break;
                case DataTypes.ERROR :
                    System.out.println(fieldEntry.error().errorCode() +" (" + fieldEntry.error().errorCodeAsString() + ")");
                    break;
                default :
                    System.out.println();
                    break;
                }
        }
    }
    
    void decode(Map map)
    {
        for(MapEntry mapEntry : map)
        {
            //APIQA
            System.out.println("Action = " + mapEntry.mapActionAsString() + ", key = " + mapEntry.key().uintValue());
            switch (mapEntry.loadType())
            {
            case DataTypes.FILTER_LIST :
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
        for(FilterEntry filterEntry : filterList)
        {
            System.out.println("ID: " + filterEntry.filterId() + " Action = " + filterEntry.filterActionAsString() + " DataType: " + DataType.asString(filterEntry.loadType()) + " Value: ");

            switch (filterEntry.loadType())
            {
            case DataTypes.ELEMENT_LIST :
                decode(filterEntry.elementList());
                break;
            case DataTypes.MAP :
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
    public static void main(String[] args) throws InterruptedException
    {
        OmmConsumer consumer = null;
        try
        {
            AppClient appClient = new AppClient();
            
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("132.88.227.161:14002").username("user"));
 
            ReqMsg reqMsg = EmaFactory.createReqMsg();
            int input = 0;

            Integer closure = (Integer)1;
            //APIQA: Tring different registerClient commands depending on input

            if (args[0].equalsIgnoreCase("-m"))
            {
                int temp = Integer.valueOf(args[1]); 

                switch(temp)
                {
                    case 0:
                       System.out.println("APIQA: Requesting directory without service name specified or filter specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), appClient); 
                       break; 
                    case 1:
                       System.out.println("APIQA: Requesting directory with service name of DIRECT_FEED specified and no filter specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED"), appClient);
		       break; 
                    case 2:
                       System.out.println("APIQA: Requesting directory with service name of DF415 specified and no filter specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415"), appClient);
                       break; 
                    case 3:
                       System.out.println("APIQA: Requesting directory without service name and with filter 0 specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(0), appClient);
                       break; 
                    case 4:
                       System.out.println("APIQA: Requesting directory without service name and with filter SERVICE_INFO_FILTER specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_INFO_FILTER), appClient);
                       break; 
                    case 5:
                       System.out.println("APIQA: Requesting directory without service name and with filter SERVICE_STATE_FILTER specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_STATE_FILTER), appClient);
                       break; 
                    case 6:
                       System.out.println("APIQA: Requesting directory without service name and with filter SERVICE_GROUP_FILTER specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_GROUP_FILTER), appClient);
                       break; 
                    case 7:
                       System.out.println("APIQA: Requesting directory without service name and with filter SERVICE_LOAD_FILTER specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_LOAD_FILTER), appClient);
                       break; 
                    case 8:
                       System.out.println("APIQA: Requesting directory without service name and with filter SERVICE_DATA_FILTER specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_DATA_FILTER), appClient);
                       break; 
                    case 9:
                       System.out.println("APIQA: Requesting directory without service name and with filter SERVICE_LINK_FILTER specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_LINK_FILTER), appClient);
                       break; 
                    case 10:
                       System.out.println("APIQA: Requesting directory with service id of 8090 specified and no filter specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090), appClient);
                       break; 
                    case 11:
                       System.out.println("APIQA: Requesting directory with service id of 8090 specified and with filter 0 specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090).filter(0), appClient);
                       break;
		    case 12:
                       System.out.println("APIQA: Requesting directory with service id of 8090 specified and with filter 29 specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090).filter(29), appClient);
		       break;
		    case 13:
                       System.out.println("APIQA: Requesting directory with service name of DIRECT_FEED specified, name of IBM.N specified, and no filter specified\n\n");
                       consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);
		       break;
		    case 14:
                       System.out.println("APIQA: Requesting directory with service name of DF415 specified, name of JPY= specified, and no filter specified\n\n");
                       consumer.registerClient(reqMsg.clear().serviceName("DF415").name("JPY="), appClient, closure);
		       break;
		    case 15:
                       System.out.println("APIQA: Requesting directory with service id of 8090 specified, name of JPY= specified, and no filter specified\n\n");
                       consumer.registerClient(reqMsg.clear().serviceId(8090).name("JPY="), appClient, closure);
		       break; 
                    default:
                       System.out.println("APIQA: Requesting directory with service id of 8090 specified and with filter 0 specified\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090).filter(0), appClient);
                       break;
                }
            }
           
                    
            //APIQA
            //long directoryHandle = consumer.registerClient(EmaFactory.createReqMsg()
            //                     .domainType(EmaRdm.MMT_DIRECTORY)
            //                     .filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_INFO_FILTER), appClient);
            //long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED"), appClient);
            // long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415"), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(0), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_INFO_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_STATE_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_GROUP_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_LOAD_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_DATA_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_LINK_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(0), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_INFO_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_STATE_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_GROUP_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_LOAD_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_DATA_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DF415").filter(com.refinitiv.ema.rdm.EmaRdm.SERVICE_LINK_FILTER), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090).filter(0), appClient);
            //consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090).filter(29), appClient);
            
            //long handle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);
            //Integer closure = (Integer)1;
            //consumer.registerClient(reqMsg.clear().serviceName("DF415").name("JPY="), appClient, closure);
            //consumer.registerClient(reqMsg.clear().serviceId(8090).name("JPY="), appClient, closure);

            Thread.sleep(60000);            // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
        } 
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
        finally 
        {
            if (consumer != null) consumer.uninitialize();
        }
    }
}
