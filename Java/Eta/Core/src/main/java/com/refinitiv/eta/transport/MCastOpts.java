/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Options used for configuring multicast specific transport options ({@link ConnectionTypes#RELIABLE_MCAST}).
 * 
 * @see ConnectOptions
 */
public interface MCastOpts
{
    /**
     * Connection will be removed from network on detection of any gap.
     * <dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
     * <dt><b>Note:</b></dt>
     * <dd>Enabling this will stop communication with all devices communicating
     * with this connection, even though all may not be affected by detected gaps.
     * If application can recover only impacted data above transport layer, this is ideal.</dd>
     * </dl>
     * 
     * @param disconnectOnGaps the disconnectOnGaps to set
     */
    public void disconnectOnGaps(boolean disconnectOnGaps);

    /**
     * Connection will be removed from network on detection of any gap.
     * <dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
     * <dt><b>Note:</b></dt>
     * <dd>Enabling this will stop communication with all devices communicating
     * with this connection, even though all may not be affected by detected gaps.
     * If application can recover only impacted data above transport layer, this is ideal.</dd>
     * </dl>
     * 
     * @return the disconnectOnGaps
     */
    public boolean disconnectOnGaps();
    
    /**
     * The time-to-live for a multicast datagram on the network. This controls the
     * number of hops content can flow over a network before it will be halted.
     * 
     * @param packetTTL the packetTTL to set
     */
    public void packetTTL(int packetTTL);
    
    /**
     * The time-to-live for a multicast datagram on the network. This controls the
     * number of hops content can flow over a network before it will be halted.
     * 
     * @return the packetTTL
     */
    public int packetTTL();

    /**
     * The RRCP tcpControlPort, used for troubleshooting RRCP using the rrdump tool.
     * 
     * @param tcpControlPort the tcpControlPort to set
     */
    public void tcpControlPort(String tcpControlPort);

    /**
     * The RRCP tcpControlPort, used for troubleshooting RRCP using the rrdump tool.
     * 
     * @return the tcpControlPort
     */
    public String tcpControlPort();
	
    /**
     * The RRCP portRoamRange.
     * 
     * @param portRoamRange the portRoamRange to set
     */
    public void portRoamRange(int portRoamRange);

    /**
     * The RRCP portRoamRange.
     * 
     * @return the portRoamRange
     */
    public int portRoamRange();
}