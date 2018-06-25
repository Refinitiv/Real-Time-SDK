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
		
	class ItemInfo
	{
		int	domain;		
		String name;		
		int	streamId;		
		boolean	symbolListData;	
		boolean isPrivateStream;
		boolean isBatchStream;
        State state = CodecFactory.createState();
									
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
	
	public boolean init(String[]args)
	{
		boolean ret;
	       	
		
		if ((ret = parseArgs(args)) == false )
		{
			return ret;	        	
		}
				
		List<ConnectionArg> connectionList = connectionList();
		ConnectionArg conn = connectionList.get(0);
		for ( ItemArg itemArg : conn.itemList())
		{		
			addItem(itemArg.itemName(), itemArg.domain(), itemArg.symbolListData(), itemArg.enablePrivateStream());
		}		
		return true;
	}
	
	// APIQA: adding new item
	public ItemInfo createItemInfo() 
	{ 
		return new ItemInfo(); 
	}
	
	public void addItem(String itemName, int domainType, boolean isSymbolList, boolean isPrivate)
	{
		ItemInfo itemInfo = new ItemInfo();
		itemInfo.domain(domainType);
		itemInfo.name(itemName);
		itemInfo.symbolListData(isSymbolList);
		itemInfo.privateStream(isPrivate);
		itemInfo.streamId(ITEMS_MIN_STREAM_ID + itemList.size());		
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
			for(int i = 0; i < providedItemList.size(); ++i)
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
		for(int i = 0; i < providedItemList.size(); ++i)
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
	
		if ((msgKey.flags() & MsgKeyFlags.HAS_NAME)> 0)
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
		for(i = 0; i < providedItemList.size(); ++i)
		{
			if (providedItemList.get(i).streamId == item.streamId)
			{
				found = true;
				break;
			}
		}
		if ( found ) 
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
    			connectionArg.qService(CommandLine.value("tsServiceName"));
    		}
    		            
    		if (CommandLine.hasArg("qSourceName"))
    		{
    			connectionArg.qSource(CommandLine.value("qSourceName"));
    		}
    		if (CommandLine.hasArg("qDestName"))
    			connectionArg.qDestList(CommandLine.values("qDestName"));
    		if (CommandLine.hasArg("tunnel"))
    		{
    			connectionArg.tunnel(CommandLine.booleanValue("tunnel"));
    		}
    		
    		if (CommandLine.hasArg("tunnelAuth"))
    			connectionArg.tunnelAuth(CommandLine.booleanValue("tunnelAuth"));
    		if (CommandLine.hasArg("tunnelDomain"))
    			connectionArg.tunnelDomain(CommandLine.intValue("tunnelDomain"));
    		if (CommandLine.hasArg("tsAuth"))
    			connectionArg.tunnelAuth(CommandLine.booleanValue("tsAuth"));
    		if (CommandLine.hasArg("tsDomain"))
    			connectionArg.tunnelDomain(CommandLine.intValue("tsDomain"));
  		   		    		       
    		List<ItemArg> itemList = new ArrayList<ItemArg>();
    		
    		List<String> itemNames = CommandLine.values("mp");    		
    		if ( itemNames != null && itemNames.size() > 0 )
    			parseItems(itemNames, DomainTypes.MARKET_PRICE, false, false, itemList);
    		
       		itemNames = CommandLine.values("mbo");
       		if ( itemNames != null && itemNames.size() > 0 )
       			parseItems(itemNames, DomainTypes.MARKET_BY_ORDER, false, false, itemList);
    		
       		itemNames = CommandLine.values("mbp");
       		if ( itemNames != null && itemNames.size() > 0 )
       			parseItems(itemNames, DomainTypes.MARKET_BY_PRICE, false, false, itemList);
    		
       		itemNames = CommandLine.values("yc");
       		if ( itemNames != null && itemNames.size() > 0 )
       			parseItems(itemNames, DomainTypes.YIELD_CURVE, false, false, itemList);
    		
       		itemNames = CommandLine.values("sl");
       		if ( itemNames != null && itemNames.size() > 0 )
       			parseItems(itemNames, DomainTypes.SYMBOL_LIST, false, false, itemList);
    		
       		itemNames = CommandLine.values("sld");
       		if ( itemNames != null && itemNames.size() > 0 )
       			parseItems(itemNames, DomainTypes.SYMBOL_LIST, false, true, itemList);   

            if (itemList.size() == 0 && !CommandLine.hasArg("qSourceName") && !CommandLine.hasArg("tunnel"))
            {
            	ItemArg itemArg = new ItemArg(DomainTypes.MARKET_PRICE, defaultItemName, false);
            	itemList.add(itemArg);            	
            }
            
            if ( tsServiceName() == null || tsServiceName().equals(""))
                connectionArg.qService(connectionArg.service());
       		
            connectionArg.itemList(itemList);
       		connectionList.add(connectionArg);           
       		
    		String value = CommandLine.value("publisherInfo");
    		if (value!= null) 
    		{
    			String [] pieces = value.split(",");
        		
    			if( pieces.length > 1 )
    			{
    				publisherId = pieces[0];
    				
    				try
    				{
    					Integer.parseInt(publisherId);
    				}
    				catch(Exception e)
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
		
	private void parseItems(List<String> itemNames, int domain, boolean isPrivateStream, boolean isSymbollistData, List<ItemArg> itemList)
	{
		for ( String itemName : itemNames)
		{    
			ItemArg itemArg = new ItemArg();
			itemArg.domain(domain);

            if (isPrivateStream) 
            	itemArg.enablePrivateStream(true);
            else 
            	itemArg.enablePrivateStream(false);
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
        
        CommandLine.addOption("qSourceName", "",  "(optional) specifies the source name for queue messages (if specified, configures consumer to receive queue messages");
        CommandLine.addOption("qDestName", "", "(optional) specifies the destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified");
        CommandLine.addOption("tunnel", "", "(optional) enables consumer to open tunnel stream and send basic text messages");
        CommandLine.addOption("tsServiceName", "", "(optional) specifies the service name for queue messages (if not specified, the service name specified in -c/-tcp is used");
        CommandLine.addOption("tsAuth", "", "(optional) causes consumer to request authentication when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging");
        CommandLine.addOption("tsDomain", "", "(optional) specifes the domain a consumer uses when opening a tunnel stream. This applies to both basic tunnel streams and those for queue messaging");
        
        CommandLine.addOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
        CommandLine.addOption("ax", "", "Specifies the Authentication Extended information.");
        CommandLine.addOption("aid", "", "Specifies the Application ID.");
    }	
}

