package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import org.apache.http.HttpHost;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

class RestConnectOptions {
	
	private Buffer _userName;
	private Buffer _password;
	private Buffer _clientId;
	private int _transport;
	private int _dataFormat;
    private Object _userSpecObject;
    private ReactorAuthTokenInfo _reactorAuthTokenInfo = new ReactorAuthTokenInfo();
    private RestCallback _authCallback;
	private RestCallback _defaultRespCallback;
	private String _location;
	private String _proxyHost;
	private int _proxyPort;
	private String _proxyUserName;
	private String _proxyPassword;
	private String _proxyDomain;
	private String _proxyLocalHostName;
	private String _proxyKrb5ConfigFile;
	private ReactorOptions _reactorOptions;

    public RestConnectOptions(ReactorOptions options)
    {
    	_userName = CodecFactory.createBuffer();
    	_password = CodecFactory.createBuffer();
    	_clientId = CodecFactory.createBuffer();
    	
    	/* This member variable is set only once and it must not be cleared. */
    	_reactorOptions = options;
    	
    	clear();
    }
    
	public void clear() 
	{
		_userName.clear();
		_password.clear();
		_clientId.clear();
		_reactorAuthTokenInfo.clear();
		_authCallback = null;
		_defaultRespCallback = null;
		_location = null;
		_userSpecObject = null;		
		_transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
		_proxyHost = null;
		_proxyPort = -1;
		_proxyUserName = null;
		_proxyPassword = null;
		_proxyDomain = null;
		_proxyLocalHostName = "localhost";
		_proxyKrb5ConfigFile = "krb5.conf";
	}
	
	public ReactorAuthTokenInfo tokenInformation()
	{
		return _reactorAuthTokenInfo;
	}
	
	public void userSpecObject(Object userSpecObject)
	{
		_userSpecObject = userSpecObject;
	}
	
	public Object userSpecObject()
	{
		return _userSpecObject;
	}
	
	public void userName (Buffer username)
	{
		ByteBuffer byteBuffer = ByteBuffer.allocate(username.length());
		username.copy(byteBuffer);
		_userName.data(byteBuffer);
	}
	
	public Buffer userName ()
	{
		return _userName;
	}
	
	public void password (Buffer password)
	{
		ByteBuffer byteBuffer = ByteBuffer.allocate(password.length());
		password.copy(byteBuffer);
		_password.data(byteBuffer);
	}
	
	public Buffer password ()
	{
		return _password;
	}	
	
	public void clientId (Buffer clientId)
	{
		ByteBuffer byteBuffer = ByteBuffer.allocate(clientId.length());
		clientId.copy(byteBuffer);
		_clientId.data(byteBuffer);
	}	
	
	public Buffer clientId ()
	{
		return _clientId;
	}	
	
	public void location(String location) {
		_location = location;
	}
	
	public String location()
	{
		return _location;
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
	               "\tuserName: " + _userName + "\n" + 
	               "\tpassword: " + _password + "\n" + 	               
	               "\tuserSpecObject: " + _userSpecObject + "\n" + 
	               "\tauthCallback: " + _authCallback + "\n" + 
	               "\tdefaultRespCallback: " + _defaultRespCallback + 
	               "\tlocation" + _location;
	}
	
	public int copy(RestConnectOptions destOpts)
    {
        if (destOpts == null)
            return TransportReturnCodes.FAILURE;

        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(this._userName.length());
        	this._userName.copy(byteBuffer);
        	destOpts._userName.data(byteBuffer);
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(this._password.length());
        	this._password.copy(byteBuffer);
        	destOpts._password.data(byteBuffer);        
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(this._clientId.length());
        	this._clientId.copy(byteBuffer);
        	destOpts._clientId.data(byteBuffer);        
        }        
        
        destOpts._reactorOptions = _reactorOptions;
        destOpts._password = _password;        
        destOpts._userSpecObject = _userSpecObject;
        destOpts._authCallback = _authCallback;
        destOpts._defaultRespCallback = _defaultRespCallback;
        destOpts._location = _location;
        return _reactorAuthTokenInfo.copy(destOpts._reactorAuthTokenInfo);
    }
	
	/**
     *  A callback function for processing authorization response received. If not present,
     * the received message will be passed to the defaultRespCallback.
     *
     * @param callback the callback
     */
    void authorizationCallback(RestCallback callback)
    {
        _authCallback = callback;
    }

    /** A callback function for processing authorization response received. If not present,
     * the received message will be passed to the defaultRespCallback.
     * 
     * @return the RestCallback
     */
    RestCallback authorizationCallback()
    {
        return _authCallback;
    }
    
    /**
     *  A callback function for processing any response received.
     *
     * @param callback the callback
     */
    void defaultRespCallback(RestCallback callback)
    {
    	_defaultRespCallback = callback;
    }

    /** A callback function for processing any response received.
     * 
     * @return the RestCallback
     */
    RestCallback defaultRespCallback()
    {
        return _defaultRespCallback;
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
}
