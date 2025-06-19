/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.transport.MCastStats;

class MCastStatsImpl implements MCastStats
{
    private long _mcastSent;
    private long _mcastRcvd;
    private long _unicastSent;
    private long _unicastRcvd;
    private long _gapsDetected;
    private long _retransReqSent;
    private long _retransReqRcvd;
    private long _retransPktsSent;
    private long _retransPktsRcvd;

    MCastStatsImpl()
    {
    }
	
    void clear()
    {
        _mcastSent = 0;
        _mcastRcvd = 0;
        _unicastSent = 0;
        _unicastRcvd = 0;
        _gapsDetected = 0;
        _retransReqSent = 0;
        _retransReqRcvd = 0;
        _retransPktsSent = 0;
        _retransPktsRcvd = 0;
    }

    @Override
    public String toString()
    {
        return "MCastStats" + "\n" + 
               "\t\tmcastSent: " + _mcastSent + "\n" +
               "\t\tmcastRcvd: " + _mcastRcvd + "\n" + 
               "\t\tunicastSent: " + _unicastSent + "\n" + 
               "\t\tunicastRcvd: " + _unicastRcvd + "\n" +
               "\t\tgapsDetected: " + _gapsDetected + "\n" + 
               "\t\tretransReqSent: " + _retransReqSent + "\n" + 
               "\t\tretransReqRcvd: " + _retransReqRcvd + "\n" + 
               "\t\tretransPktsSent: " + _retransPktsSent + "\n" + 
               "\t\tretransPktsRcvd: " + _retransPktsRcvd;
    }

    public void mcastSent(long mcastSent)
    {
        _mcastSent = mcastSent;
    }

    @Override
    public long mcastSent()
    {
        return _mcastSent;
    }

    public void mcastRcvd(long mcastRcvd)
    {
        _mcastRcvd = mcastRcvd;
    }

    @Override
    public long mcastRcvd()
    {
        return _mcastRcvd;
    }

    public void unicastSent(long unicastSent)
    {
        _unicastSent = unicastSent;
    }

    @Override
    public long unicastSent()
    {
        return _unicastSent;
    }

    public void unicastRcvd(long unicastRcvd)
    {
        _unicastRcvd = unicastRcvd;
    }

    @Override
    public long unicastRcvd()
    {
        return _unicastRcvd;
    }

    public void gapsDetected(long gapsDetected)
    {
        _gapsDetected = gapsDetected;
    }

    @Override
    public long gapsDetected()
    {
        return _gapsDetected;
    }

    public void retransReqSent(long retransReqSent)
    {
        _retransReqSent = retransReqSent;
    }

    @Override
    public long retransReqSent()
    {
        return _retransReqSent;
    }

    public void retransReqRcvd(long retransReqRcvd)
    {
        _retransReqRcvd = retransReqRcvd;
    }

    @Override
    public long retransReqRcvd()
    {
        return _retransReqRcvd;
    }

    public void retransPktsSent(long retransPktsSent)
    {
        _retransPktsSent = retransPktsSent;
    }

    @Override
    public long retransPktsSent()
    {
        return _retransPktsSent;
    }

    public void retransPktsRcvd(long retransPktsRcvd)
    {
        _retransPktsRcvd = retransPktsRcvd;
    }

    @Override
    public long retransPktsRcvd()
    {
        return _retransPktsRcvd;
    }
}
