/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "OmmConsumerActiveConfig.h"
#include "Map.h"
#include "ElementList.h"
#include "FieldList.h"
#include "ReqMsg.h"
#include "ReqMsgEncoder.h"
#include "ExceptionTranslator.h"
#include "OmmConsumerConfigImpl.h"
#include "StaticDecoder.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

OmmConsumerConfigImpl::OmmConsumerConfigImpl() :
	EmaConfigImpl(),
	_operationModel( OmmConsumerConfig::ApiDispatchEnum )
{
	_instanceNodeName = "ConsumerGroup|ConsumerList|Consumer.";
	_pEmaConfig->verifyDefaultConsumer();
}

OmmConsumerConfigImpl::~OmmConsumerConfigImpl()
{
}

void OmmConsumerConfigImpl::consumerName( const EmaString& consumerName )
{
	if ( _pProgrammaticConfigure && _pProgrammaticConfigure->specifyConsumerName( consumerName ) )
		return;

	EmaString item( "ConsumerGroup|ConsumerList|Consumer." );
	item.append( consumerName ).append( "|Name" );
	EmaString name;
	if ( get( item, name ) )
	{
		if ( ! set( "ConsumerGroup|DefaultConsumer", consumerName ) )
		{
			EmaString mergeString( "<EmaConfig><ConsumerGroup><DefaultConsumer value=\"" );
			mergeString.append( consumerName ).append( "\"/></ConsumerGroup></EmaConfig>" );
			xmlDocPtr xmlDoc = xmlReadMemory( mergeString.c_str(), mergeString.length(), NULL, "notnamed.xml", XML_PARSE_HUGE );
			if ( xmlDoc == NULL )
				return;
			xmlNodePtr _xmlNodePtr = xmlDocGetRootElement( xmlDoc );
			if ( _xmlNodePtr == NULL )
				return;
			if ( xmlStrcmp( _xmlNodePtr->name, ( const xmlChar* ) "EmaConfig" ) )
				return;
			XMLnode* tmp( new XMLnode( "EmaConfig", 0, 0 ) );
			processXMLnodePtr( tmp, _xmlNodePtr );
			_pEmaConfig->merge( tmp );
			xmlFreeDoc( xmlDoc );
			delete tmp;
		}
	}
	else
	{
		if ( consumerName == "EmaConsumer" )
		{
			XMLnode* consumerList( _pEmaConfig->find< XMLnode >( "ConsumerGroup|ConsumerList" ) );
			if ( consumerList )
			{
				EmaList< XMLnode::NameString* > theNames;
				consumerList->getNames( theNames );
				if ( theNames.empty() )
					return;
			}
			else
				return;
		}

		EmaString errorMsg( "OmmConsumerConfigImpl::consumerName parameter [" );
		errorMsg.append( consumerName ).append( "] is a non-existent consumer name" );
		throwIceException( errorMsg );
	}
}

EmaString OmmConsumerConfigImpl::getConfiguredName()
{
	EmaString retVal;

	if ( _pProgrammaticConfigure && _pProgrammaticConfigure->getDefaultConsumer( retVal ) )
		return retVal;

	if ( _pEmaConfig->get< EmaString >( "ConsumerGroup|DefaultConsumer", retVal ) )
	{
		EmaString expectedName( "ConsumerGroup|ConsumerList|Consumer." );
		expectedName.append( retVal ).append( "|Name" );
		EmaString checkValue;
		if ( _pEmaConfig->get( expectedName, checkValue ) )
			return retVal;
		else
		{
			EmaString errorMsg( "default consumer name [" );
			errorMsg.append( retVal ).append( "] is an non-existent consumer name; DefaultConsumer specification ignored" );
			_pEmaConfig->appendErrorMessage( errorMsg, OmmLoggerClient::ErrorEnum );
		}
	}

	if ( _pEmaConfig->get< EmaString >( "ConsumerGroup|ConsumerList|Consumer|Name", retVal ) )
		return retVal;

	return "EmaConsumer";
}

bool OmmConsumerConfigImpl::getDictionaryName( const EmaString& instanceName, EmaString& retVal ) const
{
	if ( !_pProgrammaticConfigure || !_pProgrammaticConfigure->getActiveDictionaryName( instanceName, retVal ) )
	{
		EmaString nodeName( _instanceNodeName );
		nodeName.append( instanceName );
		nodeName.append( "|Dictionary" );
		get<EmaString>( nodeName, retVal );
	}

	return true;
}

bool OmmConsumerConfigImpl::getDirectoryName( const EmaString& instanceName, EmaString& retVal ) const
{
	return false;
}

void OmmConsumerConfigImpl::operationModel( OmmConsumerConfig::OperationModel operationModel )
{
	_operationModel = operationModel;
}

OmmConsumerConfig::OperationModel OmmConsumerConfigImpl::operationModel() const
{
	return _operationModel;
}
