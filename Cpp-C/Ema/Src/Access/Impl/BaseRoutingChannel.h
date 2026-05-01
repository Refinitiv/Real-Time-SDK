/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 
 // These classes represent a the information for and config of a single Reactor Channel instance within the Consumer Session for the request routing concept.
 // The SessionChannel has containers for the Channels, ChannelConfig for the ChannelSet, WarmStandbyChannelConfig, and ChannelConfig for the various Warm Standby channels.
 // A Consumer Session will contain multiple instances of these classes.

#include "EmaConfigImpl.h"
#include "ActiveConfig.h"

#include "rtr/rsslTransport.h"
#include "rtr/rwfNet.h"
#include "rtr/rsslReactor.h"
#include "BaseRoutingSession.h"
#include "ChannelCallbackClient.h"
#include "LoginCallbackClient.h"

#ifndef __refinitiv_ema_access_BaseSessionChannel_h
#define __refinitiv_ema_access_BaseSessionChannel_h

namespace refinitiv {

	namespace ema {

		namespace access {
			typedef const EmaString* EmaStringPtr;
			class BaseRoutingSessionChannel;

			// This class defines the configuration of a single Reactor Channel in the Consumer Session
			// This class lifetime will be managed by the ActiveConfig, and all and all cleanup of ConsumerRoutingSessionChannelConfig objects will be through there.
			class BaseRoutingSessionChannelConfig
			{
				public:

				BaseRoutingSessionChannelConfig(const EmaString&, ActiveConfig&);
				virtual ~BaseRoutingSessionChannelConfig();


				virtual void clear();

				EmaString name;				// Name of the session channel configuration

				// For the configuration values below, all of these initialzied to the provided Active values, and overwritten if configured explicitly in XML or programmatically
				Int32			reconnectAttemptLimit;		
				Int32			reconnectMinDelay;			
				Int32			reconnectMaxDelay;			

				EmaString				xmlTraceFileName;			
				Int64					xmlTraceMaxFileSize;		
				bool					xmlTraceToFile;				
				bool					xmlTraceToStdout;			
				bool					xmlTraceToMultipleFiles;
				bool					xmlTraceWrite;
				bool					xmlTraceRead;
				bool					xmlTracePing;
				bool					xmlTracePingOnly;
				bool					xmlTraceHex;
				bool					xmlTraceDump;

				// Logger configuration.
				LoggerConfig		loggerConfig;
				bool				useActiveConfigLogger;  // If this is true, then all logging will use the configured active config logger and not the specified logger config here.
															// This should be set to true if: Logger is not specified in the config, or if the logger name matches the currently active logger config.

				ActiveConfig& activeConfig;

				// The following are used to configure the full channel, and this is what's used to pass into RsslReactorConnect.
				RsslReactorConnectOptions connectOpts;

				EmaVector< ChannelConfig* >		configChannelSet;

				BaseRoutingSessionChannel* pRoutingChannel;

			protected:
				void clearChannelSet();
				void clearReactorChannelConnectOpts();
			};

			// This class defines an instance of a single Reactor Channel in the Consumer Session
			// This is a rough equivalent of the Channel object
			// This class lifetime will be managed by the baseRoutingSession class, and all cleanup of BaseRoutingSessionChannel objects will be through there.
			// Note for timing: We're 
			class BaseRoutingSessionChannel
			{
			protected:
				class UInt16rHasher
				{
				public:
					size_t operator()(const UInt16&) const;
				};

				class UInt16Equal_To
				{
				public:
					bool operator()(const UInt16&, const UInt16&) const;
				};

				class EmaStringPtrHasher
				{
				public:
					size_t operator()(const EmaStringPtr&) const;
				};

				class EmaStringPtrEqual_To
				{
				public:
					bool operator()(const EmaStringPtr&, const EmaStringPtr&) const;
				};

			public:
				BaseRoutingSessionChannel(OmmBaseImpl&, const EmaString&, BaseRoutingSessionChannelConfig&);
				virtual ~BaseRoutingSessionChannel();

				virtual void clear();
				EmaString name;
				
				ChannelList		channelList;

				bool receivedLoginRefresh;
				bool sentChannelUpStatus;			// Indicates if the initial CHANNEL_UP status message has been sent.
				bool reconnecting;					// Set when reconnecting, unset when connected.
				RsslReactorChannel*			pReactorChannel;
				LoginInfo	loginInfo;

				bool					inPreferredHost;			// flag indicating that this session channel is currently in the preferred host operation, so do not attempt to reroute the requests
																	// This is only used with Consumer channels

				OmmBaseImpl&				baseImpl;

				OmmBaseImpl::ImplState  channelState;
				OmmLoggerClient*		pLoggerClient;

				bool					channelClosed;				// Indicates that the channel has been closed.  This is a boolean flag because there is a possibility that 
																	// the close may not be finished.

				UInt32					sessionIndex;				// Index in pRoutingSession->routingChannelList.

				bool					closeOnDownReconnecting;	// Close the channel when a DOWN_RECONNECTING event is received.  This is only set when the login is denied.

				Channel* pCurrentActiveChannel;					// This will get set on a CHANNEL_UP channel callback, and will only be cleared when the channel is closed.

				BaseRoutingSessionChannelConfig& routingChannelConfig;
				BaseRoutingSession* pRoutingSession;


				void closeReactorChannel();
			};

		}

	}

}

#endif