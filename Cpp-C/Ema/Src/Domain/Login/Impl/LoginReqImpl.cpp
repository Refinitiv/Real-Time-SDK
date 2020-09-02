/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
*|-----------------------------------------------------------------------------
*/

#include "LoginReqImpl.h"
#include "OmmInvalidUsageException.h"
#include "Utilities.h"
#include <new>

using namespace rtsdk::ema::domain::login;
using namespace rtsdk::ema::rdm;
using namespace rtsdk::ema::access;

LoginReqImpl::LoginReqImpl() :
	_pElementList(0)
{
	if (emaGetUserName(_defaultName) != 0)
		_defaultName.set("user");

	if (emaGetPosition(_defaultPosition) != 0)
		_defaultPosition.set("localhost");
	clear();
}

LoginReqImpl::LoginReqImpl(const LoginReqImpl& other) :
	_pElementList(0)
{
	*this = other;
}

LoginReqImpl::LoginReqImpl(const ReqMsg& reqMsg) :
	_pElementList(0)
{
	if (emaGetUserName(_defaultName) != 0)
		_defaultName.set("user");

	if (emaGetPosition(_defaultPosition) != 0)
		_defaultPosition.set("localhost");
	clear();

	decode(reqMsg);
}

LoginReqImpl::~LoginReqImpl()
{
	if (_pElementList)
	{
		delete _pElementList;
		_pElementList = 0;
	}
}

LoginReqImpl& LoginReqImpl::clear()
{
	_changed = true;
	_allowSuspectDataSet = true;
	_allowSuspectData = true;
	_downloadConnectionConfigSet = false;
	_downloadConnectionConfig = false;
	_providePermissionProfileSet = true;
	_providePermissionProfile = true;
	_providePermissionExpressionsSet = true;
	_providePermissionExpressions = true;
	_singleOpenSet = false;
	_singleOpen = true;
	_supportProviderDictionaryDownloadSet = false;
	_supportProviderDictionaryDownload = false;
	_roleSet = true;
	_role = LOGIN_ROLE_CONS;
	_applicationIdSet = true;
	_applicationNameSet = true;
	_applicationAuthTokenSet = false;
	_instanceIdSet = false;
	_passwordSet = false;
	_positionSet = true;
	_authenticationExtendedSet = false;
	_pauseSet = false;
	_pause = false;

	_nameSet = true;
	_nameTypeSet = true;

	_nameType = USER_NAME;
	_domainType = MMT_LOGIN;

	applicationName("ema");
	applicationId("256");

	name(_defaultName);
	position(_defaultPosition);
	

	return *this;
}

LoginReqImpl& LoginReqImpl::operator=(const LoginReqImpl& other)
{
	_changed = true;
	_allowSuspectDataSet = other._allowSuspectDataSet;
	_downloadConnectionConfigSet = other._downloadConnectionConfigSet;
	_providePermissionProfileSet = other._providePermissionProfileSet;
	_providePermissionExpressionsSet = other._providePermissionExpressionsSet;
	_singleOpenSet = other._singleOpenSet;
	_supportProviderDictionaryDownloadSet = other._supportProviderDictionaryDownloadSet;
	_roleSet = other._roleSet;
	_applicationIdSet = other._applicationIdSet;
	_applicationNameSet = other._applicationNameSet;
	_applicationAuthTokenSet = other._applicationAuthTokenSet;
	_instanceIdSet = other._instanceIdSet;
	_passwordSet = other._passwordSet;
	_positionSet = other._positionSet;
	_authenticationExtendedSet = other._authenticationExtendedSet;
	_pauseSet = other._pauseSet;
	_nameSet = other._nameSet;
	_nameTypeSet = other._nameTypeSet;

	_allowSuspectData = other._allowSuspectData;
	_downloadConnectionConfig = other._downloadConnectionConfig;
	_providePermissionProfile = other._providePermissionProfile;
	_providePermissionExpressions = other._providePermissionExpressions;
	_singleOpen = other._singleOpen;
	_supportProviderDictionaryDownload = other._supportProviderDictionaryDownload;
	_role = other._role;
	_applicationId = other._applicationId;
	_applicationName = other._applicationName;
	_applicationAuthToken = other._applicationAuthToken;
	_instanceId = other._instanceId;
	_password = other._password;
	_position = other._position;
	_authenticationExtended = other._authenticationExtended;
	_pause = other._pause;
	_name = other._name;
	_nameType = other._nameType;
	_domainType = other._domainType;

	return *this;
}

LoginReqImpl& LoginReqImpl::message(const ReqMsg& reqMsg)
{
	decode(reqMsg);

	return *this;
}

LoginReqImpl& LoginReqImpl::allowSuspectData(bool value)
{
	_changed = true;
	_allowSuspectDataSet = true;
	_allowSuspectData = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::downloadConnectionConfig(bool value)
{
	_changed = true;
	_downloadConnectionConfigSet = true;
	_downloadConnectionConfig = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::applicationId(const EmaString& value)
{
	_changed = true;
	_applicationIdSet = true;
	_applicationId = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::applicationName(const EmaString& value)
{
	_changed = true;
	_applicationNameSet = true;
	_applicationName = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::applicationAuthorizationToken(const EmaString& value)
{
	_changed = true;
	_applicationAuthTokenSet = true;
	_applicationAuthToken = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::instanceId(const EmaString& value)
{
	_changed = true;
	_instanceIdSet = true;
	_instanceId = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::password(const EmaString& value)
{
	_changed = true;
	_passwordSet = true;
	_password = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::position(const EmaString& value)
{
	_changed = true;
	_positionSet = true;
	_position = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::providePermissionExpressions(bool value)
{
	_changed = true;
	_providePermissionExpressionsSet = true;
	_providePermissionExpressions = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::providePermissionProfile(bool value)
{
	_changed = true;
	_providePermissionProfileSet = true;
	_providePermissionProfile = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::role(UInt32 value)
{
	_changed = true;
	_roleSet = true;
	_role = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::singleOpen(bool value)
{
	_changed = true;
	_singleOpenSet = true;
	_singleOpen = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::supportProviderDictionaryDownload(bool value)
{
	_changed = true;
	_supportProviderDictionaryDownloadSet = true;
	_supportProviderDictionaryDownload = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::pause(bool value)
{
	_changed = true;
	_pauseSet = true;
	_pause = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::authenticationExtended(const EmaBuffer& value)
{
	_changed = true;
	_authenticationExtendedSet = true;
	_authenticationExtended = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::name(const EmaString& value)
{
	_changed = true;
	_nameSet = true;
	_name = value;

	return *this;
}

LoginReqImpl& LoginReqImpl::nameType(const UInt32& value)
{
	_changed = true;
	_nameTypeSet = true;
	_nameType = value;

	return *this;
}

bool LoginReqImpl::hasAllowSuspectData() const
{
	return _allowSuspectDataSet;
}

bool LoginReqImpl::hasDownloadConnectionConfig() const
{
	return _downloadConnectionConfigSet;
}

bool LoginReqImpl::hasApplicationId() const
{
	return _applicationIdSet;
}

bool LoginReqImpl::hasApplicationName() const
{
	return _applicationNameSet;
}

bool LoginReqImpl::hasApplicationAuthorizationToken() const
{
	return _applicationAuthTokenSet;
}

bool LoginReqImpl::hasInstanceId() const
{
	return _instanceIdSet;
}

bool LoginReqImpl::hasPassword() const
{
	return _passwordSet;
}

bool LoginReqImpl::hasPosition() const
{
	return _positionSet;
}

bool LoginReqImpl::hasProvidePermissionExpressions() const
{
	return _providePermissionExpressionsSet;
}

bool LoginReqImpl::hasProvidePermissionProfile() const
{
	return _providePermissionProfileSet;
}

bool LoginReqImpl::hasRole() const
{
	return _roleSet;
}

bool LoginReqImpl::hasSingleOpen() const
{
	return _singleOpenSet;
}

bool LoginReqImpl::hasSupportProviderDictionaryDownload() const
{
	return _supportProviderDictionaryDownloadSet;
}

bool LoginReqImpl::hasPause() const
{
	return _pauseSet;
}

bool LoginReqImpl::hasAuthenticationExtended() const
{
	return _authenticationExtendedSet;
}

bool LoginReqImpl::hasName() const
{
	return _nameSet;
}

bool LoginReqImpl::hasNameType() const
{
	return _nameTypeSet;
}

const ReqMsg& LoginReqImpl::getMessage()
{
	_reqMsg.clear();

	_reqMsg.domainType(_domainType);
	if (_nameTypeSet)
	{
		if (_nameType == USER_AUTH_TOKEN)
		{
			_authenticationToken = _name;
			const EmaString defaultAuthName = EmaString("\0", 1);
			_reqMsg.name(defaultAuthName);
		}
		_reqMsg.nameType(_nameType);
	}

	if(_nameTypeSet && _nameType != USER_AUTH_TOKEN)
		_reqMsg.name(_name);

	if (_pauseSet)
		_reqMsg.pause(_pause);

	if (!_changed)
	{
		return _reqMsg;
	}

	encode(_reqMsg);

	_changed = false;

	return _reqMsg;
}

bool LoginReqImpl::getAllowSuspectData() const
{
	return _allowSuspectData;
}

bool LoginReqImpl::getDownloadConnectionConfig() const
{
	return _downloadConnectionConfig;
}

const EmaString& LoginReqImpl::getApplicationId() const
{
	if (!_applicationIdSet)
	{
		EmaString text(ENAME_APP_ID);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _applicationId;
}

const EmaString& LoginReqImpl::getApplicationName() const
{
	if (!_applicationNameSet)
	{
		EmaString text(ENAME_APP_NAME);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _applicationName;
}

const EmaString& LoginReqImpl::getApplicationAuthorizationToken() const
{
	if (!_applicationAuthTokenSet)
	{
		EmaString text(ENAME_APPAUTH_TOKEN);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _applicationAuthToken;
}

const EmaString& LoginReqImpl::getInstanceId() const
{
	if (!_instanceIdSet)
	{
		EmaString text(ENAME_INST_ID);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _instanceId;
}

const EmaString& LoginReqImpl::getPassword() const
{
	if (!_passwordSet)
	{
		EmaString text(ENAME_PASSWORD);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _password;
}

const EmaString& LoginReqImpl::getPosition() const
{
	if (!_positionSet)
	{
		EmaString text(ENAME_POSITION);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _position;
}

bool LoginReqImpl::getProvidePermissionExpressions() const
{
	return _providePermissionExpressions;
}

bool LoginReqImpl::getProvidePermissionProfile() const
{
	return _providePermissionProfile;
}

UInt32 LoginReqImpl::getRole() const
{
	return _role;
}

bool LoginReqImpl::getSingleOpen() const
{
	return _singleOpen;
}

bool LoginReqImpl::getSupportProviderDictionaryDownload() const
{
	return _supportProviderDictionaryDownload;
}

bool LoginReqImpl::getPause() const
{
	return _pause;
}

const EmaBuffer& LoginReqImpl::getAuthenticationExtended() const
{
	if (!_authenticationExtendedSet)
	{
		EmaString text(ENAME_AUTH_EXTENDED);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationExtended;
}

const EmaString& LoginReqImpl::getName() const
{
	if (!_nameSet)
	{
		EmaString text(ENAME_USERNAME);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _name;
}

const UInt32& LoginReqImpl::getNameType() const
{
	if (!_nameTypeSet)
	{
		EmaString text(ENAME_USERNAME_TYPE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _nameType;
}

const EmaString& LoginReqImpl::toString() const
{
	_toString.clear();

	if (_allowSuspectDataSet)
	{
		if (_allowSuspectData == true)
		{
			_toString.append("\r\n").append(ENAME_ALLOW_SUSPECT_DATA).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_ALLOW_SUSPECT_DATA).append(" : ").append("NotSupported");
		}
	}

	if (_applicationIdSet)
	{
		_toString.append("\r\n").append(ENAME_APP_ID).append(" : ").append(_applicationId);
	}

	if (_applicationNameSet)
	{
		_toString.append("\r\n").append(ENAME_APP_NAME).append(" : ").append(_applicationName);
	}

	if (_applicationAuthTokenSet)
	{
		_toString.append("\r\n").append(ENAME_APPAUTH_TOKEN).append(" : ").append(_applicationAuthToken);
	}

	if (_downloadConnectionConfigSet)
	{
		if (_downloadConnectionConfig)
		{
			_toString.append("\r\n").append(ENAME_DOWNLOAD_CON_CONFIG).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_DOWNLOAD_CON_CONFIG).append(" : ").append("NotSupported");
		}
	}

	if (_instanceIdSet)
	{
		_toString.append("\r\n").append(ENAME_INST_ID).append(" : ").append(_instanceId);
	}

	if (_passwordSet)
	{
		_toString.append("\r\n").append(ENAME_PASSWORD).append(" : ").append(_password);
	}

	if (_positionSet)
	{
		_toString.append("\r\n").append(ENAME_POSITION).append(" : ").append(_position);
	}

	if (_providePermissionExpressionsSet)
	{
		if (_providePermissionExpressions == true)
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_EXP).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_EXP).append(" : ").append("NotSupported");
		}
	}

	if (_providePermissionProfileSet)
	{
		if (_providePermissionProfile == true)
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_PROF).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_PROF).append(" : ").append("NotSupported");
		}
	}

	if (_roleSet)
	{
		if (_role == LOGIN_ROLE_CONS)
		{
			_toString.append("\r\n").append(ENAME_ROLE).append(" : ").append("Consumer");
		}
		else if (_role == LOGIN_ROLE_PROV)
		{
			_toString.append("\r\n").append(ENAME_ROLE).append(" : ").append("Provider");
		}
	}

	if (_singleOpenSet)
	{
		if (_singleOpen == true)
		{
			_toString.append("\r\n").append(ENAME_SINGLE_OPEN).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SINGLE_OPEN).append(" : ").append("NotSupported");
		}
	}

	if (_supportProviderDictionaryDownloadSet)
	{
		if (_supportProviderDictionaryDownload)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).append(" : ").append("NotSupported");
		}
	}

	if (_authenticationExtendedSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_EXTENDED).append(" : ").append(_authenticationExtended);
	}

	if (_nameSet)
	{
		if (_nameTypeSet && _nameType == USER_AUTH_TOKEN)
		{
			_toString.append("\r\n").append(ENAME_AUTH_TOKEN).append(" : ").append(_authenticationToken);
		}
		else
			_toString.append("\r\n").append(ENAME_USERNAME).append(" : ").append(_name);
	}

	if (_nameTypeSet)
	{
		_toString.append("\r\n").append(ENAME_USERNAME_TYPE).append(" : ").append(_nameType);
	}

	return _toString;
}

void LoginReqImpl::encode(ReqMsg& reqMsg) const
{
	if (_pElementList == 0)
	{
		try
		{
			_pElementList = new ElementList();
		}
		catch (std::bad_alloc&) {}

		if (!_pElementList)
			throwMeeException("Failed to allocate memory for ElementList in LoginReqImpl::encode().");
	}
	else
	{
		_pElementList->clear();
	}

	if (_allowSuspectDataSet)
	{
		_pElementList->addUInt(ENAME_ALLOW_SUSPECT_DATA, _allowSuspectData);
	}

	if (_applicationIdSet)
	{
		_pElementList->addAscii(ENAME_APP_ID, _applicationId);
	}

	if (_applicationNameSet)
	{
		_pElementList->addAscii(ENAME_APP_NAME, _applicationName);
	}

	if (_applicationAuthTokenSet)
	{
		_pElementList->addAscii(ENAME_APPAUTH_TOKEN, _applicationAuthToken);
	}

	if (_downloadConnectionConfigSet)
	{
		_pElementList->addUInt(ENAME_DOWNLOAD_CON_CONFIG, _downloadConnectionConfig);
	}

	if (_instanceIdSet)
	{
		_pElementList->addAscii(ENAME_INST_ID, _instanceId);
	}

	if (_passwordSet)
	{
		_pElementList->addAscii(ENAME_PASSWORD, _password);
	}

	if (_positionSet)
	{
		_pElementList->addAscii(ENAME_POSITION, _position);
	}

	if ( _providePermissionExpressionsSet )
	{
		_pElementList->addUInt(ENAME_PROV_PERM_EXP, _providePermissionExpressions);
	}

	if (_roleSet)
	{
		_pElementList->addUInt(ENAME_ROLE, _role);
	}

	if (_providePermissionProfileSet)
	{
		_pElementList->addUInt(ENAME_PROV_PERM_PROF, _providePermissionProfile);
	}

	if (_singleOpenSet)
	{
		_pElementList->addUInt(ENAME_SINGLE_OPEN, _singleOpen);
	}

	if (_supportProviderDictionaryDownloadSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, _supportProviderDictionaryDownload);
	}

	if (_nameTypeSet && _nameType == USER_AUTH_TOKEN)
	{
		_pElementList->addAscii(ENAME_AUTH_TOKEN, _authenticationToken);

		if (_authenticationExtendedSet)
		{
			_pElementList->addBuffer(ENAME_AUTH_EXTENDED, _authenticationExtended);
		}
	}

	_pElementList->complete();

	reqMsg.attrib(*_pElementList);
}

void LoginReqImpl::decode(const ReqMsg& reqMsg)
{

	_allowSuspectDataSet = false;
	_downloadConnectionConfigSet = false;
	_providePermissionProfileSet = false;
	_providePermissionExpressionsSet = false;
	_singleOpenSet = false;
	_supportProviderDictionaryDownloadSet = false;
	_roleSet = false;
	_pauseSet = false;
	_applicationIdSet = false;
	_applicationNameSet = false;
	_applicationAuthTokenSet = false;
	_instanceIdSet = false;
	_passwordSet = false;
	_positionSet = false;
	_authenticationExtendedSet = false;
	_nameSet = false;
	_nameTypeSet = false;

	if (reqMsg.hasName())
		name(reqMsg.getName());
	if (reqMsg.hasNameType())
		nameType(reqMsg.getNameType());

	if ( reqMsg.getAttrib().getDataType() != DataType::ElementListEnum )
	  return;

	while (reqMsg.getAttrib().getElementList().forth())
	{
		const ElementEntry& elementEntry = reqMsg.getAttrib().getElementList().getEntry();
		const EmaString& elementName = elementEntry.getName();

		try
		{
			if (elementName == ENAME_ALLOW_SUSPECT_DATA)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					allowSuspectData(true);
				}
				else
				{
					allowSuspectData(false);
				}
			}
			else if (elementName == ENAME_APP_ID)
			{
				if(elementEntry.getCode() == Data::NoCodeEnum)
					applicationId(elementEntry.getAscii());
				
			}

			else if (elementName == ENAME_APP_NAME)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					applicationName(elementEntry.getAscii());
			}
			else if (elementName == ENAME_APPAUTH_TOKEN)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					applicationAuthorizationToken(elementEntry.getAscii());
			}
			else if (elementName == ENAME_DOWNLOAD_CON_CONFIG)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					downloadConnectionConfig(true);
				}
				else
				{
					downloadConnectionConfig(false);
				}
			}
			else if (elementName == ENAME_INST_ID)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					instanceId(elementEntry.getAscii());
			}
			else if (elementName == ENAME_PASSWORD)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					password(elementEntry.getAscii());
			}
			else if (elementName == ENAME_POSITION)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					position(elementEntry.getAscii());
			}
			else if (elementName == ENAME_PROV_PERM_EXP)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					providePermissionExpressions(true);
				}
				else
				{
					providePermissionExpressions(false);
				}
			}
			else if (elementName == ENAME_PROV_PERM_PROF)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					providePermissionProfile(true);
				}
				else
				{
					providePermissionProfile(false);
				}
			}
			else if (elementName == ENAME_ROLE)
			{
				UInt64 value = elementEntry.getUInt();

				if (value == 1)
				{
					role(LOGIN_ROLE_PROV);
				}
				else if (value == 0)
				{
					role(LOGIN_ROLE_CONS);
				}
				else
				{
					EmaString text("Invalid element value of ");
					text.append(value);
					throwIueException( text, OmmInvalidUsageException::InvalidArgumentEnum );
				}

			}
			else if (elementName == ENAME_SINGLE_OPEN)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					singleOpen(true);
				}
				else
				{
					singleOpen(false);
				}
			}
			else if (elementName == ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportProviderDictionaryDownload(true);
				}
				else
				{
					supportProviderDictionaryDownload(false);
				}
			}
			else if (elementName == ENAME_AUTH_EXTENDED)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					authenticationExtended(elementEntry.getBuffer());
			}
			else if (elementName == ENAME_AUTH_TOKEN)
			{
				if (!_nameTypeSet || _nameType != USER_AUTH_TOKEN)
				{
					EmaString text("NameType must be USER_AUTH_TOKEN when element list contains AuthenticationToken");
					throwIueException( text, OmmInvalidUsageException::InvalidArgumentEnum );
				}
				if (elementEntry.getCode() == Data::NoCodeEnum)
					name(elementEntry.getAscii());
			}
		}
		catch (const OmmInvalidUsageException& iue)
		{
			EmaString text("Decoding error for ");
			text.append(elementName).append(" element. ").append(iue.getText());
			throwIueException( text, iue.getErrorCode() );
		}
	}
}
