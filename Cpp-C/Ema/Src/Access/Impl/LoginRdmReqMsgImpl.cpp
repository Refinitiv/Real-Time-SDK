/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "LoginRdmReqMsgImpl.h"
#include "EmaConfigImpl.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

/* Dummy no-op OAuth2 Consumer class client for initializing handlers */
/* This should never be used */
class DummyLoginCredentialConsumerImpl : public refinitiv::ema::access::OmmLoginCredentialConsumerClient
{
};

static DummyLoginCredentialConsumerImpl defaultLoginConsClient;

LoginRdmReqMsgImpl::LoginRdmReqMsgImpl() :
	_username(),
	_password(),
	_position(),
	_applicationId(),
	_applicationName(),
	_defaultApplicationName("ema"),
	_instanceId(),
	_channelList(),
	_client(defaultLoginConsClient)
{
	EmaString _defalutPosition;
	emaGetPosition(_defalutPosition);

	_hasLoginClient = false;
	_closure = NULL;
	rsslClearRDMLoginRequest( &_rsslRdmLoginRequest );
	rsslInitDefaultRDMLoginRequest(&_rsslRdmLoginRequest, 1);

	snprintf(_rsslRdmLoginRequest.defaultPosition, sizeof(_rsslRdmLoginRequest.defaultPosition), "%s", _defalutPosition.c_str());
	_rsslRdmLoginRequest.position.data = _rsslRdmLoginRequest.defaultPosition;
	_rsslRdmLoginRequest.position.length = (RsslUInt32)strlen(_rsslRdmLoginRequest.defaultPosition);

	_rsslRdmLoginRequest.applicationName.data = (char*)_defaultApplicationName.c_str();
	_rsslRdmLoginRequest.applicationName.length = _defaultApplicationName.length();

	_applicationName = _defaultApplicationName;

	_username.set(_rsslRdmLoginRequest.userName.data, _rsslRdmLoginRequest.userName.length);
	_position.set(_rsslRdmLoginRequest.position.data, _rsslRdmLoginRequest.position.length);
	_applicationId.set(_rsslRdmLoginRequest.applicationId.data, _rsslRdmLoginRequest.applicationId.length);
}

LoginRdmReqMsgImpl::LoginRdmReqMsgImpl(OmmLoginCredentialConsumerClient& client) :
	_username(),
	_password(),
	_position(),
	_applicationId(),
	_applicationName(),
	_defaultApplicationName("ema"),
	_instanceId(),
	_channelList(),
	_client(client)
{
	_hasLoginClient = true;
	_closure = NULL;

	EmaString _defalutPosition;
	emaGetPosition(_defalutPosition);

	rsslClearRDMLoginRequest(&_rsslRdmLoginRequest);
	rsslInitDefaultRDMLoginRequest(&_rsslRdmLoginRequest, 1);

	snprintf(_rsslRdmLoginRequest.defaultPosition, sizeof(_rsslRdmLoginRequest.defaultPosition), "%s", _defalutPosition.c_str());
	_rsslRdmLoginRequest.position.data = _rsslRdmLoginRequest.defaultPosition;
	_rsslRdmLoginRequest.position.length = (RsslUInt32)strlen(_rsslRdmLoginRequest.defaultPosition);

	_rsslRdmLoginRequest.applicationName.data = (char*)_defaultApplicationName.c_str();
	_rsslRdmLoginRequest.applicationName.length = _defaultApplicationName.length();

	_applicationName = _defaultApplicationName;

	_username.set(_rsslRdmLoginRequest.userName.data, _rsslRdmLoginRequest.userName.length);
	_position.set(_rsslRdmLoginRequest.position.data, _rsslRdmLoginRequest.position.length);
	_applicationId.set(_rsslRdmLoginRequest.applicationId.data, _rsslRdmLoginRequest.applicationId.length);
}

LoginRdmReqMsgImpl::LoginRdmReqMsgImpl(OmmLoginCredentialConsumerClient& client, void* closure) :
	_username(),
	_password(),
	_position(),
	_applicationId(),
	_applicationName(),
	_defaultApplicationName("ema"),
	_instanceId(),
	_channelList(),
	_client(client)
{
	_hasLoginClient = true;
	_closure = closure;

	EmaString _defalutPosition;
	emaGetPosition(_defalutPosition);

	rsslClearRDMLoginRequest(&_rsslRdmLoginRequest);
	rsslInitDefaultRDMLoginRequest(&_rsslRdmLoginRequest, 1);

	snprintf(_rsslRdmLoginRequest.defaultPosition, sizeof(_rsslRdmLoginRequest.defaultPosition), "%s", _defalutPosition.c_str());
	_rsslRdmLoginRequest.position.data = _rsslRdmLoginRequest.defaultPosition;
	_rsslRdmLoginRequest.position.length = (RsslUInt32)strlen(_rsslRdmLoginRequest.defaultPosition);

	_rsslRdmLoginRequest.applicationName.data = (char*)_defaultApplicationName.c_str();
	_rsslRdmLoginRequest.applicationName.length = _defaultApplicationName.length();

	_applicationName = _defaultApplicationName;

	_username.set(_rsslRdmLoginRequest.userName.data, _rsslRdmLoginRequest.userName.length);
	_position.set(_rsslRdmLoginRequest.position.data, _rsslRdmLoginRequest.position.length);
	_applicationId.set(_rsslRdmLoginRequest.applicationId.data, _rsslRdmLoginRequest.applicationId.length);
}

LoginRdmReqMsgImpl::LoginRdmReqMsgImpl(const LoginRdmReqMsgImpl& reqMsg) :
	_username(reqMsg._username),
	_password(reqMsg._password),
	_position(reqMsg._position),
	_applicationId(reqMsg._applicationId),
	_applicationName(reqMsg._applicationName),
	_defaultApplicationName("ema"),
	_instanceId(reqMsg._instanceId),
	_channelList(reqMsg._channelList),
	_arrayIndex(reqMsg._arrayIndex),
	_client(reqMsg._client),
	_closure(reqMsg._closure)
{
	_hasLoginClient = reqMsg._hasLoginClient;
	rsslClearRDMLoginRequest(&_rsslRdmLoginRequest);
	_rsslRdmLoginRequest = reqMsg._rsslRdmLoginRequest;

	if (_applicationName.length() != 0)
	{
		_rsslRdmLoginRequest.applicationName.data = (char*)_applicationName.c_str();
		_rsslRdmLoginRequest.applicationName.length = _applicationName.length();
	}
	else
	{
		_rsslRdmLoginRequest.applicationName.data = (char*)_defaultApplicationName.c_str();
		_rsslRdmLoginRequest.applicationName.length = _defaultApplicationName.length();

		_applicationName = _defaultApplicationName;
	}

	if (_username.length() != 0)
	{
		_rsslRdmLoginRequest.userName.data = (char*)_username.c_str();
		_rsslRdmLoginRequest.userName.length = _username.length();
	}

	if (_password.length() != 0)
	{
		_rsslRdmLoginRequest.password.data = (char*)_password.c_str();
		_rsslRdmLoginRequest.password.length = _password.length();
	}

	if (_position.length() != 0)
	{
		_rsslRdmLoginRequest.position.data = (char*)_position.c_str();
		_rsslRdmLoginRequest.position.length = _position.length();
	}

	if (_applicationId.length() != 0)
	{
		_rsslRdmLoginRequest.applicationId.data = (char*)_applicationId.c_str();
		_rsslRdmLoginRequest.applicationId.length = _applicationId.length();
	}

	if (_applicationName.length() != 0)
	{
		_rsslRdmLoginRequest.applicationName.data = (char*)_applicationName.c_str();
		_rsslRdmLoginRequest.applicationName.length = _applicationName.length();
	}

	if (_instanceId.length() != 0)
	{
		_rsslRdmLoginRequest.instanceId.data = (char*)_instanceId.c_str();
		_rsslRdmLoginRequest.instanceId.length = _instanceId.length();
	}

}

LoginRdmReqMsgImpl::~LoginRdmReqMsgImpl()
{
	clear();
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::clear()
{
	_username.clear();
	_password.secureClear();
	_position.clear();
	_applicationId.clear();
	_applicationName.clear();
	_instanceId.clear();

	EmaString _defalutPosition;
	emaGetPosition(_defalutPosition);

	rsslClearRDMLoginRequest( &_rsslRdmLoginRequest );
	rsslInitDefaultRDMLoginRequest(&_rsslRdmLoginRequest, 1);

	snprintf(_rsslRdmLoginRequest.defaultPosition, sizeof(_rsslRdmLoginRequest.defaultPosition), "%s", _defalutPosition.c_str());
	_rsslRdmLoginRequest.position.data = _rsslRdmLoginRequest.defaultPosition;
	_rsslRdmLoginRequest.position.length = (RsslUInt32)strlen(_rsslRdmLoginRequest.defaultPosition);

	_rsslRdmLoginRequest.applicationName.data = (char*)_defaultApplicationName.c_str();
	_rsslRdmLoginRequest.applicationName.length = _defaultApplicationName.length();

	_applicationName = _defaultApplicationName;

	_username.set(_rsslRdmLoginRequest.userName.data, _rsslRdmLoginRequest.userName.length);
	_position.set(_rsslRdmLoginRequest.position.data, _rsslRdmLoginRequest.position.length);
	_applicationId.set(_rsslRdmLoginRequest.applicationId.data, _rsslRdmLoginRequest.applicationId.length);
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::set(EmaConfigImpl* emaConfigImpl, RsslRequestMsg* pRsslRequestMsg )
{
	_rsslRdmLoginRequest.rdmMsgBase.domainType = RSSL_DMT_LOGIN;
	_rsslRdmLoginRequest.rdmMsgBase.rdmMsgType = RDM_LG_MT_REQUEST;
	_rsslRdmLoginRequest.flags = RDM_LG_RQF_NONE;

	if ( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME_TYPE )
	{
		_rsslRdmLoginRequest.userNameType = pRsslRequestMsg->msgBase.msgKey.nameType;
		_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_USERNAME_TYPE;
	}
	else
		_rsslRdmLoginRequest.flags &= ~RDM_LG_RQF_HAS_USERNAME_TYPE;

	if ( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		_username.set( pRsslRequestMsg->msgBase.msgKey.name.data, pRsslRequestMsg->msgBase.msgKey.name.length );
		_rsslRdmLoginRequest.userName.data = (char*) _username.c_str();
		_rsslRdmLoginRequest.userName.length = _username.length();
	}
	else
	{
		_username.clear();
		rsslClearBuffer( &_rsslRdmLoginRequest.userName );
	}

	if ( ( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
		pRsslRequestMsg->msgBase.msgKey.attribContainerType == RSSL_DT_ELEMENT_LIST )
	{
		RsslDecodeIterator dIter;

		rsslClearDecodeIterator( &dIter );

		RsslRet retCode = rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			EmaString temp( "Internal error. Failed to set RsslDecodeIterator's version in LoginRdmReqMsg::set(). Attributes will be skipped." );
			if(emaConfigImpl != NULL)
				emaConfigImpl->appendConfigError( temp, OmmLoggerClient::ErrorEnum );
			else
				throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return *this;
		}

		retCode = rsslSetDecodeIteratorBuffer( &dIter, &pRsslRequestMsg->msgBase.msgKey.encAttrib );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			EmaString temp( "Internal error. Failed to set RsslDecodeIterator's Buffer in LoginRdmReqMsg::set(). Attributes will be skipped." );
			if(emaConfigImpl != NULL)
				emaConfigImpl->appendConfigError(temp, OmmLoggerClient::ErrorEnum);
			else
				throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return *this;
		}

		RsslElementList elementList;
		rsslClearElementList( &elementList );
		retCode = rsslDecodeElementList( &dIter, &elementList, 0 );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			if ( retCode != RSSL_RET_NO_DATA )
			{
				EmaString temp( "Internal error while decoding element list containing login attributes. Error='" );
				temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attributes will be skipped." );
				if (emaConfigImpl != NULL)
					emaConfigImpl->appendConfigError(temp, OmmLoggerClient::ErrorEnum);
				else
					throwIueException(temp, OmmInvalidUsageException::InvalidArgumentEnum);
				return *this;
			}

			return *this;
		}

		RsslElementEntry elementEntry;
		rsslClearElementEntry( &elementEntry );

		retCode = rsslDecodeElementEntry( &dIter, &elementEntry );

		while ( retCode != RSSL_RET_END_OF_CONTAINER )
		{
			if ( retCode != RSSL_RET_SUCCESS )
			{
				EmaString temp( "Internal error while decoding element entry with a login attribute. Error='" );
				temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
				if (emaConfigImpl != NULL)
					emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
				continue;
			}

			if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_SINGLE_OPEN ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.singleOpen );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of single open. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_SINGLE_OPEN;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_ALLOW_SUSPECT_DATA ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.allowSuspectData );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of allow suspect data. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_APPID ) )
			{
				retCode = rsslDecodeBuffer( &dIter, &_rsslRdmLoginRequest.applicationId );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of application id. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_applicationId.set( _rsslRdmLoginRequest.applicationId.data, _rsslRdmLoginRequest.applicationId.length );
				_rsslRdmLoginRequest.applicationId.data = (char*) _applicationId.c_str();
				_rsslRdmLoginRequest.applicationId.length = _applicationId.length();
				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_APPNAME ) )
			{
				retCode = rsslDecodeBuffer( &dIter, &_rsslRdmLoginRequest.applicationName );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of application name. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_applicationName.set( _rsslRdmLoginRequest.applicationName.data, _rsslRdmLoginRequest.applicationName.length );
				_rsslRdmLoginRequest.applicationName.data = (char*) _applicationName.c_str();
				_rsslRdmLoginRequest.applicationName.length = _applicationName.length();
				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_NAME;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_POSITION ) )
			{
				retCode = rsslDecodeBuffer( &dIter, &_rsslRdmLoginRequest.position );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of position. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_position.set( _rsslRdmLoginRequest.position.data, _rsslRdmLoginRequest.position.length );
				_rsslRdmLoginRequest.position.data = (char*) _position.c_str();
				_rsslRdmLoginRequest.position.length = _position.length();
				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_POSITION;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_PASSWORD ) )
			{
				retCode = rsslDecodeBuffer( &dIter, &_rsslRdmLoginRequest.password );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of password. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_password.set( _rsslRdmLoginRequest.password.data, _rsslRdmLoginRequest.password.length );
				_rsslRdmLoginRequest.password.data = (char*) _password.c_str();
				_rsslRdmLoginRequest.password.length = _password.length();
				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_PASSWORD;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_PROV_PERM_PROF ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.providePermissionProfile );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of provide permission profile. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_PROV_PERM_EXP ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.providePermissionExpressions );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of provide permission expressions. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.supportProviderDictionaryDownload );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of support provider dictionary download. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_SUPPORT_PROV_DIC_DOWNLOAD;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_DOWNLOAD_CON_CONFIG ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.downloadConnectionConfig );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of download connection configure. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_DOWNLOAD_CONN_CONFIG;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_INST_ID ) )
			{
				retCode = rsslDecodeBuffer( &dIter, &_rsslRdmLoginRequest.instanceId );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of instance Id. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_instanceId.set( _rsslRdmLoginRequest.instanceId.data, _rsslRdmLoginRequest.instanceId.length );
				_rsslRdmLoginRequest.instanceId.data = (char*) _instanceId.c_str();
				_rsslRdmLoginRequest.instanceId.length = _instanceId.length();
				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_INSTANCE_ID;
			}
			else if ( rsslBufferIsEqual( &elementEntry.name, &RSSL_ENAME_ROLE ) )
			{
				retCode = rsslDecodeUInt( &dIter, &_rsslRdmLoginRequest.role );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					EmaString temp( "Internal error while decoding login attribute of role. Error='" );
					temp.append( rsslRetCodeToString( retCode ) ).append( "'. Attribute will be skipped." );
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_ROLE;
			}
			else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_AUTHN_TOKEN))
			{
				retCode = rsslDecodeBuffer(&dIter, &_rsslRdmLoginRequest.userName);
				if (retCode != RSSL_RET_SUCCESS)
				{
					EmaString temp("Internal error while decoding login attribute of authentication token. Error='");
					temp.append(rsslRetCodeToString(retCode)).append("'. Attribute will be skipped.");
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_authenticationToken.set(_rsslRdmLoginRequest.userName.data, _rsslRdmLoginRequest.userName.length);
				_rsslRdmLoginRequest.userName.data = (char*)_authenticationToken.c_str();
				_rsslRdmLoginRequest.userName.length = _authenticationToken.length();
			}
			else if (rsslBufferIsEqual(&elementEntry.name, &RSSL_ENAME_AUTHN_EXTENDED))
			{
				retCode = rsslDecodeBuffer(&dIter, &_rsslRdmLoginRequest.authenticationExtended);
				if (retCode != RSSL_RET_SUCCESS)
				{
					EmaString temp("Internal error while decoding login attribute of authentication token. Error='");
					temp.append(rsslRetCodeToString(retCode)).append("'. Attribute will be skipped.");
					if (emaConfigImpl != NULL)
						emaConfigImpl->appendConfigError(temp, OmmLoggerClient::WarningEnum);
					continue;
				}

				_authenticationExtended.setFrom(_rsslRdmLoginRequest.authenticationExtended.data, _rsslRdmLoginRequest.authenticationExtended.length);
				_rsslRdmLoginRequest.authenticationExtended.data = (char*)_authenticationExtended.c_buf();
				_rsslRdmLoginRequest.authenticationExtended.length = _authenticationExtended.length();
				_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
			}

			retCode = rsslDecodeElementEntry( &dIter, &elementEntry );
		}
	}

	return *this;
}

RsslRDMLoginRequest* LoginRdmReqMsgImpl::get()
{
	return &_rsslRdmLoginRequest;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::username( const EmaString& value )
{
	_username = value;
	_rsslRdmLoginRequest.userName.data = (char*) _username.c_str();
	_rsslRdmLoginRequest.userName.length = _username.length();
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::position( const EmaString& value )
{
	_position = value;
	_rsslRdmLoginRequest.position.data = (char*) _position.c_str();
	_rsslRdmLoginRequest.position.length = _position.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_POSITION;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::password( const EmaString& value )
{
	_password = value;
	_rsslRdmLoginRequest.password.data = (char*) _password.c_str();
	_rsslRdmLoginRequest.password.length = _password.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_PASSWORD;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::applicationId( const EmaString& value )
{
	_applicationId = value;
	_rsslRdmLoginRequest.applicationId.data = (char*) _applicationId.c_str();
	_rsslRdmLoginRequest.applicationId.length = _applicationId.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::applicationName( const EmaString& value )
{
	_applicationName = value;
	_rsslRdmLoginRequest.applicationName.data = (char*) _applicationName.c_str();
	_rsslRdmLoginRequest.applicationName.length = _applicationName.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_NAME;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::applicationAuthorizationToken(const EmaString& value)
{
	_applicationAuthorizationToken = value;
	_rsslRdmLoginRequest.applicationAuthorizationToken.data = (char*)_applicationAuthorizationToken.c_str();
	_rsslRdmLoginRequest.applicationAuthorizationToken.length = _applicationAuthorizationToken.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::authenticationExtended(const EmaBuffer& value)
{
	_authenticationExtended = value;
	_rsslRdmLoginRequest.authenticationExtended.data = (char*)_authenticationExtended.c_buf();
	_rsslRdmLoginRequest.authenticationExtended.length = _authenticationExtended.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
	return *this;
}


LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::instanceId( const EmaString& value )
{
	_instanceId = value;
	_rsslRdmLoginRequest.instanceId.data = (char*) _instanceId.c_str();
	_rsslRdmLoginRequest.instanceId.length = _instanceId.length();
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_INSTANCE_ID;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::setChannelList(const EmaString& channelList)
{
	_channelList = channelList;
	
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::setRole( RDMLoginRoleTypes role )
{
	_rsslRdmLoginRequest.role = role;
	_rsslRdmLoginRequest.flags |= RDM_LG_RQF_HAS_ROLE;
	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::clearPassword()
{
	_password.secureClear();
	_rsslRdmLoginRequest.password.data = NULL;
	_rsslRdmLoginRequest.password.length = 0;
	_rsslRdmLoginRequest.flags &= ~RDM_LG_RQF_HAS_PASSWORD;

	return *this;
}

LoginRdmReqMsgImpl& LoginRdmReqMsgImpl::arrayIndex(UInt8 value)
{
	_arrayIndex = value;
	return *this;
}

const EmaString& LoginRdmReqMsgImpl::channelList()
{
	return _channelList;
}

const EmaString& LoginRdmReqMsgImpl::getUserName()
{
	return _username;
}

const EmaString& LoginRdmReqMsgImpl::getPassword()
{
	return _password;
}

void* LoginRdmReqMsgImpl::getClosure()
{
	return _closure;
}

bool LoginRdmReqMsgImpl::hasLoginClient()
{
	return _hasLoginClient;
}

UInt8 LoginRdmReqMsgImpl::getArrayIndex()
{
	return _arrayIndex;
}

OmmLoginCredentialConsumerClient& LoginRdmReqMsgImpl::getClient()
{
	return _client;
}
