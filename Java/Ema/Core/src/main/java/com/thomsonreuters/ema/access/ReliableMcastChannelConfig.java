///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.transport.ConnectionTypes;

class ReliableMcastChannelConfig extends ChannelConfig
{
	String				recvAddress;
	String				recvServiceName;
	String				unicastServiceName;
	String				sendAddress;
	String				sendServiceName;
	String				tcpControlPort;
	String				hsmInterface;
	String				hsmMultAddress;
	String				hsmPort;
	int					hsmInterval;
	boolean				disconnectOnGap;
	int					packetTTL;
	int					ndata;
	int					nmissing;
	int					nrreq;
	int					tdata;
	int					trreq;
	int					twait;
	int					tbchold;
	int					tpphold;
	int					pktPoolLimitHigh;
	int					pktPoolLimitLow;
	int					userQLimit;

	ReliableMcastChannelConfig()
	{
		clear();
	}

	@Override
	void clear()
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.RELIABLE_MCAST;
		recvAddress = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		recvServiceName = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		unicastServiceName = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		sendAddress = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		sendServiceName = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmInterface = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		tcpControlPort = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmMultAddress = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmPort = ActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmInterval = 0;
		packetTTL	= ActiveConfig.DEFAULT_PACKET_TTL;
	    ndata = ActiveConfig.DEFAULT_NDATA;
	    nmissing = ActiveConfig.DEFAULT_NMISSING;
	    nrreq = ActiveConfig.DEFAULT_NREQ;
	    pktPoolLimitHigh = ActiveConfig.DEFAULT_PKT_POOLLIMIT_HIGH;
	    pktPoolLimitLow = ActiveConfig.DEFAULT_PKT_POOLLIMIT_LOW;
	    tdata = ActiveConfig.DEFAULT_TDATA;
	    trreq = ActiveConfig.DEFAULT_TRREQ;
	    twait = ActiveConfig.DEFAULT_TWAIT;
	    tbchold = ActiveConfig.DEFAULT_TBCHOLD;
	    tpphold = ActiveConfig.DEFAULT_TPPHOLD;
	    userQLimit = ActiveConfig.DEFAULT_USER_QLIMIT;
		disconnectOnGap = false;
	}
}

