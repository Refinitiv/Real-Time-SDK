/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
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
	if ( _pProgrammaticConfigure && _pProgrammaticConfigure->specifyNiProviderName( providerName ) )
		return;

	EmaString item( "NiProviderGroup|NiProviderList|NiProvider." );
	item.append( providerName ).append( "|Name" );
	EmaString name;
	if ( get( item, name ) )
	{
		if ( ! set( "NiProviderGroup|DefaultNiProvider", providerName ) )
		{
			EmaString mergeString( "<EmaConfig><NiProviderGroup><DefaultNiProvider value=\"" );
			mergeString.append( providerName ).append( "\"/></NiProviderGroup></EmaConfig>" );
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
		if ( providerName == "EmaNiProvider" )
		{
			XMLnode* niProviderList( _pEmaConfig->find< XMLnode >( "NiProviderGroup|NiProviderList" ) );
			if ( niProviderList )
			{
				EmaList< XMLnode::NameString* > theNames;
				niProviderList->getNames( theNames );
				if ( theNames.empty() )
					return;
			}
			else
				return;
		}

		EmaString errorMsg( "OmmNiProviderConfigImpl::providerName parameter [" );
		errorMsg.append( providerName ).append( "] is a non-existent provider name" );
		throwIceException( errorMsg );
	}
}

EmaString OmmNiProviderConfigImpl::getConfiguredName()
{
	EmaString retVal;

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

	return "EmaNiProvider";
}

bool OmmNiProviderConfigImpl::getDictionaryName( const EmaString& , EmaString& ) const
{
	return false;
}

bool OmmNiProviderConfigImpl::getDirectoryName( const EmaString& instanceName, EmaString& retVal ) const
{
	if ( !_pProgrammaticConfigure || !_pProgrammaticConfigure->getActiveDirectoryName( instanceName, retVal ) )
	{
		EmaString nodeName( _instanceNodeName );
		nodeName.append( instanceName );
		nodeName.append( "|Directory" );

		get<EmaString>( nodeName, retVal );
	}

	return true;
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
