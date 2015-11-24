/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "EmaRdm.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

const EmaString LoginDomainString( "Login Domain" );
const EmaString DirectoryDomainString( "Directory Domain" );
const EmaString DictionaryDomainString( "Dictionary Domain" );
const EmaString MarketPriceDomainString( "MarketPrice Domain" );
const EmaString MarketByOrderDomainString( "MarketByOrder Domain" );
const EmaString MarketByPriceDomainString( "MarketByPrice Domain" );
const EmaString MarketMakerDomainString( "MarketMaker Domain" );
const EmaString SymbolListDomainString( "SymbolList Domain" );
const EmaString ServiceProviderStatusDomainString( "ServiceProviderStatus Domain" );
const EmaString HistoryDomainString( "History Domain" );
const EmaString HeadlineDomainString( "Headline Domain" );
const EmaString StoryDomainString( "Story Domain" );
const EmaString ReplayHeadlineDomainString( "ReplayHeadline Domain" );
const EmaString ReplayHistoryDomainString( "ReplayHistory Domain" );
const EmaString TransactionDomainString( "Transaction Domain" );
const EmaString YieldCurveDomainString( "YieldCurve Domain" );
const EmaString ContributionDomainString( "Contribution Domain" );
const EmaString ProviderAdminDomainString( "ProviderAdmin Domain" );
const EmaString AnalyticsDomainString( "Analytics Domain" );
const EmaString ReferenceDomainString( "Reference Domain" );
const EmaString NewsTextAnalyticsDomainString( "NewsTextAnalytics Domain" );
const EmaString SystemDomainString( "System Domain" );

EmaString UnknownDomainString;

const EmaString& rdmDomainToString( UInt16 domain )
{
	switch ( domain )
	{
	case MMT_LOGIN :
		return LoginDomainString;
	case MMT_DIRECTORY :
		return DirectoryDomainString;
	case MMT_DICTIONARY :
		return DictionaryDomainString;
	case MMT_MARKET_PRICE :
		return MarketPriceDomainString;
	case MMT_MARKET_BY_ORDER :
		return MarketByOrderDomainString;
	case MMT_MARKET_BY_PRICE :
		return MarketByPriceDomainString;
	case MMT_MARKET_MAKER :
		return MarketMakerDomainString;
	case MMT_SYMBOL_LIST :
		return SymbolListDomainString;
	case MMT_SERVICE_PROVIDER_STATUS :
		return ServiceProviderStatusDomainString;
	case MMT_HISTORY :
		return HistoryDomainString;
	case MMT_HEADLINE :
		return HeadlineDomainString;
	case MMT_STORY :
		return StoryDomainString;
	case MMT_REPLAYHEADLINE :
		return ReplayHeadlineDomainString;
	case MMT_REPLAYSTORY :
		return ReplayHistoryDomainString;
	case MMT_TRANSACTION :
		return TransactionDomainString;
	case MMT_YIELD_CURVE :
		return YieldCurveDomainString;
	case MMT_CONTRIBUTION :
		return ContributionDomainString;
	case MMT_PROVIDER_ADMIN :
		return ProviderAdminDomainString;
	case MMT_ANALYTICS :
		return AnalyticsDomainString;
	case MMT_REFERENCE :
		return ReferenceDomainString;
	case MMT_NEWS_TEXT_ANALYTICS :
		return NewsTextAnalyticsDomainString;
	case MMT_SYSTEM :
		return SystemDomainString;
	default :
		return UnknownDomainString.set( "Unknown RDM Domain. Value='" ).append( (UInt32) domain ).append( "'" );
	}
}

const EmaString UserNameLoginNameTypeString( "UserName" );
const EmaString UserEmailAddressLoginNameTypeString( "UserEmailAddress" );
const EmaString UserTokenLoginNameTypeString( "UserToken" );
EmaString UnknownLoginNameTypeString;

const EmaString& loginNameTypeToString( UInt8 nameType )
{
	switch ( nameType )
	{
	case USER_NAME :
		return UserNameLoginNameTypeString;
	case USER_EMAIL_ADDRESS :
		return UserEmailAddressLoginNameTypeString;
	case USER_TOKEN :
		return UserTokenLoginNameTypeString;
	default :
		return UnknownLoginNameTypeString.set( "Unknown Login NameType. Value='" ).append( (UInt32)nameType ).append( "'" );
	}
}

