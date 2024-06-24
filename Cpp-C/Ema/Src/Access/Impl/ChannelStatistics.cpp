/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2020 LSEG. All rights reserved.                    --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelStatsImpl.h"
#include "ChannelStatistics.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

ChannelStatistics::ChannelStatistics()
{
  clear();
}

ChannelStatistics::ChannelStatistics( const UInt64 tcpRetransmitCount ) :
	_tcpRetransmitCount( tcpRetransmitCount )
 {
 }

ChannelStatistics::~ChannelStatistics()
{
}

void ChannelStatistics::clear()
{
	_hasTcpRetransmitCount = false;
	_tcpRetransmitCount = 0;
}

UInt64 ChannelStatistics::getTcpRetransmitCount() const
{
	if (!hasTcpRetransmitCount())
	{
		EmaString temp("Attempt to getTcpRetransmitCount() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return _tcpRetransmitCount;
}

ChannelStatistics& ChannelStatistics::tcpRetransmitCount( UInt64 cpRetransmitCount )
{
	_hasTcpRetransmitCount = true;
	_tcpRetransmitCount = cpRetransmitCount;
	return *this;
}

/* cs has been cleared and calling function has verified that channel arguments are non-null.
 * Calling function has also updated hostname and ipAddress
 */
void refinitiv::ema::access::getChannelStats(const RsslReactorChannel* rsslReactorChannel,
												 ChannelStatistics& cs) {
	// create channel stats info
	RsslErrorInfo rsslErrorInfo;
	RsslReactorChannelStats rsslReactorChannelStats;

	cs.clear();

	// if channel is closed, rsslReactorGetChannelStats does not succeed
	if (rsslReactorGetChannelStats(const_cast<RsslReactorChannel*>(rsslReactorChannel),
		&rsslReactorChannelStats, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		EmaString temp("Call to rsslReactorGetChannelStats() failed. Internal sysError='");
		temp.append(rsslErrorInfo.rsslError.sysError)
			.append("' Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append("' ")
			.append("' Error Location='").append(rsslErrorInfo.errorLocation).append("' ")
			.append("' Error text='").append(rsslErrorInfo.rsslError.text).append("'. ");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	if (rsslReactorChannelStats.rsslChannelStats.tcpStats.flags & RSSL_TCP_STATS_RETRANSMIT)
	{
		cs.tcpRetransmitCount(rsslReactorChannelStats.rsslChannelStats.tcpStats.tcpRetransmitCount);
	}
}
