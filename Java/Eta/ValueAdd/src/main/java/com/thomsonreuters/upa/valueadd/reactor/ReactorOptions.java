package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;

/**
 * ReactorOptions to be used in the {@link ReactorFactory#createReactor(ReactorOptions,
 * ReactorErrorInfo)} call.
 */
public class ReactorOptions
{
    private Buffer _serviceDiscoveryURL = CodecFactory.createBuffer();
    private Buffer _tokenServiceURL = CodecFactory.createBuffer();	
    Object _userSpecObj = null;
    boolean _xmlTracing = false;

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
    	return _tokenServiceURL.data(tokenServiceURL.data(), 
    			tokenServiceURL.position(), tokenServiceURL.length());
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
    
    
    boolean xmlTracing()
    {
        return _xmlTracing;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _userSpecObj = null;
        _xmlTracing = false;
        _serviceDiscoveryURL.data("https://api.refinitiv.com/streaming/pricing/v1");
        _tokenServiceURL.data("https://api.refinitiv.com/auth/oauth2/beta1/token");        
    }
    
    /*
     * Performs a deep copy from a specified ReactorOptions into this ReactorOptions.
     */
    void copy(ReactorOptions options)
    {
        _userSpecObj = options._userSpecObj;
        _xmlTracing =  options._xmlTracing;
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._serviceDiscoveryURL.length());
        	options._serviceDiscoveryURL.copy(byteBuffer);
        	_serviceDiscoveryURL.data(byteBuffer);
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(options._tokenServiceURL.length());
        	options._tokenServiceURL.copy(byteBuffer);
        	_tokenServiceURL.data(byteBuffer);
        }
    }
}
