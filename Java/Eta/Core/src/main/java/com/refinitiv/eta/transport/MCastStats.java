/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Multicast statistics returned by {@link Channel#info(ChannelInfo, Error)} call.
 * 
 * @see ChannelInfo
 */
public interface MCastStats
{
    /**
     * This is the number of multicast packets sent by this channel.
     * 
     * @return the mcastSent
     */
    public long mcastSent();

    /**
     * This is the number of multicast packets received by this channel.
     * 
     * @return the mcastRcvd
     */
    public long mcastRcvd();

    /**
     * This is the number of unicast UDP packets sent by this channel.
     * 
     * @return the unicastSent
     */
    public long unicastSent();

    /**
     * This is the number of unicast UDP packets received by this channel.
     * 
     * @return the unicastRcvd
     */
    public long unicastRcvd();

    /**
     * This is the number of unrecoverable gaps that have been detected on this channel.
     * This value includes gaps detected for both multicast and unicast data.
     * Positive values indicate a possible network problem, more severe as value is larger
     * 
     * @return the gapsDetected
     */
    public long gapsDetected();

    /**
     * This is the number of retransmission requests sent by this channel,
     * populated only for reliable multicast connection types.
     * This value includes retransmit requests for both multicast and unicast data.
     * Positive values indicate a possible network problem, more severe as value is larger
     * 
     * @return the retransReqSent
     */
    public long retransReqSent();

    /**
     * This is the number of retransmission requests received by this channel,
     * populated only for reliable multicast connection types.
     * This value includes retransmit requests for both multicast and unicast data.
     * Positive values indicate a possible network problem, more severe as value is larger
     * 
     * @return the retransReqRcvd
     */
    public long retransReqRcvd();

    /**
     * This is the number of retransmitted packets sent by this channel,
     * populated only for reliable multicast connection types.
     * This value includes retransmit packets for both multicast and unicast data.
     * Positive values indicate a possible network problem, more severe as value is larger
     * 
     * @return the retransPktsSent
     */
    public long retransPktsSent();

    /**
     * This is the number of retransmitted packets received by this channel,
     * populated only for reliable multicast connection types.
     * This value includes retransmit packets for both multicast and unicast data.
     * Positive values indicate a possible network problem, more severe as value is larger
     * 
     * @return the retransPktsRcvd
     */
    public long retransPktsRcvd();
}
