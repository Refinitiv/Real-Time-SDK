/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.net.URI;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;

import org.apache.hc.core5.http.HttpHost;
import org.apache.hc.core5.net.URIBuilder;
import org.apache.hc.client5.http.utils.URIUtils;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

/**
 * ReactorOptions to be used in the {@link ReactorFactory#createReactor(ReactorOptions,
 * ReactorErrorInfo)} call.
 */
public class ReactorOptions
{
	/**
     * Statistics flags for class of service.
     */
    public static class StatisticFlags
    {
        // StatisticFlags class cannot be instantiated
        private StatisticFlags()
        {
            throw new AssertionError();
        }

        public static final int NONE =  0x00000000;
        public static final int READ =  0x00000001;
        public static final int WRITE = 0x00000002;
        public static final int PING =  0x00000004;
    }
    
    /* Defined default token service and service discovery URLs*/
    static final int DEFAULT_HTTPS_PORT = 443;
    static final int DEFAULT_HTTP_PORT = 80;
    static final String DEFAULT_SCHEME = "https";
    static final String API_GATEWAY_HOST = "api.refinitiv.com";
    static final String SERVICE_DISCOVERY_PATH = "/streaming/pricing/v1/";

    static final long DEFAULT_XML_TRACE_MAX_FILE_SIZE = 100000000;
	
    private Buffer _serviceDiscoveryURL = CodecFactory.createBuffer();
    private Buffer _tokenServiceURL_V1 = CodecFactory.createBuffer();
    private Buffer _tokenServiceURL_V2 = CodecFactory.createBuffer();
    private Buffer _tempTokenURL = CodecFactory.createBuffer();
    private Buffer _tempDiscoveryURL = CodecFactory.createBuffer();
    private HttpHost _serviceDiscoveryHost;
    private int _restRequestTimeout; /* Socket read timeout(millisecond) for REST requests */
    private double _tokenReissueRatio;
    private int _reissueTokenAttemptLimit;
    private int _reissueTokenAttemptInterval;
    private ReactorRestProxyOptions _restProxyOptions = new ReactorRestProxyOptions();

    private ReactorDebuggerOptions _debuggerOptions = new ReactorDebuggerOptionsImpl();

    Object _userSpecObj = null;
    boolean _xmlTracing = false;
    boolean _xmlTraceToFile = false;
    long 	_xmlTraceMaxFileSize = DEFAULT_XML_TRACE_MAX_FILE_SIZE;
    String	_xmlTraceFileName;
    boolean	_xmlTraceToMultipleFiles = false;
    boolean _xmlTraceWrite = false;
    boolean _xmlTraceRead = false;
    boolean _xmlTracePing = false;
    int _statistics = StatisticFlags.NONE;

    ReactorOptions()
    {
    	clear();
    }

    /**
     * Specifies a user defined object that can be useful for identifying a
     * specific instance of a Reactor or coupling this Reactor with other user
     * created information.
     * 
     * @param userSpecObj the userSpecObj
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the userSpecObj is not
     *         null, otherwise {@link ReactorReturnCodes#PARAMETER_INVALID}.
     */
    public int userSpecObj(Object userSpecObj)
    {
        if (userSpecObj == null)
            return ReactorReturnCodes.PARAMETER_INVALID;

        _userSpecObj = userSpecObj;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the userSpecObj.
     * 
     * @return the userSpecObj.
     */
    public Object userSpecObj()
    {
        return _userSpecObj;
    }
    
    /**
     * Enables XML tracing for the Reactor. The Reactor prints the XML
     * representation of all OMM message when enabled.
     */
    public void enableXmlTracing()
    {
        _xmlTracing = true;
    }
    public void enableXmlTraceToFile()
    {
        _xmlTraceToFile = true;
    }
    public void setXmlTraceMaxFileSize(long size)
    {
        _xmlTraceMaxFileSize = size;
    }
    public void setXmlTraceFileName(String fileName)
    {
        _xmlTraceFileName = fileName;
    }
    public void enableXmlTraceToMultipleFiles()
    {
        _xmlTraceToMultipleFiles = true;
    }
    public void enableXmlTraceWrite()
    {
        _xmlTraceWrite = true;
    }
    public void enableXmlTraceRead()
    {
        _xmlTraceRead = true;
    }
    public void enableXmlTracePing()
    {
        _xmlTracePing = true;
    }
    public void xmlTraceWrite(boolean xmlTraceWrite)
    {
        _xmlTraceWrite = xmlTraceWrite;
    }
    public void xmlTraceRead(boolean xmlTraceRead)
    {
        _xmlTraceRead = xmlTraceRead;
    }
    public void xmlTracePing(boolean xmlTracePing)
    {
        _xmlTracePing = xmlTracePing;
    }

    /**
     * A URL for the LDP service discovery
     *
     * @param serviceDiscoveryURL the URL for the LDP service discovery
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} on success, if data is null, 
     * 		   or if position or length is outside of the data's capacity.
     *         {@link ReactorReturnCodes#PARAMETER_INVALID}.
     */
    public int serviceDiscoveryURL(Buffer serviceDiscoveryURL)
    {
    	if(serviceDiscoveryURL == null || serviceDiscoveryURL.length() == 0)
    	{
    		return ReactorReturnCodes.PARAMETER_INVALID;
    	}
    	
    	try 
    	{
    		URI uri = new URIBuilder(serviceDiscoveryURL.toString()).build();
    		HttpHost serviceDiscoveryHost = URIUtils.extractHost(uri);
    		
    		if(serviceDiscoveryHost == null)
    			return ReactorReturnCodes.PARAMETER_INVALID;
    		
    		_serviceDiscoveryHost = serviceDiscoveryHost;
    		
    		/* Checks whether the port is specified */
    		if(_serviceDiscoveryHost.getPort() == -1)
    		{
    			if(_serviceDiscoveryHost.getSchemeName().equals(DEFAULT_SCHEME))
    			{
    				_serviceDiscoveryHost = new HttpHost(_serviceDiscoveryHost.getSchemeName(), _serviceDiscoveryHost.getHostName(), DEFAULT_HTTPS_PORT);
    			}
    			else
    			{
    				_serviceDiscoveryHost = new HttpHost(_serviceDiscoveryHost.getSchemeName(), _serviceDiscoveryHost.getHostName(), DEFAULT_HTTP_PORT);
    			}
    		}
    		
		} catch (URISyntaxException e) {
			return ReactorReturnCodes.PARAMETER_INVALID;
		}
    	
    	return _serviceDiscoveryURL.data(serviceDiscoveryURL.data(), 
    			serviceDiscoveryURL.position(), serviceDiscoveryURL.length());
    }

    /**
     * A URL for the LDP service discovery
     * 
     * @return the serviceDiscoveryURL
     */
    public Buffer serviceDiscoveryURL()
    {
        return _serviceDiscoveryURL;
    }    
    
    
    /**
     * A URL of the token service to get an access token and a refresh token. 
     * This is used for querying LDP service
     * discovery and subscribing data from LDP.
     *
     * @param tokenServiceURL the URL for the LDP service discovery
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} on success, if data is null, 
     * 		   or if position or length is outside of the data's capacity.
     *         {@link ReactorReturnCodes#PARAMETER_INVALID}.     
     *          
     */
    public int tokenServiceURL(Buffer tokenServiceURL)
    {
    	if(_tokenServiceURL_V1 == null)
    	{
    		_tokenServiceURL_V1 = CodecFactory.createBuffer();
    	}
    	
    	return _tokenServiceURL_V1.data(tokenServiceURL.data(),
    			tokenServiceURL.position(), tokenServiceURL.length());
    }

    /**
     * A URL of the token service to get an access token and a refresh token.
     * This is used for querying LDP service
     * discovery and subscribing data from LDP.
     *
     * @param tokenServiceURL the URL for the LDP service discovery
     *
     * @return {@link ReactorReturnCodes#SUCCESS} on success, if data is null,
     * 		   or if position or length is outside of the data's capacity.
     *         {@link ReactorReturnCodes#PARAMETER_INVALID}.
     *
     */
    public int tokenServiceURL_V1(Buffer tokenServiceURL)
    {
    	if(_tokenServiceURL_V1 == null)
    	{
    		_tokenServiceURL_V1 = CodecFactory.createBuffer();
    	}

    	return _tokenServiceURL_V1.data(tokenServiceURL.data(),
    			tokenServiceURL.position(), tokenServiceURL.length());
    }

    /**
     * A URL of the token service to get an access token and a refresh token.
     * This is used for querying LDP service
     * discovery and subscribing data from LDP.
     *
     * @param tokenServiceURL the URL for the LDP service discovery
     *
     * @return {@link ReactorReturnCodes#SUCCESS} on success, if data is null,
     * 		   or if position or length is outside of the data's capacity.
     *         {@link ReactorReturnCodes#PARAMETER_INVALID}.
     *
     */
    public int tokenServiceURL_V2(Buffer tokenServiceURL)
    {
    	if(_tokenServiceURL_V2 == null)
    	{
    		_tokenServiceURL_V2 = CodecFactory.createBuffer();
    	}
    	
    	return _tokenServiceURL_V2.data(tokenServiceURL.data(),
    			tokenServiceURL.position(), tokenServiceURL.length());
    }
    
    public void statistics(int statistics)
    {
    	_statistics = statistics;
    }
    
    public boolean readStatSet()
    {
    	return (_statistics & StatisticFlags.READ) != 0;
    }
    
    public boolean writeStatSet()
    {
    	return (_statistics & StatisticFlags.WRITE) != 0;
    }
    
    public boolean pingStatSet()
    {
    	return (_statistics & StatisticFlags.PING) != 0;
    }
    
    /**
     * a URL of the token service to get an access token and a refresh token. 
     * This is used for querying LDP service
     * discovery and subscribing data from LDP.
     * 
     * @return the tokenServiceURL
     */
    public Buffer tokenServiceURL()
    {
        return _tokenServiceURL_V1;
    }

    /**
     * a URL of the token service to get an access token and a refresh token.
     * This is used for querying LDP service
     * discovery and subscribing data from LDP.
     *
     * @return the tokenServiceURL
     */
    public Buffer tokenServiceURL_V1()
    {
        return _tokenServiceURL_V1;
    }

    /**
     * a URL of the token service to get an access token and a refresh token.
     * This is used for querying LDP service
     * discovery and subscribing data from LDP.
     *
     * @return the tokenServiceURL
     */
    public Buffer tokenServiceURL_V2()
    {
        return _tokenServiceURL_V2;
    }
    
    /**
     * Returns the maximum time for the REST requests is allowed to take for token service and service discovery.
     * 
     * @return the REST request timeout(millisecond)
     */
    public int restRequestTimeout()
    {
    	return _restRequestTimeout;
    }
    
    /**
     * Specifies a maximum time(millisecond) for the REST requests is allowed to take for token service and service discovery.
     * The default request timeout is 45000 milliseconds.
     * 
     * @param requestTimeout specifies request timeout in millisecond
     */
    public void restRequestTimeout(int requestTimeout)
    {
    	_restRequestTimeout = requestTimeout;
    }
    
    /**
     * Returns the ratio to multiply with access token validity time(second) for retrieving access token from the token service.
     * 
     * @return the token reissue ratio
     */
    public double tokenReissueRatio()
    {
    	return _tokenReissueRatio;
    }
    
    /**
     * Specifies a ratio to multiply with access token validity time(second) for retrieving access token from the token service.
     * The default token reissue ratio is 0.8. The valid range is between 0.05 to 0.95.
     * 
     * @param tokenReissueRatio specifies request timeout
     */
    public void tokenReissueRatio(double tokenReissueRatio)
    {
    	_tokenReissueRatio = tokenReissueRatio;
    }
    
    /**
     * Returns the maximum number of times the Reactor will attempt to reissue the token. If the value is  -1, there is no limit.
     * 
     * @return the maximum number of token reissue attempts
     */
    public int reissueTokenAttemptLimit()
    {
    	return _reissueTokenAttemptLimit;
    }
    
    /**
     * Specifies a maximum number of times the Reactor will attempt to reissue the token. If set to -1, there is no limit.
     * 
     * @param reissueTokenAttempLimit specifies maximum number of token reissue attempts
     */
    public void reissueTokenAttemptLimit(int reissueTokenAttempLimit)
    {
    	_reissueTokenAttemptLimit = reissueTokenAttempLimit;
    }
    
    /**
     * Returns the interval time for the Reactor will wait before attempting to reissue the token after a request failure, in milliseconds.
     * 
     * @return the token reissue attempt interval
     */
    public int reissueTokenAttemptInterval()
    {
    	return _reissueTokenAttemptInterval;
    }
    
    /**
     * Specifies an interval time for the Reactor will wait before attempting to reissue the token after a request failure, in milliseconds.
     * 
     * @param reissueTokenAttemptInterval specifies an interval time for token reissue attempts
     */
    public void reissueTokenAttemptInterval(int reissueTokenAttemptInterval)
    {
    	_reissueTokenAttemptInterval = reissueTokenAttemptInterval;
    }
   
    boolean xmlTracing()
    {
        return _xmlTracing;
    }
    boolean xmlTraceToFile()
    {
        return _xmlTraceToFile;
    }
    long xmlTraceMaxFileSize()
    {
        return _xmlTraceMaxFileSize;
    }
    String  xmlTraceFileName()
    {
        return _xmlTraceFileName;
    }
    boolean xmlTraceToMultipleFiles()
    {
        return _xmlTraceToMultipleFiles;
    }

    boolean xmlTraceWrite()
    {
        return _xmlTraceWrite;
    }
    boolean xmlTraceRead()
    {
        return _xmlTraceRead;
    }
    boolean xmlTracePing()
    {
        return _xmlTracePing;
    }
    
    public int statistics()
    {
        return _statistics;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _userSpecObj = null;
        _xmlTracing = false;
        _xmlTraceToFile = false;
        _xmlTraceMaxFileSize = DEFAULT_XML_TRACE_MAX_FILE_SIZE;
        _xmlTraceFileName = null;
        _xmlTraceToMultipleFiles = false;
        _xmlTraceRead = true;
        _xmlTraceWrite = true;
        _xmlTracePing = true;
        _statistics = StatisticFlags.NONE;
        _serviceDiscoveryURL.data(DEFAULT_SCHEME + "://" + API_GATEWAY_HOST + SERVICE_DISCOVERY_PATH);
        _serviceDiscoveryHost = new HttpHost(DEFAULT_SCHEME, API_GATEWAY_HOST, DEFAULT_HTTPS_PORT);
        _restRequestTimeout = 45000;
        _tokenReissueRatio = 0.8;
        _reissueTokenAttemptLimit = -1;
        _reissueTokenAttemptInterval = 5000;
        _debuggerOptions.clear();
    }
    
    /*
     * Performs a deep copy from a specified ReactorOptions into this ReactorOptions.
     */
    void copy(ReactorOptions options)
    {
        _userSpecObj = options._userSpecObj;
        _xmlTracing =  options._xmlTracing;
        _xmlTraceToFile = options._xmlTraceToFile;
        _xmlTraceMaxFileSize = options._xmlTraceMaxFileSize;
        _xmlTraceFileName = options._xmlTraceFileName;
        _xmlTraceToMultipleFiles = options._xmlTraceToMultipleFiles;
        _xmlTraceWrite =  options._xmlTraceWrite;
        _xmlTracePing =  options._xmlTracePing;
        _xmlTraceRead =  options._xmlTraceRead;
        _statistics =  options._statistics;
        _tokenReissueRatio = options._tokenReissueRatio;
        _reissueTokenAttemptLimit = (options._reissueTokenAttemptLimit < -1) ? -1 : options._reissueTokenAttemptLimit;
        _reissueTokenAttemptInterval = options._reissueTokenAttemptInterval;
        _restRequestTimeout = options._restRequestTimeout;
        
        if(options.serviceDiscoveryURL() != null)
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._serviceDiscoveryURL.length());
        	options._serviceDiscoveryURL.copy(byteBuffer);
        	_serviceDiscoveryURL.data(byteBuffer);
        	_serviceDiscoveryHost = new HttpHost(options.serviceDiscoveryHost().getSchemeName(), options.serviceDiscoveryHost().getHostName(),
        			options.serviceDiscoveryHost().getPort());
        }
        
        if(options._tokenServiceURL_V1 != null)
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._tokenServiceURL_V1.length());
        	options._tokenServiceURL_V1.copy(byteBuffer);
        	_tokenServiceURL_V1.data(byteBuffer);
        }

        if(options._tokenServiceURL_V2 != null)
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._tokenServiceURL_V2.length());
        	options._tokenServiceURL_V2.copy(byteBuffer);
        	_tokenServiceURL_V2.data(byteBuffer);
        }
        
        if(options._restProxyOptions != null)
        {
        	options._restProxyOptions.copy(_restProxyOptions);
        }

        options._debuggerOptions.copy(_debuggerOptions);
    }
    
    /* 
     * Returns HTTPHost for the service discovery used by Rest non-blocking request
    */
    HttpHost serviceDiscoveryHost()
    {
    	return _serviceDiscoveryHost;
    }

    /**
     * Getter for the debugger options
     * @return current ReactorDebuggerOptions instance
     */
    public ReactorDebuggerOptions debuggerOptions() {
        return _debuggerOptions;
    }

    /**
     * Setter for the debugger options
     * @param debuggerOptions ReactorDebuggerOptions instance to be set
     */
    public void debuggerOptions(ReactorDebuggerOptions debuggerOptions) {
        if (debuggerOptions != null) {
            debuggerOptions.copy(this._debuggerOptions);
        }
    }
    
    /**
     * Getter for the RestProxyOptions
     * @return ReactorRestProxyOptions
     */
    public ReactorRestProxyOptions restProxyOptions()
    {
    	return _restProxyOptions;
    }
}
