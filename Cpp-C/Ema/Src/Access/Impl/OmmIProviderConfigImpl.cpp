/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmIProviderConfigImpl.h"
#include "OmmServerBaseImpl.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmIProviderConfigImpl::OmmIProviderConfigImpl( const EmaString & path ) :
	EmaConfigServerImpl( path ),
	_operationModel( OmmIProviderConfig::ApiDispatchEnum ),
	_adminControlDirectory( OmmIProviderConfig::ApiControlEnum ),
	_adminControlDictionary( OmmIProviderConfig::ApiControlEnum )
{
	_instanceNodeName = "IProviderGroup|IProviderList|IProvider.";

	_pEmaConfig->verifyDefaultDirectory();
}

OmmIProviderConfigImpl::~OmmIProviderConfigImpl()
{
}

void OmmIProviderConfigImpl::clear()
{
	EmaConfigServerImpl::clear();
	_operationModel = OmmIProviderConfig::ApiDispatchEnum;
	_adminControlDirectory = OmmIProviderConfig::ApiControlEnum;
	_adminControlDictionary = OmmIProviderConfig::ApiControlEnum;
}

void OmmIProviderConfigImpl::providerName( const EmaString& providerName )
{
	// Keep the session name to check laster after all configuration methods is called
	_configSessionName.set(providerName);
}

void OmmIProviderConfigImpl::validateSpecifiedSessionName()
{
	if (_configSessionName.empty())
		return;

	if (_pProgrammaticConfigure && _pProgrammaticConfigure->specifyIProviderName(_configSessionName))
		return;

	EmaString item("IProviderGroup|IProviderList|IProvider.");
	item.append(_configSessionName).append("|Name");
	EmaString name;
	if (get(item, name))
	{
		if (!set("IProviderGroup|DefaultIProvider", _configSessionName))
		{
			EmaString mergeString("<EmaConfig><IProviderGroup><DefaultIProvider value=\"");
			mergeString.append(_configSessionName).append("\"/></IProviderGroup></EmaConfig>");
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
		if (_configSessionName == DEFAULT_IPROV_NAME)
		{
			XMLnode* iProviderList(_pEmaConfig->find< XMLnode >("IProviderGroup|IProviderList"));
			if (iProviderList)
			{
				EmaList< XMLnode::NameString* > theNames;
				iProviderList->getNames(theNames);
				if (theNames.empty())
					return;
			}
			else
				return;
		}

		EmaString errorMsg("OmmIProviderConfigImpl::providerName parameter [");
		errorMsg.append(_configSessionName).append("] is a non-existent provider name");
		_configSessionName.clear();
		throwIceException(errorMsg);
	}
}

EmaString OmmIProviderConfigImpl::getConfiguredName()
{
	if (!_configSessionName.empty())
		return _configSessionName;

	EmaString retVal;

	if (_pProgrammaticConfigure && _pProgrammaticConfigure->getDefaultIProvider(retVal))
		return retVal;

	if (_pEmaConfig->get< EmaString >("IProviderGroup|DefaultIProvider", retVal))
	{
		EmaString expectedName("IProviderGroup|IProviderList|IProvider.");
		expectedName.append(retVal).append("|Name");
		EmaString checkValue;
		if (_pEmaConfig->get(expectedName, checkValue))
			return retVal;
		else
		{
			EmaString errorMsg("default IProvider name [");
			errorMsg.append(retVal).append("] is an non-existent IProvider name; DefaultIProvider specification ignored");
			_pEmaConfig->appendErrorMessage(errorMsg, OmmLoggerClient::ErrorEnum);
		}
	}

	if (_pEmaConfig->get< EmaString >("IProviderGroup|IProviderList|IProvider|Name", retVal))
		return retVal;

	return DEFAULT_IPROV_NAME;
}

void OmmIProviderConfigImpl::operationModel( OmmIProviderConfig::OperationModel operationModel )
{
	_operationModel = operationModel;
}

void OmmIProviderConfigImpl::adminControlDirectory( OmmIProviderConfig::AdminControl control )
{
	_adminControlDirectory = control;
}

void OmmIProviderConfigImpl::adminControlDictionary( OmmIProviderConfig::AdminControl control )
{
	_adminControlDictionary = control;
}

OmmIProviderConfig::OperationModel OmmIProviderConfigImpl::getOperationModel() const
{
	return _operationModel;
}

OmmIProviderConfig::AdminControl OmmIProviderConfigImpl::getAdminControlDirectory() const
{
	return _adminControlDirectory;
}

OmmIProviderConfig::AdminControl OmmIProviderConfigImpl::getAdminControlDictionary() const
{
	return _adminControlDictionary;
}
