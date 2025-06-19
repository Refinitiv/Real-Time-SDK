/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Options used for configuring a unified/fully connected mesh network.
 * 
 * @see ConnectOptions
 */
public interface UnifiedNetworkInfo
{
    /**
     * Address or hostname to connect to/join for all inbound and outbound data.
     * All data is exchanged on this hostName:serviceName combination.
     * 
     * @param address the address to set
     */
    public void address(String address);

    /**
     * Address or hostname to connect to/join for all inbound and outbound data.
     * All data is exchanged on this hostName:serviceName combination.
     * 
     * @return the address
     */
    public String address();

    /**
     * Port number or service name to connect to/join for all inbound and outbound data.
     * All data is exchanged on this hostName:serviceName combination.
     * 
     * @param serviceName the serviceName to set
     */
    public void serviceName(String serviceName);

    /**
     * Port number or service name to connect to/join for all inbound and outbound data.
     * All data is exchanged on this hostName:serviceName combination.
     * 
     * @return the serviceName
     */
    public String serviceName();

    /**
     * A character representation of an IP address or hostname associated with
     * the local network interface to use for sending and receiving content.
     * This value is intended for use in systems which have multiple network
     * interface cards, and if not specified the default network interface will be used.
     * 
     * @param interfaceName the interfaceName to set
     */
    public void interfaceName(String interfaceName);

    /**
     * A character representation of an IP address or hostname associated with
     * the local network interface to use for sending and receiving content.
     * This value is intended for use in systems which have multiple network
     * interface cards, and if not specified the default network interface will be used.
     * 
     * @return the interfaceName
     */
    public String interfaceName();
    
    /**
     * Port number or service name for any unicast messages such as ACK/NAK
     * traffic or retransmit requests.
     * Only used with connectionType of {@link ConnectionTypes#RELIABLE_MCAST}.
     * 
     * @param unicastServiceName the unicastServiceName to set
     */
    public void unicastServiceName(String unicastServiceName);

    /**
     * Port number or service name for any unicast messages such as ACK/NAK
     * traffic or retransmit requests.
     * Only used with connectionType of {@link ConnectionTypes#RELIABLE_MCAST}.
     * 
     * @return the unicastServiceName
     */
    public String unicastServiceName();

}