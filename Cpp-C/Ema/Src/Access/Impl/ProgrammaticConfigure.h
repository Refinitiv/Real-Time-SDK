/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ProgrammaticConfigure_h
#define __thomsonreuters_ema_access_ProgrammaticConfigure_h

#include "OmmConsumerConfig.h"
#include "Map.h"
#include "EmaVector.h"
#include "EmaList.h"
#include "ConfigErrorHandling.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ReliableMcastChannelConfig;

class ChannelConfig;

class ProgrammaticConfigure
{
public:
	ProgrammaticConfigure(const Map &, EmaConfigErrorList& );

	void addConfigure(const Map &);

	bool getDefaultConsumer(EmaString &);

	bool specifyConsumerName ( const EmaString& consumerName );

	bool getActiveChannelName( const EmaString&, EmaString& );

	bool getActiveChannelSet( const EmaString&, EmaString& );

	bool getActiveLoggerName( const EmaString&, EmaString& );

	bool getActiveDictionaryName( const EmaString&, EmaString& );

	void retrieveUserConfig( const EmaString&, ActiveConfig& );

	void retrieveChannelConfig( const EmaString&, ActiveConfig&, bool, ChannelConfig* fileCfg = 0 );

	void retrieveLoggerConfig( const EmaString&, ActiveConfig& );

	void retrieveDictionaryConfig( const EmaString&, ActiveConfig& );

private:
	static void retrieveGroupAndListName( const Map&, EmaString& groupName, EmaString& listName );

	static bool retrieveDefaultConsumer(const Map &, EmaString&);

	static void retrieveDependencyNames( const Map&, const EmaString&, UInt8& flags, EmaString&, EmaString&, EmaString&, EmaString&);

	static void retrieveUser( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	static void retrieveChannel( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, bool, ChannelConfig* );

	static void retrieveChannelInfo(const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, bool, ChannelConfig* );

	static bool setReliableMcastChannelInfo( ReliableMcastChannelConfig *, UInt64& flags, ReliableMcastChannelConfig &, EmaString&, ChannelConfig * );

	static void retrieveLogger( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	static void retrieveDictionary( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	static bool validateConsumerName(const Map &, const EmaString&);

	void clear();

	ProgrammaticConfigure(const ProgrammaticConfigure &);
	ProgrammaticConfigure & operator=(const ProgrammaticConfigure&);

	EmaString _consumerName;
	EmaString _channelName;
	EmaString _loggerName;
	EmaString _dictionaryName;
	EmaString _channelSet;

	bool _overrideConsName;
	bool _loadnames;
	UInt8 _nameflags;

	EmaVector<const Map*> _configList;

	EmaConfigErrorList& _emaConfigErrList;
};

}

}

}

#endif // __thomsonreuters_ema_access_ProgrammaticConfigure_h
