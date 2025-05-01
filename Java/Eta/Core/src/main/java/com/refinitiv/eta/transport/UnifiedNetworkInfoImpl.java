/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.UnifiedNetworkInfo;

class UnifiedNetworkInfoImpl implements UnifiedNetworkInfo
{
    private String _address;
    private String _serviceName;
    private int _port;
    private String _interfaceName;
    private String _unicastServiceName;

    UnifiedNetworkInfoImpl()
    {
    }

    /* Make a deep copy of this object to the specified object.
     * 
     * destUnified is the destination object.
     */
    void copy(UnifiedNetworkInfoImpl destUnified)
    {
        destUnified._port = _port;

        if (_address != null)
            destUnified._address = new String(_address);
        else
            destUnified._address = null;

        if (_serviceName != null)
            destUnified._serviceName = new String(_serviceName);
        else
            destUnified._serviceName = null;

        if (_interfaceName != null)
            destUnified._interfaceName = new String(_interfaceName);
        else
            destUnified._interfaceName = null;

        if (_unicastServiceName != null)
            destUnified._unicastServiceName = new String(_unicastServiceName);
        else
            destUnified._unicastServiceName = null;
    }

    @Override
    public String toString()
    {
        return "UnifiedNetworkInfo" + "\n" + 
               "\t\t\taddress: " + _address + "\n" + 
               "\t\t\tserviceName: " + _serviceName + "\n" + 
               "\t\t\tinterfaceName: " + _interfaceName + "\n" + 
               "\t\t\tunicastServiceName: " + _unicastServiceName;
    }

    @Override
    public void address(String address)
    {
        assert (address != null) : "address must be non-null";

        _address = address;
    }

    @Override
    public String address()
    {
        return _address;
    }

    @Override
    public void serviceName(String serviceName)
    {
        assert (serviceName != null) : "serviceName must be non-null";

        _serviceName = serviceName;

        // set port also
        try
        {
            // the service is specified as a port number
            _port = Integer.parseInt(serviceName);
        }
        catch (Exception e)
        {
            // the service is a name
            _port = GetServiceByName.getServiceByName(serviceName);
        }
    }

    @Override
    public String serviceName()
    {
        return _serviceName;
    }

    int port()
    {
        return _port;
    }

    @Override
    public void interfaceName(String interfaceName)
    {
        _interfaceName = interfaceName;
    }

    @Override
    public String interfaceName()
    {
        return _interfaceName;
    }

    @Override
    public void unicastServiceName(String unicastServiceName)
    {
        assert (unicastServiceName != null) : "unicastServiceName must be non-null";

        _unicastServiceName = unicastServiceName;
    }

    @Override
    public String unicastServiceName()
    {
        return _unicastServiceName;
    }

    void clear()
    {
        _address = null;
        _serviceName = null;
        _port = 0;
        _interfaceName = null;
        _unicastServiceName = null;
    }

}
