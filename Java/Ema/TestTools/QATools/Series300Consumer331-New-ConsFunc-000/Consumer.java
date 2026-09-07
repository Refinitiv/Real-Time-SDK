/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series300.ex331_Directory_Streaming;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.domain.directory.DirectoryRefresh;
import com.refinitiv.ema.domain.directory.DirectoryRequest;
import com.refinitiv.ema.domain.directory.DirectoryStatus;
import com.refinitiv.ema.domain.directory.DirectoryUpdate;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;


class AppClient implements OmmConsumerClient
{
    private final DirectoryRefresh directoryRefresh =  EmaFactory.Domain.createDirectoryRefresh();
    private final DirectoryStatus directoryStatus =  EmaFactory.Domain.createDirectoryStatus();
    private final DirectoryUpdate directoryUpdate =  EmaFactory.Domain.createDirectoryUpdate();

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

        if (refreshMsg.domainType() == EmaRdm.MMT_DIRECTORY)
        {
            directoryRefresh.clear();
            directoryRefresh.message(refreshMsg);
            System.out.println(directoryRefresh);
        }
        else
        {
            System.out.println(refreshMsg);
        }

        System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

        if (updateMsg.domainType() == EmaRdm.MMT_DIRECTORY)
        {
            directoryUpdate.clear();
            directoryUpdate.message(updateMsg);
            System.out.println(directoryUpdate);
        }
        else
        {
            System.out.println(updateMsg);
        }

        System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.domainType() == EmaRdm.MMT_DIRECTORY)
        {
            directoryStatus.clear();
            directoryStatus.message(statusMsg);
            System.out.println(directoryStatus);
        }
        else
        {
            if (statusMsg.hasState())
                System.out.println("Item State: " +statusMsg.state());
        }

        System.out.println();
	}
	
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event) {}
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event){}
	public void onAllMsg(Msg msg, OmmConsumerEvent event){}
}

public class Consumer 
{
    public static int FILTER = 0;
    public static String SERVICE = "DIRECT_FEED";
    public static String HOST = "localhost";
    public static String PORT = "14002";
    public static String ITEM = "IBM.N";
    public static int OPTION = 0;
    public static int SLEEPTIME = 0; 

        public static void printHelp()
    {
        System.out.println("\nOptions:\n" + 
            "  -?\t\t\tShows this usage\n\n" + 
            "  -f <source directory filter in decimal; default = no filter is specified>\n" +
            "     Possible values for filter, valid range = 0-63:\n" +
            "     0 :  No Filter \n" +   
            "     1 :  SERVICE_INFOFILTER 0x01 \n" +   
            "     2 :  SERVICE_STATEFILTER 0x02 \n" +
            "     4 :  SERVICE_GROUPFILTER 0x04 \n" +
            "     8 :  SERVICE_LOADFILTER 0x08 \n" +
            "    16 :  SERVICE_DATAFILTER 0x10 \n" +
            "    32 :  SERVICE_LINKFILTER 0x20 \n" +
            "    ?? :  Mix of above values upto 63 \n\n" +
            "    -service :  Service name \n\n" +
            "    -host : Host address \n\n" +
            "    -p : Port \n\n" +
            "    -item : item \n\n" +
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
                Consumer.FILTER = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-service"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer.SERVICE = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-host"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer.HOST = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-m"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer.OPTION = Integer.parseInt(argv[idx]);
                ++idx;
            }
             else if (0 == argv[idx].compareToIgnoreCase("-s"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer.SLEEPTIME = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-p"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer.PORT = argv[idx];
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-item"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer.ITEM = argv[idx];
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

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
        
		try
		{   
            if ( !readCommandlineArgs(args) ) return;
			AppClient appClient = new AppClient();

            ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host(Consumer.HOST + ":" + Consumer.PORT).username("user"));

            DirectoryRequest directoryRequest =  EmaFactory.Domain.createDirectoryRequest();
      


          //APIQA: Tring different registerClient commands depending on input  
          switch(Consumer.OPTION)
            {
                default:
                case 0:
                case 5:
                   if (Consumer.FILTER >= 0) {
                       System.out.println("********APIQA: Requesting directory without service name, service id, and filter=" + Consumer.FILTER + "\n\n");
                       
                       consumer.registerClient(directoryRequest.message().filter(Consumer.FILTER), appClient); 
                   } else {
                       System.out.println("********APIQA: Requesting directory without service name, service id\n\n");
                       consumer.registerClient(directoryRequest.message(), appClient); 
                   } 
                   break; 
                case 1:
                case 2: 
                   if (Consumer.FILTER >= 0) {
                       System.out.println("********APIQA: Requesting directory with service=" + Consumer.SERVICE + " and filter=" + Consumer.FILTER + "\n\n");
                       consumer.registerClient(directoryRequest.message().serviceName(Consumer.SERVICE).filter(Consumer.FILTER), appClient);
                   } else {
                       System.out.println("********APIQA: Requesting directory with service=" + Consumer.SERVICE + "\n\n");
                       consumer.registerClient(directoryRequest.message().serviceName(Consumer.SERVICE), appClient);
                   } 
                   break; 
                case 3:
                case 4:
                   if (Consumer.FILTER >= 0) {
                       System.out.println("********APIQA: Requesting directory with service=serviceID and filter=" + Consumer.FILTER + "\n\n");
                       consumer.registerClient(directoryRequest.message().serviceId(8090).filter(Consumer.FILTER), appClient);
                   } else {
                       System.out.println("********APIQA: Requesting directory with service=serviceID\n\n");
                       consumer.registerClient(directoryRequest.message().serviceId(8090), appClient);
                   }
                   break;
            }
            if ( ( Consumer.OPTION == 2 ) || ( Consumer.OPTION == 4 ) || ( Consumer.OPTION == 5 ) )
            {
                   if (Consumer.SLEEPTIME > 0 ) {
                       System.out.println("********APIQA: Sleeping (in seconds): " + Consumer.SLEEPTIME + "\n");
                       Thread.sleep(Consumer.SLEEPTIME * 1000);            // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
                   }
                   if ( ( Consumer.OPTION == 2 ) || ( Consumer.OPTION == 5 ) ) {
                       System.out.println("********APIQA: Requesting item wth service=" + Consumer.SERVICE + "\n\n"); 
                       consumer.registerClient(reqMsg.clear().serviceName(Consumer.SERVICE).name(Consumer.ITEM), appClient);
                   } else {
                       System.out.println("********APIQA: Requesting item wth service=serviceID\n\n"); 
                       consumer.registerClient(reqMsg.clear().serviceId(8090).name(Consumer.ITEM), appClient);
                   }
            }
			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
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
