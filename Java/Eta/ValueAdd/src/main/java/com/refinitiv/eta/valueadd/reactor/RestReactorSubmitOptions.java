/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

class RestReactorSubmitOptions
{
    Object _userSpecObj;
    ReactorAuthTokenInfo _tokenInfo;
    RestConnectOptions _connectOptions;
    
    /**
     * Instantiates a new reactor submit options.
     */
    public RestReactorSubmitOptions()
    {
    	clear();
    }
    
    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _userSpecObj = null;
        _tokenInfo = null;
        _connectOptions = null;
    }
    
    /**
     * User-specified object to return as the application receives events related to this request.
     * 
     * @return the user-specified object
     */
    public Object userSpecObj()
    {
        return _userSpecObj;
    }
    
    /**
     * User-specified object to return as the application receives events related to this request.
     *
     * @param userSpecObj the user spec obj
     */
    public void userSpecObj(Object userSpecObj)
    {
        _userSpecObj = userSpecObj;
    }

    public void tokenInformation(ReactorAuthTokenInfo tokenInfo)
    {
        _tokenInfo = tokenInfo;
    }
    
    public ReactorAuthTokenInfo tokenInformation()
    {
    	return _tokenInfo;
    }
    
    public void connectOptions(RestConnectOptions options)
    {
    	_connectOptions = options;
    }
    
    public RestConnectOptions connectOptions()
    {
    	return _connectOptions;
    }
}
