/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerClient_h
#define __thomsonreuters_ema_access_OmmConsumerClient_h

/**
	@class thomsonreuters::ema::access::OmmConsumerClient OmmConsumerClient.h "Access/Include/OmmConsumerClient.h"
	@brief OmmConsumerClient class provides callback interfaces to pass received messages.

	Application needs to implement an application client class inheriting from OmmConsumerClient.
	In its own class, application needs to override callback methods it desires to use for item processing.
	Default empty callback methods are implemented by OmmConsumerClient class.

	Application may chose to implement specific callbacks (e.g., onUpdateMsg()) or a general callback
	(e.g., onAllMsg()).

	\remark Thread safety of all the methods in this class depends on the user's implementation.

	@see OmmConsumer,
		Msg,
		AckMsg,
		GenericMsg,
		PostMsg,
		RefreshMsg,
		StatusMsg,
		UpdateMsg
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmConsumerEvent;
class Msg;
class RefreshMsg;
class UpdateMsg;
class StatusMsg;
class GenericMsg;
class AckMsg;

class EMA_ACCESS_API OmmConsumerClient
{
public :

	///@name Callbacks
	//@{
	/** Invoked upon receiving a refresh message. 
		Refresh message may be a start, interim or final part.
		@param[out] refreshMsg received refreshMsg
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& consumerEvent ) {}

	/** Invoked upon receiving an update message. 
		Update messages may be interlaced within a multiple part refresh message sequence.
		@param[out] updateMsg received updateMsg
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& consumerEvent ) {}

	/** Invoked upon receiving a status message. 
		@param[out] statusMsg received statusMsg
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& consumerEvent ) {}

	/** Invoked upon receiving any generic message.
		Application need only implement this method if a desire to process messages of type GenericMsg.
		@param[out] genericMsg received genericMsg
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent ) {}

	/** Invoked upon receiving any ack message.
		Application need only implement this method if a desire to process messages of type AckMsg.
		@param[out] ackMsg received ackMsg
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent ) {}

	/** Invoked upon receiving any message.
		Application need only implement this method if a desire to process all message types.
		@param[out] msg received message
		@param[out] consumerEvent identifies open item for which this message is received
		@return void
	*/
	virtual void onAllMsg( const Msg& msg, const OmmConsumerEvent& consumerEvent ) {}
	//@}

protected :

	OmmConsumerClient();
	virtual ~OmmConsumerClient();

private :

	OmmConsumerClient( const OmmConsumerClient& );
	OmmConsumerClient& operator=( const OmmConsumerClient& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerClient_h
