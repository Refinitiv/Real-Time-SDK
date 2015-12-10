/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __User_Error_Handler__h
#define __User_Error_Handler__h

#include "OmmConsumerErrorClient.h"
#include "OmmNiProviderErrorClient.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class UserErrorHandler {
public:
  UserErrorHandler( OmmConsumerErrorClient& client )
    : _consumerErrorClient( &client ), _niProviderErrorClient( 0 ) {}
  UserErrorHandler( OmmNiProviderErrorClient& client ) 
    : _consumerErrorClient( 0 ), _niProviderErrorClient( &client ) {}
  bool hasUserErrorHandler() { return (_consumerErrorClient != 0 || _niProviderErrorClient != 0); }

  void onInaccessibleLogFile( const EmaString& filename, const EmaString& text )
  {
    if ( _consumerErrorClient )
      _consumerErrorClient->onInaccessibleLogFile( filename, text );
    else if ( _niProviderErrorClient )
      _niProviderErrorClient->onInaccessibleLogFile( filename, text );
  }

  void onInvalidHandle( UInt64 handle, const EmaString& text ) {
    if ( _consumerErrorClient )
      _consumerErrorClient->onInvalidHandle( handle, text );
    else if ( _niProviderErrorClient )
      _niProviderErrorClient->onInvalidHandle( handle, text );
  }

  virtual void onInvalidUsage( const EmaString& text)
  {
    if ( _consumerErrorClient )
      _consumerErrorClient->onInvalidUsage( text );
    else if ( _niProviderErrorClient )
      _niProviderErrorClient->onInvalidUsage( text );
  }

  virtual void onMemoryExhaustion( const EmaString& text)
  {
    if ( _consumerErrorClient )
      _consumerErrorClient->onMemoryExhaustion( text );
    else if ( _niProviderErrorClient )
      _niProviderErrorClient->onMemoryExhaustion( text );
  }

  virtual void onSystemError( Int64 code, void* ptr, const EmaString& text )
  {
    if ( _consumerErrorClient )
      _consumerErrorClient->onSystemError( code, ptr, text );
    else if ( _niProviderErrorClient )
      _niProviderErrorClient->onSystemError( code, ptr, text );
  }
  

private:
  UserErrorHandler() {}
  OmmConsumerErrorClient* _consumerErrorClient;
  OmmNiProviderErrorClient* _niProviderErrorClient;
};

}

}

}
#endif
