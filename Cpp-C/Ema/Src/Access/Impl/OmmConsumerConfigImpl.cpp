/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.             --
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
#include "OmmInvalidUsageException.h"
#include "OmmConsumerConfigImpl.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

OmmConsumerConfigImpl::OmmConsumerConfigImpl(const EmaString & path) :
	EmaConfigImpl(path),
	_operationModel( OmmConsumerConfig::ApiDispatchEnum ),
	_pOmmRestLoggingClient(0)
{
	_instanceNodeName = "ConsumerGroup|ConsumerList|Consumer.";
	_pEmaConfig->verifyDefaultConsumer();
	_reactorOAuthCredentialList = NULL;
	_reactorOAuthCredentialCount = 0;
	_dataDictionary = NULL;
	_shouldCopyIntoAPI = false;
}

OmmConsumerConfigImpl::~OmmConsumerConfigImpl()
{
	_oAuthPassword.secureClear();
	if (_reactorOAuthCredentialList != NULL)
	{

	}

	if (_dataDictionary && _shouldCopyIntoAPI)
	{
		_dataDictionary->_pImpl->decDataDictionaryRefCount();
		if (!_dataDictionary->_pImpl->getDataDictionaryRefCount())
			delete _dataDictionary;
	}
}

void OmmConsumerConfigImpl::consumerName( const EmaString& consumerName )
{
	// Keep the session name to check laster after all configuration methods is called	
	_configSessionName.set(consumerName);
}

void OmmConsumerConfigImpl::validateSpecifiedSessionName()
{
	if (_configSessionName.empty())
		return;

	if (_pProgrammaticConfigure && _pProgrammaticConfigure->specifyConsumerName(_configSessionName))
		return;

	EmaString item("ConsumerGroup|ConsumerList|Consumer.");
	item.append(_configSessionName).append("|Name");
	EmaString name;
	if (get(item, name))
	{
		if (!set("ConsumerGroup|DefaultConsumer", _configSessionName))
		{
			EmaString mergeString("<EmaConfig><ConsumerGroup><DefaultConsumer value=\"");
			mergeString.append(_configSessionName).append("\"/></ConsumerGroup></EmaConfig>");
			xmlDocPtr xmlDoc = xmlReadMemory(mergeString.c_str(), mergeString.length(), "notnamed.xml", NULL, XML_PARSE_HUGE);
			if (xmlDoc == NULL)
				return;
			xmlNodePtr _xmlNodePtr = xmlDocGetRootElement(xmlDoc);
			if (_xmlNodePtr == NULL)
				return;
			if (xmlStrcmp(_xmlNodePtr->name, (const xmlChar*) "EmaConfig"))
				return;
			XMLnode* tmp(new XMLnode("EmaConfig", 0, 0));
			processXMLnodePtr(tmp, _xmlNodePtr);
			_pEmaConfig->merge(tmp);
			xmlFreeDoc(xmlDoc);
			delete tmp;
		}
	}
	else
	{
		if (_configSessionName == DEFAULT_CONS_NAME)
		{
			XMLnode* consumerList(_pEmaConfig->find< XMLnode >("ConsumerGroup|ConsumerList"));
			if (consumerList)
			{
				EmaList< XMLnode::NameString* > theNames;
				consumerList->getNames(theNames);
				if (theNames.empty())
					return;
			}
			else
				return;
		}

		EmaString errorMsg("OmmConsumerConfigImpl::consumerName parameter [");
		errorMsg.append(_configSessionName).append("] is a non-existent consumer name");
		_configSessionName.clear();
		throwIceException(errorMsg);
	}
}

EmaString OmmConsumerConfigImpl::getConfiguredName()
{
	if(!_configSessionName.empty())
		return _configSessionName;

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

	return DEFAULT_CONS_NAME;
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

void OmmConsumerConfigImpl::setOmmRestLoggingClient(OmmRestLoggingClient* pOmmRestLoggingClient, void* closure)
{
	_pOmmRestLoggingClient = pOmmRestLoggingClient;
	_pRestLoggingClosure = closure;
}

OmmRestLoggingClient* OmmConsumerConfigImpl::getOmmRestLoggingClient() const
{
	return _pOmmRestLoggingClient;
}

void* OmmConsumerConfigImpl::getRestLoggingClosure() const
{
	return _pRestLoggingClosure;
}

void OmmConsumerConfigImpl::dataDictionary(const refinitiv::ema::rdm::DataDictionary& dataDictionary, bool shouldCopyIntoAPI)
{
	if (dataDictionary.isFieldDictionaryLoaded() && dataDictionary.isEnumTypeDefLoaded())
	{
		if (shouldCopyIntoAPI)
		{
			_dataDictionary = new DataDictionary(dataDictionary);
			_dataDictionary->_pImpl->incDataDictionaryRefCount();
			_shouldCopyIntoAPI = shouldCopyIntoAPI;
		}
		else
			_dataDictionary = const_cast<DataDictionary*>(&dataDictionary);

	}
	else
	{
		EmaString errorMsg("The dictionary information is not fully loaded in the passed DataDictionary object.");
		throwIueException(errorMsg, OmmInvalidUsageException::InvalidArgumentEnum);
	}
}

refinitiv::ema::rdm::DataDictionary* OmmConsumerConfigImpl::dataDictionary() const
{
	return (refinitiv::ema::rdm::DataDictionary*)_dataDictionary;
}

bool OmmConsumerConfigImpl::isShouldCopyIntoAPI()
{
	return _shouldCopyIntoAPI;
}

void OmmConsumerConfigImpl::clear()
{
	EmaConfigImpl::clear();

	if (_dataDictionary && _shouldCopyIntoAPI)
	{
		_dataDictionary->_pImpl->decDataDictionaryRefCount();
		if (!_dataDictionary->_pImpl->getDataDictionaryRefCount())
			delete _dataDictionary;
	}
	_shouldCopyIntoAPI = false;
	_dataDictionary = NULL;
}

