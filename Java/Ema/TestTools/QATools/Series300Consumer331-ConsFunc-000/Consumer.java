///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

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
    public static int _OPTION = 0;
    public static int _FILTER = -1;
    public static int _SLEEPTIME = 0; 

    public static void printHelp()
    {
        System.out.println("\nOptions:\n" + 
            "  -?\t\t\tShows this usage\n\n" + 
            "  -f <source directory filter in decimal; default = no filter is specified>\n" +
            "     Possible values for filter, valid range = 0-63:\n" +
            "     0 :  No Filter \n" +   
            "     1 :  SERVICE_INFO_FILTER 0x01 \n" +   
            "     2 :  SERVICE_STATE_FILTER 0x02 \n" +
            "     4 :  SERVICE_GROUP_FILTER 0x04 \n" +
            "     8 :  SERVICE_LOAD_FILTER 0x08 \n" +
            "    16 :  SERVICE_DATA_FILTER 0x10 \n" +
            "    32 :  SERVICE_LINK_FILTER 0x20 \n" +
            "    ?? :  Mix of above values upto 63 \n\n" +
            "  -m <option>; default = option 0\n" +
            "     Possible values for option, valid range = 0-4:\n" +
            "     0 :  Request source directory without serviceName or serviceId\n" +   
            "     1 :  Request source directory with serviceName\n" +   
            "     2 :  Request source directory with serviceName; Request item on that service\n" +   
            "     3 :  Request source directory with serviceId\n" +
            "     4 :  Request source directory with serviceId; Request item on that service\n\n" +   
            "  -s <amount of time to wait before requesting an item in seconds; default = no wait>\n" +
            "     This option only applies to -m 2 or -m 4\n" +
            " \n");
    }

    public static boolean readCommandlineArgs(String[] argv)
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
            else if (0 == argv[idx].compareToIgnoreCase("-f"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._FILTER = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-m"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._OPTION = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-s"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._SLEEPTIME = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else
            {
                printHelp();
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args) throws InterruptedException
    {
        OmmConsumer consumer = null;
        try
        {
            if ( !readCommandlineArgs(args) ) return;
            AppClient appClient = new AppClient();
            
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
 
            ReqMsg reqMsg = EmaFactory.createReqMsg();

            Integer closure = (Integer)1;

            //APIQA: Tring different registerClient commands depending on input
      
            switch(Consumer._OPTION)
            {
                default:
                case 0:
                case 5:
                   if (Consumer._FILTER >= 0) {
                       System.out.println("********APIQA: Requesting directory without service name, service id, and filter=" + Consumer._FILTER + "\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(Consumer._FILTER), appClient); 
                   } else {
                       System.out.println("********APIQA: Requesting directory without service name, service id\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), appClient); 
                   } 
                   break; 
                case 1:
                case 2: 
                   if (Consumer._FILTER >= 0) {
                       System.out.println("********APIQA: Requesting directory with service=DIRECT_FEED and filter=" + Consumer._FILTER + "\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED").filter(Consumer._FILTER), appClient);
                   } else {
                       System.out.println("********APIQA: Requesting directory with service=DIRECT_FEED\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED"), appClient);
                   } 
                   break; 
                case 3:
                case 4:
                   if (Consumer._FILTER >= 0) {
                       System.out.println("********APIQA: Requesting directory with service=serviceID and filter=" + Consumer._FILTER + "\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090).filter(Consumer._FILTER), appClient);
                   } else {
                       System.out.println("********APIQA: Requesting directory with service=serviceID\n\n");
                       consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(8090), appClient);
                   }
                   break;
            }
            if ( ( Consumer._OPTION == 2 ) || ( Consumer._OPTION == 4 ) || ( Consumer._OPTION == 5 ) )
            {
                   if (Consumer._SLEEPTIME > 0 ) {
                       System.out.println("********APIQA: Sleeping (in seconds): " + Consumer._SLEEPTIME + "\n");
                       Thread.sleep(Consumer._SLEEPTIME * 1000);            // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
                   }
                   if ( ( Consumer._OPTION == 2 ) || ( Consumer._OPTION == 5 ) ) {
                       System.out.println("********APIQA: Requesting item wth service=DIRECT_FEED\n\n"); 
                       consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);
                   } else {
                       System.out.println("********APIQA: Requesting item wth service=serviceID\n\n"); 
                       consumer.registerClient(reqMsg.clear().serviceId(8090).name("IBM.N"), appClient);
                   }
            }
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
