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

	/**
	 * Instantiates a new connection arg.
	 *
	 * @param connectionType the connection type
	 * @param service the service
	 * @param hostname the hostname
	 * @param port the port
	 * @param itemList the item list
	 */
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
	
	/**
	 * Instantiates a new connection arg.
	 */
	public ConnectionArg()
	{
		this.tunnelAuth = false;
		this.tunnelDomain = DomainTypes.SYSTEM;
	}

	/**
	 * Connection type.
	 *
	 * @return the int
	 */
	public int connectionType()
	{
		return connectionType;
	}
	
	/**
	 * Connection type.
	 *
	 * @param connectionType the connection type
	 */
	public void connectionType(int connectionType)
	{
		this.connectionType = connectionType;
	}

	/**
	 * Service.
	 *
	 * @return the string
	 */
	public String service()
	{
		return service;		
	}
	
	/**
	 * Service.
	 *
	 * @param service the service
	 */
	public void service(String service)
	{
		this.service = service;		
	}
	
	/**
	 * Hostname.
	 *
	 * @return the string
	 */
	public String hostname()
	{
		return hostname;
	}
	
	/**
	 * Hostname.
	 *
	 * @param hostname the hostname
	 */
	public void hostname(String hostname)
	{
		this.hostname = hostname;
	}
	
	
	/**
	 * Port.
	 *
	 * @return the string
	 */
	public String port()
	{
		return port;		
	}
	
	/**
	 * Port.
	 *
	 * @param port the port
	 */
	public void port(String port)
	{
		this.port = port;		
	}
	
	/**
	 * Send address.
	 *
	 * @return the string
	 */
	public String sendAddress()
	{
		return sendAddress;		
	}
	
	/**
	 * Send port.
	 *
	 * @return the string
	 */
	public String sendPort()
	{
		return sendPort;		
	}
	
	/**
	 * Interface name.
	 *
	 * @return the string
	 */
	public String interfaceName()
	{
		return interfaceName;		
	}
	
	/**
	 * Interface name.
	 *
	 * @param interfaceName the interface name
	 */
	public void interfaceName(String interfaceName)
	{
		this.interfaceName = interfaceName;		
	}
	
	/**
	 * Recv address.
	 *
	 * @return the string
	 */
	public String recvAddress()
	{
		return recvAddress;		
	}
	
	/**
	 * Recv port.
	 *
	 * @return the string
	 */
	public String recvPort()
	{
		return recvPort;		
	}
	
	/**
	 * Unicast port.
	 *
	 * @return the string
	 */
	public String unicastPort()
	{
		return unicastPort;		
	}
	
	/**
	 * Item list.
	 *
	 * @return the list
	 */
	public List<ItemArg> itemList()
	{
		return itemList;
	}
	
	/**
	 * Item list.
	 *
	 * @param list the list
	 */
	public void itemList(List<ItemArg> list)
	{
		this.itemList = list;
	}
	
    /**
     * Q service.
     *
     * @return the string
     */
    public String qService()
    {
        return qService;        
    }

    /**
     * Q service.
     *
     * @param qService the q service
     */
    public void qService(String qService)
    {
        this.qService = qService;        
    }
    
    /**
     * Q source.
     *
     * @return the string
     */
    public String qSource()
    {
        return qSource;        
    }
    
    /**
     * Q source.
     *
     * @param qSource the q source
     */
    public void qSource(String qSource)
    {
        this.qSource = qSource;        
    }


    /**
     * Q dest list.
     *
     * @return the list
     */
    public List<String> qDestList()
	{
	    return qDestList;
	}
    
    /**
     * Q dest list.
     *
     * @param qDestlist the q destlist
     */
    public void qDestList(List<String> qDestlist)
	{
	    this.qDestList = qDestlist;
	}    
 
    /**
     * Tunnel.
     *
     * @return true, if successful
     */
    public boolean tunnel()
    {
        return tunnel;        
    }    
    
    /**
     * Tunnel.
     *
     * @param isTunnel the is tunnel
     */
    public void tunnel(boolean isTunnel)
    {
        this.tunnel = isTunnel;        
    }    

    /**
     * Tunnel auth.
     *
     * @return true, if successful
     */
    public boolean tunnelAuth()
    {
        return tunnelAuth;        
    }  
    
    /**
     * Tunnel auth.
     *
     * @param isTunnelAuth the is tunnel auth
     */
    public void tunnelAuth(boolean isTunnelAuth)
    {
        this.tunnelAuth = isTunnelAuth;        
    }      

    /**
     * Tunnel domain.
     *
     * @return the int
     */
    public int tunnelDomain()
    {
        return tunnelDomain;        
    }  
    
    /**
     * Tunnel domain.
     *
     * @param domain the domain
     */
    public void tunnelDomain(int domain)
    {
        this.tunnelDomain = domain;        
    }  
}
