package com.thomsonreuters.upa.valueadd.examples.common;

import java.util.ArrayList;
import java.util.List;
import com.thomsonreuters.upa.rdm.DomainTypes;

/** Connection argument class for the Value Add consumer and
 * non-interactive provider applications. */
public class ConnectionArg
{
	int connectionType; /* type of the connection */
	String service; /* name of service to request items from on this connection */
	
	/* non-segmented connection */
	String hostname; /* hostname of provider to connect to */
	String port; /* port of provider to connect to */

	/* segmented connection */
	String sendAddress;	/* multicast address that the provider will be sending data to */
	String sendPort; /* port on the send multicast address that the provider will be sending data to */
	String interfaceName; /* interface that the provider will be using.  This is optional */
	String recvAddress; /* multicast address that the provider will be reading data from */
	String recvPort; /* port on the receive multicast address that the provider will be reading data from */
	String unicastPort; /* unicast port for unicast data */
	
	List<ItemArg> itemList; /* item list for this connection */
	
    String qService; /* service name for queue messages (if not specified, the service name specified in -c/-tcp is used) */
    String qSource; /* source name for queue messages (if specified, configures consumer to receive queue messages) */
    List<String> qDestList = new ArrayList<String>(); /* destination name for queue messages (if specified, configures consumer to send queue messages to this name, multiple instances may be specified) */
    boolean tunnel; /* enables consumer to open tunnel stream and send basic text messages */
	boolean tunnelAuth; /* When consumer opens a tunnel stream, indicate that authentication is expected. */
	int tunnelDomain; /* Domain type to use for tunnel streams. */

	public ConnectionArg(int connectionType, String service, String hostname, String port, List<ItemArg> itemList)
	{
		this.connectionType = connectionType;
		this.service = service;
		this.hostname = hostname;
		this.port = port;
		this.itemList = itemList;
		this.tunnelAuth = false;
		this.tunnelDomain = DomainTypes.SYSTEM;
	}
	
	public ConnectionArg()
	{
		this.tunnelAuth = false;
		this.tunnelDomain = DomainTypes.SYSTEM;
	}

	public int connectionType()
	{
		return connectionType;
	}
	
	public void connectionType(int connectionType)
	{
		this.connectionType = connectionType;
	}

	public String service()
	{
		return service;		
	}
	
	public void service(String service)
	{
		this.service = service;		
	}
	
	public String hostname()
	{
		return hostname;
	}
	
	public void hostname(String hostname)
	{
		this.hostname = hostname;
	}
	
	
	public String port()
	{
		return port;		
	}
	
	public void port(String port)
	{
		this.port = port;		
	}
	
	public String sendAddress()
	{
		return sendAddress;		
	}
	
	public String sendPort()
	{
		return sendPort;		
	}
	
	public String interfaceName()
	{
		return interfaceName;		
	}
	
	public void interfaceName(String interfaceName)
	{
		this.interfaceName = interfaceName;		
	}
	
	public String recvAddress()
	{
		return recvAddress;		
	}
	
	public String recvPort()
	{
		return recvPort;		
	}
	
	public String unicastPort()
	{
		return unicastPort;		
	}
	
	public List<ItemArg> itemList()
	{
		return itemList;
	}
	
	public void itemList(List<ItemArg> list)
	{
		this.itemList = list;
	}
	
    public String qService()
    {
        return qService;        
    }

    public void qService(String qService)
    {
        this.qService = qService;        
    }
    
    public String qSource()
    {
        return qSource;        
    }
    
    public void qSource(String qSource)
    {
        this.qSource = qSource;        
    }


    public List<String> qDestList()
	{
	    return qDestList;
	}
    
    public void qDestList(List<String> qDestlist)
	{
	    this.qDestList = qDestlist;
	}    
 
    public boolean tunnel()
    {
        return tunnel;        
    }    
    
    public void tunnel(boolean isTunnel)
    {
        this.tunnel = isTunnel;        
    }    

    public boolean tunnelAuth()
    {
        return tunnelAuth;        
    }  
    
    public void tunnelAuth(boolean isTunnelAuth)
    {
        this.tunnelAuth = isTunnelAuth;        
    }      

    public int tunnelDomain()
    {
        return tunnelDomain;        
    }  
    
    public void tunnelDomain(int domain)
    {
        this.tunnelDomain = domain;        
    }  
}
