/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the projects LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProvider.h"
#include "OmmNiProviderImpl.h"
#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"
#include "LoginCallbackClient.h"
#include "RefreshMsgEncoder.h"
#include "UpdateMsgEncoder.h"
#include "StatusMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "Utilities.h"
#include "EmaRdm.h"
#include "OmmQosDecoder.h"

#include <ctype.h>
#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

OmmNiProviderImpl::OmmNiProviderImpl( const OmmNiProviderConfig& config ) :
	_activeConfig(),
	OmmBaseImpl( _activeConfig ),
	_handleToStreamInfo(),
	_serviceNameToServiceId(),
	_serviceIdToServiceName(),
	_serviceNameList(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted( false )
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();
	initialize( config._pImpl );

	_handleToStreamInfo.rehash( _activeConfig.itemCountHint );
	_serviceNameToServiceId.rehash( _activeConfig.serviceCountHint );
	_serviceIdToServiceName.rehash( _activeConfig.serviceCountHint );
}

OmmNiProviderImpl::OmmNiProviderImpl( const OmmNiProviderConfig& config, OmmProviderErrorClient& client ) :
	_activeConfig(),
	OmmBaseImpl( _activeConfig, client ),
	_handleToStreamInfo(),
	_serviceNameToServiceId(),
	_serviceIdToServiceName(),
	_serviceNameList(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted( false )
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();
	initialize( config._pImpl );

	_handleToStreamInfo.rehash( _activeConfig.itemCountHint );
	_serviceNameToServiceId.rehash( _activeConfig.serviceCountHint );
	_serviceIdToServiceName.rehash( _activeConfig.serviceCountHint );
}

OmmNiProviderImpl::~OmmNiProviderImpl()
{
	OmmBaseImpl::uninitialize( false, false );

	removeItems();
}

void OmmNiProviderImpl::removeItems()
{
	_bIsStreamIdZeroRefreshSubmitted = false;

	_serviceIdToServiceName.clear();
	_serviceNameToServiceId.clear();

	_handleToStreamInfo.clear();

	for ( UInt32 idx = 0; idx < _serviceNameList.size(); ++idx )
		if ( _serviceNameList[idx] )
			delete _serviceNameList[idx];

	_serviceNameList.clear();

	for ( UInt32 idx = 0; idx < _streamInfoList.size(); ++idx )
		if ( _streamInfoList[idx] )
			delete _streamInfoList[idx];

	_streamInfoList.clear();
}

void OmmNiProviderImpl::populateDefaultService( ServiceConfig& service ) const
{
	service.infoFilter.acceptingConsumerStatus = 0;
	service.infoFilter.capabilities.push_back( 6 );
	service.infoFilter.capabilities.push_back( 7 );
	service.infoFilter.capabilities.push_back( 8 );
	service.infoFilter.capabilities.push_back( 9 );
	service.infoFilter.dictionariesUsed.push_back( "RWFFld" );
	service.infoFilter.dictionariesUsed.push_back( "RWFEnum" );
	service.infoFilter.dictionariesProvided.clear();
	service.infoFilter.isSource = 0;
	RsslQos rsslQos;
	rsslClearQos( &rsslQos );
	rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
	service.infoFilter.qos.push_back( rsslQos );
	service.infoFilter.serviceId = 0;
	service.infoFilter.supportsOutOfBandSnapshots = 0;
	service.infoFilter.supportsQosRange = 0;
	service.infoFilter.vendorName.clear();
	service.infoFilter.itemList.clear();
	service.serviceName = "NI_PUB";
	service.stateFilter.acceptingRequests = 0;
	service.stateFilter.isStatusConfigured = false;
	service.stateFilter.serviceState = 1;
}

void OmmNiProviderImpl::readCustomConfig( EmaConfigImpl* pConfigImpl )
{
	pConfigImpl->getDirectoryName( _activeConfig.configuredName, _activeConfig.directoryConfig.directoryName );

	if ( _activeConfig.directoryConfig.directoryName.empty() )
		pConfigImpl->get<EmaString>( "DirectoryGroup|DefaultDirectory", _activeConfig.directoryConfig.directoryName );

	if ( _activeConfig.directoryConfig.directoryName.empty() )
		pConfigImpl->get< EmaString >( "DirectoryGroup|DirectoryList|Directory|Name", _activeConfig.directoryConfig.directoryName );

	if ( _activeConfig.directoryConfig.directoryName.empty() )
	{
		ServiceConfig service;
		populateDefaultService( service );
		_activeConfig.directoryConfig.addService( service );
	}
	else
	{
		EmaString directoryNodeName( "DirectoryGroup|DirectoryList|Directory." );
		directoryNodeName.append( _activeConfig.directoryConfig.directoryName ).append( "|" );

		EmaString name;
		if ( !pConfigImpl->get< EmaString >( directoryNodeName + "Name", name ) )
		{
			EmaString errorMsg( "no configuration exists for ni provider directory [" );
			errorMsg.append( directoryNodeName ).append( "]. Will use directory defaults" );
			pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

			ServiceConfig service;
			populateDefaultService( service );
			_activeConfig.directoryConfig.addService( service );
		}
		else
		{
			EmaVector< EmaString > serviceNames;

			pConfigImpl->getServiceNames( _activeConfig.directoryConfig.directoryName, serviceNames );

			if ( serviceNames.empty() )
			{
				EmaString errorMsg( "specified ni provider directory [" );
				errorMsg.append( directoryNodeName ).append( "] contains no services. Will use directory defaults" );
				pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

				ServiceConfig service;
				populateDefaultService( service );
				_activeConfig.directoryConfig.addService( service );
			}
			else
			{
				const UInt16 maxUInt16 = 0xFFFF;

				if ( serviceNames.size() > maxUInt16 )
				{
					EmaString errorMsg( "Number of configured services is greater than allowed maximum. Some services will be dropped." );
					pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
				}

				EmaVector< UInt16 > validConfiguredServiceIds;
				for ( UInt32 idx = 0; idx < serviceNames.size() && idx < maxUInt16; ++idx )
				{
					UInt64 tempUInt64 = 0;
					EmaString serviceNodeName( directoryNodeName + "Service." + serviceNames[idx] + "|" );
					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "InfoFilter|ServiceId", tempUInt64 ) )
						if ( tempUInt64 <= maxUInt16 )
						{
							if ( -1 == validConfiguredServiceIds.getPositionOf( (UInt16) tempUInt64 ) )
								validConfiguredServiceIds.push_back( (UInt16) tempUInt64 );
						}
				}

				EmaVector< UInt16 > usedServiceIds;
				UInt32 emaAssignedServiceId = 0;
				for ( UInt32 idx = 0; idx < serviceNames.size() && idx < maxUInt16; ++idx )
				{
					ServiceConfig service;

					service.serviceName = serviceNames[idx];

					EmaString serviceNodeName( directoryNodeName + "Service." + serviceNames[idx] + "|" );

					UInt64 tempUInt64 = 0;
					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "StateFilter|ServiceState", tempUInt64 ) )
						service.stateFilter.serviceState = tempUInt64 > 0 ? 1 : 0;
					else
						service.stateFilter.serviceState = 1;

					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "StateFilter|AcceptingRequests", tempUInt64 ) )
						service.stateFilter.acceptingRequests = tempUInt64 > 0 ? 1 : 0;
					else
						service.stateFilter.acceptingRequests = 1;

					service.stateFilter.isStatusConfigured = false;

					OmmState::StreamState tempStreamState;
					if ( pConfigImpl->get< OmmState::StreamState >( serviceNodeName + "StateFilter|Status|StreamState", tempStreamState ) )
					{
						service.stateFilter.status.streamState = tempStreamState;
						service.stateFilter.isStatusConfigured = true;
					}
					else
						service.stateFilter.status.streamState = OmmState::OpenEnum;

					OmmState::DataState tempDataState;
					if ( pConfigImpl->get< OmmState::DataState >( serviceNodeName + "StateFilter|Status|DataState", tempDataState ) )
					{
						service.stateFilter.status.dataState = tempDataState;
						service.stateFilter.isStatusConfigured = true;
					}
					else
						service.stateFilter.status.dataState = OmmState::OkEnum;

					OmmState::StatusCode tempStatusCode;
					if ( pConfigImpl->get< OmmState::StatusCode >( serviceNodeName + "StateFilter|Status|StatusCode", tempStatusCode ) )
					{
						service.stateFilter.status.code = tempStatusCode;
						service.stateFilter.isStatusConfigured = true;
					}
					else
						service.stateFilter.status.code = OmmState::NoneEnum;

					if ( pConfigImpl->get< EmaString >( serviceNodeName + "StateFilter|Status|StatusText", service.stateFilter.statusText ) )
					{
						service.stateFilter.status.text.data = (char*) service.stateFilter.statusText.c_str();
						service.stateFilter.status.text.length = service.stateFilter.statusText.length();
						service.stateFilter.isStatusConfigured = true;
					}
					else
						rsslClearBuffer( &service.stateFilter.status.text );

					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "InfoFilter|ServiceId", tempUInt64 ) )
					{
						if ( tempUInt64 > maxUInt16 )
						{
							EmaString errorMsg( "service [" );
							errorMsg.append( serviceNodeName ).append( "] specifies out of range ServiceId (value of " ).append( tempUInt64 ).append( "). Will drop this service." );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
							continue;
						}

						service.infoFilter.serviceId = (UInt16) tempUInt64;
						if ( usedServiceIds.getPositionOf( service.infoFilter.serviceId ) > -1 )
						{
							EmaString errorMsg( "service [" );
							errorMsg.append( serviceNodeName ).append( "] specifies the same ServiceId (value of " ).append( tempUInt64 ).append( ") as already specified by another service. Will drop this service." );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
							continue;
						}

						usedServiceIds.push_back( service.infoFilter.serviceId );
					}
					else
					{
						while ( emaAssignedServiceId <= maxUInt16 &&
							validConfiguredServiceIds.getPositionOf( emaAssignedServiceId ) > -1 )
							++emaAssignedServiceId;

						if ( emaAssignedServiceId > maxUInt16 )
						{
							EmaString errorMsg( "EMA ran out of assignable service ids. Will drop rest of the services" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
							break;
						}

						EmaString errorMsg( "service [" );
						errorMsg.append( serviceNodeName ).append( "] contains no ServiceId. EMA will assign a value of " ).append( emaAssignedServiceId );
						pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

						service.infoFilter.serviceId = (UInt16) emaAssignedServiceId;
						usedServiceIds.push_back( service.infoFilter.serviceId );
					}

					if ( !pConfigImpl->get< EmaString >( serviceNodeName + "InfoFilter|Vendor", service.infoFilter.vendorName ) )
						service.infoFilter.vendorName.clear();

					if ( !pConfigImpl->get< EmaString >( serviceNodeName + "InfoFilter|ItemList", service.infoFilter.itemList ) )
						service.infoFilter.itemList.clear();

					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "InfoFilter|IsSource", tempUInt64 ) )
						service.infoFilter.isSource = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.isSource = 0;

					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "InfoFilter|SupportsQoSRange", tempUInt64 ) )
						service.infoFilter.supportsQosRange = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.supportsQosRange = 0;

					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "InfoFilter|SupportsOutOfBandSnapshots", tempUInt64 ) )
						service.infoFilter.supportsOutOfBandSnapshots = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.supportsOutOfBandSnapshots = 0;

					if ( pConfigImpl->get< UInt64 >( serviceNodeName + "InfoFilter|AcceptingConsumerStatus", tempUInt64 ) )
						service.infoFilter.acceptingConsumerStatus = tempUInt64 > 0 ? 1 : 0;
					else
						service.infoFilter.acceptingConsumerStatus = 0;

					EmaVector< EmaString > valueList;
					pConfigImpl->getAsciiAttributeValueList( serviceNodeName + "InfoFilter|DictionariesProvided", "DictionariesProvidedEntry", valueList );

					for ( UInt32 idx = 0; idx < valueList.size(); ++idx )
					{
						EmaString dictionaryNodeName( "DictionaryGroup|DictionaryList|Dictionary." );
						dictionaryNodeName.append( valueList[idx] ).append( "|" );

						EmaString name;
						if ( !pConfigImpl->get< EmaString >( dictionaryNodeName + "Name", name ) )
						{
							EmaString errorMsg( "no configuration exists for dictionary [" );
							errorMsg.append( dictionaryNodeName ).append( "]. Will use dictionary defaults" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );
						}

						EmaString dictionaryItemName;
						if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "RdmFieldDictionaryItemName", dictionaryItemName ) )
						{
							dictionaryItemName.set( "RWFFld" );

							EmaString errorMsg( "no configuration exists for RdmFieldDictionaryItemName in dictionary [" );
							errorMsg.append( dictionaryNodeName ).append( "]. Will use default value of RWFFld" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );
						}

						service.infoFilter.dictionariesProvided.push_back( dictionaryItemName );

						if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "EnumTypeDefItemName", dictionaryItemName ) )
						{
							dictionaryItemName.set( "RWFEnum" );

							EmaString errorMsg( "no configuration exists for EnumTypeDefItemName in dictionary [" );
							errorMsg.append( dictionaryNodeName ).append( "]. Will use default value of RWFEnum" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );
						}

						service.infoFilter.dictionariesProvided.push_back( dictionaryItemName );
					}

					pConfigImpl->getAsciiAttributeValueList( serviceNodeName + "InfoFilter|DictionariesUsed", "DictionariesUsedEntry", valueList );

					for ( UInt32 idx = 0; idx < valueList.size(); ++idx )
					{
						EmaString dictionaryNodeName( "DictionaryGroup|DictionaryList|Dictionary." );
						dictionaryNodeName.append( valueList[idx] ).append( "|" );

						EmaString name;
						if ( !pConfigImpl->get< EmaString >( dictionaryNodeName + "Name", name ) )
						{
							EmaString errorMsg( "no configuration exists for consumer dictionary [" );
							errorMsg.append( dictionaryNodeName ).append( "]. Will use dictionary defaults" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );
						}

						EmaString dictionaryItemName;
						if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "RdmFieldDictionaryItemName", dictionaryItemName ) )
						{
							dictionaryItemName.set( "RWFFld" );

							EmaString errorMsg( "no configuration exists for RdmFieldDictionaryItemName in dictionary [" );
							errorMsg.append( dictionaryNodeName ).append( "]. Will use default value of RWFFld" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );
						}

						service.infoFilter.dictionariesUsed.push_back( dictionaryItemName );

						if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "EnumTypeDefItemName", dictionaryItemName ) )
						{
							dictionaryItemName.set( "RWFEnum" );

							EmaString errorMsg( "no configuration exists for EnumTypeDefItemName in dictionary [" );
							errorMsg.append( dictionaryNodeName ).append( "]. Will use default value of RWFEnum" );
							pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );
						}

						service.infoFilter.dictionariesUsed.push_back( dictionaryItemName );
					}

					pConfigImpl->getAsciiAttributeValueList( serviceNodeName + "InfoFilter|Capabilities", "CapabilitiesEntry", valueList );

					for ( UInt32 idx = 0; idx < valueList.size(); ++idx )
					{
						if ( isdigit( valueList[idx].c_str()[0] ) )
						{
							UInt64 domainType;
							if ( sscanf( valueList[idx].c_str(), "%llu", &domainType ) == 1 )
							{
								if ( domainType > maxUInt16 )
								{
									EmaString errorMsg( "specified service [" );
									errorMsg.append( serviceNodeName ).append( "] contains out of range capability = " ).append( domainType ).append( ". Will drop this capability." );
									pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
									continue;
								}
								else
								{
									if ( service.infoFilter.capabilities.getPositionOf( (UInt16) domainType ) == -1 )
										service.infoFilter.capabilities.push_back( (UInt16) domainType );
								}
							}
							else
							{
								EmaString errorMsg( "failed to read or convert a capability from the specified service [" );
								errorMsg.append( serviceNodeName ).append( "]. Will drop this capability. Its value is = " ).append( valueList[idx] );
								pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
								continue;
							}
						}
						else
						{
							static struct
							{
								const char* configInput;
								UInt16 convertedValue;
							} converter[] =
							{
								{ "MMT_LOGIN", MMT_LOGIN },
								{ "MMT_DIRECTORY", MMT_DIRECTORY },
								{ "MMT_DICTIONARY", MMT_DICTIONARY },
								{ "MMT_MARKET_PRICE", MMT_MARKET_PRICE },
								{ "MMT_MARKET_BY_ORDER", MMT_MARKET_BY_ORDER },
								{ "MMT_MARKET_BY_PRICE", MMT_MARKET_BY_PRICE },
								{ "MMT_MARKET_MAKER", MMT_MARKET_MAKER },
								{ "MMT_SYMBOL_LIST", MMT_SYMBOL_LIST },
								{ "MMT_SERVICE_PROVIDER_STATUS", MMT_SERVICE_PROVIDER_STATUS },
								{ "MMT_HISTORY", MMT_HISTORY },
								{ "MMT_HEADLINE", MMT_HEADLINE },
								{ "MMT_REPLAYHEADLINE", MMT_REPLAYHEADLINE },
								{ "MMT_REPLAYSTORY", MMT_REPLAYSTORY },
								{ "MMT_TRANSACTION", MMT_TRANSACTION },
								{ "MMT_YIELD_CURVE", MMT_YIELD_CURVE },
								{ "MMT_CONTRIBUTION", MMT_CONTRIBUTION },
								{ "MMT_PROVIDER_ADMIN", MMT_PROVIDER_ADMIN },
								{ "MMT_ANALYTICS", MMT_ANALYTICS },
								{ "MMT_REFERENCE", MMT_REFERENCE },
								{ "MMT_NEWS_TEXT_ANALYTICS", MMT_NEWS_TEXT_ANALYTICS },
								{ "MMT_SYSTEM", MMT_SYSTEM },
							};

							bool found = false;
							for ( int i = 0; i < sizeof converter / sizeof converter[0]; ++i )
							{
								if ( !strcmp( converter[i].configInput, valueList[idx] ) )
								{
									found = true;
									if ( service.infoFilter.capabilities.getPositionOf( converter[i].convertedValue ) == -1 )
										service.infoFilter.capabilities.push_back( converter[i].convertedValue );
									break;
								}
							}

							if ( !found )
							{
								EmaString errorMsg( "failed to read or convert a capability from the specified service [" );
								errorMsg.append( serviceNodeName ).append( "]. Will drop this capability. Its value is = " ).append( valueList[idx] );
								pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
								continue;
							}
						}
					}

					if ( service.infoFilter.capabilities.empty() )
					{
						EmaString errorMsg( "specified service [" );
						errorMsg.append( serviceNodeName ).append( "] contains no capabilities. Will drop this service." );
						pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
						continue;
					}

					for ( UInt32 idx = 0; idx < service.infoFilter.capabilities.size() - 1; ++idx )
					{
						if ( service.infoFilter.capabilities[idx] > service.infoFilter.capabilities[idx + 1] )
						{
							UInt16 temp = service.infoFilter.capabilities[idx];
							service.infoFilter.capabilities[idx] = service.infoFilter.capabilities[idx + 1];
							service.infoFilter.capabilities[idx + 1] = temp;
							idx = 0;
						}
					}

					RsslQos rsslQos;
					EmaVector< XMLnode* > entryNodeList;
					pConfigImpl->getEntryNodeList( serviceNodeName + "InfoFilter|QoS", "QoSEntry", entryNodeList );

					if ( entryNodeList.empty() )
					{
						EmaString errorMsg( "no configuration exists for service QoS [" );
						errorMsg.append( serviceNodeName + "InfoFilter|QoS" ).append( "]. Will use default QoS" );
						pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

						rsslClearQos( &rsslQos );
						rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
						rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
						service.infoFilter.qos.push_back( rsslQos );
					}
					else
					{
						const UInt32 maxUInt32 = 0xFFFFFFFF;

						for ( UInt32 idx = 0; idx < entryNodeList.size(); ++idx )
						{
							EmaString rateString;
							if ( !entryNodeList[idx]->get< EmaString >( "Rate", rateString ) )
							{
								EmaString errorMsg( "no configuration exists for service QoS Rate [" );
								errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate" ).append( "]. Will use default Rate" );
								pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

								rateString.set( "Rate:TickByTick" );
							}

							UInt32 rate;
							if ( isdigit( rateString.c_str()[0] ) )
							{
								UInt64 temp;
								if ( sscanf( rateString.c_str(), "%llu", &temp ) == 1 )
								{
									if ( temp > maxUInt32 )
									{
										EmaString errorMsg( "specified service QoS::Rate [" );
										errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate" ).append( "] is greater than allowed maximum. Will use maximum Rate." );
										errorMsg.append( " Suspect Rate value is " ).append( rateString );
										pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

										rate = maxUInt32;
									}
									else
										rate = (UInt32) temp;
								}
								else
								{
									EmaString errorMsg( "failed to read or convert a QoS Rate from the specified service [" );
									errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate" ).append( "]. Will use default Rate." );
									errorMsg.append( " Suspect Rate value is " ).append( rateString );
									pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );

									rate = 0;
								}
							}
							else
							{
								static struct
								{
									const char* configInput;
									UInt32 convertedValue;
								} converter[] =
								{
									{ "Rate::TickByTick", 0 },
									{ "Rate::JustInTimeConflated", 0xFFFFFF00 },
								};

								bool found = false;
								for ( int i = 0; i < sizeof converter / sizeof converter[0]; ++i )
								{
									if ( !strcmp( converter[i].configInput, rateString ) )
									{
										found = true;
										rate = converter[i].convertedValue;
										break;
									}
								}

								if ( !found )
								{
									EmaString errorMsg( "failed to read or convert a QoS Rate from the specified service [" );
									errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Rate" ).append( "]. Will use default Rate." );
									errorMsg.append( " Suspect Rate value is " ).append( rateString );
									pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );

									rate = 0;
								}
							}

							EmaString timelinessString;
							if ( !entryNodeList[idx]->get< EmaString >( "Timeliness", timelinessString ) )
							{
								EmaString errorMsg( "no configuration exists for service QoS Timeliness [" );
								errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness" ).append( "]. Will use default Timeliness" );
								pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

								timelinessString.set( "Timeliness:RealTime" );
							}

							UInt32 timeliness;
							if ( isdigit( timelinessString.c_str()[0] ) )
							{
								UInt64 temp;
								if ( sscanf( timelinessString.c_str(), "%llu", &temp ) == 1 )
								{
									if ( temp > maxUInt32 )
									{
										EmaString errorMsg( "specified service QoS::Timeliness [" );
										errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness" ).append( "] is greater than allowed maximum. Will use maximum Timeliness." );
										errorMsg.append( " Suspect Timeliness value is " ).append( timelinessString );
										pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::WarningEnum );

										timeliness = maxUInt32;
									}
									else
										timeliness = (UInt32) temp;
								}
								else
								{
									EmaString errorMsg( "failed to read or convert a QoS Timeliness from the specified service [" );
									errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness" ).append( "]. Will use default Timeliness." );
									errorMsg.append( " Suspect Timeliness value is " ).append( timelinessString );
									pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );

									timeliness = 0;
								}
							}
							else
							{
								static struct
								{
									const char* configInput;
									UInt32 convertedValue;
								} converter[] =
								{
									{ "Timeliness::RealTime", 0 },
									{ "Timeliness::InexactDelayed", 0xFFFFFFFF },
								};

								bool found = false;
								for ( int i = 0; i < sizeof converter / sizeof converter[0]; ++i )
								{
									if ( !strcmp( converter[i].configInput, timelinessString ) )
									{
										found = true;
										timeliness = converter[i].convertedValue;
										break;
									}
								}

								if ( !found )
								{
									EmaString errorMsg( "failed to read or convert a QoS Timeliness from the specified service [" );
									errorMsg.append( serviceNodeName + "InfoFilter|QoS|QoSEntry|Timeliness" ).append( "]. Will use default Timeliness." );
									errorMsg.append( " Suspect Timeliness value is " ).append( timelinessString );
									pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );

									timeliness = 0;
								}
							}

							rsslClearQos( &rsslQos );
							OmmQosDecoder::convertToRssl( &rsslQos, timeliness, rate );
							service.infoFilter.qos.push_back( rsslQos );
						}
					}

					_activeConfig.directoryConfig.addService( service );
				}
			}
		}
	}

	_activeConfig.pDirectoryRefreshMsg = pConfigImpl->getDirectoryRefreshMsg();

	if ( ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveDirectoryConfig( _activeConfig.directoryConfig.directoryName, _activeConfig );
	}

	EmaString instanceNodeName( pConfigImpl->getInstanceNodeName() );
	instanceNodeName.append( _activeConfig.configuredName ).append( "|" );

	UInt64 tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "RefreshFirstRequired", tmp ) )
		_activeConfig.refreshFirstRequired = ( tmp > 0 ? true : false );

	tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "MergeSourceDirectoryStreams", tmp ) )
		_activeConfig.mergeSourceDirectoryStreams = ( tmp > 0 ? true : false );

	tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "RecoverUserSubmitSourceDirectory", tmp ) )
		_activeConfig.recoverUserSubmitSourceDirectory = ( tmp > 0 ? true : false );

	tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "RemoveItemsOnDisconnect", tmp ) )
		_activeConfig.removeItemsOnDisconnect = ( tmp > 0 ? true : false );

	_activeConfig.directoryAdminControl = static_cast<OmmNiProviderConfigImpl*>( pConfigImpl )->getAdminControlDirectory();

	if ( ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveCustomConfig( _activeConfig.configuredName, _activeConfig );
	}
}

const EmaString& OmmNiProviderImpl::getInstanceName() const
{
	return OmmBaseImpl::getInstanceName();
}

void OmmNiProviderImpl::freeMemory( RsslRDMDirectoryRefresh& directoryRefresh, RsslBuffer& rsslMsgBuffer )
{
	for ( UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp )
		if ( directoryRefresh.serviceList[temp].info.qosList )
			delete[] directoryRefresh.serviceList[temp].info.qosList;

	for ( UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp )
		if ( directoryRefresh.serviceList[temp].info.dictionariesUsedList )
			delete[] directoryRefresh.serviceList[temp].info.dictionariesUsedList;

	for ( UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp )
		if ( directoryRefresh.serviceList[temp].info.dictionariesProvidedList )
			delete[] directoryRefresh.serviceList[temp].info.dictionariesProvidedList;

	for ( UInt32 temp = 0; temp < directoryRefresh.serviceCount; ++temp )
		if ( directoryRefresh.serviceList[temp].info.capabilitiesList )
			delete[] directoryRefresh.serviceList[temp].info.capabilitiesList;

	if ( directoryRefresh.serviceList )
		delete[] directoryRefresh.serviceList;

	if ( rsslMsgBuffer.data )
		free( rsslMsgBuffer.data );
}

void OmmNiProviderImpl::loadDirectory()
{
	if ( _activeConfig.directoryAdminControl == OmmNiProviderConfig::UserControlEnum )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "DirectoryAdminControl = UserControl" );
		return;
	}

	if ( !_activeConfig.pDirectoryRefreshMsg )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Using SourceDirectory configuration from EmaConfig.xml file & config( const Data& )" );

		RsslBuffer rsslMsgBuffer;
		rsslMsgBuffer.length = 4096;
		rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

		if ( !rsslMsgBuffer.data )
		{
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslEncIterator eIter;
		rsslClearEncodeIterator( &eIter );

		if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) )
		{
			free( rsslMsgBuffer.data );
			handleIue( "Internal error. Failed to set encode iterator version in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer ) )
		{
			free( rsslMsgBuffer.data );
			handleIue( "Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslRDMDirectoryRefresh directoryRefresh;
		rsslClearRDMDirectoryRefresh( &directoryRefresh );

		directoryRefresh.flags = RDM_DR_RFF_CLEAR_CACHE;

		directoryRefresh.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER;

		directoryRefresh.state.text.data = ( char* )"Refresh Complete";
		directoryRefresh.state.text.length = 16;

		directoryRefresh.serviceCount = _activeConfig.directoryConfig.getServiceList().size();

		try
		{
			directoryRefresh.serviceList = new RsslRDMService[directoryRefresh.serviceCount];
		}
		catch ( std::bad_alloc )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		for ( UInt32 idx = 0; idx < directoryRefresh.serviceCount; ++idx )
		{
			rsslClearRDMService( &directoryRefresh.serviceList[idx] );

			directoryRefresh.serviceList[idx].action = RSSL_MPEA_ADD_ENTRY;
			directoryRefresh.serviceList[idx].flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;
			directoryRefresh.serviceList[idx].serviceId = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.serviceId;

			directoryRefresh.serviceList[idx].info.action = RSSL_FTEA_SET_ENTRY;

			if ( !_activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.vendorName.empty() )
			{
				directoryRefresh.serviceList[idx].info.vendor.data = (char*) _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.vendorName.c_str();
				directoryRefresh.serviceList[idx].info.vendor.length = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.vendorName.length();
				directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_VENDOR;
			}

			if ( !_activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.itemList.empty() )
			{
				directoryRefresh.serviceList[idx].info.itemList.data = (char*) _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.itemList.c_str();
				directoryRefresh.serviceList[idx].info.itemList.length = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.itemList.length();
				directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
			}

			directoryRefresh.serviceList[idx].info.acceptingConsumerStatus = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.acceptingConsumerStatus;
			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;

			directoryRefresh.serviceList[idx].info.isSource = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.isSource;
			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;

			directoryRefresh.serviceList[idx].info.supportsOutOfBandSnapshots = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.supportsOutOfBandSnapshots;
			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;

			directoryRefresh.serviceList[idx].info.supportsQosRange = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.supportsQosRange;
			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;

			directoryRefresh.serviceList[idx].info.capabilitiesCount = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.capabilities.size();

			try
			{
				directoryRefresh.serviceList[idx].info.capabilitiesList = new UInt64[directoryRefresh.serviceList[idx].info.capabilitiesCount];
			}
			catch ( std::bad_alloc )
			{
				freeMemory( directoryRefresh, rsslMsgBuffer );
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
				return;
			}

			for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.capabilitiesCount; ++jdx )
				directoryRefresh.serviceList[idx].info.capabilitiesList[jdx] = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.capabilities[jdx];

			directoryRefresh.serviceList[idx].info.dictionariesProvidedCount = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.dictionariesProvided.size();

			if ( directoryRefresh.serviceList[idx].info.dictionariesProvidedCount )
			{
				try
				{
					directoryRefresh.serviceList[idx].info.dictionariesProvidedList = new RsslBuffer[directoryRefresh.serviceList[idx].info.dictionariesProvidedCount];
				}
				catch ( std::bad_alloc )
				{
					freeMemory( directoryRefresh, rsslMsgBuffer );
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
					return;
				}


				for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.dictionariesProvidedCount; ++jdx )
				{
					directoryRefresh.serviceList[idx].info.dictionariesProvidedList[jdx].data = (char*) _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.dictionariesProvided[jdx].c_str();
					directoryRefresh.serviceList[idx].info.dictionariesProvidedList[jdx].length = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.dictionariesProvided[jdx].length();
				}

				directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
			}

			directoryRefresh.serviceList[idx].info.dictionariesUsedCount = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.dictionariesUsed.size();

			if ( directoryRefresh.serviceList[idx].info.dictionariesUsedCount )
			{
				try
				{
					directoryRefresh.serviceList[idx].info.dictionariesUsedList = new RsslBuffer[directoryRefresh.serviceList[idx].info.dictionariesUsedCount];
				}
				catch ( std::bad_alloc )
				{
					freeMemory( directoryRefresh, rsslMsgBuffer );
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
					return;
				}

				for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.dictionariesUsedCount; ++jdx )
				{
					directoryRefresh.serviceList[idx].info.dictionariesUsedList[jdx].data = (char*) _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.dictionariesUsed[jdx].c_str();
					directoryRefresh.serviceList[idx].info.dictionariesUsedList[jdx].length = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.dictionariesUsed[jdx].length();
				}

				directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
			}

			directoryRefresh.serviceList[idx].info.qosCount = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.qos.size();

			if ( directoryRefresh.serviceList[idx].info.qosCount )
			{
				try
				{
					directoryRefresh.serviceList[idx].info.qosList = new RsslQos[directoryRefresh.serviceList[idx].info.qosCount];
				}
				catch ( std::bad_alloc )
				{
					freeMemory( directoryRefresh, rsslMsgBuffer );
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
					return;
				}

				for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.qosCount; ++jdx )
					directoryRefresh.serviceList[idx].info.qosList[jdx] = _activeConfig.directoryConfig.getServiceList()[idx]->infoFilter.qos[jdx];

				directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_QOS;
			}

			directoryRefresh.serviceList[idx].info.serviceName.data = (char*) _activeConfig.directoryConfig.getServiceList()[idx]->serviceName.c_str();
			directoryRefresh.serviceList[idx].info.serviceName.length = _activeConfig.directoryConfig.getServiceList()[idx]->serviceName.length();

			directoryRefresh.serviceList[idx].state.acceptingRequests = _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.acceptingRequests;
			directoryRefresh.serviceList[idx].state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
			directoryRefresh.serviceList[idx].state.action = RSSL_FTEA_SET_ENTRY;
			directoryRefresh.serviceList[idx].state.serviceState = _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.serviceState;

			if ( _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.isStatusConfigured )
			{
				directoryRefresh.serviceList[idx].state.flags |= RDM_SVC_STF_HAS_STATUS;
				directoryRefresh.serviceList[idx].state.status.streamState = _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.status.streamState;
				directoryRefresh.serviceList[idx].state.status.dataState = _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.status.dataState;
				directoryRefresh.serviceList[idx].state.status.code = _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.status.code;
				directoryRefresh.serviceList[idx].state.status.text = _activeConfig.directoryConfig.getServiceList()[idx]->stateFilter.status.text;
			}
		}

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );

		while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
		{
			free( rsslMsgBuffer.data );

			rsslMsgBuffer.length += rsslMsgBuffer.length;
			rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

			if ( !rsslMsgBuffer.data )
			{
				freeMemory( directoryRefresh, rsslMsgBuffer );
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
				return;
			}

			clearRsslErrorInfo( &rsslErrorInfo );
			retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );
		}

		if ( retCode != RSSL_RET_SUCCESS )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );

			EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in OmmNiProviderImpl::loadDirectory()" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			handleIue( temp );
			return;
		}

		try
		{
			_activeConfig.pDirectoryRefreshMsg = new AdminRefreshMsg( 0 );
		}
		catch ( std::bad_alloc )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslDecodeIterator decIter;
		rsslClearDecodeIterator( &decIter );

		if ( rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) != RSSL_RET_SUCCESS )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		if ( rsslSetDecodeIteratorBuffer( &decIter, &rsslMsgBuffer ) != RSSL_RET_SUCCESS )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslRefreshMsg rsslRefreshMsg;
		rsslClearRefreshMsg( &rsslRefreshMsg );
		if ( rsslDecodeMsg( &decIter, (RsslMsg*) &rsslRefreshMsg ) != RSSL_RET_SUCCESS )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleIue( "Internal error. Failed to decode message in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		_activeConfig.pDirectoryRefreshMsg->set( &rsslRefreshMsg );

		freeMemory( directoryRefresh, rsslMsgBuffer );
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Using SourceDirectory configuration from addAdminMsg( const RefreshMsg& )" );

		RsslMsg* pTempRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

		if ( pTempRsslMsg->msgBase.containerType != RSSL_DT_MAP )
		{
			EmaString temp( "Attempt to submit RefreshMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is " );
			temp += DataType( dataType[pTempRsslMsg->msgBase.containerType] ).toString();
			handleIue( temp );
			return;
		}
	}

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

	EmaString temp;
	RsslBuffer swapBuffer;
	rsslClearBuffer( &swapBuffer );

	if ( !decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, &swapBuffer, temp ) )
	{
		if ( swapBuffer.data ) free( swapBuffer.data );
		handleIue( temp );
		return;
	}

	RsslBuffer origPayload;

	if ( swapBuffer.data )
	{
		origPayload = submitMsgOpts.pRsslMsg->msgBase.encDataBody;
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = swapBuffer;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	Channel* pChannel = getChannelCallbackClient().getChannelList().front();
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( swapBuffer.data )
		{
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
			free( swapBuffer.data );
		}

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory()." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directory was sent out on the wire." );
	}

	if ( swapBuffer.data )
	{
		AdminRefreshMsg temp( 0 );
		temp.set( (RsslRefreshMsg*) submitMsgOpts.pRsslMsg );
		*_activeConfig.pDirectoryRefreshMsg = temp;

		submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
		free( swapBuffer.data );
	}
}

void OmmNiProviderImpl::reLoadDirectory()
{
	reLoadConfigSourceDirectory();
	reLoadUserSubmitSourceDirectory();
}

void OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()
{
	if ( !_activeConfig.recoverUserSubmitSourceDirectory )
		return;

	if ( _activeConfig.userSubmittedDirectoryConfig.getServiceList().size() == 0 )
		return;

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Reload of user submitted source directories." );

	RsslBuffer rsslMsgBuffer;
	rsslMsgBuffer.length = 4096;
	rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

	if ( !rsslMsgBuffer.data )
	{
		handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );

	if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) )
	{
		free( rsslMsgBuffer.data );
		handleIue( "Internal error. Failed to set encode iterator version in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer ) )
	{
		free( rsslMsgBuffer.data );
		handleIue( "Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslRDMDirectoryRefresh directoryRefresh;
	rsslClearRDMDirectoryRefresh( &directoryRefresh );

	directoryRefresh.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER;

	directoryRefresh.state.text.data = ( char* )"Refresh Complete";
	directoryRefresh.state.text.length = 16;

	directoryRefresh.serviceCount = _activeConfig.userSubmittedDirectoryConfig.getServiceList().size();

	try
	{
		directoryRefresh.serviceList = new RsslRDMService[directoryRefresh.serviceCount];
	}
	catch ( std::bad_alloc )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );
		handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	for ( UInt32 idx = 0; idx < directoryRefresh.serviceCount; ++idx )
	{
		rsslClearRDMService( &directoryRefresh.serviceList[idx] );

		directoryRefresh.serviceList[idx].action = RSSL_MPEA_ADD_ENTRY;
		directoryRefresh.serviceList[idx].flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;
		directoryRefresh.serviceList[idx].serviceId = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.serviceId;

		directoryRefresh.serviceList[idx].info.action = RSSL_FTEA_SET_ENTRY;

		if ( !_activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.vendorName.empty() )
		{
			directoryRefresh.serviceList[idx].info.vendor.data = (char*) _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.vendorName.c_str();
			directoryRefresh.serviceList[idx].info.vendor.length = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.vendorName.length();
			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_VENDOR;
		}

		if ( !_activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.itemList.empty() )
		{
			directoryRefresh.serviceList[idx].info.itemList.data = (char*) _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.itemList.c_str();
			directoryRefresh.serviceList[idx].info.itemList.length = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.itemList.length();
			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
		}

		directoryRefresh.serviceList[idx].info.acceptingConsumerStatus = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.acceptingConsumerStatus;
		directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;

		directoryRefresh.serviceList[idx].info.isSource = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.isSource;
		directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;

		directoryRefresh.serviceList[idx].info.supportsOutOfBandSnapshots = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.supportsOutOfBandSnapshots;
		directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;

		directoryRefresh.serviceList[idx].info.supportsQosRange = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.supportsQosRange;
		directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;

		directoryRefresh.serviceList[idx].info.capabilitiesCount = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.capabilities.size();

		try
		{
			directoryRefresh.serviceList[idx].info.capabilitiesList = new UInt64[directoryRefresh.serviceList[idx].info.capabilitiesCount];
		}
		catch ( std::bad_alloc )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
			return;
		}

		for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.capabilitiesCount; ++jdx )
			directoryRefresh.serviceList[idx].info.capabilitiesList[jdx] = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.capabilities[jdx];

		directoryRefresh.serviceList[idx].info.dictionariesProvidedCount = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.dictionariesProvided.size();

		if ( directoryRefresh.serviceList[idx].info.dictionariesProvidedCount )
		{
			try
			{
				directoryRefresh.serviceList[idx].info.dictionariesProvidedList = new RsslBuffer[directoryRefresh.serviceList[idx].info.dictionariesProvidedCount];
			}
			catch ( std::bad_alloc )
			{
				freeMemory( directoryRefresh, rsslMsgBuffer );
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
				return;
			}


			for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.dictionariesProvidedCount; ++jdx )
			{
				directoryRefresh.serviceList[idx].info.dictionariesProvidedList[jdx].data = (char*) _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.dictionariesProvided[jdx].c_str();
				directoryRefresh.serviceList[idx].info.dictionariesProvidedList[jdx].length = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.dictionariesProvided[jdx].length();
			}

			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
		}

		directoryRefresh.serviceList[idx].info.dictionariesUsedCount = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.dictionariesUsed.size();

		if ( directoryRefresh.serviceList[idx].info.dictionariesUsedCount )
		{
			try
			{
				directoryRefresh.serviceList[idx].info.dictionariesUsedList = new RsslBuffer[directoryRefresh.serviceList[idx].info.dictionariesUsedCount];
			}
			catch ( std::bad_alloc )
			{
				freeMemory( directoryRefresh, rsslMsgBuffer );
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
				return;
			}

			for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.dictionariesUsedCount; ++jdx )
			{
				directoryRefresh.serviceList[idx].info.dictionariesUsedList[jdx].data = (char*) _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.dictionariesUsed[jdx].c_str();
				directoryRefresh.serviceList[idx].info.dictionariesUsedList[jdx].length = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.dictionariesUsed[jdx].length();
			}

			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
		}

		directoryRefresh.serviceList[idx].info.qosCount = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.qos.size();

		if ( directoryRefresh.serviceList[idx].info.qosCount )
		{
			try
			{
				directoryRefresh.serviceList[idx].info.qosList = new RsslQos[directoryRefresh.serviceList[idx].info.qosCount];
			}
			catch ( std::bad_alloc )
			{
				freeMemory( directoryRefresh, rsslMsgBuffer );
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
				return;
			}

			for ( UInt32 jdx = 0; jdx < directoryRefresh.serviceList[idx].info.qosCount; ++jdx )
				directoryRefresh.serviceList[idx].info.qosList[jdx] = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->infoFilter.qos[jdx];

			directoryRefresh.serviceList[idx].info.flags |= RDM_SVC_IFF_HAS_QOS;
		}

		directoryRefresh.serviceList[idx].info.serviceName.data = (char*) _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->serviceName.c_str();
		directoryRefresh.serviceList[idx].info.serviceName.length = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->serviceName.length();

		directoryRefresh.serviceList[idx].state.acceptingRequests = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.acceptingRequests;
		directoryRefresh.serviceList[idx].state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
		directoryRefresh.serviceList[idx].state.action = RSSL_FTEA_SET_ENTRY;
		directoryRefresh.serviceList[idx].state.serviceState = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.serviceState;

		if ( _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.isStatusConfigured )
		{
			directoryRefresh.serviceList[idx].state.flags |= RDM_SVC_STF_HAS_STATUS;
			directoryRefresh.serviceList[idx].state.status.streamState = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.status.streamState;
			directoryRefresh.serviceList[idx].state.status.dataState = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.status.dataState;
			directoryRefresh.serviceList[idx].state.status.code = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.status.code;
			directoryRefresh.serviceList[idx].state.status.text = _activeConfig.userSubmittedDirectoryConfig.getServiceList()[idx]->stateFilter.status.text;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( rsslMsgBuffer.data );

		rsslMsgBuffer.length += rsslMsgBuffer.length;
		rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

		if ( !rsslMsgBuffer.data )
		{
			freeMemory( directoryRefresh, rsslMsgBuffer );
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		clearRsslErrorInfo( &rsslErrorInfo );
		retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );

		EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in OmmNiProviderImpl::loadDirectory()" );
		temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}

	RsslDecodeIterator decIter;
	rsslClearDecodeIterator( &decIter );

	if ( rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) != RSSL_RET_SUCCESS )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );
		handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	if ( rsslSetDecodeIteratorBuffer( &decIter, &rsslMsgBuffer ) != RSSL_RET_SUCCESS )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );
		handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslRefreshMsg rsslRefreshMsg;
	rsslClearRefreshMsg( &rsslRefreshMsg );
	if ( rsslDecodeMsg( &decIter, (RsslMsg*) &rsslRefreshMsg ) != RSSL_RET_SUCCESS )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );
		handleIue( "Internal error. Failed to decode message in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) &rsslRefreshMsg;

	EmaString temp;
	RsslBuffer swapBuffer;
	rsslClearBuffer( &swapBuffer );

	if ( _activeConfig.removeItemsOnDisconnect && !decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, &swapBuffer, temp ) )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );

		if ( swapBuffer.data ) free( swapBuffer.data );
		handleIue( temp );
		return;
	}

	RsslBuffer origPayload;

	if ( swapBuffer.data )
	{
		origPayload = submitMsgOpts.pRsslMsg->msgBase.encDataBody;
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = swapBuffer;
	}

	clearRsslErrorInfo( &rsslErrorInfo );
	Channel* pChannel = getChannelCallbackClient().getChannelList().front();
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		freeMemory( directoryRefresh, rsslMsgBuffer );

		if ( swapBuffer.data )
		{
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
			free( swapBuffer.data );
		}

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory()." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "User submitted source directoies were sent out on the wire after reconnect." );
	}

	if ( swapBuffer.data )
	{
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
		free( swapBuffer.data );
	}

	freeMemory( directoryRefresh, rsslMsgBuffer );
}

void OmmNiProviderImpl::storeUserSubmitSourceDirectory( RsslMsg* pMsg )
{
	if ( !_activeConfig.recoverUserSubmitSourceDirectory )
		return;

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Storing user submitted source directories for connection recovery." );

	RsslDecodeIterator decIter;
	rsslClearDecodeIterator( &decIter );

	if ( rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) != RSSL_RET_SUCCESS )
	{
		handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::storeUserSubmitSourceDirectory()" );
		return;
	}

	if ( rsslSetDecodeIteratorBuffer( &decIter, &pMsg->msgBase.encDataBody ) != RSSL_RET_SUCCESS )
	{
		handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::storeUserSubmitSourceDirectory()" );
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRDMDirectoryMsg userSubmitSourceDirectory;

	RsslBuffer tempBuffer, tempBufferOrig;
	tempBufferOrig.length = 4096;
	tempBufferOrig.data = (char*) malloc( tempBufferOrig.length * sizeof( char ) );
	if ( !tempBufferOrig.data )
	{
		handleMee( "Failed to allocate memory in OmmNiProviderImpl::storeUserSubmitSourceDirectory()" );
		return;
	}

	tempBuffer = tempBufferOrig;

	RsslRet retCode;

	while ( RSSL_RET_BUFFER_TOO_SMALL == ( retCode = rsslDecodeRDMDirectoryMsg( &decIter, pMsg, &userSubmitSourceDirectory, &tempBuffer, &rsslErrorInfo ) ) )
	{
		free( tempBufferOrig.data );
		tempBufferOrig.length += tempBufferOrig.length;
		tempBufferOrig.data = (char*) malloc( tempBufferOrig.length * sizeof( char ) );
		if ( !tempBufferOrig.data )
		{
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::storeUserSubmitSourceDirectory()" );
			return;
		}

		tempBuffer = tempBufferOrig;
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		free( tempBufferOrig.data );

		EmaString temp( "Internal error: failed to decode RsslRDMDirectoryMsg in OmmNiProviderImpl::storeUserSubmitSourceDirectory()" );
		temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}

	UInt32 serviceCount = 0;
	RsslRDMService* pServiceList = 0;

	switch ( pMsg->msgBase.msgClass )
	{
	case RSSL_MC_REFRESH:
		serviceCount = userSubmitSourceDirectory.refresh.serviceCount;
		pServiceList = userSubmitSourceDirectory.refresh.serviceList;
		break;
	case RSSL_MC_UPDATE:
		serviceCount = userSubmitSourceDirectory.update.serviceCount;
		pServiceList = userSubmitSourceDirectory.update.serviceList;
		break;
	default:
		break;
	}

	for ( UInt32 idx = 0; idx < serviceCount; ++idx )
	{
		switch ( ( *pServiceList ).action )
		{
		case RSSL_MPEA_ADD_ENTRY:
		{
			ServiceConfig service;

			if ( pServiceList->flags & RDM_SVCF_HAS_STATE )
			{
				if ( pServiceList->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS )
					service.stateFilter.acceptingRequests = pServiceList->state.acceptingRequests;

				service.stateFilter.serviceState = pServiceList->state.serviceState;

				if ( pServiceList->state.flags & RDM_SVC_STF_HAS_STATUS )
				{
					service.stateFilter.isStatusConfigured = true;
					service.stateFilter.status = pServiceList->state.status;
					service.stateFilter.statusText.set( pServiceList->state.status.text.data, pServiceList->state.status.text.length );
					service.stateFilter.status.text.data = (char*) service.stateFilter.statusText.c_str();
					service.stateFilter.status.text.length = service.stateFilter.statusText.length();
				}
			}

			if ( pServiceList->flags & RDM_SVCF_HAS_INFO )
			{
				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS )
					service.infoFilter.acceptingConsumerStatus = pServiceList->info.acceptingConsumerStatus;

				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_IS_SOURCE )
					service.infoFilter.isSource = pServiceList->info.isSource;

				for ( UInt32 i = 0; i < pServiceList->info.capabilitiesCount; ++i )
					service.infoFilter.capabilities.push_back( *( pServiceList->info.capabilitiesList++ ) );

				for ( UInt32 i = 0; i < pServiceList->info.dictionariesProvidedCount; ++i )
				{
					service.infoFilter.dictionariesProvided.push_back( EmaString( pServiceList->info.dictionariesProvidedList->data, pServiceList->info.dictionariesProvidedList->length ) );
					pServiceList->info.dictionariesProvidedList++;
				}

				for ( UInt32 i = 0; i < pServiceList->info.dictionariesUsedCount; ++i )
				{
					service.infoFilter.dictionariesUsed.push_back( EmaString( pServiceList->info.dictionariesUsedList->data, pServiceList->info.dictionariesUsedList->length ) );
					pServiceList->info.dictionariesUsedList++;
				}

				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST )
					service.infoFilter.itemList.set( pServiceList->info.itemList.data, pServiceList->info.itemList.length );

				service.infoFilter.serviceId = pServiceList->serviceId;

				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_QOS )
					for ( UInt32 i = 0; i < pServiceList->info.qosCount; ++i )
						service.infoFilter.qos.push_back( *( pServiceList->info.qosList++ ) );

				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS )
					service.infoFilter.supportsOutOfBandSnapshots = pServiceList->info.supportsOutOfBandSnapshots;

				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE )
					service.infoFilter.supportsQosRange = pServiceList->info.supportsQosRange;

				if ( pServiceList->info.flags & RDM_SVC_IFF_HAS_VENDOR )
					service.infoFilter.vendorName.set( pServiceList->info.vendor.data, pServiceList->info.vendor.length );

				service.serviceName.set( pServiceList->info.serviceName.data, pServiceList->info.serviceName.length );
			}

			_activeConfig.userSubmittedDirectoryConfig.addService( service );
		}
		break;
		case RSSL_MPEA_DELETE_ENTRY:
			_activeConfig.userSubmittedDirectoryConfig.removeService( ( *pServiceList ).serviceId );
			break;
		case RSSL_MPEA_UPDATE_ENTRY:
		{
			ServiceConfig* pService = _activeConfig.userSubmittedDirectoryConfig.getService( ( *pServiceList ).serviceId );
			if ( pService )
			{
				if ( pServiceList->flags & RDM_SVCF_HAS_STATE )
				{
					if ( pServiceList->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS )
						pService->stateFilter.acceptingRequests = pServiceList->state.acceptingRequests;

					pService->stateFilter.serviceState = pServiceList->state.serviceState;

					if ( pServiceList->state.flags & RDM_SVC_STF_HAS_STATUS )
					{
						pService->stateFilter.isStatusConfigured = true;
						pService->stateFilter.status = pServiceList->state.status;
						pService->stateFilter.statusText.set( pServiceList->state.status.text.data, pServiceList->state.status.text.length );
						pService->stateFilter.status.text.data = (char*) pService->stateFilter.statusText.c_str();
						pService->stateFilter.status.text.length = pService->stateFilter.statusText.length();
					}
				}
			}
		}
		break;
		default:
			break;
		}

		++pServiceList;
	}

	free( tempBufferOrig.data );
}

void OmmNiProviderImpl::reLoadConfigSourceDirectory()
{
	if ( !_activeConfig.pDirectoryRefreshMsg )
		return;

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Reload of configured source directories." );

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

	EmaString temp;
	RsslBuffer swapBuffer;
	rsslClearBuffer( &swapBuffer );

	if ( _activeConfig.removeItemsOnDisconnect && !decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, &swapBuffer, temp ) )
	{
		if ( swapBuffer.data ) free( swapBuffer.data );
		handleIue( temp );
		return;
	}

	RsslBuffer origPayload;

	if ( swapBuffer.data )
	{
		origPayload = submitMsgOpts.pRsslMsg->msgBase.encDataBody;
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = swapBuffer;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	Channel* pChannel = getChannelCallbackClient().getChannelList().front();
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( swapBuffer.data )
		{
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
			free( swapBuffer.data );
		}

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::reLoadConfigSourceDirectory()." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directoies were sent out on the wire after reconnect." );
	}

	if ( swapBuffer.data )
	{
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
		free( swapBuffer.data );
	}
}

void OmmNiProviderImpl::processChannelEvent( RsslReactorChannelEvent* pEvent )
{
	switch ( pEvent->channelEventType )
	{
	case RSSL_RC_CET_CHANNEL_DOWN:
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		if ( _activeConfig.removeItemsOnDisconnect )
			removeItems();
		break;
	default:
		break;
	}
}

void OmmNiProviderImpl::loadDictionary()
{
}

void OmmNiProviderImpl::createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& )
{
}

void OmmNiProviderImpl::createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& )
{
}

void OmmNiProviderImpl::addSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_SET( fd, &_readFds );
	FD_SET( fd, &_exceptFds );
#else
	addFd( fd, POLLIN | POLLERR | POLLHUP );
#endif
}

void OmmNiProviderImpl::removeSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_CLR( fd, &_readFds );
	FD_CLR( fd, &_exceptFds );
#else
	removeFd( fd );
#endif
}

Int64 OmmNiProviderImpl::dispatch( Int64 timeOut )
{
	if ( _activeConfig.operationModel == OmmNiProviderConfig::UserDispatchEnum && !_atExit )
		return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread, _bMsgDispatched );

	return OmmProvider::TimeoutEnum;
}

UInt64 OmmNiProviderImpl::registerClient( const ReqMsg& reqMsg, OmmProviderClient& ommProvClient, void* closure, UInt64 parentHandle )
{
	_userLock.lock();

	UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( reqMsg, ommProvClient, closure, parentHandle ) : 0;

	if ( handle )
	{
		try
		{
			// todo ... check for possible conflict with a user assigned handle

			StreamInfoPtr pTemp = new StreamInfo( reinterpret_cast<Item*>( handle )->getStreamId() );
			_handleToStreamInfo.insert( handle, pTemp );
			_streamInfoList.push_back( pTemp );
		}
		catch ( std::bad_alloc )
		{
			_pItemCallbackClient->unregister( handle );
			_userLock.unlock();
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::registerClient()" );
			return 0;
		}
	}

	_userLock.unlock();

	return handle;
}

void OmmNiProviderImpl::unregister( UInt64 handle )
{
	_userLock.lock();

	StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( !pTempStreamInfoPtr )
	{
		_userLock.unlock();
		return;
	}

	if ( ( *pTempStreamInfoPtr )->_streamId < 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to unregister a handle that was not registered." );
		return;
	}

	_streamInfoList.removeValue( *pTempStreamInfoPtr );
	delete *pTempStreamInfoPtr;
	_handleToStreamInfo.erase( handle );

	_userLock.unlock();

	OmmBaseImpl::unregister( handle );
}

void OmmNiProviderImpl::submit( const RefreshMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const RefreshMsgEncoder&>( msg.getEncoder() ).getRsslRefreshMsg();

	bool bHandleAdded = false;

	RsslBuffer swapBuffer;
	RsslBuffer origPayload;
	rsslClearBuffer( &swapBuffer );

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamId > 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const RefreshMsg& ) using a registered handle." );
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received RefreshMsg with SourceDirectory domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit RefreshMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is " );
			temp += DataType( dataType[submitMsgOpts.pRsslMsg->msgBase.containerType] ).toString();
			handleIue( temp );
			return;
		}

		EmaString temp;

		if ( !decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, &swapBuffer, temp ) )
		{
			if ( swapBuffer.data ) free( swapBuffer.data );

			_userLock.unlock();
			handleIue( temp );
			return;
		}

		submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

		if ( _activeConfig.mergeSourceDirectoryStreams )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = 0;
		}
		else
		{
			if ( pStreamInfoPtr )
			{
				submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			}
			else
			{
				try
				{
					submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();
					StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc )
				{
					pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_userLock.unlock();
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )" );
					return;
				}
			}
		}

		if ( swapBuffer.data )
		{
			origPayload = submitMsgOpts.pRsslMsg->msgBase.encDataBody;
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = swapBuffer;
		}

		storeUserSubmitSourceDirectory( submitMsgOpts.pRsslMsg );
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received RefreshMsg with market domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		const RefreshMsgEncoder& enc = static_cast<const RefreshMsgEncoder&>( msg.getEncoder() );

		if ( pStreamInfoPtr )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			if ( submitMsgOpts.pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY )
			{
				submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = ( *pStreamInfoPtr )->_serviceId;
				submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			}
		}
		else if ( enc.hasServiceName() )
		{
			const EmaString& serviceName = enc.getServiceName();
			RsslUInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );
			if ( !pServiceId )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit initial RefreshMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this RefreshMsg." );
				handleIue( temp );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit initial RefreshMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this RefreshMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

			submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )" );
				return;
			}
		}
		else if ( enc.hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit initial RefreshMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this RefreshMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

			submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )" );
				return;
			}
		}
		else
		{
			_userLock.unlock();
			handleIue( "Attempt to submit initial RefreshMsg without service name or id. Dropping this RefreshMsg." );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( bHandleAdded )
		{
			StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
			_streamInfoList.removeValue( *pTempStreamInfoPtr );
			delete *pTempStreamInfoPtr;
			_handleToStreamInfo.erase( handle );
			pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
		}

		_userLock.unlock();

		if ( swapBuffer.data )
		{
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
			free( swapBuffer.data );
		}

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const RefreshMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );

		return;
	}

	if ( swapBuffer.data )
	{
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
		free( swapBuffer.data );
	}

	if ( submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedEnum ||
		submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedRecoverEnum ||
		submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedRedirectedEnum )
	{
		StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
		_streamInfoList.removeValue( *pTempStreamInfoPtr );
		delete *pTempStreamInfoPtr;
		_handleToStreamInfo.erase( handle );
		pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
	}

	if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
		_bIsStreamIdZeroRefreshSubmitted = true;

	_userLock.unlock();
}

void OmmNiProviderImpl::submit( const UpdateMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const UpdateMsgEncoder&>( msg.getEncoder() ).getRsslUpdateMsg();

	bool bHandleAdded = false;

	RsslBuffer swapBuffer;
	RsslBuffer origPayload;
	rsslClearBuffer( &swapBuffer );

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamId > 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const UpdateMsg& ) using a registered handle." );
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received UpdateMsg with SourceDirectory domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit UpdateMsg with SourceDirectory domain using container with wrong data type. Expected is Map. Passed in is " );
			temp += DataType( dataType[submitMsgOpts.pRsslMsg->msgBase.containerType] ).toString();
			handleIue( temp );
			return;
		}

		EmaString temp;

		if ( !decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, &swapBuffer, temp ) )
		{
			if ( swapBuffer.data ) free( swapBuffer.data );

			_userLock.unlock();
			handleIue( temp );
			return;
		}

		if ( _activeConfig.mergeSourceDirectoryStreams )
		{
			if ( _activeConfig.refreshFirstRequired && !_bIsStreamIdZeroRefreshSubmitted )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
				temp.append( handle ).append( "." );
				handleIhe( handle, temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId = 0;
		}
		else
		{
			if ( pStreamInfoPtr )
			{
				submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			}
			else
			{
				if ( _activeConfig.refreshFirstRequired )
				{
					_userLock.unlock();
					EmaString temp( "Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
					temp.append( handle ).append( "." );
					handleIhe( handle, temp );
					return;
				}

				try
				{
					submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();
					StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc )
				{
					pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_userLock.unlock();
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const UpdateMsg& )" );
					return;
				}
			}
		}

		if ( swapBuffer.data )
		{
			origPayload = submitMsgOpts.pRsslMsg->msgBase.encDataBody;
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = swapBuffer;
		}

		storeUserSubmitSourceDirectory( submitMsgOpts.pRsslMsg );
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received UpdateMsg with market domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		const UpdateMsgEncoder& enc = static_cast<const UpdateMsgEncoder&>( msg.getEncoder() );

		if ( pStreamInfoPtr )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			if ( submitMsgOpts.pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY )
			{
				submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = ( *pStreamInfoPtr )->_serviceId;
				submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			}
		}
		else if ( _activeConfig.refreshFirstRequired )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit UpdateMsg while RefreshMsg was not submitted on this stream yet. Handle = " );
			temp.append( handle ).append( "." );
			handleIhe( handle, temp );
			return;
		}
		else if ( enc.hasServiceName() )
		{
			const EmaString& serviceName = enc.getServiceName();
			RsslUInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );
			if ( !pServiceId )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this UpdateMsg." );
				handleIue( temp );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this UpdateMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

			submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const UpdateMsg& )" );
				return;
			}
		}
		else if ( enc.hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this UpdateMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )" );
				return;
			}
		}
		else
		{
			_userLock.unlock();
			handleIue( "Attempt to submit UpdateMsg without service name or id. Dropping this UpdateMsg." );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( bHandleAdded )
		{
			StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
			_streamInfoList.removeValue( *pTempStreamInfoPtr );
			delete *pTempStreamInfoPtr;
			_handleToStreamInfo.erase( handle );
			pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
		}

		_userLock.unlock();

		if ( swapBuffer.data )
		{
			submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
			free( swapBuffer.data );
		}

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const UpdateMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}

	if ( swapBuffer.data )
	{
		submitMsgOpts.pRsslMsg->msgBase.encDataBody = origPayload;
		free( swapBuffer.data );
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit( const StatusMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const StatusMsgEncoder&>( msg.getEncoder() ).getRsslStatusMsg();

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamId > 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const StatusMsg& ) using a registered handle." );
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received StatusMsg with SourceDirectory domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( _activeConfig.mergeSourceDirectoryStreams )
		{
			if ( _activeConfig.refreshFirstRequired && !_bIsStreamIdZeroRefreshSubmitted )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
				temp.append( handle ).append( "." );
				handleIhe( handle, temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId = 0;
		}
		else
		{
			if ( pStreamInfoPtr )
			{
				submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			}
			else
			{
				if ( _activeConfig.refreshFirstRequired )
				{
					_userLock.unlock();
					EmaString temp( "Attempt to submit StatusMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
					temp.append( handle ).append( "." );
					handleIhe( handle, temp );
					return;
				}

				try
				{
					submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();
					StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc )
				{
					pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_userLock.unlock();
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
					return;
				}
			}
		}
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received StatusMsg with market domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		const StatusMsgEncoder& enc = static_cast<const StatusMsgEncoder&>( msg.getEncoder() );

		if ( pStreamInfoPtr )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			if ( submitMsgOpts.pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY )
			{
				submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = ( *pStreamInfoPtr )->_serviceId;
				submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			}
		}
		else if ( _activeConfig.refreshFirstRequired )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit StatusMsg while RefreshMsg was not submitted on this stream yet. Handle = " );
			temp.append( handle ).append( "." );
			handleIhe( handle, temp );
			return;
		}
		else if ( enc.hasServiceName() )
		{
			const EmaString& serviceName = enc.getServiceName();
			RsslUInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );
			if ( !pServiceId )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this StatusMsg." );
				handleIue( temp );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this StatusMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

			submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
				return;
			}
		}
		else if ( enc.hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit StatusMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this StatusMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId = -pChannel->getNextStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId );
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
				return;
			}
		}
		else
		{
			_userLock.unlock();
			handleIue( "Attempt to submit StatusMsg without service name or id. Dropping this StatusMsg." );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( bHandleAdded )
		{
			StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
			_streamInfoList.removeValue( *pTempStreamInfoPtr );
			delete *pTempStreamInfoPtr;
			_handleToStreamInfo.erase( handle );
			pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
		}

		_userLock.unlock();

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const StatusMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );

		return;
	}

	if ( submitMsgOpts.pRsslMsg->statusMsg.state.streamState == OmmState::ClosedEnum ||
		submitMsgOpts.pRsslMsg->statusMsg.state.streamState == OmmState::ClosedRecoverEnum ||
		submitMsgOpts.pRsslMsg->statusMsg.state.streamState == OmmState::ClosedRedirectedEnum )
	{
		StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
		_streamInfoList.removeValue( *pTempStreamInfoPtr );
		delete *pTempStreamInfoPtr;
		_handleToStreamInfo.erase( handle );
		pChannel->returnStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit( const GenericMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const GenericMsgEncoder&>( msg.getEncoder() ).getRsslGenericMsg();

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Received GenericMsg; Handle = " );
		temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr )
	{
		if ( ( *pStreamInfoPtr )->_streamId > 0 )
		{
			_userLock.unlock();
			OmmBaseImpl::submit( msg, handle );
			return;
		}

		submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
	}
	else
	{
		_userLock.unlock();
		EmaString temp( "Attempt to submit GenericMsg on stream that is not open yet. Handle = " );
		temp.append( handle ).append( "." );
		handleIhe( handle, temp );
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		_userLock.unlock();
		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const GenericMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::setRsslReactorChannelRole( RsslReactorChannelRole& role )
{
	RsslReactorOMMNIProviderRole& niProviderRole = role.ommNIProviderRole;
	rsslClearOMMNIProviderRole( &niProviderRole );
	niProviderRole.pLoginRequest = getLoginCallbackClient().getLoginRequest();
	niProviderRole.pDirectoryRefresh = 0;
	niProviderRole.loginMsgCallback = OmmBaseImpl::loginCallback;
	niProviderRole.base.defaultMsgCallback = OmmBaseImpl::itemCallback;
	niProviderRole.base.channelEventCallback = OmmBaseImpl::channelCallback;
}

bool OmmNiProviderImpl::decodeSourceDirectory( RwfBuffer* pInBuffer, RsslBuffer* pOutBuffer, EmaString& errorText )
{
	if ( !pOutBuffer )
	{
		errorText.set( "Internal error: pOutBuffer was not assigned in OmmNiProviderImpl::decodeSourceDirectory()." );
		return false;
	}

	RsslRet retCode = RSSL_RET_SUCCESS;
	RsslDecodeIterator dIter;
	rsslClearDecodeIterator( &dIter );

	retCode = rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		errorText.set( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::decodeServiceNameAndId(). Reason = " );
		errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
		return false;
	}

	retCode = rsslSetDecodeIteratorBuffer( &dIter, pInBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		errorText.set( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::decodeServiceNameAndId(). Reason = " );
		errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
		return false;
	}

	RsslMap rsslMap;
	rsslClearMap( &rsslMap );

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Begin decoding of SourceDirectory." );

	retCode = rsslDecodeMap( &dIter, &rsslMap );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		errorText.set( "Internal error. Failed to decode rsslMap in OmmNiProviderImpl::decodeServiceNameAndId(). Reason = " );
		errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
		return false;
	}
	else if ( retCode == RSSL_RET_NO_DATA )
	{
		if ( OmmLoggerClient::WarningEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::WarningEnum, "Passed in SourceDirectory map contains no entries (e.g. there is no service specified)." );

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "End decoding of SourceDirectory." );

		return true;
	}

	switch ( rsslMap.keyPrimitiveType )
	{
	case RSSL_DT_UINT:
		if ( !decodeSourceDirectoryKeyUInt( rsslMap, dIter, errorText ) )
			return false;
		break;
	case RSSL_DT_ASCII_STRING:
		if ( !decodeSourceDirectoryKeyAscii( rsslMap, dIter, errorText ) )
			return false;

		if ( !swapServiceNameAndId( pInBuffer, pOutBuffer, errorText ) )
			return false;
		break;
	default:
		errorText.set( "Attempt to specify SourceDirectory info with a Map using key DataType of " );
		errorText += DataType( dataType[rsslMap.keyPrimitiveType] ).toString();
		errorText += EmaString( " while the expected key DataType is " );
		errorText += DataType( DataType::UIntEnum ).toString();
		return false;
	}

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "End decoding of SourceDirectory." );

	return true;
}

bool OmmNiProviderImpl::decodeSourceDirectoryKeyAscii( RsslMap& rsslMap, RsslDecodeIterator& dIter, EmaString& errorText )
{
	RsslRet retCode = RSSL_RET_SUCCESS;
	RsslBuffer serviceNameBuffer;
	rsslClearBuffer( &serviceNameBuffer );

	UInt64 emaAssignedServiceId = 0;
	RsslMapEntry rsslMapEntry;
	rsslClearMapEntry( &rsslMapEntry );
	while ( ( retCode = rsslDecodeMapEntry( &dIter, &rsslMapEntry, &serviceNameBuffer ) ) != RSSL_RET_END_OF_CONTAINER )
	{
		if ( retCode != RSSL_RET_SUCCESS )
		{
			errorText.set( "Internal error: Failed to Decode Map Entry. Reason = " );
			errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
			return false;
		}

		EmaString serviceName( serviceNameBuffer.data, serviceNameBuffer.length );

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Begin decoding of Service with name of " );
			temp.append( serviceName ).append( ". Action = " );
			switch ( rsslMapEntry.action )
			{
			case RSSL_MPEA_UPDATE_ENTRY:
				temp.append( "Update" );
				break;
			case RSSL_MPEA_ADD_ENTRY:
				temp.append( "Add" );
				break;
			case RSSL_MPEA_DELETE_ENTRY:
				temp.append( "Delete" );
				break;
			}

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( rsslMapEntry.action == RSSL_MPEA_DELETE_ENTRY )
		{
			UInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );

			if ( pServiceId )
			{
				UInt64 serviceId = *pServiceId;

				EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find( serviceId );
				if ( !pServiceNamePtr )
				{
					errorText.set( "Internal error: mismatch between _serviceNameToServiceId and _serviceIdToServiceName tables." )
						.append( " ServiceName = " ).append( serviceName ).append( " serviceId = " ).append( serviceId ).append( ". " );
					return false;
				}

				EmaStringPtr pTemp = *pServiceNamePtr;
				_serviceNameToServiceId.erase( pTemp );
				_serviceIdToServiceName.erase( serviceId );
				_serviceNameList.removeValue( pTemp );
				delete pTemp;
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "End decoding of Service with name of " );
				temp.append( serviceName );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}

			continue;
		}
		else if ( rsslMapEntry.action == RSSL_MPEA_ADD_ENTRY )
		{
			UInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );

			if ( pServiceId )
			{
				errorText.set( "Attempt to add a service with name of " );
				errorText.append( serviceName ).append( " and id of " ).append( *pServiceId ).append( " while a service with the same name is already added." );
				return false;
			}
		}

		if ( rsslMap.containerType != RSSL_DT_FILTER_LIST )
		{
			errorText.set( "Attempt to specify Service with a container of " );
			errorText += DataType( dataType[rsslMap.containerType] ).toString();
			errorText += EmaString( " rather than the expected " );
			errorText += DataType( DataType::FilterListEnum ).toString();
			return false;
		}

		RsslFilterList rsslFilterList;
		RsslFilterEntry rsslFilterEntry;
		rsslClearFilterList( &rsslFilterList );
		rsslClearFilterEntry( &rsslFilterEntry );

		retCode = rsslDecodeFilterList( &dIter, &rsslFilterList );

		if ( retCode < RSSL_RET_SUCCESS )
		{
			errorText.set( "Internal error: Failed to Decode FilterList. Reason = " );
			errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
			return false;
		}
		else if ( retCode == RSSL_RET_NO_DATA )
		{
			if ( OmmLoggerClient::WarningEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Service with name of " );
				temp.append( serviceName ).append( " contains no FilterEntries. Skipping this service." );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::WarningEnum, temp );
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "End decoding of Service with name of " );
				temp.append( serviceName );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}

			continue;
		}

		while ( ( retCode = rsslDecodeFilterEntry( &dIter, &rsslFilterEntry ) ) != RSSL_RET_END_OF_CONTAINER )
		{
			if ( retCode < RSSL_RET_SUCCESS )
			{
				errorText.set( "Internal error: Failed to Decode Filter Entry. Reason = " );
				errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
				return false;
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Begin decoding of FilterEntry with id of " );
				temp.append( rsslFilterEntry.id );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}

			if ( rsslFilterEntry.id == RDM_DIRECTORY_SERVICE_INFO_ID )
			{
				if ( rsslMapEntry.action == RSSL_MPEA_UPDATE_ENTRY )
				{
					errorText.set( "Attempt to update Infofilter of service with name of " );
					errorText.append( serviceName ).append( " while this is not allowed." );
					return false;
				}

				if ( ( ( rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE ) && rsslFilterEntry.containerType != RSSL_DT_ELEMENT_LIST ) ||
					rsslFilterList.containerType != RSSL_DT_ELEMENT_LIST )
				{
					RsslContainerType type = ( rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE ) ? rsslFilterEntry.containerType : rsslFilterList.containerType;
					errorText.set( "Attempt to specify Service InfoFilter with a container of " );
					errorText += DataType( dataType[type] ).toString();
					errorText += EmaString( " rather than the expected " );
					errorText += DataType( DataType::ElementListEnum ).toString();
					return false;
				}

				RsslElementList	rsslElementList;
				RsslElementEntry rsslElementEntry;
				rsslClearElementList( &rsslElementList );
				rsslClearElementEntry( &rsslElementEntry );

				if ( ( retCode = rsslDecodeElementList( &dIter, &rsslElementList, NULL ) ) < RSSL_RET_SUCCESS )
				{
					errorText.set( "Internal error: Failed to Decode Element List. Reason = " );
					errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
					return false;
				}

				bool bServiceIdEntryFound = false;

				while ( ( retCode = rsslDecodeElementEntry( &dIter, &rsslElementEntry ) ) != RSSL_RET_END_OF_CONTAINER )
				{
					if ( retCode < RSSL_RET_SUCCESS )
					{
						errorText.set( "Internal error: Failed to Decode ElementEntry. Reason = " );
						errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
						return false;
					}

					if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Decoding of ElementEntry with name of " );
						temp.append( EmaString( rsslElementEntry.name.data, rsslElementEntry.name.length ) );
						getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
					}

					static const RsslBuffer ENAME_SERVICE_ID = { 9 , ( char* )"ServiceID" };
					if ( !bServiceIdEntryFound && rsslBufferIsEqual( &rsslElementEntry.name, &ENAME_SERVICE_ID ) )
					{
						if ( rsslElementEntry.dataType != RSSL_DT_UINT )
						{
							errorText.set( "Attempt to specify Service Id with a " );
							errorText += DataType( dataType[rsslElementEntry.dataType] ).toString();
							errorText += EmaString( " rather than the expected " );
							errorText += DataType( DataType::UIntEnum ).toString();
							return false;
						}

						UInt64 serviceId = 0;

						retCode = rsslDecodeUInt( &dIter, &serviceId );
						if ( retCode < RSSL_RET_SUCCESS )
						{
							errorText.set( "Internal error: Failed to Decode UInt. Reason = " );
							errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
							return false;
						}
						else if ( retCode == RSSL_RET_BLANK_DATA )
						{
							errorText.set( "Attempt to specify Service Id with a blank UInt for service name of " );
							errorText.append( serviceName );
							return false;
						}

						bServiceIdEntryFound = true;

						if ( _serviceIdToServiceName.find( serviceId ) )
						{
							errorText.set( "Attempt to add a service with name of " );
							errorText.append( serviceName ).append( " and id of " ).append( serviceId ).append( " while a service with the same id is already added." );

							return false;
						}

						EmaStringPtr pServiceName = new EmaString( serviceName );
						_serviceNameToServiceId.insert( pServiceName, serviceId );
						_serviceIdToServiceName.insert( serviceId, pServiceName );
						_serviceNameList.push_back( pServiceName );

						if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
						{
							EmaString temp( "Detected Service with name of " );
							temp.append( *pServiceName ).append( " and Id of " ).append( serviceId );
							getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
						}
					}
				}

				if ( !bServiceIdEntryFound )
				{
					const UInt16 maxUInt16 = 0xFFFF;
					bool found = true;
					while ( _serviceIdToServiceName.find( emaAssignedServiceId ) )
					{
						if ( emaAssignedServiceId == maxUInt16 )
						{
							found = false;
							break;
						}
						++emaAssignedServiceId;
					}

					if ( !found )
					{
						errorText.set( "All service ids are used." );
						return false;
					}

					EmaStringPtr pServiceName = new EmaString( serviceName );
					_serviceNameToServiceId.insert( pServiceName, emaAssignedServiceId );
					_serviceIdToServiceName.insert( emaAssignedServiceId, pServiceName );
					_serviceNameList.push_back( pServiceName );

					if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Assigned service id of " );
						temp.append( emaAssignedServiceId ).append( " to service with name of " ).append( *pServiceName );
						getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
					}
				}
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "End decoding of FilterEntry with id of " );
				temp.append( rsslFilterEntry.id );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}
		}
	}

	return true;
}

bool OmmNiProviderImpl::decodeSourceDirectoryKeyUInt( RsslMap& rsslMap, RsslDecodeIterator& dIter, EmaString& errorText )
{
	RsslRet retCode = RSSL_RET_SUCCESS;
	RsslUInt64 serviceId = 0;
	RsslMapEntry rsslMapEntry;
	rsslClearMapEntry( &rsslMapEntry );
	while ( ( retCode = rsslDecodeMapEntry( &dIter, &rsslMapEntry, &serviceId ) ) != RSSL_RET_END_OF_CONTAINER )
	{
		if ( retCode != RSSL_RET_SUCCESS )
		{
			errorText.set( "Internal error: Failed to Decode Map Entry. Reason = " );
			errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
			return false;
		}

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Begin decoding of Service with id of " );
			temp.append( serviceId ).append( ". Action = " );
			switch ( rsslMapEntry.action )
			{
			case RSSL_MPEA_UPDATE_ENTRY:
				temp.append( "Update" );
				break;
			case RSSL_MPEA_ADD_ENTRY:
				temp.append( "Add" );
				break;
			case RSSL_MPEA_DELETE_ENTRY:
				temp.append( "Delete" );
				break;
			}

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( rsslMapEntry.action == RSSL_MPEA_DELETE_ENTRY )
		{
			EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find( serviceId );

			if ( pServiceNamePtr )
			{
				UInt64* pServiceId = _serviceNameToServiceId.find( *pServiceNamePtr );
				if ( !pServiceId )
				{
					errorText.set( "Internal error: mismatch between _serviceIdToServiceName and _serviceNameToServiceId tables." )
						.append( " ServiceName = " ).append( **pServiceNamePtr ).append( " serviceId = " ).append( serviceId );
					return false;
				}

				EmaStringPtr pTemp = *pServiceNamePtr;
				_serviceNameToServiceId.erase( pTemp );
				_serviceIdToServiceName.erase( serviceId );
				_serviceNameList.removeValue( pTemp );
				delete pTemp;
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "End decoding of Service with id of " );
				temp.append( serviceId );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}

			continue;
		}
		else if ( rsslMapEntry.action == RSSL_MPEA_ADD_ENTRY )
		{
			EmaStringPtr* pServiceNamePtr = _serviceIdToServiceName.find( serviceId );

			if ( pServiceNamePtr )
			{
				errorText.set( "Attempt to add a service with name of " );
				errorText.append( **pServiceNamePtr ).append( " and id of " ).append( serviceId ).append( " while a service with the same id is already added." );
				return false;
			}
		}

		if ( rsslMap.containerType != RSSL_DT_FILTER_LIST )
		{
			errorText.set( "Attempt to specify Service with a container of " );
			errorText += DataType( dataType[rsslMap.containerType] ).toString();
			errorText += EmaString( " rather than the expected " );
			errorText += DataType( DataType::FilterListEnum ).toString();
			return false;
		}

		RsslFilterList rsslFilterList;
		RsslFilterEntry rsslFilterEntry;
		rsslClearFilterList( &rsslFilterList );
		rsslClearFilterEntry( &rsslFilterEntry );

		retCode = rsslDecodeFilterList( &dIter, &rsslFilterList );

		if ( retCode < RSSL_RET_SUCCESS )
		{
			errorText.set( "Internal error: Failed to Decode FilterList. Reason = " );
			errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
			return false;
		}
		else if ( retCode == RSSL_RET_NO_DATA )
		{
			if ( OmmLoggerClient::WarningEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Service with id of " );
				temp.append( serviceId ).append( " contains no FilterEntries. Skipping this service." );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::WarningEnum, temp );
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "End decoding of Service with id of " );
				temp.append( serviceId );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}

			continue;
		}

		while ( ( retCode = rsslDecodeFilterEntry( &dIter, &rsslFilterEntry ) ) != RSSL_RET_END_OF_CONTAINER )
		{
			if ( retCode < RSSL_RET_SUCCESS )
			{
				errorText.set( "Internal error: Failed to Decode Filter Entry. Reason = " );
				errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
				return false;
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Begin decoding of FilterEntry with id of " );
				temp.append( rsslFilterEntry.id );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}

			if ( rsslFilterEntry.id == RDM_DIRECTORY_SERVICE_INFO_ID )
			{
				if ( rsslMapEntry.action == RSSL_MPEA_UPDATE_ENTRY )
				{
					errorText.set( "Attempt to update Infofilter of service with id of " );
					errorText.append( serviceId ).append( " while this is not allowed." );
					return false;
				}

				if ( ( ( rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE ) && rsslFilterEntry.containerType != RSSL_DT_ELEMENT_LIST ) ||
					rsslFilterList.containerType != RSSL_DT_ELEMENT_LIST )
				{
					RsslContainerType type = ( rsslFilterEntry.flags & RSSL_FTEF_HAS_CONTAINER_TYPE ) ? rsslFilterEntry.containerType : rsslFilterList.containerType;
					errorText.set( "Attempt to specify Service InfoFilter with a container of " );
					errorText += DataType( dataType[type] ).toString();
					errorText += EmaString( " rather than the expected " );
					errorText += DataType( DataType::ElementListEnum ).toString();
					return false;
				}

				RsslElementList	rsslElementList;
				RsslElementEntry rsslElementEntry;
				rsslClearElementList( &rsslElementList );
				rsslClearElementEntry( &rsslElementEntry );

				if ( ( retCode = rsslDecodeElementList( &dIter, &rsslElementList, NULL ) ) < RSSL_RET_SUCCESS )
				{
					errorText.set( "Internal error: Failed to Decode Element List. Reason = " );
					errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
					return false;
				}

				bool bServiceNameEntryFound = false;

				while ( ( retCode = rsslDecodeElementEntry( &dIter, &rsslElementEntry ) ) != RSSL_RET_END_OF_CONTAINER )
				{
					if ( retCode < RSSL_RET_SUCCESS )
					{
						errorText.set( "Internal error: Failed to Decode ElementEntry. Reason = " );
						errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
						return false;
					}

					if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Decoding of ElementEntry with name of " );
						temp.append( EmaString( rsslElementEntry.name.data, rsslElementEntry.name.length ) );
						getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
					}

					if ( !bServiceNameEntryFound && rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_NAME ) )
					{
						if ( rsslElementEntry.dataType != RSSL_DT_ASCII_STRING )
						{
							errorText.set( "Attempt to specify Service Name with a " );
							errorText += DataType( dataType[rsslElementEntry.dataType] ).toString();
							errorText += EmaString( " rather than the expected " );
							errorText += DataType( DataType::AsciiEnum ).toString();
							return false;
						}

						RsslBuffer serviceNameBuffer;
						rsslClearBuffer( &serviceNameBuffer );

						retCode = rsslDecodeBuffer( &dIter, &serviceNameBuffer );
						if ( retCode < RSSL_RET_SUCCESS )
						{
							errorText.set( "Internal error: Failed to Decode Buffer. Reason = " );
							errorText.append( rsslRetCodeToString( retCode ) ).append( ". " );
							return false;
						}
						else if ( retCode == RSSL_RET_BLANK_DATA )
						{
							errorText.set( "Attempt to specify Service Name with a blank ascii string for service id of " );
							errorText.append( serviceId );
							return false;
						}

						bServiceNameEntryFound = true;

						EmaStringPtr pServiceName = new EmaString( serviceNameBuffer.data, serviceNameBuffer.length );

						if ( _serviceNameToServiceId.find( pServiceName ) )
						{
							errorText.set( "Attempt to add a service with name of " );
							errorText.append( *pServiceName ).append( " and id of " ).append( serviceId ).append( " while a service with the same id is already added." );

							delete pServiceName;
							return false;
						}

						_serviceNameToServiceId.insert( pServiceName, serviceId );
						_serviceIdToServiceName.insert( serviceId, pServiceName );
						_serviceNameList.push_back( pServiceName );

						if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
						{
							EmaString temp( "Detected Service with name of " );
							temp.append( *pServiceName ).append( " and Id of " ).append( serviceId );
							getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
						}
					}
				}

				if ( !bServiceNameEntryFound )
				{
					errorText.set( "Attempt to specify service InfoFilter without required Service Name for service id of " );
					errorText.append( serviceId );
					return false;
				}
			}

			if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "End decoding of FilterEntry with id of " );
				temp.append( rsslFilterEntry.id );
				getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
			}
		}
	}

	return true;
}

bool OmmNiProviderImpl::realocateBuffer( RsslBuffer* pBuffer1, RsslBuffer* pBuffer2, RsslEncodeIterator* pEncIter, EmaString& errorText )
{
	RsslBuffer* pTemp1 = pBuffer1->data ? pBuffer1 : pBuffer2;
	RsslBuffer* pTemp2 = pBuffer1->data ? pBuffer2 : pBuffer1;

	pTemp2->length = pTemp1->length << 1;
	pTemp2->data = (char*) malloc( pTemp2->length * sizeof( char ) );

	if ( !pTemp2->data )
	{
		free( pTemp1->data );
		rsslClearBuffer( pTemp1 );

		errorText.set( "Failed to allocate memory in OmmNiProviderImpl::swapServiceNameAndId()" );
		return false;
	}

	rsslRealignEncodeIteratorBuffer( pEncIter, pTemp2 );

	free( pTemp1->data );
	rsslClearBuffer( pTemp1 );

	return true;
}

bool OmmNiProviderImpl::swapServiceNameAndId( RsslBuffer* pInBuffer, RsslBuffer* pOutBuffer, EmaString& errorText )
{
	RsslDecodeIterator dIter;
	rsslClearDecodeIterator( &dIter );
	rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetDecodeIteratorBuffer( &dIter, pInBuffer );

	RsslMap inRsslMap;
	rsslClearMap( &inRsslMap );

	pOutBuffer->length = 4096;
	pOutBuffer->data = (char*) malloc( pOutBuffer->length * sizeof( char ) );
	if ( !pOutBuffer->data )
	{
		pOutBuffer->length = 0;
		errorText.set( "Failed to allocate memory in OmmNiProviderImpl::swapServiceNameAndId()" );
		return false;
	}

	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );
	rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &eIter, pOutBuffer );

	RsslMap outRsslMap;
	rsslClearMap( &outRsslMap );
	outRsslMap.keyPrimitiveType = RSSL_DT_UINT;
	outRsslMap.flags = RSSL_MPF_NONE;
	outRsslMap.containerType = RSSL_DT_FILTER_LIST;

	rsslEncodeMapInit( &eIter, &outRsslMap, 0, 0 );

	if ( RSSL_RET_NO_DATA == rsslDecodeMap( &dIter, &inRsslMap ) )
	{
		rsslEncodeMapComplete( &eIter, RSSL_TRUE );
		pOutBuffer->length = rsslGetEncodedBufferLength( &eIter );
		return true;
	}

	RsslBuffer serviceNameBuffer;
	rsslClearBuffer( &serviceNameBuffer );

	RsslMapEntry inRsslMapEntry;
	rsslClearMapEntry( &inRsslMapEntry );

	RsslMapEntry outRsslMapEntry;

	RsslBuffer reallocateHelper;
	rsslClearBuffer( &reallocateHelper );

	while ( RSSL_RET_END_OF_CONTAINER != rsslDecodeMapEntry( &dIter, &inRsslMapEntry, &serviceNameBuffer ) )
	{
		rsslClearMapEntry( &outRsslMapEntry );
		outRsslMapEntry.flags = RSSL_MPEF_NONE;
		outRsslMapEntry.action = inRsslMapEntry.action;

		EmaString serviceName( serviceNameBuffer.data, serviceNameBuffer.length );

		UInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );

		if ( inRsslMapEntry.action == RSSL_MPEA_DELETE_ENTRY )
		{
			while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeMapEntry( &eIter, &outRsslMapEntry, pServiceId ) )
			{
				if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
				{
					if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
					return false;
				}
			}

			continue;
		}

		RsslFilterList inRsslFilterList;
		RsslFilterEntry inRsslFilterEntry;
		rsslClearFilterList( &inRsslFilterList );
		rsslClearFilterEntry( &inRsslFilterEntry );

		RsslFilterList outRsslFilterList;
		rsslClearFilterList( &outRsslFilterList );
		outRsslFilterList.containerType = RSSL_DT_ELEMENT_LIST;
		outRsslFilterList.flags = RSSL_FTF_NONE;

		while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeMapEntryInit( &eIter, &outRsslMapEntry, pServiceId, 0 ) )
		{
			rsslEncodeMapEntryComplete( &eIter, RSSL_FALSE );
			if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
			{
				if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
				return false;
			}
		}

		while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeFilterListInit( &eIter, &outRsslFilterList ) )
		{
			rsslEncodeFilterListComplete( &eIter, RSSL_FALSE );
			if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
			{
				if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
				return false;
			}
		}

		if ( RSSL_RET_NO_DATA == rsslDecodeFilterList( &dIter, &inRsslFilterList ) )
		{
			rsslEncodeFilterListComplete( &eIter, RSSL_TRUE );
			rsslEncodeMapEntryComplete( &eIter, RSSL_TRUE );

			continue;
		}

		RsslFilterEntry outRsslFilterEntry;

		while ( RSSL_RET_END_OF_CONTAINER != rsslDecodeFilterEntry( &dIter, &inRsslFilterEntry ) )
		{
			rsslClearFilterEntry( &outRsslFilterEntry );
			outRsslFilterEntry = inRsslFilterEntry;

			if ( inRsslFilterEntry.id == RDM_DIRECTORY_SERVICE_INFO_ID )
			{
				rsslClearBuffer( &outRsslFilterEntry.encData );

				RsslElementList	inRsslElementList;
				RsslElementEntry inRsslElementEntry;
				rsslClearElementList( &inRsslElementList );
				rsslClearElementEntry( &inRsslElementEntry );

				RsslElementList	outRsslElementList;
				rsslClearElementList( &outRsslElementList );

				if ( RSSL_TRUE == rsslElementListHasInfo( &inRsslElementList ) )
				{
					outRsslElementList.elementListNum = inRsslElementList.elementListNum;
					outRsslElementList.flags |= RSSL_ELF_HAS_ELEMENT_LIST_INFO;
				}

				while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeFilterEntryInit( &eIter, &outRsslFilterEntry, 0 ) )
				{
					rsslEncodeFilterEntryComplete( &eIter, RSSL_FALSE );
					if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
					{
						if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
						return false;
					}
				}

				if ( RSSL_RET_NO_DATA == rsslDecodeElementList( &dIter, &inRsslElementList, NULL ) )
				{
					while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeElementListInit( &eIter, &outRsslElementList, 0, 0 ) )
					{
						rsslEncodeElementListComplete( &eIter, RSSL_FALSE );
						if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
						{
							if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
							return false;
						}
					}

					rsslEncodeElementListComplete( &eIter, RSSL_TRUE );
					rsslEncodeFilterEntryComplete( &eIter, RSSL_TRUE );

					continue;
				}

				outRsslElementList.flags |= RSSL_ELF_HAS_STANDARD_DATA;

				while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeElementListInit( &eIter, &outRsslElementList, 0, 0 ) )
				{
					rsslEncodeElementListComplete( &eIter, RSSL_FALSE );
					if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
					{
						if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
						return false;
					}
				}

				RsslElementEntry outRsslElementEntry;

				outRsslElementEntry.dataType = RSSL_DT_ASCII_STRING;
				rsslClearBuffer( &outRsslElementEntry.encData );
				outRsslElementEntry.name = RSSL_ENAME_NAME;

				while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeElementEntry( &eIter, &outRsslElementEntry, &serviceNameBuffer ) )
				{
					if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
					{
						if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
						return false;
					}
				}

				while ( RSSL_RET_END_OF_CONTAINER != rsslDecodeElementEntry( &dIter, &inRsslElementEntry ) )
				{
					rsslClearElementEntry( &outRsslElementEntry );

					static const RsslBuffer ENAME_SERVICE_ID = { 9 , ( char* )"ServiceID" };
					if ( RSSL_FALSE == rsslBufferIsEqual( &inRsslElementEntry.name, &ENAME_SERVICE_ID ) )
					{
						outRsslElementEntry = inRsslElementEntry;
						while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeElementEntry( &eIter, &outRsslElementEntry, 0 ) )
						{
							if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
							{
								if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
								return false;
							}
						}
					}
				}

				rsslEncodeElementListComplete( &eIter, RSSL_TRUE );
				rsslEncodeFilterEntryComplete( &eIter, RSSL_TRUE );
			}
			else
			{
				while ( RSSL_RET_BUFFER_TOO_SMALL == rsslEncodeFilterEntry( &eIter, &outRsslFilterEntry ) )
				{
					if ( !realocateBuffer( pOutBuffer, &reallocateHelper, &eIter, errorText ) )
					{
						if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;
						return false;
					}
				}
			}
		}

		rsslEncodeFilterListComplete( &eIter, RSSL_TRUE );
		rsslEncodeMapEntryComplete( &eIter, RSSL_TRUE );
	}

	rsslEncodeMapComplete( &eIter, RSSL_TRUE );

	if ( !pOutBuffer->data ) *pOutBuffer = reallocateHelper;

	pOutBuffer->length = rsslGetEncodedBufferLength( &eIter );

	return true;
}

size_t OmmNiProviderImpl::UInt64rHasher::operator()( const UInt64& value ) const
{
	return value;
}

bool OmmNiProviderImpl::UInt64Equal_To::operator()( const UInt64& x, const UInt64& y ) const
{
	return x == y;
}

size_t OmmNiProviderImpl::EmaStringPtrHasher::operator()( const EmaStringPtr& value ) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while ( n-- )
		result = ( ( result % magic ) << 8 ) + (size_t) * s++;
	return result;
}

bool OmmNiProviderImpl::EmaStringPtrEqual_To::operator()( const EmaStringPtr& x, const EmaStringPtr& y ) const
{
	return *x == *y;
}

bool OmmNiProviderImpl::isApiDispatching() const
{
	return _activeConfig.operationModel == OmmNiProviderConfig::ApiDispatchEnum ? true : false;
}

bool OmmNiProviderImpl::getServiceId( const EmaString& serviceName, UInt64& serviceId )
{
	bool retCode = false;

	UInt64* pServiceId = _serviceNameToServiceId.find( &serviceName );

	if ( pServiceId )
	{
		serviceId = *pServiceId;
		retCode = true;
	}

	return retCode;
}

bool OmmNiProviderImpl::getServiceName( UInt64 serviceId, EmaString& serviceName )
{
	bool retCode = false;

	EmaStringPtr* pServiceName = _serviceIdToServiceName.find( serviceId );

	if ( pServiceName )
	{
		serviceName = **pServiceName;
		retCode = true;
	}

	return retCode;
}
