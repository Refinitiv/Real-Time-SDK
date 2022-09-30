/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Enumerated types indicating warm standby modes. See {@link ReactorWarmStandbyGroup}.
 */
public class ReactorWarmStandbyMode 
{
    // ReactorWarmStandbyMode class cannot be instantiated
    private ReactorWarmStandbyMode()
    {
        throw new AssertionError();
    }
    
    /**
     * Unknown warm standby mode.  This is an initializer placeholder, and should not be used in any ReactorWarmStandbyGroup.mode().
     */
    public static final int NONE =  0;
    
    /**
     * Login based warm standby mode.
     * The login mode is connection based.  Upon the configured active server's connection getting 
     * established and login accepted, the configured active will be the active connection for all 
     * requests until the connection is lost, at which point the reactor will round-robin through the 
     * remaining connections to the next active connection.  
     */
    public static final int LOGIN_BASED =  1;	
    
    /**
     * Service based warm standby mode.
     * This mode is service-based.  The reactor watchlist will connect to the configured active server, 
     * perform a login request, and request the source directory after the login is accepted.  Upon 
     * receiving a source directory response from the active, the reactor will set the active server
     * connection as the active connection for the provided services. The reactor will then connect to
     * the remaining standby connection lists. If a new service is provided by a standby connection, 
     * the first one to provide it will become the active for that service.
     * 
     * If specific services are configured in {@link ReactorPerServiceBasedOptions#serviceNameList()}, 
     * the reactor will attempt to set the initial active connection to that configured server. Any servers
     * that are not configured with that service will be set to standby if they are connected before
     * the configured server.
     */
    public static final int SERVICE_BASED = 2;	// Per service based warm standby mode.
}
