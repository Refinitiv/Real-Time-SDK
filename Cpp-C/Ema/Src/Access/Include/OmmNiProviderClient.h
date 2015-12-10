/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommNiProviderClient_h
#define __thomsonreuters_ema_access_ommNiProviderClient_h

/**
	@class thomsonreuters::ema::access::OmmNiProviderClient OmmNiProviderClient.h "Access/Include/OmmNiProviderClient.h"
	@brief OmmNiProviderClient class provides callback interfaces to pass received messages.

	Application needs to implement an application client class inheriting from OmmNiProviderClient.
	In its own class, application needs to override callback methods it desires to use for item processing.
	Default empty callback methods are implemented by OmmNiProviderClient class.

	Application may chose to implement specific callbacks (e.g., onUpdateMsg()) or a general callback
	(e.g., onAllMsg()).

	\remark Thread safety of all the methods in this class depends on the user's implementation.

	The following code snippet shows basic usage of OmmNiProviderClient class to print recevied messages to screen.

	\code

	class AppClient : public OmmNiProviderClient
	{
		void onRefreshMsg( const RefreshMsg& , const OmmNiProviderEvent& );
		void onUpdateMsg( const UpdateMsg& , const OmmNiProviderEvent& );
		void onStatusMsg( const StatusMsg& , const OmmNiProviderEvent&);
	};

	void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmNiProviderEvent& ) 
	{
		cout << "Handle " << event.getHandle() << endl
			<< "Closure " << event.getClosure() << endl
			<< refreshMsg << endl;
	}

	void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmNiProviderEvent& ) 
	{
		cout << "Handle " << event.getHandle() << endl
			<< "Closure " << event.getClosure() << endl
			<< updateMsg << endl;
	}

	void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmNiProviderEvent& ) 
	{
		cout << "Handle " << event.getHandle() << endl
			<< "Closure " << event.getClosure() << endl
			<< statusMsg << endl;
	}

	\endcode

	@see OmmNiProvider,
		Msg,
		AckMsg,
		GenericMsg,
		RefreshMsg,
		StatusMsg,
		UpdateMsg
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmNiProviderEvent;
class Msg;
class RefreshMsg;
class UpdateMsg;
class StatusMsg;
class GenericMsg;

class EMA_ACCESS_API OmmNiProviderClient
{
public :

	///@name Callbacks
	//@{
	/** Invoked upon receiving a refresh message. 
		Refresh message may be a start, interim or final part.
		@param[out] refreshMsg received refreshMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onRefreshMsg( const RefreshMsg& refreshMsg, const OmmNiProviderEvent& event ) {}

	/** Invoked upon receiving an update message. 
		Update messages may be interlaced within a multiple part refresh message sequence.
		@param[out] updateMsg received updateMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onUpdateMsg( const UpdateMsg& updateMsg, const OmmNiProviderEvent& event ) {}

	/** Invoked upon receiving a status message. 
		@param[out] statusMsg received statusMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onStatusMsg( const StatusMsg& statusMsg, const OmmNiProviderEvent& event ) {}

	/** Invoked upon receiving any generic message.
		Application need only implement this method if a desire to process messages of type GenericMsg.
		@param[out] genericMsg received genericMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onGenericMsg( const GenericMsg& genericMsg, const OmmNiProviderEvent& event ) {}

	/** Invoked upon receiving any message.
		Application need only implement this method if a desire to process all message types.
		@param[out] msg received message
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onAllMsg( const Msg& msg, const OmmNiProviderEvent& event ) {}
	//@}

protected :

	OmmNiProviderClient();
	virtual ~OmmNiProviderClient();

private :

	OmmNiProviderClient( const OmmNiProviderClient& );
	OmmNiProviderClient& operator=( const OmmNiProviderClient& );
};

}

}

}

#endif //__thomsonreuters_ema_access_ommNiProviderClient_h
