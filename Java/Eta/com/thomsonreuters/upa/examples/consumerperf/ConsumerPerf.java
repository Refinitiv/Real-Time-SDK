package com.thomsonreuters.upa.examples.consumerperf;

import java.lang.management.ManagementFactory;
import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.sun.management.OperatingSystemMXBean; // introduced in Java 7, and may not be available in non-Oracle JDKs (see below)
//// import java.lang.management.OperatingSystemMXBean // non-Oracle JDKs may need to import this version instead of the one above

public class ConsumerPerf
{
	/* connection retry time in seconds */
	final int CONSUMER_CONNECTION_RETRY_TIME = 15;
	/* run-time in seconds */
	final int CONSUMER_RUNTIME = 30;

	final int MAX_CONS_THREADS  = 4;
	
	/* Login information*/
	final String defaultUsername = "user";
	final String applicationId = "256";
	/* password */
	final String pword = "mypassword";
	/* instance id */
	final String instanceId = "instance1";
	
	private static final String ITEM_FILE_PARAM = "-itemFile";
	private static final String ITEM_COUNT_PARAM = "-itemCount";
	
	int selectCount;
	int notselect;

	ConsumerInfo[] consThreadInfo;
	ConsumerStats[] prevStats;
	ConsumerStats tempStats;
	
	int _imageRetrievalTime = 0;
    int _itemCount = 0;

	static
	{
		System.loadLibrary("rsslRsslJNI");
	}
	
	{
		consThreadInfo = new ConsumerInfo[MAX_CONS_THREADS];
		prevStats = new ConsumerStats[MAX_CONS_THREADS];
		for (int i = 0; i < MAX_CONS_THREADS; i++)
		{
			consThreadInfo[i] = new ConsumerInfo();
			prevStats[i] = new ConsumerStats();
		}
		tempStats = new ConsumerStats();
	}

	void initStats(ConsumerStats stats)
	{
		stats.updateCount = 0;
		stats.latencyAvg = 0;
		stats.latencyRunningSum = 0;
		stats.latencyCount = 0;
		stats.latencyMax = 0;
		stats.latencyMin = 0x7fffffff;
		stats.cpuAvg = 0.0;
		stats.cpuCount = 0;
		stats.latencyAvg = 0.0;
		stats.latencyCount = 0;
	}

	void initConsumerInfo(ConsumerInfo consInfo)
	{
	    consInfo.srvrHostname = "localhost";
	    consInfo.srvrPortNo = "14002";
	    consInfo.serviceName = "DIRECT_FEED";
		consInfo.interfacePresent = false;
	    consInfo.pingTimeoutServer = 0;
	    consInfo.pingTimeoutClient = 0;
		consInfo.ConsumerRuntime = 0;
	    consInfo.shouldRecoverConnection = true;
	    consInfo.mpItemReq = false;
	    consInfo.connectionType = ConnectionTypes.SOCKET;
		consInfo.consumerType = ConsumerType.TIER_ZERO_SELECT;
		consInfo.itemRequestCount = 10;
		consInfo.decodeType = DecodeType.FULL;
		
		initStats(consInfo.stats);
		
		consInfo.guaranteedOutputBuffers = 10000;
		consInfo.numInputBuffers = 15;

		consInfo.requestsSent = false;
		consInfo.channelActive = false;
		consInfo.consumerChannel = null;
		
		consInfo.shutdown = false;
		
		/* login response information */
		consInfo.username = "user";
	}
	
	public void run(String[] args)
    {
		Error error = TransportFactory.createError();
		int i;
		boolean start = false;
		//ChannelInfo chnlInfo = TransportFactory.createChannelInfo();
		int startTime = -1;
		int endTime = CONSUMER_RUNTIME;
		int consThreadCount = 0;
		boolean allImagesReceived = false;

		for(i = 0; i < MAX_CONS_THREADS; ++i)
		{
			initConsumerInfo(consThreadInfo[i]);
			initStats(prevStats[i]);
			consThreadInfo[i].stats.totalMsgs = 0;
		}
		
		initStats(tempStats);

	    /* Check usage and retrieve operating parameters */
		if (args.length == 0) /* use default operating parameters */
		{
			System.out.println("\nUsing default operating parameters...\n");
			System.out.println("srvrHostname: " + consThreadInfo[0].srvrHostname);
			System.out.println("srvrPortNo: " + consThreadInfo[0].srvrPortNo);
			System.out.println("serviceName: " + consThreadInfo[0].serviceName);
		}
		else if (args.length > 1) /* all operating parameters entered by user */
		{
			i = 0;

			while(i < args.length)
			{
				if(args[i].equals("-uname"))
				{
					i += 2;
					consThreadInfo[0].username = args[i-1];
				}
				else if(args[i].equals("-h"))
				{
					i += 2;
					consThreadInfo[0].srvrHostname = args[i-1];
				}
				else if(args[i].equals("-p"))
				{
					i += 2;
					consThreadInfo[0].srvrPortNo = args[i-1];
				}
				else if(args[i].equals("-interfaceName"))
				{
					i += 2;
					consThreadInfo[0].interfaceName = args[i-1];
					consThreadInfo[0].interfacePresent = true;
				}
				else if(args[i].equals("-s"))
				{
					i += 2;
					consThreadInfo[0].serviceName = args[i-1];
				}
				else if(args[i].equals("-mp"))
				{
					i += 2;
					/* add item count in market price handler */
					try
					{
						consThreadInfo[0].itemRequestCount = Integer.parseInt(args[i-1]);
					}
					catch (NumberFormatException e)
					{
						consThreadInfo[0].itemRequestCount = 0;
					}
				}
				else if(args[i].equals("-connectionType"))
				{
					i += 2;
					if(args[i-1].equals("socket"))
					{
						consThreadInfo[0].connectionType = ConnectionTypes.SOCKET;
					}
					else if(args[i-1].equals("tunnel"))
					{
						consThreadInfo[0].connectionType = ConnectionTypes.ENCRYPTED;
					}
					else if(args[i-1].equals("http"))
					{
						consThreadInfo[0].connectionType = ConnectionTypes.HTTP;
					}
					/*else if(args[i-1].equals("shmem"))
					{
						consThreadInfo[0].connectionType = ConnectionTypes.UNIDIR_SHMEM;
						consThreadInfo[0].consumerType = ConsumerType.TIER_ZERO_READ;
					}
					else if(args[i-1].equals("rrcp"))
					{
						consThreadInfo[0].connectionType = ConnectionTypes.RELIABLE_MCAST;
						consThreadInfo[0].consumerType = ConsumerType.TIER_ZERO_SELECT;
					}*/
					else
						System.out.println("ERROR.  " + args[i-1] + " not supported.");
				}
				else if(args[i].equals("-consumerType"))
				{
					i += 2;
					if(args[i-1].equals("T0Select"))
					{
						consThreadInfo[0].consumerType = ConsumerType.TIER_ZERO_SELECT;
					}
					else if(args[i-1].equals("T0Read"))
					{
						consThreadInfo[0].consumerType = ConsumerType.TIER_ZERO_READ;
					}	
				}
				else if(args[i].equals("-decodeType"))
				{
					i += 2;
					if(args[i-1].equals("full"))
						consThreadInfo[0].decodeType = DecodeType.FULL;
					else if(args[i-1].equals("partial"))
						consThreadInfo[0].decodeType = DecodeType.PARTIAL;
					else if(args[i-1].equals("header"))
						consThreadInfo[0].decodeType = DecodeType.HEADER_ONLY;
					else
						System.out.println("ERROR.  " + args[i-1] + " not supported.");
				}
				else if(args[i].equals("-outputBuff"))
				{
					i += 2;
					try
					{
						consThreadInfo[0].guaranteedOutputBuffers = Integer.parseInt(args[i-1]);
					}
					catch (NumberFormatException e)
					{
						consThreadInfo[0].guaranteedOutputBuffers = 0;
					}
				}
				else if(args[i].equals("-inputBuff"))
				{
					i += 2;
					try
					{
						consThreadInfo[0].numInputBuffers = Integer.parseInt(args[i-1]);
					}
					catch (NumberFormatException e)
					{
						consThreadInfo[0].numInputBuffers = 0;
					}
				}
				else if(args[i].equals("-runtime"))
				{
					i += 2;
					try
					{
						endTime = Integer.parseInt(args[i-1]);
					}
					catch (NumberFormatException e)
					{
						endTime = 0;
					}
				}
                else if(args[i].equals(ITEM_FILE_PARAM))
                {
                    i += 2;
                    for (ConsumerInfo info : consThreadInfo)
                    {
                        info.itemFilename = args[i-1];
                    }
                }
                else if(args[i].equals(ITEM_COUNT_PARAM))
                {
                    i += 2;
                    
                    try
                    {
                        _itemCount = Integer.parseInt(args[i-1]);
                    }
                    catch (NumberFormatException e)
                    {
                        System.out.println(String.format("Error: \"%s\" is an invalid value for the %s parameter", args[i-1], ITEM_COUNT_PARAM));
                        System.exit(-1);
                    }
                    
                    for (ConsumerInfo info : consThreadInfo)
                    {
                        info.itemCount = _itemCount;
                    }
                }				
				else
				{
					System.out.println("Usage: ConsumerPerf or\nConsumerPerf [-uname <LoginUsername>] [-h <SrvrHostname>] [-p <SrvrPortNo>] [-s <ServiceName>] [-mp <MarketPrice Item Count>]>");
					System.out.println("[-itemFile <filename>] [-itemCount <Item Count>]");
					System.out.println("[-connectionType <socket/tunnel/http/shmem/rrcp>] [-consumerType <T0Select/T0Read>] [-cpuCons <Cons CPU bind>]");
					System.out.println("[-decodeType <full/partial/header>] [-outputBuff <guaranteedOutputBuffers>] [-inputBuff <numInputBuffers>] [-runtime <runtime>] ");
					System.out.println("[-interfaceName <interface>]");
					System.out.println("\n -mp For each occurrence , requests item using Market Price domain.");
					System.exit(-1);
				}
			}

			System.out.println("\nInput arguments...\n");
			System.out.println("srvrHostname: " + consThreadInfo[0].srvrHostname);
			System.out.println("srvrPortNo: " + consThreadInfo[0].srvrPortNo);
			System.out.println("serviceName: " + consThreadInfo[0].serviceName);
		}
		else
		{

			System.out.println("Usage: ConsumerPerf or\nConsumerPerf [-uname <LoginUsername>] [-h <SrvrHostname>] [-p <SrvrPortNo>] [-s <ServiceName>] [-mp <MarketPrice Item Count>]>");
			System.out.println("[-connectionType <socket/tunnel/http/shmem/rrcp>] [-consumerType <T0Select/T0Read>] [-cpuCons <Cons CPU bind>]");
			System.out.println("[-decodeType <full/partial/header>] [-outputBuff <guaranteedOutputBuffers>] [-inputBuff <numInputBuffers>] [-runtime <runtime>] ");
			System.out.println("[-interfaceName <interface>] [-initDelay <Initial delay(seconds)>]");
			System.out.println("\n -mp For each occurrence , requests item using Market Price domain.");
			System.exit(-1);
		}
		
		if (consThreadInfo[0].itemFilename == null || consThreadInfo[0].itemFilename.isEmpty())
		{
		    System.out.println(String.format("Error: you must specify an item filename via the %s parameter.", ITEM_FILE_PARAM));
		    System.exit(-1);
		}
		
        if (consThreadInfo[0].itemCount < 1)
        {
            System.out.println(String.format("Error: you must specify a value for the %s parameter that is >= 1", ITEM_COUNT_PARAM));
            System.exit(-1);
        }
		
		parseItemFile(consThreadInfo);
		
		consThreadCount = 1;

		/* Initialize Transport */
		/* globalLocking(false) is used since this is a single threaded application. */
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);
		if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
		{
			System.out.println("Initialize(): failed <" + error.text() + ">");
			System.exit(-1);
		}

		/* Spawn consumer threads */
		for(i = 0; i < consThreadCount; ++i)
		{
			if(consThreadInfo[i].consumerType == ConsumerType.TIER_ZERO_SELECT)
			{
				new Thread(new TierZeroSelectHandler(consThreadInfo[i])).start();
			}
			else if(consThreadInfo[i].consumerType == ConsumerType.TIER_ZERO_READ)
			{
				new Thread(new TierZeroReadHandler(consThreadInfo[i])).start();
			}
			
			System.out.println("thread Spawned!");
		}

		/* Init market price thread(s) */

		final String currentUsageFormat = " CPU: %6.2f%% MEM: %6.2fMB\n";
		
		StringBuilder statsOutput = new StringBuilder();

        // note: Java 7 (Oracle JDK) introduced the following method for obtaining the current proceses's CPU usage.
        // See http://sellmic.com/blog/2011/07/21/hidden-java-7-features-cpu-load-monitoring/ for details
        OperatingSystemMXBean osBean = ManagementFactory.getPlatformMXBean(OperatingSystemMXBean.class);
		
		System.out.println("Starting polling.");
		/* main polling thread here */
		while(true)
		{
			for(i = 0; i < consThreadCount; i++)
			{
				/* wait until all images are received before collecting statistics */
				if (allImagesReceived == false)
				{
					if (consThreadInfo[i].imagesReceived < consThreadInfo[i].itemCount)
					{
						System.out.println("Waiting for all images - expecting: " + consThreadInfo[i].itemCount + ", received: " + consThreadInfo[i].imagesReceived);
						if (i == 0)
						{
							++startTime;	
						}
						continue;
					}
					else
					{
						System.out.println("Waiting for all images - expecting: " + consThreadInfo[i].itemCount + ", received: " + consThreadInfo[i].imagesReceived);
						allImagesReceived = true;
						if (i == 0)
						{
							_imageRetrievalTime = startTime + 1;
							startTime = -1;
						}
					}
				}
				
				if (i == 0)
				{
					++startTime;	
				}
				
                double currentProcessCpuLoad = osBean.getProcessCpuLoad() * 100; // What % CPU load this current JVM is taking, from 0.0-1.0
                double currentMemoryUsage = (osBean.getCommittedVirtualMemorySize() / 1048576.0);
			    
				if(consThreadInfo[i].stats.updateCount > 0)
				{
					if(start == true)
					{
						tempStats.updateCount = consThreadInfo[i].stats.updateCount;
						tempStats.latencyAvg = consThreadInfo[i].stats.latencyAvg;
						tempStats.latencyRunningSum = consThreadInfo[i].stats.latencyRunningSum;
						tempStats.latencyCount = consThreadInfo[i].stats.latencyCount;
						tempStats.totalMsgs = consThreadInfo[i].stats.totalMsgs;
						
						consThreadInfo[i].stats.latencyAvg = 0;
						consThreadInfo[i].stats.latencyCount = 0;
						
						// Update the cumulative average for CPU and memory usage, which is equal to the previous cumulative average plus the difference between the latest data point and the previous average divided by the number of points received so far
						consThreadInfo[i].stats.cpuAvg = consThreadInfo[i].stats.cpuAvg + ((currentProcessCpuLoad - consThreadInfo[i].stats.cpuAvg) / ++consThreadInfo[i].stats.cpuCount); 
						consThreadInfo[i].stats.memoryAvg = consThreadInfo[i].stats.memoryAvg + ((currentMemoryUsage - consThreadInfo[i].stats.memoryAvg) / ++consThreadInfo[i].stats.memoryCount);
						
						//Thus the current cumulative average for a new data point . When all of the data points arrive (i = N), the cumulative average will equal the final average.
						
						statsOutput.setLength(0);
						statsOutput.append("Thread: ");
						statsOutput.append(i);
						statsOutput.append("  Update count: ");
						statsOutput.append((tempStats.updateCount - prevStats[i].updateCount));
						statsOutput.append("  Latency: ");
						statsOutput.append(tempStats.latencyAvg);
						statsOutput.append("us  Latency count: ");
						statsOutput.append(tempStats.latencyCount);
						statsOutput.append("  Total messages: ");
						statsOutput.append(tempStats.totalMsgs);
	                    if (consThreadInfo[i].consumerChannel != null && consThreadInfo[i].consumerChannel.selectableChannel() != null)
	                    {
	                        statsOutput.append(" Connected: ");
	                        statsOutput.append((consThreadInfo[i].consumerChannel.state() > ChannelState.INITIALIZING) ? true : false);
	                        statsOutput.append(" Channel state: ");
	                        statsOutput.append(ChannelState.toString(consThreadInfo[i].consumerChannel.state()));
	                    }
	                    statsOutput.append(String.format(currentUsageFormat, currentProcessCpuLoad, currentMemoryUsage));
	                    
						System.out.println(statsOutput.toString());

						prevStats[i].updateCount = tempStats.updateCount;
						
						if(startTime > 0)
						{
							prevStats[i].latencyAvg = (prevStats[i].latencyAvg*(double)prevStats[i].latencyCount + tempStats.latencyAvg*(double)tempStats.latencyCount)/
									((double)prevStats[i].latencyCount + (double)tempStats.latencyCount);
							prevStats[i].latencyRunningSum = tempStats.latencyRunningSum;
							prevStats[i].latencyCount += tempStats.latencyCount;
						}
						
						/*if(((startTime % 5) == 0) && (consThreadInfo[i].connectionType == ConnectionTypes.RELIABLE_MCAST))
						{
							consThreadInfo[i].consumerChannel.info(chnlInfo, error);
							
							statsOutput.setLength(0);
							statsOutput.append("RRCP INFO:  Packets Sent: ");
							statsOutput.append(chnlInfo.sysSendBufSize());
							statsOutput.append("  Packets received: ");
							statsOutput.append(chnlInfo.sysRecvBufSize());
							statsOutput.append(" Retransmit sent: ");
							statsOutput.append(chnlInfo.maxOutputBuffers());
							statsOutput.append(" Retransmit requests: ");
							statsOutput.append(chnlInfo.guaranteedOutputBuffers());
							statsOutput.toString();
							System.out.println(statsOutput.toString());
						}*/
					}
					else {
						start = true;
						initStats(consThreadInfo[i].stats);
						statsOutput.setLength(0);
						statsOutput.append("Thread ");
						statsOutput.append(i);
						statsOutput.append(" started consuming data.");
						System.out.println(statsOutput.toString());
					}
				}
				else
				{
                    consThreadInfo[i].stats.cpuAvg = currentProcessCpuLoad;
                    ++consThreadInfo[i].stats.cpuCount;
                    
                    consThreadInfo[i].stats.memoryAvg = currentMemoryUsage;
                    ++consThreadInfo[i].stats.memoryCount;
				    
                    statsOutput.setLength(0);
                    statsOutput.append("Thread ");
                    statsOutput.append(i);
                    statsOutput.append(" has no data. ");
                    
                    if (consThreadInfo[i].consumerChannel != null && consThreadInfo[i].consumerChannel.selectableChannel() != null)
                    {
                        statsOutput.append("Connected: ");
                        statsOutput.append((consThreadInfo[i].consumerChannel.state() > ChannelState.INITIALIZING) ? true : false);
                        statsOutput.append(" Channel state: ");
                        statsOutput.append(ChannelState.toString(consThreadInfo[i].consumerChannel.state()));
                    }
                    statsOutput.append(String.format(currentUsageFormat, currentProcessCpuLoad, currentMemoryUsage));

                    System.out.println(statsOutput.toString());
				}
			}

			if(startTime == endTime)
			{
				Shutdown(startTime);
				return;
			}
			
			try
			{
				Thread.sleep(1000); // millisecond sleep
			}
			catch (Exception e)
			{
				System.out.println("Thread.sleep Exception: " + e.getLocalizedMessage());
				System.exit(-1);
			}
		}
    }
	
	private void parseItemFile(ConsumerInfo[] consumerInfo)
	{
	    List<Item> items = new ArrayList<Item>();
	    List<Item> latencyItems = new ArrayList<Item>();
	    
	    // note: the same consumer info is applicable to all threads
	    if (consumerInfo != null && consumerInfo[0] != null)
	    {
	        String itemFilename = consumerInfo[0].itemFilename; 
	        
	        // parse the item file
	        ItemFileParser parser = ItemFactory.createItemFileParser();
	        System.out.println(String.format("Parsing item file \"%s\"...", itemFilename));
	        if (parser.parse(itemFilename, items, latencyItems))
	        {
	            System.out.println("...parsing complete.");
	            for (ConsumerInfo info : consumerInfo)
	            {
	                info.items = items;
	                info.latencyItems = latencyItems;
	            }
	        }
	        else
	        {
	            System.out.println(String.format("Error: Failed to parse item file \"%s\"", itemFilename));
	            System.exit(-1);
	        }
	    }
	}
	
	void Shutdown(int runtime)
	{
	    try
	    {
    		int i;
    
    		Error error = TransportFactory.createError();
    		double stdDev;
    
    		System.out.println("\nShutting down.\n-- SUMMARY --\n");
    
    		for(i = 0; i < MAX_CONS_THREADS; i++)
    		{
    			consThreadInfo[i].shutdown = true;
    			
    			if(consThreadInfo[i].channelActive == true)
    			{
    				stdDev = Math.sqrt((double)prevStats[i].latencyRunningSum/(double)prevStats[i].latencyCount);
    
    				StringBuilder sb = new StringBuilder();
    				
    				sb.append("Average CPU=");
    				sb.append(String.format("%.2f%%\n", consThreadInfo[i].stats.cpuAvg));
    				sb.append("Average Latency=");
    				sb.append(String.format("%.3f usec\n", prevStats[i].latencyAvg));
                    sb.append("Average MEM=");
                    sb.append(String.format("%.2fMB\n", consThreadInfo[i].stats.memoryAvg));
    				sb.append("Std Dev Latency=");
    				sb.append(String.format("%.3f", stdDev));
    				sb.append(" usec\n");
    				sb.append("Max Latency=");
    				sb.append(String.format("%.3f", consThreadInfo[i].stats.latencyMax));
    				sb.append(" usec\n");
    				sb.append("Min Latency=");
    				sb.append(String.format("%.3f", consThreadInfo[i].stats.latencyMin));
    				sb.append(" usec\n");
    				sb.append("Image Retrieval Time=");
    				sb.append(String.format("%.3f", (float)_imageRetrievalTime));
    				sb.append(" sec\n");
    				sb.append("Avg Image Rate=");
    				sb.append(String.format("%.3f", (float)_itemCount/(float)_imageRetrievalTime));
    				sb.append(" images/sec\n");
    				
    				System.out.println(sb.toString());
    				
    				consThreadInfo[i].consumerChannel.close(error);
    			}
    		}
	    }
		finally
		{
		    Transport.uninitialize();
		}
	}

	public static void main(String[] args)
	{
		ConsumerPerf consumerperf = new ConsumerPerf();
		consumerperf.run(args);
	}
}
