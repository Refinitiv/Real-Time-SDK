/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_Ema_h
#define __refinitiv_ema_Ema_h

/**
	@file Ema.h "Ema.h"
	@brief Ema.h file is a collection of all the EMA public header files.
	
	For simplicity EMA applications may just include this one file instead of listing
	individual header files.
*/

#include "Access/Include/Common.h"

#include "Access/Include/EmaBuffer.h"
#include "Access/Include/EmaBufferU16.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/EmaVector.h"
#include "Access/Include/RmtesBuffer.h"

#include "Rdm/Include/EmaRdm.h"

#include "Access/Include/DataType.h"

#include "Access/Include/Data.h"
#include "Access/Include/ComplexType.h"

#include "Access/Include/OmmAnsiPage.h"
#include "Access/Include/OmmAscii.h"
#include "Access/Include/OmmBuffer.h"
#include "Access/Include/OmmDate.h"
#include "Access/Include/OmmDateTime.h"
#include "Access/Include/OmmDouble.h"
#include "Access/Include/OmmEnum.h"
#include "Access/Include/OmmError.h"
#include "Access/Include/OmmFloat.h"
#include "Access/Include/OmmInt.h"
#include "Access/Include/OmmOpaque.h"
#include "Access/Include/OmmQos.h"
#include "Access/Include/OmmReal.h"
#include "Access/Include/OmmRmtes.h"
#include "Access/Include/OmmState.h"
#include "Access/Include/OmmTime.h"
#include "Access/Include/OmmUInt.h"
#include "Access/Include/OmmUtf8.h"
#include "Access/Include/OmmXml.h"

#include "Access/Include/OmmArray.h"
#include "Access/Include/DateTimeStringFormat.h"
#include "Access/Include/ElementList.h"
#include "Access/Include/FieldList.h"
#include "Access/Include/FilterList.h"
#include "Access/Include/Map.h"
#include "Access/Include/Series.h"
#include "Access/Include/Vector.h"

#include "Access/Include/Attrib.h"
#include "Access/Include/Payload.h"
#include "Access/Include/SummaryData.h"
#include "Access/Include/Key.h"

#include "Access/Include/OmmArrayEntry.h"
#include "Access/Include/ElementEntry.h"
#include "Access/Include/FieldEntry.h"
#include "Access/Include/FilterEntry.h"
#include "Access/Include/MapEntry.h"
#include "Access/Include/SeriesEntry.h"
#include "Access/Include/VectorEntry.h"

#include "Access/Include/Msg.h"
#include "Access/Include/AckMsg.h"
#include "Access/Include/GenericMsg.h"
#include "Access/Include/PostMsg.h"
#include "Access/Include/RefreshMsg.h"
#include "Access/Include/ReqMsg.h"
#include "Access/Include/StatusMsg.h"
#include "Access/Include/UpdateMsg.h"

#include "Access/Include/OmmException.h"
#include "Access/Include/OmmUnsupportedDomainTypeException.h"
#include "Access/Include/OmmInvalidConfigurationException.h"
#include "Access/Include/OmmInvalidHandleException.h"
#include "Access/Include/OmmInvalidUsageException.h"
#include "Access/Include/OmmInaccessibleLogFileException.h"
#include "Access/Include/OmmMemoryExhaustionException.h"
#include "Access/Include/OmmOutOfRangeException.h"
#include "Access/Include/OmmSystemException.h"
#include "Access/Include/OmmJsonConverterException.h"

#include "Access/Include/OmmConsumer.h"
#include "Access/Include/OmmConsumerClient.h"
#include "Access/Include/OmmConsumerConfig.h"
#include "Access/Include/OmmConsumerErrorClient.h"
#include "Access/Include/OmmConsumerEvent.h"
#include "Access/Include/OmmConsumerRestLoggingEvent.h"
#include "Access/Include/OmmRestLoggingClient.h"

#include "Access/Include/OmmProvider.h"
#include "Access/Include/OmmProviderClient.h"
#include "Access/Include/OmmProviderConfig.h"
#include "Access/Include/OmmProviderErrorClient.h"
#include "Access/Include/OmmProviderEvent.h"
#include "Access/Include/OmmIProviderConfig.h"
#include "Access/Include/OmmNiProviderConfig.h"

#include "Access/Include/TunnelStreamRequest.h"
#include "Access/Include/ChannelInformation.h"
#include "Access/Include/ChannelStatistics.h"

#include "Domain/Login/Include/Login.h"

#include "Rdm/Include/DataDictionary.h"
#include "Rdm/Include/DictionaryEntry.h"
#include "Rdm/Include/EnumType.h"
#include "Rdm/Include/EnumTypeTable.h"
#include "Rdm/Include/DictionaryUtility.h"

#include "Access/Include/ServiceEndpointDiscovery.h"
#include "Access/Include/ServiceEndpointDiscoveryClient.h"
#include "Access/Include/ServiceEndpointDiscoveryEvent.h"
#include "Access/Include/ServiceEndpointDiscoveryInfo.h"
#include "Access/Include/ServiceEndpointDiscoveryOption.h"
#include "Access/Include/ServiceEndpointDiscoveryResp.h"

#include "Access/Include/IOCtlCode.h"
#include "Access/Include/IOCtlReactorCode.h"

#include "Access/Include/SessionInfo.h"
#include "Access/Include/ConsumerSessionInfo.h"
#include "Access/Include/ProviderSessionInfo.h"

#include "Access/Include/GetTime.h"

#endif // __refinitiv_ema_Ema_h
