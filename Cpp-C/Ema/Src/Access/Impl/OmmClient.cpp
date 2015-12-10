/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmClient.h"

using namespace thomsonreuters::ema::access;

namespace thomsonreuters { namespace ema { namespace access {

template<>
void OmmClient<OmmConsumerClient>::onRefreshMsg( const RefreshMsg& msg, const OmmConsumerEvent& event ) {
  _theClient->onRefreshMsg( msg, event );
}

template<>
void OmmClient<OmmConsumerClient>::onUpdateMsg( const UpdateMsg& msg, const OmmConsumerEvent& event ) {
  _theClient->onUpdateMsg( msg, event );
}

template<>
void OmmClient<OmmNiProviderClient>::onRefreshMsg( const RefreshMsg& msg, const OmmConsumerEvent& event ) {}

template<>
void OmmClient<OmmNiProviderClient>::onUpdateMsg( const UpdateMsg& msg, const OmmConsumerEvent& event ) {}

}}}
