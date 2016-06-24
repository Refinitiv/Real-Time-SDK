/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ProgrammaticConfigure_h
#define __thomsonreuters_ema_access_ProgrammaticConfigure_h

#include "Map.h"
#include "EmaVector.h"
#include "EmaList.h"
#include "ConfigErrorHandling.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ActiveConfig;
class ReliableMcastChannelConfig;
class ChannelConfig;

class ProgrammaticConfigure
{
public:

	ProgrammaticConfigure( const Map&, EmaConfigErrorList& );

	void addConfigure( const Map& );

	bool getDefaultConsumer( EmaString& );

	bool getDefaultNiProvider( EmaString& );

	bool specifyConsumerName( const EmaString& consumerName );

	bool specifyNiProviderName( const EmaString& );

	bool getActiveChannelName( const EmaString&, EmaString& );

	bool getActiveChannelSet( const EmaString&, EmaString& );

	bool getActiveLoggerName( const EmaString&, EmaString& );

	bool getActiveDictionaryName( const EmaString&, EmaString& );

	bool getActiveDirectoryName( const EmaString&, EmaString& );

	void retrieveCommonConfig( const EmaString&, ActiveConfig& );

	void retrieveCustomConfig( const EmaString&, ActiveConfig& );

	void retrieveChannelConfig( const EmaString&, ActiveConfig&, bool, ChannelConfig* fileCfg = 0 );

	void retrieveLoggerConfig( const EmaString&, ActiveConfig& );

	void retrieveDictionaryConfig( const EmaString&, ActiveConfig& );

	void retrieveDirectoryConfig( const EmaString&, ActiveConfig& );

	void clear();

private:

	void retrieveGroupAndListName( const Map&, EmaString& groupName, EmaString& listName );

	bool retrieveDefaultConsumer( const Map&, EmaString& );

	bool retrieveDefaultNiProvider( const Map&, EmaString& );

	void retrieveDependencyNames( const Map&, const EmaString&, UInt8& flags, EmaString&, EmaString&, EmaString&, EmaString& , EmaString& );

	void retrieveInstanceCommonConfig( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	void retrieveInstanceCustomConfig( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	void retrieveChannel( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, bool, ChannelConfig* );

	void retrieveChannelInfo( const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, bool, ChannelConfig* );

	bool setReliableMcastChannelInfo( ReliableMcastChannelConfig*, UInt64& flags, ReliableMcastChannelConfig&, EmaString&, ChannelConfig* );

	void retrieveLogger( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	void retrieveDictionary( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	void retrieveDirectory( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	bool validateConsumerName( const Map&, const EmaString& );

	bool validateNiProviderName( const Map&, const EmaString& );

	ProgrammaticConfigure( const ProgrammaticConfigure& );

	ProgrammaticConfigure& operator=( const ProgrammaticConfigure& );

	EmaString	_consumerName;
	EmaString	_niProviderName;
	EmaString	_channelName;
	EmaString	_loggerName;
	EmaString	_dictionaryName;
	EmaString	_directoryName;
	EmaString	_channelSet;

	bool		_overrideConsName;
	bool		_overrideNiProvName;
	bool		_dependencyNamesLoaded;
	UInt8		_nameflags;

	EmaVector<const Map*>	_configList;
	EmaConfigErrorList&		_emaConfigErrList;
};

}

}

}

#endif // __thomsonreuters_ema_access_ProgrammaticConfigure_h
