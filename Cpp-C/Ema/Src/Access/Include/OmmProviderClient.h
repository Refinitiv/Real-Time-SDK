/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommProviderClient_h
#define __thomsonreuters_ema_access_ommProviderClient_h

/**
	@class thomsonreuters::ema::access::OmmProviderClient OmmProviderClient.h "Access/Include/OmmProviderClient.h"
	@brief OmmProviderClient class provides callback interfaces to pass received messages.

	Application may need to implement an application client class inheriting from OmmProviderClient.
	In its own class, application needs to override callback methods it desires to use for item processing.
	Default empty callback methods are implemented by OmmProviderClient class.

	Application may chose to implement specific callbacks (e.g., onRefreshMsg()) or a general callback
	(e.g., onAllMsg()).

	\remark Thread safety of all the methods in this class depends on the user's implementation.

	The following code snippet shows basic usage of OmmProviderClient class to print recevied messages to screen.

	\code

	class AppClient : public OmmProviderClient
	{
		void onRefreshMsg( const RefreshMsg& , const OmmProviderEvent& );
		void onStatusMsg( const StatusMsg& , const OmmProviderEvent& );
	};

	void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& event ) 
	{
		cout << "Handle " << event.getHandle() << endl
			<< "Closure " << event.getClosure() << endl
			<< refreshMsg << endl;
	}

	void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& event ) 
	{
		cout << "Handle " << event.getHandle() << endl
			<< "Closure " << event.getClosure() << endl
			<< statusMsg << endl;
	}

	\endcode

	@see OmmProvider,
		Msg,
		AckMsg,
		GenericMsg,
		RefreshMsg,
		StatusMsg
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmProviderEvent;
class Msg;
class RefreshMsg;
class StatusMsg;
class GenericMsg;
class PostMsg;
class ReqMsg;

class EMA_ACCESS_API OmmProviderClient
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
	virtual void onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& event ) {}

	/** Invoked upon receiving a status message. 
		@param[out] statusMsg received statusMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& event ) {}

	/** Invoked upon receiving a generic message.
		Application need only implement this method if a desire to process messages of type GenericMsg.
		@param[out] genericMsg received genericMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onGenericMsg( const GenericMsg& genericMsg, const OmmProviderEvent& event ) {}

	/** Invoked upon receiving any message.
		Application need only implement this method if a desire to process all message types.
		@param[out] msg received message
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onAllMsg( const Msg& msg, const OmmProviderEvent& event ) {}

	// IProv

	/** invoked upon receiving a post message.
		Application need only implement this method if a desire to process messages of type PostMsg.
		@param[out] postMsg received postcMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onPostMsg( const PostMsg& postMsg, const OmmProviderEvent& event ) {}

	/** invoked upon receiving an initial item request message.
		Application need only implement this method if a desire to process intial messages of type ReqMsg.
		@param[out] reqMsg received reqMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& event ) {}

	/** invoked upon receiving a reissue request.
		Application need only implement this method if a desire to process reissue messages of type ReqMsg.
		@param[out] reqMsg received reqMsg
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onReissue( const ReqMsg& reqMsg, const OmmProviderEvent& event ) {}

	/** invoked upon receiving a close request message.
		Application need only implement this method if a desire to process item close messages.
		@param[out] event identifies open item for which this message is received
		@return void
	*/
	virtual void onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event) {}
	//@}

protected :

	OmmProviderClient();
	virtual ~OmmProviderClient();

private :

	OmmProviderClient( const OmmProviderClient& );
	OmmProviderClient& operator=( const OmmProviderClient& );
};

}

}

}

#endif //__thomsonreuters_ema_access_ommProviderClient_h
