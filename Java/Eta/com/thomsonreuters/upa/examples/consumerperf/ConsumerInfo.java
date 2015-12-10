package com.thomsonreuters.upa.examples.consumerperf;

import java.nio.channels.Selector;
import java.util.List;

import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

public class ConsumerInfo
{
	Thread consThreads;
    Channel consumerChannel;
    String srvrHostname;
    String srvrPortNo;
    String serviceName;
	String interfaceName;
	boolean interfacePresent;
    int pingTimeoutServer;
    int pingTimeoutClient;
    int ConsumerRuntime;
    boolean shouldRecoverConnection;
    boolean mpItemReq;
    int connectionType;
	int itemRequestCount;
	int consumerType;
	int decodeType;
	long nextSendPingTime;
	volatile int itemCount;
	volatile int imagesReceived;
	String itemFilename;
	List<Item> items;
    List<Item> latencyItems; 
	
	private final Error _pingError = TransportFactory.createError();
	
	ConsumerStats stats;
	
	LocalFieldSetDefDb fListSetDef;
	DataDictionary fieldDictionary;
	int latStreamId;

	Selector selector;
	
	int guaranteedOutputBuffers;
	int numInputBuffers;
	
	String username;
	boolean channelActive;
	boolean requestsSent;
	volatile boolean shutdown;
	
	public ConsumerInfo()
	{
		stats = new ConsumerStats();
		try
		{
			selector = Selector.open();
		}
		catch (Exception e)
		{
			System.out.println("Selector() Exception: " + e.getMessage());
		}
	}
	
    public void handlePing()
    {
        if (pingTimeoutClient != 0 && consumerChannel != null)
        {        
            long currentTime = System.currentTimeMillis();
            
            // initialize nextSendPingTime if 0
            if (nextSendPingTime == 0)
            {
            	nextSendPingTime = currentTime + (pingTimeoutClient * 1000);
            }
            
            // handle client pings
            if (currentTime >= nextSendPingTime)
            {
                // send ping to server
                System.out.println("Sending ping");
                if (consumerChannel.ping(_pingError) < TransportReturnCodes.SUCCESS)
                {
                    System.out.println("Error sending ping: " + _pingError.text());
                }
                
                // set time to send next ping from client
                nextSendPingTime = currentTime + (pingTimeoutClient * 1000);
            }
        }
    }

}
