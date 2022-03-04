/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.rdm.ClassesOfService;

/** Tunnel stream request information for a tunnel stream listener. */
public class TunnelStreamRequestEvent
{
    ReactorChannel _reactorChannel;
    int _domainType;
    int _streamId;
    int _serviceId;
    String _name;
    RequestMsg _msg;
    ClassOfService _classOfService = new ClassOfService();
    ReactorErrorInfo _errorInfo;
    long _classOfServiceFilter;
    
    /**
     * Returns the ReactorChannel of the TunnelStream request.
     *
     * @return the reactor channel
     */
    public ReactorChannel reactorChannel()
    {
        return _reactorChannel;
    }

    /**
     * Reactor channel.
     *
     * @param reactorChannel the reactor channel
     */
    void reactorChannel(ReactorChannel reactorChannel)
    {
        _reactorChannel = reactorChannel;
    }

    /**
     * Returns the domain type of the TunnelStream request.
     *
     * @return the int
     */
    public int domainType()
    {
        return _domainType;
    }

    /**
     * Domain type.
     *
     * @param domainType the domain type
     */
    void domainType(int domainType)
    {
        _domainType = domainType;
    }

    /**
     * Returns the stream id of the TunnelStream request.
     *
     * @return the int
     */
    public int streamId()
    {
        return _streamId;
    }

    /**
     * Stream id.
     *
     * @param streamId the stream id
     */
    void streamId(int streamId)
    {
        _streamId = streamId;
    }

    /**
     * Returns the service identifier of the TunnelStream request.
     *
     * @return the int
     */
    public int serviceId()
    {
        return _serviceId;
    }

    /**
     * Service id.
     *
     * @param serviceId the service id
     */
    void serviceId(int serviceId)
    {
        _serviceId = serviceId;
    }
    
    /**
     * Returns the name of the TunnelStream request.
     *
     * @return the string
     */
    public String name()
    {
        return _name;
    }

    /**
     * Name.
     *
     * @param name the name
     */
    void name(String name)
    {
        _name = name;
    }

    /**
     * Returns the TunnelStream request message.
     *
     * @return the request msg
     */
    public RequestMsg msg()
    {
        return _msg;
    }

    /**
     * Msg.
     *
     * @param msg the msg
     */
    void msg(RequestMsg msg)
    {
        _msg = msg;
    }

    /**
     * The ReactorErrorInfo associated with this TunnelStream request.
     * 
     * @return ReactorErrorInfo
     */
    public ReactorErrorInfo errorInfo()
    {
        return _errorInfo;
    }
    
    /**
     * Error info.
     *
     * @param errorInfo the error info
     */
    void errorInfo(ReactorErrorInfo errorInfo)
    {
        _errorInfo = errorInfo;
    }
    
    /**
     * Returns the class of service of the TunnelStream request.
     *
     * @return the class of service
     * @see ClassOfService
     */
    public ClassOfService classOfService()
    {
        return _classOfService;
    }

    /**
     * Returns the class of service filter of the TunnelStream request.
     * These are a combination of the ClassesOfService.FilterFlags.
     *
     * @return the long
     * @see ClassesOfService
     */
    public long classOfServiceFilter()
    {
        return _classOfServiceFilter;
    }
    
    /**
     * Class of service filter.
     *
     * @param filter the filter
     */
    void classOfServiceFilter(long filter)
    {
        _classOfServiceFilter = filter;
    }

    /**
     * Clears the TunnelStreamRequestEvent for re-use.
     */
    public void clear()
    {
        _domainType = 0;
        _streamId = 0;
        _serviceId = 0;
        _name = null;
        _classOfService.clear();
        _classOfServiceFilter = 0;
    }
}
