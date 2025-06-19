/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ChannelStatistics_h
#define __refinitiv_ema_access_ChannelStatistics_h

/**
   @class refinitiv::ema::access::ChannelStatistics ChannelStatistics.h "Access/Include/ChannelStatistics.h"
   @brief ChannelStats provides statistical information on application channel(s).

   For IProvider applications, this channel information is about channels used by
   clients to connect to the IProvider application.

   For Consumer and NiProvider applications, this channel information is about the
   outbound channel (e.g., the channel used by a Consumer application to connect to
   an ADS) used to connect for receiving data (Consumer) or sending data (NiProvider).

   Examples of ChannelStatistics usage are found in the example:
   (IProvider) IProvider/400_Series/460__MarketPrice__RTT
*/   

#include "Access/Include/EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

class EmaString;

class EMA_ACCESS_API ChannelStatistics
{
public:

  ///@name Constructor
  //@{
  /** Constructs ChannelStatistics.
   */
	ChannelStatistics();
  //@}
	

  ///@name Constructor
  //@{
  /** Constructs ChannelStatistics.
	  @param[in] tcpRetransmitCount
   */
	ChannelStatistics( const UInt64 tcpRetransmitCount );
  //@}

  ///@name Destructor
  //@{
  /** Destructor.
   */
  virtual ~ChannelStatistics();
  //@}

  ///@name Accessors
  //@{
  /** Clears the ChannelStatistics
	  \remark invoking clear() resets all member variables to their default values
  */
  void clear();

  /** Indicates presence of TCP Retransmition Count.
	  @return true if TCP Retransmition Count is set; false otherwise
  */
  bool hasTcpRetransmitCount() const { return _hasTcpRetransmitCount; }

  /** Gets the TCP Retransmition Count.
	  @throw OmmInvalidUsageException if hasTcpRetransmitCount() returns false
	  @return the TCP Retransmition Count
  */
  UInt64 getTcpRetransmitCount() const;
  
  //@}

  ///@name Operations
  /** Specifies port number that was used to connect to the server (Consumer, NiProvider)
	  @param[in] port specifies port number
	  @return reference to this object
   */
  ChannelStatistics& tcpRetransmitCount(const UInt64 tcpRetransmitCount);
  //@}

private:
	bool _hasTcpRetransmitCount;
	UInt64 _tcpRetransmitCount;
};

}

}

}

#endif //__refinitiv_ema_access_ChannelStatistics_h

