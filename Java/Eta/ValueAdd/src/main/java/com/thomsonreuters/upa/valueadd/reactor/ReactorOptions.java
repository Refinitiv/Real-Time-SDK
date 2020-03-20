package com.thomsonreuters.upa.valueadd.reactor;

import java.net.URI;
import java.net.URISyntaxException;
import java.nio.ByteBuffer;

import org.apache.http.HttpHost;
import org.apache.http.client.utils.URIBuilder;
import org.apache.http.client.utils.URIUtils;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;

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
    static final String TOKEN_SERVICE_PATH = "/auth/oauth2/v1/token";
	
    private Buffer _serviceDiscoveryURL = CodecFactory.createBuffer();
    private Buffer _tokenServiceURL = CodecFactory.createBuffer();
    private HttpHost _serviceDiscoveryHost;
    private HttpHost _tokenServiceHost;
    private int _restRequestTimeout; /* Socket read timeout(millisecond) for REST requests */
    private double _tokenReissueRatio;
    private int _reissueTokenAttemptLimit;
    private int _reissueTokenAttemptInterval;
    
    Object _userSpecObj = null;
    boolean _xmlTracing = false;
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

    /**
     * A URL for the EDP-RT service discovery 
     *
     * @param serviceDiscoveryURL the URL for the EDP-RT service discovery
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
    				_serviceDiscoveryHost = new HttpHost(_serviceDiscoveryHost.getHostName(), DEFAULT_HTTPS_PORT, _serviceDiscoveryHost.getSchemeName());
    			}
    			else
    			{
    				_serviceDiscoveryHost = new HttpHost(_serviceDiscoveryHost.getHostName(), DEFAULT_HTTP_PORT, _serviceDiscoveryHost.getSchemeName());
    			}
    		}
    		
		} catch (URISyntaxException e) {
			return ReactorReturnCodes.PARAMETER_INVALID;
		}
    	
    	return _serviceDiscoveryURL.data(serviceDiscoveryURL.data(), 
    			serviceDiscoveryURL.position(), serviceDiscoveryURL.length());
    }

    /**
     * A URL for the EDP-RT service discovery 
     * 
     * @return the serviceDiscoveryURL
     */
    public Buffer serviceDiscoveryURL()
    {
        return _serviceDiscoveryURL;
    }    
    
    
    /**
     * A URL of the token service to get an access token and a refresh token. 
     * This is used for querying EDP-RT service
     * discovery and subscribing data from EDP-RT.
     *
     * @param tokenServiceURL the URL for the EDP-RT service discovery
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} on success, if data is null, 
     * 		   or if position or length is outside of the data's capacity.
     *         {@link ReactorReturnCodes#PARAMETER_INVALID}.     
     *          
     */
    public int tokenServiceURL(Buffer tokenServiceURL)
    {
    	if(tokenServiceURL == null || tokenServiceURL.length() == 0)
    	{
    		return ReactorReturnCodes.PARAMETER_INVALID;
    	}
    	
    	try 
    	{
    		URI uri = new URIBuilder(tokenServiceURL.toString()).build();
    		HttpHost tokenServiceHost = URIUtils.extractHost(uri);
    		
    		if(tokenServiceHost == null)
    			return ReactorReturnCodes.PARAMETER_INVALID;
    		
    		_tokenServiceHost = tokenServiceHost;
    		
    		/* Checks whether the port is specified */
    		if(_tokenServiceHost.getPort() == -1)
    		{
    			if(_tokenServiceHost.getSchemeName().equals(DEFAULT_SCHEME))
    			{
    				_tokenServiceHost = new HttpHost(_tokenServiceHost.getHostName(), DEFAULT_HTTPS_PORT, _tokenServiceHost.getSchemeName());
    			}
    			else
    			{
    				_tokenServiceHost = new HttpHost(_tokenServiceHost.getHostName(), DEFAULT_HTTP_PORT, _tokenServiceHost.getSchemeName());
    			}
    		}
    		
		} catch (URISyntaxException e) {
			return ReactorReturnCodes.PARAMETER_INVALID;
		}
    	
    	return _tokenServiceURL.data(tokenServiceURL.data(), 
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
     * This is used for querying EDP-RT service
     * discovery and subscribing data from EDP-RT.
     * 
     * @return the tokenServiceURL
     */
    public Buffer tokenServiceURL()
    {
        return _tokenServiceURL;
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
        _statistics = StatisticFlags.NONE;
        _serviceDiscoveryURL.data(DEFAULT_SCHEME + "://" + API_GATEWAY_HOST + SERVICE_DISCOVERY_PATH);
        _tokenServiceURL.data(DEFAULT_SCHEME + "://" + API_GATEWAY_HOST + TOKEN_SERVICE_PATH);         
		_serviceDiscoveryHost = new HttpHost(API_GATEWAY_HOST, DEFAULT_HTTPS_PORT, DEFAULT_SCHEME);
        _tokenServiceHost = new HttpHost(API_GATEWAY_HOST, DEFAULT_HTTPS_PORT, DEFAULT_SCHEME);
        _restRequestTimeout = 45000;
        _tokenReissueRatio = 0.8;
        _reissueTokenAttemptLimit = -1;
        _reissueTokenAttemptInterval = 5000;
    }
    
    /*
     * Performs a deep copy from a specified ReactorOptions into this ReactorOptions.
     */
    void copy(ReactorOptions options)
    {
        _userSpecObj = options._userSpecObj;
        _xmlTracing =  options._xmlTracing;
        _statistics =  options._statistics;
        _tokenReissueRatio = options._tokenReissueRatio;
        _reissueTokenAttemptLimit = (options._reissueTokenAttemptLimit < -1) ? -1 : options._reissueTokenAttemptLimit;
        _reissueTokenAttemptInterval = options._reissueTokenAttemptInterval;
        _restRequestTimeout = options._restRequestTimeout;
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._serviceDiscoveryURL.length());
        	options._serviceDiscoveryURL.copy(byteBuffer);
        	_serviceDiscoveryURL.data(byteBuffer);
        	_serviceDiscoveryHost = new HttpHost(options.serviceDiscoveryHost().getHostName(),
        			options.serviceDiscoveryHost().getPort(), options.serviceDiscoveryHost().getSchemeName());
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._tokenServiceURL.length());
        	options._tokenServiceURL.copy(byteBuffer);
        	_tokenServiceURL.data(byteBuffer);
        	_tokenServiceHost = new HttpHost(options.tokenServiceHost().getHostName(),
        			options.tokenServiceHost().getPort(), options.tokenServiceHost().getSchemeName());
        }
    }
    
    /* 
     * Returns HTTPHost for the service discovery used by Rest non-blocking request
    */
    HttpHost serviceDiscoveryHost()
    {
    	return _serviceDiscoveryHost;
    }
    
    /* 
     * Returns HTTPHost for the token service used by Rest non-blocking request
     */
    HttpHost tokenServiceHost()
    {
    	return _tokenServiceHost;
    }
}
