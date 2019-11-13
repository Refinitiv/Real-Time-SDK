/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmNiProviderConfigImpl::OmmNiProviderConfigImpl( const EmaString & path ) :
	EmaConfigImpl( path ),
	_operationModel( OmmNiProviderConfig::ApiDispatchEnum ),
	_adminControlDirectory( OmmNiProviderConfig::ApiControlEnum )
{
	_instanceNodeName = "NiProviderGroup|NiProviderList|NiProvider.";

	_loginRdmReqMsg.setRole( RDM_LOGIN_ROLE_PROV );

	_pEmaConfig->verifyDefaultNiProvider();

	_pEmaConfig->verifyDefaultDirectory();
}

OmmNiProviderConfigImpl::~OmmNiProviderConfigImpl()
{
}

void OmmNiProviderConfigImpl::clear()
{
	EmaConfigImpl::clear();
	_operationModel = OmmNiProviderConfig::ApiDispatchEnum;
	_adminControlDirectory = OmmNiProviderConfig::ApiControlEnum;
}

void OmmNiProviderConfigImpl::providerName( const EmaString& providerName )
{
	// Keep the session name to check laster after all configuration methods is called
	_configSessionName.set(providerName);
}

void OmmNiProviderConfigImpl::validateSpecifiedSessionName()
{
	if (_configSessionName.empty())
		return;

	if (_pProgrammaticConfigure && _pProgrammaticConfigure->specifyNiProviderName(_configSessionName))
		return;

	EmaString item("NiProviderGroup|NiProviderList|NiProvider.");
	item.append(_configSessionName).append("|Name");
	EmaString name;
	if (get(item, name))
	{
		if (!set("NiProviderGroup|DefaultNiProvider", _configSessionName))
		{
			EmaString mergeString("<EmaConfig><NiProviderGroup><DefaultNiProvider value=\"");
			mergeString.append(_configSessionName).append("\"/></NiProviderGroup></EmaConfig>");
			xmlDocPtr xmlDoc = xmlReadMemory(mergeString.c_str(), mergeString.length(), NULL, "notnamed.xml", XML_PARSE_HUGE);
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
		if (_configSessionName == DEFAULT_NIPROV_NAME)
		{
			XMLnode* niProviderList(_pEmaConfig->find< XMLnode >("NiProviderGroup|NiProviderList"));
			if (niProviderList)
			{
				EmaList< XMLnode::NameString* > theNames;
				niProviderList->getNames(theNames);
				if (theNames.empty())
					return;
			}
			else
				return;
		}

		EmaString errorMsg("OmmNiProviderConfigImpl::providerName parameter [");
		errorMsg.append(_configSessionName).append("] is a non-existent provider name");
		_configSessionName.clear();
		throwIceException(errorMsg);
	}
}

EmaString OmmNiProviderConfigImpl::getConfiguredName()
{
	if (!_configSessionName.empty())
		return _configSessionName;

	EmaString retVal;
	
	if (_pProgrammaticConfigure && _pProgrammaticConfigure->getDefaultNiProvider(retVal))
		return retVal;

	if ( _pEmaConfig->get< EmaString >( "NiProviderGroup|DefaultNiProvider", retVal ) )
	{
		EmaString expectedName( "NiProviderGroup|NiProviderList|NiProvider." );
		expectedName.append( retVal ).append( "|Name" );
		EmaString checkValue;
		if ( _pEmaConfig->get( expectedName, checkValue ) )
			return retVal;
		else
		{
			EmaString errorMsg( "default NiProvider name [" );
			errorMsg.append( retVal ).append( "] is an non-existent NiProvider name; DefaultNiProvider specification ignored" );
			_pEmaConfig->appendErrorMessage( errorMsg, OmmLoggerClient::ErrorEnum );
		}
	}

	if ( _pEmaConfig->get< EmaString >( "NiProviderGroup|NiProviderList|NiProvider|Name", retVal ) )
		return retVal;

	return DEFAULT_NIPROV_NAME;
}

void OmmNiProviderConfigImpl::operationModel( OmmNiProviderConfig::OperationModel operationModel )
{
	_operationModel = operationModel;
}

void OmmNiProviderConfigImpl::adminControlDirectory( OmmNiProviderConfig::AdminControl control )
{
	_adminControlDirectory = control;
}

OmmNiProviderConfig::OperationModel OmmNiProviderConfigImpl::getOperationModel() const
{
	return _operationModel;
}

OmmNiProviderConfig::AdminControl OmmNiProviderConfigImpl::getAdminControlDirectory() const
{
	return _adminControlDirectory;
}

RsslReactorOAuthCredential* OmmNiProviderConfigImpl::getReactorOAuthCredential()
{
	return NULL;
}
