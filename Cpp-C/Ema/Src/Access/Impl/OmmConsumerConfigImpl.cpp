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

OmmConsumerConfigImpl::OmmConsumerConfigImpl()
#ifdef WIN32
#pragma warning( disable : 4355 )
#endif
{
	_userNodeName = "ConsumerGroup|ConsumerList|Consumer.";
	verifyXMLdefaultConsumer();
}

OmmConsumerConfigImpl::~OmmConsumerConfigImpl()
{
}

void OmmConsumerConfigImpl::consumerName( const EmaString& consumerName )
{
	if ( _pProgrammaticConfigure && _pProgrammaticConfigure->specifyConsumerName( consumerName ) )
	{
		return;
	}

	EmaString item( "ConsumerGroup|ConsumerList|Consumer." );
	item.append( consumerName ).append( "|Name" );
	EmaString name;
	if ( get( item, name ) ) {
		if ( ! set( "ConsumerGroup|DefaultConsumer", consumerName) )
		{
			EmaString mergeString( "<EmaConfig><ConsumerGroup><DefaultConsumer value=\"" );
			mergeString.append( consumerName ).append( "\"/></ConsumerGroup></EmaConfig>" );
			xmlDocPtr xmlDoc = xmlReadMemory( mergeString.c_str(), mergeString.length(), NULL, "notnamed.xml", XML_PARSE_HUGE );
			if ( xmlDoc == NULL )
				return;
			xmlNodePtr _xmlNodePtr = xmlDocGetRootElement( xmlDoc );
			if ( _xmlNodePtr == NULL )
				return;
			if ( xmlStrcmp(_xmlNodePtr->name, (const xmlChar *) "EmaConfig" ) )
				return;
			XMLnode * tmp( new XMLnode("EmaConfig", 0, 0) );
			processXMLnodePtr(tmp, _xmlNodePtr);
			_pEmaConfig->merge(tmp);
			xmlFreeDoc( xmlDoc );
			delete tmp;
		}
	}
	else
	{
		if ( consumerName == "EmaConsumer" )
		{
			XMLnode * consumerList( _pEmaConfig->find< XMLnode >( "ConsumerGroup|ConsumerList" ) );
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
		errorMsg.append( consumerName ).append( "] is an non-existent consumer name" );
		throwIceException( errorMsg );
	}

}

void OmmConsumerConfigImpl::verifyXMLdefaultConsumer()
{
	_pEmaConfig->verifyDefaultConsumer();
}

EmaString OmmConsumerConfigImpl::getUserName() const
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


void XMLnode::verifyDefaultConsumer()
{
	const EmaString searchString("ConsumerGroup|DefaultConsumer" );
	const EmaString * defaultConsumerName ( find< EmaString >( searchString ) );
	if ( defaultConsumerName )
	{
		XMLnode * consumerList( find< XMLnode >( "ConsumerGroup|ConsumerList" ) );
		if ( consumerList )
		{
			EmaList< NameString* > theNames;
			consumerList->getNames( theNames );

			if ( theNames.empty() && *defaultConsumerName == "EmaConsumer" )
				return;

			NameString * name( theNames.pop_front() );
			while ( name )
			{
				if ( *name == *defaultConsumerName )
					return;
				else name = theNames.pop_front();
			}
			EmaString errorMsg( "specified default consumer name [" );
			errorMsg.append( *defaultConsumerName ).append( "] was not found in the configured consumers" );
			throwIceException( errorMsg );
		}
		else if ( *defaultConsumerName != "EmaConsumer" )
		{
			EmaString errorMsg( "default consumer name [" );
			errorMsg.append( *defaultConsumerName ).append( "] was specified, but no consumers were configured" );
			throwIceException( errorMsg );
		}
	}
}
