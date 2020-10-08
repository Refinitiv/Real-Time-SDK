package com.refinitiv.eta.valueadd.reactor;

import org.apache.http.HttpHost;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.TransportReturnCodes;

class RestConnectOptions {
	
	private int _transport;
	private int _dataFormat;
    private RestResultClosure _resultClosure;
	private String _proxyHost;
	private int _proxyPort;
	private String _proxyUserName;
	private String _proxyPassword;
	private String _proxyDomain;
	private String _proxyLocalHostName;
	private String _proxyKrb5ConfigFile;
	private ReactorOptions _reactorOptions;

	// Temporary redirect flags/locations
    private boolean _authRedirect = false;           
    private String _authRedirectLocation = null;
    private boolean _discoveryRedirect = false;
    private String _discoveryRedirectLocation = null;
	
    public RestConnectOptions(ReactorOptions options)
    {    	
    	/* This member variable is set only once and it must not be cleared. */
    	_reactorOptions = options;
    	
    	clear();
    }
    
	public void clear() 
	{
		_resultClosure = null;		
		_transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
		_dataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_RWF;
		_proxyHost = null;
		_proxyPort = -1;
		_proxyUserName = null;
		_proxyPassword = null;
		_proxyDomain = null;
		_proxyLocalHostName = "localhost";
		_proxyKrb5ConfigFile = "krb5.conf";
		
		_authRedirect = false;
		_authRedirectLocation = null;
		_discoveryRedirect = false;
		_discoveryRedirectLocation = null;
	}
	
	final static  int convertToPortNumber(Buffer proxyPort)
	{
		int port = -1;
		if(proxyPort.length() > 0)
		{
			try
			{
				port = Integer.parseInt(proxyPort.toString());
			}
			catch(NumberFormatException e){}
		}
		return port;
	}
	
	void applyServiceDiscoveryOptions(ReactorServiceDiscoveryOptions options)
	{
		dataFormat(options.dataFormat());
		transport(options.transport());
		proxyHost(options.proxyHostName().toString());
		proxyPort(RestConnectOptions.convertToPortNumber(options.proxyPort()));
		proxyUserName(options.proxyUserName().toString());
		proxyPassword(options.proxyPassword().toString());
		proxyDomain(options.proxyDomain().toString());
		proxyLocalHostName(options.proxyLocalHostName().toString());
		proxyKRB5ConfigFile(options.proxyKRB5ConfigFile().toString());    
	}
	
	void applyProxyInfo(ConnectOptions connectOptions)
	{
		if(connectOptions.tunnelingInfo() != null)
		{
			proxyHost(connectOptions.tunnelingInfo().HTTPproxyHostName());
			proxyPort(connectOptions.tunnelingInfo().HTTPproxyPort());
		}
		
		if(connectOptions.credentialsInfo() != null)
		{
			proxyUserName(connectOptions.credentialsInfo().HTTPproxyUsername());
			proxyPassword(connectOptions.credentialsInfo().HTTPproxyPasswd());
			proxyDomain(connectOptions.credentialsInfo().HTTPproxyDomain());
			proxyLocalHostName(connectOptions.credentialsInfo().HTTPproxyLocalHostname());
			proxyKRB5ConfigFile(connectOptions.credentialsInfo().HTTPproxyKRB5configFile());
		}
	}
	
	public void restResultClosure(RestResultClosure resultClosure)
	{
		_resultClosure = resultClosure;
	}
	
	public RestResultClosure restResultClosure()
	{
		return _resultClosure;
	}
	
	public String tokenServiceURL()
	{
		return _reactorOptions.tokenServiceURL().toString();
	}
	
	public String serviceDiscoveryURL()
	{
		return _reactorOptions.serviceDiscoveryURL().toString();
	}
	
	public HttpHost tokenServiceHost()
	{
		return _reactorOptions.tokenServiceHost();
	}
	
	public HttpHost serviceDiscoveryHost()
	{
		return _reactorOptions.serviceDiscoveryHost();
	}
	
	public String toString()
	{
		 return "RestConnectOptions" + "\n" + 
	               "\ttokenServiceURL: " + tokenServiceURL() + "\n" +
	               "\tserviceDiscoveryURL: " + serviceDiscoveryURL() + "\n" +             
	               "\tuserSpecObject: " + _resultClosure + "\n";
	}
	
	public int copy(RestConnectOptions destOpts)
    {
        if (destOpts == null)
            return TransportReturnCodes.FAILURE;
        
        destOpts._reactorOptions = _reactorOptions;     
        destOpts._resultClosure = _resultClosure;
        return TransportReturnCodes.SUCCESS;
    }
    
    void dataFormat(int dataFormat)
    {
    	_dataFormat = dataFormat;
    }
    
    void transport(int transport)
    {
    	_transport = transport;
    }

    int dataFormat()
    {
    	return _dataFormat;
    }
    
    int transport()
    {
    	return _transport;
    }
    
    void proxyHost(String proxyHost)
    {
    	_proxyHost = proxyHost;
    }
    
    String proxyHost()
    {
    	return _proxyHost;
    }
    
    void proxyPort(int proxyPort)
    {
    	_proxyPort = proxyPort;
    }
    
    int proxyPort()
    {
    	return _proxyPort;
    }
    
    void proxyUserName(String proxyUserName)
    {
    	_proxyUserName = proxyUserName;
    }
    
    String proxyUserName()
    {
    	return _proxyUserName;
    }
    
    void proxyPassword(String proxyPassword)
    {
    	_proxyPassword = proxyPassword;
    }
    
    String proxyPassword()
    {
    	return _proxyPassword;
    }
    
    void proxyDomain(String proxyDomain)
    {
    	_proxyDomain = proxyDomain;
    }
    
    String proxyDomain()
    {
    	return _proxyDomain;
    }
    
    void proxyLocalHostName(String proxyLocalHostName)
    {
    	_proxyLocalHostName = proxyLocalHostName;
    }
    
    String proxyLocalHostName()
    {
    	return _proxyLocalHostName;
    }
    
    void proxyKRB5ConfigFile(String proxyKrb5ConfigFile)
    {
    	_proxyKrb5ConfigFile = proxyKrb5ConfigFile;
    }
    
    String proxyKRB5ConfigFile()
    {
    	return _proxyKrb5ConfigFile;
    }
    
    public boolean authRedirect() {
    	return _authRedirect;
    }
    
    public void authRedirect(boolean value) {
    	_authRedirect = value;
    }
    
    public String authRedirectLocation() {
    	return _authRedirectLocation;
    }
    
    public void authRedirectLocation(String value) {
    	_authRedirectLocation = value;
    }
    
    public boolean  discoveryRedirect() {
    	return _discoveryRedirect;
    }
    
    public void discoveryRedirect(boolean value) {
    	_discoveryRedirect = value;
    }
    
    public String discoveryRedirectLocation() {
    	return _discoveryRedirectLocation;
    }
    
    public void discoveryRedirectLocation(String value) {
    	_discoveryRedirectLocation = value;
    }

    public ReactorOptions reactorOptions() {
    	return _reactorOptions;
    }
    
}
