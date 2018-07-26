package com.thomsonreuters.upa.valueadd.examples.watchlistconsumer;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.shared.CommandLine;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.valueadd.examples.common.ConnectionArg;
import com.thomsonreuters.upa.valueadd.examples.common.ItemArg;

public class WatchlistConsumerConfig
{
    private String publisherId;
    private String publisherAddress;
    private boolean enableEncrypted;
    private boolean enableHttp;
    private boolean cacheOption;
    private int cacheInterval;
    // APIQA:
    // eventCounters keeps track of all information from any -e1/e2/e3 etc arguments
    private ArrayList<EventCounter> eventCounters = new ArrayList<EventCounter>();
    private static final int defaultSingleOpen = 1;
    private static final int defaultAllowSuspect = 1;
    // breakpoint is an indicator of the split in the list of -mp items in which
    // those before the breakpoint are all requested initially,
    // and those after breakpoint are requested after some number of events has
    // occurred (which should be specified by the -e1, -e2, or -e3 arguments)
    private int breakpoint;
    // maxInitialStreamId is the stream Id of the last item reuqested intially
    private int maxInitialStreamId;
    // delayIntialRequest, if specified, will delay all initial requests until after Src Dir Refresh
    private boolean delayInitialRequest;
    // END APIQA

    List<ConnectionArg> connectionList = new ArrayList<ConnectionArg>();

    // default server host name
    private static final String defaultSrvrHostname = "localhost";

    // default server port number
    private static final String defaultSrvrPortNo = "14002";

    // default service name
    private static final String defaultServiceName = "DIRECT_FEED";

    // default item name
    private static final String defaultItemName = "TRI.N";

    private static final int defaultRuntime = 600;

    private int MAX_ITEMS = 128;
    private int ITEMS_MIN_STREAM_ID = 5;

    private ArrayList<ItemInfo> itemList = new ArrayList<ItemInfo>();
    private ArrayList<ItemInfo> providedItemList = new ArrayList<ItemInfo>();

    // APIQA: allMPItems is a list of all mp item arguments, whereas itemList
    // only contains those that are requested intially, and as new items are
    // requested via requestNewItem the item information from allMPItems is used
    // to make a new ItemInfo object which is then added to itemList
    private ArrayList<ItemArg> allMPItems = new ArrayList<ItemArg>();

    // APIQA: EventCounter class created to keep track of -e1/e2/e3 etc
    // arguments; a EventCounter object is created for each -e1/e2/e3 command
    // line argument
    class EventCounter
    {
        // delay is how many instances of the specific event are to pass until
        // specified items are requested
        int delay;

        // eventType specified which type of reques, ex. e2 is eventType 2 which
        // means update messages
        int eventType;
        
        // reissueType and reissueViewId are applicable only to reissue even "-e4", the reissue types are:
         // 1 View
        //  2 Pause
        //  3 Resume
        // (For example the command line argument: -e4 6::1:1,V3 
        // the reissue type is View internally stored as 1, and the reissueViewId is 3.)
        // For event types other than -e4, the value is 0 for reissueType.
       int reissueType;
       int reissueViewId;

        // startIdx and endIdx specify which indexes in list of mp arguments are
        // to be requested
        int startIdx;
        int endIdx;

        public EventCounter(int eventType, int delay, int startIdx, int endIdx, int reissueType, int reissueViewId)
        {
            this.eventType = eventType;
            this.delay = delay;
            this.startIdx = startIdx;
            this.endIdx = endIdx;
            this.reissueType = reissueType;
            this.reissueViewId = reissueViewId;
      }

        public int delay()
        {
            return delay;
        }

        public int startIdx()
        {
            return startIdx;
        }

        public int endIdx()
        {
            return endIdx;
        }

        public int eventType()
        {
            return eventType;
        }
        public int reissueType()
        {
            return reissueType;
        }
        public int reissueViewId()
        {
            return reissueViewId;
        }

        public String printEventCounter()
        {
            return "eventtype " + eventType + "  dalay " + delay + "   sIdx " + startIdx + "    eIdx " + endIdx + "    reissueType " + reissueType;
        }
    }

    // END APIQA

    class ItemInfo
    {
        int domain;
        String name;
        int streamId;
        boolean symbolListData;
        boolean isPrivateStream;
        boolean isBatchStream;
        State state = CodecFactory.createState();

        // APIQA:individual items can be set to be snapshot, private stream,
        // and/or specific view
        boolean isView = false;
        boolean isSnapshot = false;
        boolean isMsgKeyInUpdates = false;
        int viewId = 0;

        // END APIQA

        public int domain()
        {
            return domain;
        }
        public void domain(int domainType)
        {
            this.domain = domainType;
        }
        public String name()
        {
            return name;
        }
        public void name(String name)
        {
            this.name = name;
        }
        public int streamId()
        {
            return streamId;
        }
        public void streamId(int streamId)
        {
            this.streamId = streamId;
        }
        public boolean symbolListData()
        {
            return symbolListData;
        }
        public void symbolListData(boolean symbolListData)
        {
            this.symbolListData = symbolListData;
        }
        public boolean isPrivateStream()
        {
            return isPrivateStream;
        }
        public void privateStream(boolean isPrivateStream)
        {
            this.isPrivateStream = isPrivateStream;
        }

        // APIQA: adding isView isSnapshot and viewId
        public boolean isView()
        {
            return isView;
        }

        public void viewStream(boolean isView)
        {
            // System.out.println
            // ("-------APIQA: In WatchlistConsumerConfig.java:  Setting is view to"
            // + isView);
            this.isView = isView;
        }

        public boolean isSnapshot()
        {
            return isSnapshot;
        }

        public void snapshotStream(boolean isSnapshot)
        {
            // System.out.println
            // ("-------APIQA: In WatchlistConsumerConfig.java:  Setting is Snapshot to "
            // + isSnapshot);
            this.isSnapshot = isSnapshot;
        }
        public boolean isMsgKeyInUpdates()
        {
            return isMsgKeyInUpdates;
        }

        public void msgKeyInUpdatesStream(boolean isMsgKeyInUpdates)
        {
            // System.out.println
            // ("-------APIQA: In WatchlistConsumerConfig.java:  Setting is MsgKeyInUpdates to "
            // + isMsgKeyInUpdates);
            this.isMsgKeyInUpdates = isMsgKeyInUpdates;
        }

        public int viewId()
        {
            return viewId;
        }

        public void viewId(int viewId)
        {
            // System.out.println
            // ("-------APIQA: In WatchlistConsumerConfig.java:  Setting view id to: "
            // + viewId);
            this.viewId = viewId;
        }
 
        // END APIQA:

        public boolean isBatchStream()
        {
            return isBatchStream;
        }
        public void batchStream(boolean isBatchStream)
        {
            this.isBatchStream = isBatchStream;
        }
        public State state()
        {
            return state;
        }
        public void state(State state)
        {
            this.state = state;
        }
    }

    public boolean init(String[] args)
    {
        boolean ret;

		
        if ((ret = parseArgs(args)) == false)
        {
            return ret;
        }

        List<ConnectionArg> connectionList = connectionList();
        ConnectionArg conn = connectionList.get(0);
        for (ItemArg itemArg : conn.itemList())
        {
            // APIQA: interface of addItem changed
            addItem(itemArg.itemName(), itemArg.domain(), itemArg.symbolListData(), itemArg.enablePrivateStream(), itemArg.enableView(), itemArg.viewId(), itemArg.enableSnapshot(), itemArg.enableMsgKeyInUpdates());
        }
        return true;
    }

    public ItemInfo createItemInfo()
    {
        return new ItemInfo();
    }

    // APIQA: interface of addItem changed
    public void addItem(String itemName, int domainType, boolean isSymbolList, boolean isPrivate, boolean isView, int viewId, boolean isSnapshot, boolean isMsgKeyInUpdates)
    {
        ItemInfo itemInfo = new ItemInfo();
        itemInfo.domain(domainType);
        itemInfo.name(itemName);
        itemInfo.symbolListData(isSymbolList);
        itemInfo.privateStream(isPrivate);
        // APIQA:
        itemInfo.viewStream(isView);
        itemInfo.snapshotStream(isSnapshot);
        itemInfo.msgKeyInUpdatesStream(isMsgKeyInUpdates);
        itemInfo.viewId(viewId);
        // END APIQA
        itemInfo.streamId(ITEMS_MIN_STREAM_ID + itemList.size());
        System.out.println("------------APIQA: " + itemInfo.name() + ": StreamID = " + itemInfo.streamId() + ", PrivateFlag = " + itemInfo.isPrivateStream() + ", SnapShotFlag= "
                + itemInfo.isSnapshot() + ", ViewFlag= " + itemInfo.isView() + ", ViewId= " + itemInfo.viewId()+ ", MsgKeyInUpdatesFlag= " + itemInfo.isMsgKeyInUpdates());

        itemList.add(itemInfo);

        if (itemList.size() >= MAX_ITEMS)
        {
            System.out.println("Config Error: Example only supports up to %d items " + MAX_ITEMS);
            System.exit(-1);
        }
    }

    public ItemInfo getItemInfo(int streamId)
    {
        if (streamId > 0)
        {
            if (streamId >= ITEMS_MIN_STREAM_ID && streamId < itemList.size() + ITEMS_MIN_STREAM_ID)
                return itemList.get(streamId - ITEMS_MIN_STREAM_ID);
            else
                return null;
        }
        else if (streamId < 0)
        {
            for (int i = 0; i < providedItemList.size(); ++i)
            {
                if (providedItemList.get(i).streamId() == streamId)
                    return providedItemList.get(i);
            }
            return null;
        }
        else
            return null;
    }

    public ItemInfo addProvidedItemInfo(int streamId, MsgKey msgKey, int domainType)
    {
        ItemInfo item = null;

        /* Check if item is already present. */
        for (int i = 0; i < providedItemList.size(); ++i)
        {
            if (providedItemList.get(i).streamId == streamId)
            {
                item = providedItemList.get(i);
                break;
            }
        }

        /* Add new item. */
        if (item == null)
        {
            if (providedItemList.size() == MAX_ITEMS)
            {
                System.out.println("Too many provided items.\n");
                return null;
            }
            item = new ItemInfo();
            providedItemList.add(item);
        }

        if ((msgKey.flags() & MsgKeyFlags.HAS_NAME) > 0)
        {
            item.name = msgKey.name().toString();
        }
        else
        {
            item.name = null;
        }

        item.streamId(streamId);
        item.domain(domainType);

        return item;
    }

    public void removeProvidedItemInfo(ItemInfo item)
    {
        int i = 0;
        boolean found = false;
        for (i = 0; i < providedItemList.size(); ++i)
        {
            if (providedItemList.get(i).streamId == item.streamId)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            providedItemList.remove(i);
        }
        return;
    }

    public boolean parseArgs(String[] args)
    {
        for (int i = 0; i < args.length; i++)
        {
            if (args[i].contains("runtime"))
            {
                args[i] = args[i].replace("runtime", "runTime");
            }
            if (args[i].contains("uname"))
            {
                args[i] = args[i].replace("uname", "u");
            }
            if (args[i].contains("connectionType"))
            {
                args[i] = args[i].replace("connectionType", "c");
            }
        }

        addCommandLineArgs();
        try
        {
            CommandLine.parseArgs(args);

            ConnectionArg connectionArg = new ConnectionArg();

            String connectionType = CommandLine.value("c");
            if (connectionType.equals("socket"))
            {
                connectionArg.connectionType(ConnectionTypes.SOCKET);
            }
            else if (connectionType.equals("encrypted"))
            {
                connectionArg.connectionType(ConnectionTypes.ENCRYPTED);
                enableEncrypted = true;
            }
            else if (connectionType.equals("http"))
            {
                connectionArg.connectionType(ConnectionTypes.HTTP);
                enableHttp = true;
            }
            connectionArg.service(serviceName());

            connectionArg.hostname(CommandLine.value("h"));
            connectionArg.port(CommandLine.value("p"));
            connectionArg.interfaceName(CommandLine.value("if"));

            if (CommandLine.hasArg("tsServiceName"))
            {
    			connectionArg.tsService(CommandLine.value("tsServiceName"));
            }

            if (CommandLine.hasArg("tunnel"))
            {
                connectionArg.tunnel(CommandLine.booleanValue("tunnel"));
            }

            if (CommandLine.hasArg("tsAuth"))
                connectionArg.tunnelAuth(CommandLine.booleanValue("tsAuth"));
            if (CommandLine.hasArg("tsDomain"))
                connectionArg.tunnelDomain(CommandLine.intValue("tsDomain"));

            List<ItemArg> itemList = new ArrayList<ItemArg>();

            // APIQA
            if (CommandLine.hasArg("delayInitialRequest"))
            {
                delayInitialRequest = true;
            }

            List<String> itemNames = CommandLine.values("mp");
            if (itemNames != null)
            {
                int numMPItems = itemNames.size();

                // APIQA:an EventCounter object is made to represent every
                // 'event' (e1, e2, e3, e4, e5) argument
                itemNames = CommandLine.values("e1");
                if (itemNames != null && itemNames.size() > 0)
                {
                    for (String item : itemNames)
                    {
                        addEventCounter("e1", item, numMPItems);
                    }
                }

                itemNames = CommandLine.values("e2");
                if (itemNames != null && itemNames.size() > 0)
                {
                    for (String item : itemNames)
                    {
                        addEventCounter("e2", item, numMPItems);
                    }
                }

                itemNames = CommandLine.values("e3");
                if (itemNames != null && itemNames.size() > 0)
                {
                    for (String item : itemNames)
                    {
                        addEventCounter("e3", item, numMPItems);
                    }
                }
                itemNames = CommandLine.values("e4");
                if (itemNames != null && itemNames.size() > 0)
                {
                    for (String item : itemNames)
                    {
                        addEventCounter("e4", item, numMPItems);
                    }
                }
                itemNames = CommandLine.values("e5");
                if (itemNames != null && itemNames.size() > 0)
                {
                    for (String item : itemNames)
                    {
                        addEventCounter("e5", item, numMPItems);
                    }
                }
                itemNames = CommandLine.values("mp");

                // breakpoint is an index in the list of mp arguments sepcified
                // all items before the breakpoint will be requested intially,
                // and ones after are assumed to be requested after some numebr
                // of events specified by an -e1/e2/e3 argument
                int bPoint = determineBreakpoint(itemNames);

                /*APIQA: all market price items are added to itemNames
                *handleMarketPriceArgs handles adding the correct items (those that will be requested intitially) to itemList
                *and puts all mp items in a different List called allMPItems, which is indexed in the same order that the 
                *items appeared in command line arguments
                */
                handleMarketPriceArgs(itemNames, bPoint, itemList);
            }

            // END APIQA

            itemNames = CommandLine.values("mbo");
            if (itemNames != null && itemNames.size() > 0)
                parseItems(itemNames, DomainTypes.MARKET_BY_ORDER, false, false, false, false, 0, false, itemList);

            itemNames = CommandLine.values("mbp");
            if (itemNames != null && itemNames.size() > 0)
                parseItems(itemNames, DomainTypes.MARKET_BY_PRICE, false, false, false, false, 0, false, itemList);

            itemNames = CommandLine.values("yc");
            if (itemNames != null && itemNames.size() > 0)
                parseItems(itemNames, DomainTypes.YIELD_CURVE, false, false, false, false, 0, false, itemList);

            itemNames = CommandLine.values("sl");
            if (itemNames != null && itemNames.size() > 0)
                parseItems(itemNames, DomainTypes.SYMBOL_LIST, false, false, false, false, 0, false, itemList);

            itemNames = CommandLine.values("sld");
            if (itemNames != null && itemNames.size() > 0)
                parseItems(itemNames, DomainTypes.SYMBOL_LIST, false, true, false, false, 0, false, itemList);

            if (itemList.size() == 0 && !CommandLine.hasArg("tunnel"))
            {
                ItemArg itemArg = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName, false);
                itemList.add(itemArg);
            }

            if (tsServiceName() == null || tsServiceName().equals(""))
                connectionArg.tsService(connectionArg.service());

            connectionArg.itemList(itemList);
            connectionList.add(connectionArg);

            maxInitialStreamId = ITEMS_MIN_STREAM_ID + itemList.size();

            String value = CommandLine.value("publisherInfo");
            if (value != null)
            {
                String[] pieces = value.split(",");

                if (pieces.length > 1)
                {
                    publisherId = pieces[0];

                    try
                    {
                        Integer.parseInt(publisherId);
                    }
                    catch (Exception e)
                    {
                        System.err.println("Error loading command line arguments:\t");
                        System.out.println("publisherId within publisherinfo should be an integer number");
                        System.out.println("Consumer exits...");
                        System.exit(CodecReturnCodes.FAILURE);
                    }
                    publisherAddress = pieces[1];
                }
            }
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.out.println("Consumer exits...");
            System.exit(CodecReturnCodes.FAILURE);
        }

        return true;
    }

    String serviceName()
    {
        return CommandLine.value("s");
    }

    String tsServiceName()
    {
        return CommandLine.value("tsServiceName");
    }

    List<ConnectionArg> connectionList()
    {
        return connectionList;
    }

    String userName()
    {
        return CommandLine.value("u");
    }

    boolean enableView()
    {
        return CommandLine.booleanValue("view");
    }

    boolean enablePost()
    {
        return CommandLine.booleanValue("post");
    }

    boolean enableOffpost()
    {
        return CommandLine.booleanValue("offpost");
    }
    
	// APIQA
	
	boolean enablePostMultipart()
	{
		return CommandLine.booleanValue("postmultipart");
	}	
	
	boolean testOnlyLoginClose()
	{
		return CommandLine.booleanValue("testOnlyLoginClose");
	}
	boolean reqItemBeforeLogin()
	{
		return CommandLine.booleanValue("reqItemBeforeLogin");
	}
	boolean loginCloseAfterLoginStatus()
	{
		return CommandLine.booleanValue("loginCloseAfterLoginStatus");
	}
	boolean loginPauseAndResume()
	{
		return CommandLine.booleanValue("loginPauseAndResume");
	}
	boolean reissueDirEvery5Updates()
	{
		return CommandLine.booleanValue("reissueDirEvery5Updates");
	}
	int reissueDirWithSID()
	{
		return CommandLine.intValue("reissueDirWithSID");
	}
	int reqDirWithSID()
	{
		return CommandLine.intValue("reqDirWithSID");
	}
	boolean hasReqQos()
	{
		return CommandLine.booleanValue("hasReqQos");
	}
	boolean qosDynamic()
	{
		return CommandLine.booleanValue("qosDynamic");
	}
	int qosRate()
	{
		return CommandLine.intValue("qosRate");
	}
	int qosRateInfo()
	{
		return CommandLine.intValue("qosRateInfo");
	}
	int qosTimeliness()
	{
		return CommandLine.intValue("qosTimeliness");
	}
	int qosTimeInfo()
	{
		return CommandLine.intValue("qosTimeInfo");
	}
	int worstQosRate()
	{
		return CommandLine.intValue("worstQosRate");
	}
	int worstQosRateInfo()
	{
		return CommandLine.intValue("worstQosRateInfo");
	}
	int worstQosTimeliness()
	{
		return CommandLine.intValue("worstQosTimeliness");
	}
	int worstQosTimeInfo()
	{
		return CommandLine.intValue("worstQosTimeInfo");
	}
	// End APIQA
	

    String publisherId()
    {
        return publisherId;
    }

    String publisherAddress()
    {
        return publisherAddress;
    }

    boolean enableSnapshot()
    {
        return CommandLine.booleanValue("snapshot");
    }

    int runtime()
    {
        return CommandLine.intValue("runTime");
    }

    boolean enableXmlTracing()
    {
        return CommandLine.booleanValue("x");
    }

    boolean enableEncrypted()
    {
        return enableEncrypted;
    }

    boolean enableHttp()
    {
        return enableHttp;
    }

    boolean enableProxy()
    {
        return CommandLine.booleanValue("proxy");
    }

    String proxyHostname()
    {
        return CommandLine.value("ph");
    }

    String proxyPort()
    {
        return CommandLine.value("pp");
    }

    String proxyUsername()
    {
        return CommandLine.value("plogin");
    }

    String proxyPassword()
    {
        return CommandLine.value("ppasswd");
    }

    String proxyDomain()
    {
        return CommandLine.value("pdomain");
    }

    String krbFile()
    {
        return CommandLine.value("krbfile");
    }

    String keyStoreFile()
    {
        return CommandLine.value("keyfile");
    }

    String keystorePassword()
    {
        return CommandLine.value("keypasswd");
    }

    boolean cacheOption()
    {
        return cacheOption;
    }

    int cacheInterval()
    {
        return cacheInterval;
    }

    String authenticationToken()
    {
        return CommandLine.value("at");
    }

    String authenticationExtended()
    {
        return CommandLine.value("ax");
    }

    String applicationId()
    {
        return CommandLine.value("aid");
    }

	int itemCount()
	{
		return itemList.size();
	}
	
	public List<ItemInfo> itemList()
	{
		return itemList;
	}
	
	//APIQA:
	public boolean delayInitialRequest()
	{
		return delayInitialRequest;
	}
	public ArrayList<EventCounter> eventCounters()
	{
		return eventCounters;
	}

	int isSingleOpen()
	{
		return CommandLine.intValue("singleOpen");
	}

	int allowSuspectData()
	{
		return CommandLine.intValue("allowSuspect");
	}

	int numEventCounters()
	{
		return eventCounters.size();
	}

	//addEventCounter parses -e1/e2/e3 argument eg. -e2 5::2:6
	//the '5' in the above example is the amount of occurance of event 1 you want to happen until requesting specified items
	//'2' is the index of the first mp item in list of mp items to request once 5 event1's have occured
	//'6' is the index of the last mp item in list of mp items to request once 5 event 1's have occured
	//doing -e1 7::2 (not specifying an end index) will result in only item indexed 2 to be requested, not a range
	// NOTE: On -e4 event 4, Please read the following on how the event 4 is used:
	// Example arguments: -h 48.22.4.1 -p 14002 -s ELEKTRON_DD  -mp GOOG.O -mp TRI.N  -e2 5::1 -e4 10::1:1,P -x
	//  Notes: Make sure that -e2 event is prior to using -e4 because the way the counter works is unless an -e2
	//         event opens up a new item the re-issue will not work.
	// In the above example "-e4 10::1:1,P" would mean after -e2 opened the item "TRI.N" the "-e4 10::1:1,P" means:
	//      -e4 -Reissue event
	//      10::1:1,P send reissue after 10 updates on item indexed 1 which is 'TRI.N' in the above example. (because end index is also 1 '1:1'.
	//      ',' after comma the reissue action V3 -View change V3, P pause, R resume. In the above example reissue action is P i.e pause
	private void addEventCounter(String varName, String value, int numMPItems)
	{
		int eType=9;
		String eTypeString=varName.split("e")[1];
		try{
			eType=Integer.parseInt(eTypeString);
		}
		catch(Exception e)
		{
			System.err.println("Error loading command line arguments:\t");
			System.out.println("Invalid event input");
			System.exit(CodecReturnCodes.FAILURE);
		}

		int delayNum=0;
		String[] partsOfValue= value.split("::");
		try
		{
			delayNum=Integer.parseInt(partsOfValue[0]);
		}
		catch(Exception e)
		{
			System.err.println("Error loading command line arguments:\t");
			System.out.println("Invalid delay input");
			System.exit(CodecReturnCodes.FAILURE);
		}
		
		int sIdx=0;int eIdx=numMPItems;
		String reissueTypeStr = "";
		int reissueType = 0;
		int reissueViewId = 0;

		if(partsOfValue[1].contains(":"))
		{
			try
			{
				sIdx=Integer.parseInt(partsOfValue[1].split(":")[0]);
			}
			catch(Exception e)
			{
				System.err.println("Error loading command line arguments:\t");
				System.out.println("Invalid startIdx input");
				System.exit(CodecReturnCodes.FAILURE);
			}
			try
			{
				if(partsOfValue[1].contains(",") == false)
					eIdx=(Integer.parseInt(partsOfValue[1].split(":")[1]) )+1;
				else
				{
					eIdx=(Integer.parseInt(partsOfValue[1].split(":")[1].split(",")[0]));
				}
			}
			catch(Exception e)
			{
				System.err.println("Error loading command line arguments:\t");
				System.out.println("Invalid endIdx input");
				System.exit(CodecReturnCodes.FAILURE);
			}
			if(eIdx>numMPItems)
			{
				System.out.println("Invalid end index to event range entered. Default will set to last item.");
				eIdx=numMPItems;
			}
		}
		else
		{
			try
			{
				sIdx=Integer.parseInt(partsOfValue[1]);
			}
			catch(Exception e)
			{
				System.err.println("Error loading command line arguments:\t");
				System.out.println("Invalid startIdx input");
				System.exit(CodecReturnCodes.FAILURE);
			}
			eIdx=sIdx+1;
		}
		if(partsOfValue[1].contains(","))
		{
			if(sIdx == eIdx)
				eIdx =sIdx+1;
			try
			{
				reissueTypeStr=partsOfValue[1].split(",")[1];
				if(reissueTypeStr.contains("V"))
				{
					reissueType = 1; // View
					reissueViewId = Integer.parseInt(reissueTypeStr.split("V")[1]);
				}
				else if(reissueTypeStr.contains("P"))
				{
					reissueType = 2; // Pause
				}
				else if(reissueTypeStr.contains("R"))
				{
					reissueType = 3; // Resume
				}
			}
			catch(Exception e)
			{
				System.err.println("Error loading command line arguments:\t");
				System.out.println("Invalid startIdx input");
				System.exit(CodecReturnCodes.FAILURE);
			}
		}

		
		
		EventCounter e = new EventCounter(eType, delayNum, sIdx, eIdx, reissueType, reissueViewId);
		eventCounters.add(e);
	}

	public ArrayList<EventCounter> getEventCounters(int eventType)
	{
		ArrayList<EventCounter> eCounters = new ArrayList<EventCounter>();
		for(EventCounter eCounter : eventCounters)
		{
			if(eCounter.eventType() == eventType)
			{
				eCounters.add(eCounter);
			}
		}
		return eCounters;
	}
	public ArrayList<EventCounter> getEventCounters()
	{
		return eventCounters;
	}

	/*APIQA:Looks through all 'event' (e1, e2, e3) arguments and determines the breakpoint.
	*Brekapoint represent the the first index of market price items that should not be initially requested
	*ASSUMES that all market price items after that one are going to be requested after a delay as well
	*Therefore, all market price items that are to be requested intitally have to be listed before all delayed items
	*/
	private int determineBreakpoint(List<String> itemNames)
	{
		int bpoint=itemNames.size();
		if(eventCounters.size()>0)
		{
			for(int eventCounterIndex=0; eventCounterIndex < eventCounters.size() ; ++eventCounterIndex)
			{
				if(eventCounters.get(eventCounterIndex).startIdx() < bpoint)
				{
					bpoint= eventCounters.get(eventCounterIndex).startIdx();
				}
			}
		}
		breakpoint=bpoint;
		return bpoint;
	}

	//APIQA:returns current breakpoint, can be used externally
	public int breakpoint()
	{
		return breakpoint;
	}

	//APIQA:returns last streamId used initially
	public int lastInitialStreamId()
	{
		return maxInitialStreamId;
	}

	//APIQA:returns requested attribute about the ItemArg at index i in allMPItems
	public boolean getMPItemInfo(int i, String attrib)
	{
		if(i >= allMPItems.size())
		{
			System.out.println("Invalid index input for getMPItem, default returning false");
			return false;
		}
		if(attrib.equals("isSnapshot"))
		{
			return allMPItems.get(i).enableSnapshot();
		}
		else if(attrib.equals("isView"))
		{
			return allMPItems.get(i).enableView();
		}
		else if(attrib.equals("isPrivate"))
		{
			return allMPItems.get(i).enablePrivateStream();
		}
		if(attrib.equals("isMsgKeyInUpdates"))
		{
			return allMPItems.get(i).enableMsgKeyInUpdates();
		}
		System.out.println("Invalid attribute requested. Default returning False");
		return false;
	}

	public int getViewId(int i)
	{
		return allMPItems.get(i).viewId();
	}
	
	public String getNameOfItem(int i)
	{
		return allMPItems.get(i).itemName();
	}

	//APIQA:itemNames contains all the values of -mp inputs (example  TRI:PSV2 which is requesting a private stream snapshot view 2 for TRI)
	//all the items get added to allMPItems but only those that will be requested initially (AKA the items are are before the 'breakpoint') will be added to itemList
	//the items are aren't added to itemList will be added when they are first requested via the requestNewItem method in WatchlistConsumer.java
	void handleMarketPriceArgs(List<String> itemNames, int breakpoint, List<ItemArg> list)
	{
		for(int index=0; index < itemNames.size(); ++index)
		{
			String itemName= itemNames.get(index);
			boolean isPrivateStream = false;
			boolean isView = false;
			boolean isSnapshot = false;
			boolean isMsgKeyInUpdates = false;
			int viewId= 0;
			String name = itemName;
			//APIQA: parse for S, P, V, K
			if(itemName.contains(":"))
			{
				String[] splitName = itemName.split(":");
				name=splitName[0];
				if(splitName[1].contains("P"))
					isPrivateStream = true;
				if(splitName[1].contains("S"))
					isSnapshot = true;
				if(splitName[1].contains("K"))
					isMsgKeyInUpdates = true;
				if(splitName[1].contains("V"))
				{
					isView = true;
					try
					{
						viewId=Integer.parseInt(splitName[1].split("V")[1]);
					}
					catch(Exception e)
					{
						System.err.println("Error loading command line arguments:\t");
						System.out.println("Invalid view argument");
						System.exit(CodecReturnCodes.FAILURE);
					}
				}
			}
			ItemArg itemArg = new ItemArg();
			itemArg.domain( DomainTypes.MARKET_PRICE );
			itemArg.enablePrivateStream(isPrivateStream);
			itemArg.enableSnapshot(isSnapshot);
			itemArg.enableMsgKeyInUpdates(isMsgKeyInUpdates);
			itemArg.enableView(isView);
			itemArg.viewId(viewId);
			itemArg.itemName(name);
			allMPItems.add(itemArg);
			if(index < breakpoint)
			{
				list.add(itemArg);
			}
		}
	}

//END APIQA

// APIQA: Added more parameters
    private void parseItems(List<String> itemNames, int domain, boolean isPrivateStream, boolean isSymbollistData, boolean isView, boolean isSnapshot, int viewId, boolean isMsgKeyInUpdates, List<ItemArg> itemList)
    {
        for (String itemName : itemNames)
        {
            ItemArg itemArg = new ItemArg();
            itemArg.domain(domain);

            if (isPrivateStream)
                itemArg.enablePrivateStream(true);
            else
                itemArg.enablePrivateStream(false);

            // APIQA:
            itemArg.enablePrivateStream(isPrivateStream);
            itemArg.enableSnapshot(isSnapshot);
            itemArg.enableView(isView);
            itemArg.viewId(viewId);
            itemArg.enableMsgKeyInUpdates(isMsgKeyInUpdates);
            // END APIQA

            if (isSymbollistData)
                itemArg.symbolListData(true);

            itemArg.itemName(itemName);
            itemList.add(itemArg);

        }
    }

    void addCommandLineArgs()
    {
        CommandLine.programName("WatchlistConsumer");
        CommandLine.addOption("mp", "For each occurrence, requests item using Market Price domain.");
        CommandLine.addOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
        CommandLine.addOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");
        CommandLine.addOption("yc", "For each occurrence, requests item using Yield Curve domain. Default is no yield curve requests.");
        CommandLine.addOption("view", "Specifies each request using a basic dynamic view. Default is false.");
        CommandLine.addOption("post", "Specifies that the application should attempt to send post messages on the first requested Market Price item (i.e., on-stream). Default is false.");
        CommandLine.addOption("offpost", "Specifies that the application should attempt to send post messages on the login stream (i.e., off-stream).");
        //APIQA
        CommandLine.addOption("postmultipart", "Specifies that the application should attempt to send post multi part messages, default is false.");
        //END APIQA
        CommandLine.addOption("publisherInfo", "Specifies that the application should add user provided publisher Id and publisher ipaddress when posting");
        CommandLine.addOption("snapshot", "Specifies each request using non-streaming. Default is false(i.e. streaming requests.)");
        CommandLine.addOption("sl", "Requests symbol list using Symbol List domain. (symbol list name optional). Default is no symbol list requests.");
        CommandLine.addOption("sld", "Requests item on the Symbol List domain and data streams for items on that list.  Default is no symbol list requests.");
        CommandLine.addOption("h", defaultSrvrHostname, "Server host name");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number");
        CommandLine.addOption("if", (String)null, "Interface name");
        CommandLine.addOption("s", defaultServiceName, "Service name");
        CommandLine.addOption("u", "Login user name. Default is system user name.");
        CommandLine.addOption("c", "Socket", "Specifies the connection type that the connection should use. Possible values are: 'Socket', 'http', 'encrypted'");

        // APIQA
        CommandLine.addOption("singleOpen", defaultSingleOpen, "SingleOpen default set to 1, can be specified to 0 or 1");
        CommandLine.addOption("allowSuspect", defaultAllowSuspect, "Allow suspect data default set to 1, can be set to 0 or 1");
        CommandLine.addOption("testOnlyLoginClose", "Only close login stream when runtime expires");
        CommandLine.addOption("reqItemBeforeLogin", "Test request item befor login stream establishes");
        CommandLine.addOption("loginCloseAfterLoginStatus", "Send login close message from rdmLoginMsgCallback after getting a login status message");
        CommandLine.addOption("loginPauseAndResume", "Send login pause after 3 item updates and login resume after 3 src dir updates");
        CommandLine.addOption("reissueDirEvery5Updates", "Send dorectory reissue every 5 updates received upto 30 updates.");
        CommandLine.addOption("reissueDirWithSID", "Send dorectory reissue with specify serviceId.");
        CommandLine.addOption("reqDirWithSID", "Send Directory request at the initial with specify serviceId.");
        CommandLine.addOption("hasReqQos", "Enable hasQos on the request.");
        CommandLine.addOption("qosDynamic", "dynamic or static qos.");
        CommandLine.addOption("qosRate", "qosRate.");
        CommandLine.addOption("qosTimeliness", "qosTimeliness.");
        CommandLine.addOption("qosTimeInfo", "qosTimeInfo.");
        CommandLine.addOption("qosRateInfo", "qosRateInfo.");
        CommandLine.addOption("worstQosRate", "worstQosRate.");
        CommandLine.addOption("worstQosTimeliness", "worstQosTimeliness.");
        CommandLine.addOption("worstQosTimeInfo", "worstQosTimeInfo.");
        CommandLine.addOption("worstQosRateInfo", "worstQosRateInfo.");
        // END APIQA

        CommandLine.addOption("runTime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("x", "Provides XML tracing of messages.");

        CommandLine.addOption("proxy", "Specifies that the application will make a tunneling connection (http or encrypted) through a proxy server, default is false");
        CommandLine.addOption("ph", "", "Proxy server host name");
        CommandLine.addOption("pp", "", "Proxy port number");
        CommandLine.addOption("plogin", "", "User Name on proxy server");
        CommandLine.addOption("ppasswd", "", "Password on proxy server");
        CommandLine.addOption("pdomain", "", "Proxy server domain");
        CommandLine.addOption("krbfile", "", "KRB File location and name");
        CommandLine.addOption("keyfile", "", "Keystore file location and name");
        CommandLine.addOption("keypasswd", "", "Keystore password");

        CommandLine.addOption("tunnel", "", "(optional) enables consumer to open tunnel stream and send basic text messages");
        CommandLine.addOption("tsServiceName", "", "(optional) specifies the service name for tunnel stream messages (if not specified, the service name specified in -c/-tcp is used");
        CommandLine.addOption("tsAuth", "", "(optional) causes consumer to request authentication when opening a tunnel stream. This applies to basic tunnel streams");
        CommandLine.addOption("tsDomain", "", "(optional) specifes the domain a consumer uses when opening a tunnel stream. This applies to basic tunnel streams");

        CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
        CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
        CommandLine.addOption("aid", "", "Specifies the Application ID.");

        // APIQA
        CommandLine.addOption("e1", "Request new view item(s) after specified number of Src Directory UPDATES");
        CommandLine.addOption("e2", "Request new view item(s) after specified number of defaultMsg UPDATES");
        CommandLine.addOption("e3", "Request new view item(s) after specified number of Channel_down_reconnecting events");
        CommandLine.addOption("e4", "Reissue on item(s) after item is established and streaming");
        CommandLine.addOption("e5", "Request new view item(s) after specified number of defaultMsg REFRESHES");
        CommandLine.addOption("delayInitialRequest", "Delay sending initial request for items until after source directory refresh");
        // END APIQA
    }
}

