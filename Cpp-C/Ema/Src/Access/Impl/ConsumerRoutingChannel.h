/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
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
#include "ConsumerRoutingSession.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "LoginCallbackClient.h"

#ifndef __refinitiv_ema_access_SessionChannel_h
#define __refinitiv_ema_access_SessionChannel_h

namespace refinitiv {

	namespace ema {

		namespace access {
			typedef const EmaString* EmaStringPtr;
			class ConsumerRoutingSessionChannel;

			// This class defines the configuration of a single Reactor Channel in the Consumer Session
			// This class lifetime will be managed by the ActiveConfig, and all and all cleanup of ConsumerRoutingSessionChannelConfig objects will be through there.
			class ConsumerRoutingSessionChannelConfig
			{
				public:

				ConsumerRoutingSessionChannelConfig(const EmaString&, ActiveConfig&);
				virtual ~ConsumerRoutingSessionChannelConfig();


				void clear();

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

				// Preferred host
				bool            enablePreferredHostOptions;				// This defaults to false in all cases
				EmaString		phDetectionTimeSchedule;
				UInt32          phDetectionTimeInterval;
				EmaString       preferredChannelName;
				EmaString       preferredWSBChannelName;
				bool			phFallBackWithInWSBGroup;

				// Logger configuration.
				LoggerConfig		loggerConfig;
				bool				useActiveConfigLogger;  // If this is true, then all logging will use the configured active config logger and not the specified logger config here.
															// This should be set to true if: Logger is not specified in the config, or if the logger name matches the currently active logger config.

				
				ActiveConfig& activeConfig;

				// The following are used to configure the full channel, and this is what's used to pass into RsslReactorConnect.
				RsslReactorConnectOptions connectOpts;

				EmaVector< ChannelConfig* >		configChannelSet;
				EmaVector< WarmStandbyChannelConfig* >  configWarmStandbySet;
				EmaVector< ChannelConfig* >		configChannelSetForWSB;

				ConsumerRoutingSessionChannel* pRoutingChannel;

			protected:
				void clearChannelSet();
				void clearWSBChannelSet();
				void clearChannelSetForWSB();
				void clearReactorChannelConnectOpts();
			};

			// This class defines an instance of a single Reactor Channel in the Consumer Session
			// This is a rough equivalent of the Channel object
			// This class lifetime will be managed by the consumerRoutingSession class, and all cleanup of ConsumerRoutingSessionChannel objects will be through there.
			// Note for timing: We're 
			class ConsumerRoutingSessionChannel
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
				ConsumerRoutingSessionChannel(OmmBaseImpl&, const EmaString&, ConsumerRoutingSessionChannelConfig&);
				virtual ~ConsumerRoutingSessionChannel();

				void clear();
				EmaString name;
				
				ChannelList		channelList;

				bool receivedLoginRefresh;
				bool sentChannelUpStatus;			// Indicates if the initial CHANNEL_UP status message has been sent.
				bool reconnecting;					// Set when reconnecting, unset when connected.
				RsslReactorChannel*			pReactorChannel;
				LoginInfo	loginInfo;

				OmmBaseImpl&				baseImpl;
				ConsumerRoutingSessionChannelConfig& routingChannelConfig;

				ConsumerRoutingSession* pRoutingSession;

				OmmBaseImpl::ImplState  channelState;
				OmmLoggerClient*		pLoggerClient;

				bool					channelClosed;				// Indicates that the channel has been closed.  This is a boolean flag because there is a possibility that 
																	// the close may not be finished.

				bool					inPreferredHost;			// flag indicating that this session channel is currently in the preferred host operation, so do not attempt to reroute the requests

				UInt32					sessionIndex;				// Index in pRoutingSession->routingChannelList.

				bool					closeOnDownReconnecting;	// Close the channel when a DOWN_RECONNECTING event is received.  This is only set when the login is denied.


				// Hash tables and a direct list of services 
				HashTable<UInt16, DirectoryPtr, UInt16rHasher, UInt16Equal_To> serviceById;			// keyed by the concrete service for this channel
																									// Note: hashTable.find returns a **Directory pointer.
				HashTable<EmaStringPtr, DirectoryPtr, EmaStringPtrHasher, EmaStringPtrEqual_To> serviceByName;	// keyed by the concrete service for this channel

				EmaList< Directory* > serviceList;

				ItemList routedRequestList;			// ItemList of items routed to this channel.

				Channel* pCurrentActiveChannel;					// This will get set on a CHANNEL_UP channel callback, and will only be cleared when the channel is closed.

				void closeReactorChannel();

			
			};

		}

	}

}

#endif