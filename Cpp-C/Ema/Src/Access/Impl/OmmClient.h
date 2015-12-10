/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmClient_h
#define __thomsonreuters_ema_access_OmmClient_h

#include "OmmConsumerClient.h"
#include "OmmConsumerEvent.h"
#include "OmmNiProviderClient.h"
#include "OmmNiProviderEvent.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ClientFunctions {
public:
  virtual void onRefreshMsg( const RefreshMsg&, const OmmConsumerEvent& ) {}
  virtual void onAllMsg( const Msg&, const OmmConsumerEvent& ) {}
  virtual void onStatusMsg( const StatusMsg&, const OmmConsumerEvent& ) {}
  virtual void onAckMsg( const AckMsg&, const OmmConsumerEvent& ) {}
  virtual void onGenericMsg( const GenericMsg&, const OmmConsumerEvent& ) {}
  virtual void onUpdateMsg( const UpdateMsg&, const OmmConsumerEvent& ) {}
  virtual ~ClientFunctions() {}
protected:
  ClientFunctions() {}
};

template< class C >
class OmmClient : public ClientFunctions {
public:
  virtual void onRefreshMsg( const RefreshMsg&, const OmmConsumerEvent& );
  virtual void onAllMsg( const Msg&, const OmmConsumerEvent& ) {}
  virtual void onStatusMsg( const StatusMsg&, const OmmConsumerEvent& ) {}
  virtual void onAckMsg( const AckMsg&, const OmmConsumerEvent& ) {}
  virtual void onGenericMsg( const GenericMsg&, const OmmConsumerEvent& ) {}
  virtual void onUpdateMsg( const UpdateMsg&, const OmmConsumerEvent& );
  OmmClient( C* c ) : _theClient( c ) {}
private:
  OmmClient();
  C* _theClient;
};

}

}

}

#endif
