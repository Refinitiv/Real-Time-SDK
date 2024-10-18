/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.
*|                See the project's LICENSE.md for details.
*|           Copyright (C) 2019-2020, 2024 LSEG. All rights reserved.        --
*|-----------------------------------------------------------------------------
*/

#include "LoginStatusImpl.h"
#include "OmmInvalidUsageException.h"
#include <new>

using namespace refinitiv::ema::domain::login;
using namespace refinitiv::ema::rdm;
using namespace refinitiv::ema::access;

LoginStatusImpl::LoginStatusImpl() :
	_pElementList(0),
	_rsslState(new RsslState())
#ifdef __EMA_COPY_ON_SET__
	, _statusText()
#endif
{
	clear();
}

LoginStatusImpl::LoginStatusImpl(const LoginStatusImpl& other) :
	_pElementList(0),
	_rsslState(new RsslState())
#ifdef __EMA_COPY_ON_SET__
	, _statusText()
#endif
{
	*this = other;
}

LoginStatusImpl::LoginStatusImpl(const StatusMsg& statusMsg) :
	_pElementList(0),
	_rsslState(new RsslState())
#ifdef __EMA_COPY_ON_SET__
	, _statusText()
#endif
{
	clear();

	decode(statusMsg);
}

LoginStatusImpl::~LoginStatusImpl()
{
	if (_pElementList)
	{
		delete _pElementList;
		_pElementList = 0;
	}

	if (_rsslState)
	{
		delete _rsslState;
		_rsslState = 0;
	}
}

LoginStatusImpl& LoginStatusImpl::clear()
{
	_changed = true;
	_authenticationErrorCodeSet = false;
	_authenticationErrorTextSet = false;

	_nameSet = false;
	_nameTypeSet = true;
	_stateSet = false;

	rsslClearState(_rsslState);

	_nameType = USER_NAME;
	_domainType = MMT_LOGIN;

#ifdef __EMA_COPY_ON_SET__
	_statusText.clear();
#endif

	return *this;
}

LoginStatusImpl& LoginStatusImpl::operator=(const LoginStatusImpl& other)
{
	_changed = true;
	_authenticationErrorCodeSet = other._authenticationErrorCodeSet;
	_authenticationErrorTextSet = other._authenticationErrorTextSet;
	_nameSet = other._nameSet;
	_nameTypeSet = other._nameTypeSet;
	_stateSet = other._stateSet;

	_authenticationErrorCode = other._authenticationErrorCode;
	_authenticationErrorText = other._authenticationErrorText;
	_name = other._name;
	_nameType = other._nameType;
	_stateText = other._stateText;
	_rsslState->streamState = other._state.getStreamState();
	_rsslState->dataState = other._state.getDataState();
	_rsslState->code = other._state.getCode();
#ifdef __EMA_COPY_ON_SET__
	_statusText = other._statusText;
	_rsslState->text.data = (char*)_statusText.c_str();
	_rsslState->text.length = _statusText.length();
#else
	_rsslState->text.data = (char*)other._state.getStatusText().c_str();
	_rsslState->text.length = other._state.getStatusText().length();
#endif
	_domainType = other._domainType;

	return *this;
}

LoginStatusImpl& LoginStatusImpl::message(const StatusMsg& statusMsg)
{
	decode(statusMsg);

	return *this;
}

LoginStatusImpl& LoginStatusImpl::authenticationErrorCode(const UInt64& value)
{
	_changed = true;
	_authenticationErrorCodeSet = true;
	_authenticationErrorCode = value;

	return *this;
}

LoginStatusImpl& LoginStatusImpl::authenticationErrorText(const EmaString& value)
{
	_changed = true;
	_authenticationErrorTextSet = true;
	_authenticationErrorText = value;

	return *this;
}

LoginStatusImpl& LoginStatusImpl::name(const EmaString& value)
{
	_changed = true;
	_nameSet = true;
	_name = value;

	return *this;
}

LoginStatusImpl& LoginStatusImpl::nameType(const UInt32& value)
{
	_changed = true;
	_nameTypeSet = true;
	_nameType = value;

	return *this;
}

LoginStatusImpl& LoginStatusImpl::state(const OmmState::StreamState& streamState, const OmmState::DataState dataState, const UInt8& statusCode, const EmaString& statusText)
{
	_changed = true;
	_stateSet = true;

	_rsslState->streamState = streamState;
	_rsslState->dataState = dataState;
	_rsslState->code = statusCode;
#ifdef __EMA_COPY_ON_SET__
	_statusText = statusText;
	_rsslState->text.data = (char*)_statusText.c_str();
	_rsslState->text.length = _statusText.length();
#else
	_rsslState->text.data = (char*)statusText.c_str();
	_rsslState->text.length = statusText.length();
#endif
	_stateText.setInt(_rsslState->text.data, _rsslState->text.length, false);

	return *this;
}

LoginStatusImpl& LoginStatusImpl::state(const OmmState& state)
{
	_changed = true;
	_stateSet = true;

	_rsslState->streamState = state.getStreamState();
	_rsslState->dataState = state.getDataState();
	_rsslState->code = (UInt8)state.getStatusCode();
#ifdef __EMA_COPY_ON_SET__
	_statusText = state.getStatusText();
	_rsslState->text.data = (char*)_statusText.c_str();
	_rsslState->text.length = _statusText.length();
#else
	_rsslState->text.data = (char*)state.getStatusText().c_str();
	_rsslState->text.length = state.getStatusText().length();
#endif
	_stateText.setInt(_rsslState->text.data, _rsslState->text.length, false);

	return *this;
}

bool LoginStatusImpl::hasAuthenticationErrorCode() const
{
	return _authenticationErrorCodeSet;
}

bool LoginStatusImpl::hasAuthenticationErrorText() const
{
	return _authenticationErrorTextSet;
}

bool LoginStatusImpl::hasName() const
{
	return _nameSet;
}

bool LoginStatusImpl::hasNameType() const
{
	return _nameTypeSet;
}

bool LoginStatusImpl::hasState() const
{
	return _stateSet;
}

const StatusMsg& LoginStatusImpl::getMessage()
{
	_statusMsg.clear();

	_statusMsg.domainType(_domainType);
	if (_nameTypeSet)
		_statusMsg.nameType(_nameType);
	if (_nameSet)
		_statusMsg.name(_name);
	if (_stateSet)
		_statusMsg.state((OmmState::StreamState)_rsslState->streamState, (OmmState::DataState)_rsslState->dataState, _rsslState->code, _stateText);

	if (!_changed)
	{
		return _statusMsg;
	}

	encode(_statusMsg);

	_changed = false;

	return _statusMsg;
}

const UInt64& LoginStatusImpl::getAuthenticationErrorCode() const
{
	if (!_authenticationErrorCodeSet)
	{
		EmaString text(ENAME_AUTH_ERRORCODE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationErrorCode;
}

const EmaString& LoginStatusImpl::getAuthenticationErrorText() const
{
	if (!_authenticationErrorTextSet)
	{
		EmaString text(ENAME_AUTH_ERRORTEXT);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationErrorText;
}

const EmaString& LoginStatusImpl::getName() const
{
	if (!_nameSet)
	{
		EmaString text(ENAME_USERNAME);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _name;
}

const UInt32& LoginStatusImpl::getNameType() const
{
	if (!_nameTypeSet)
	{
		EmaString text(ENAME_USERNAME_TYPE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _nameType;
}

const OmmState& LoginStatusImpl::getState() const
{
	if (!_stateSet)
	{
		EmaString text(ENAME_STATE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}
	 
	_state._pDecoder->setRsslData(_rsslState);

	return _state;
}


const EmaString& LoginStatusImpl::toString() const
{
	_toString.clear();

	if (_authenticationErrorCodeSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_ERRORCODE).append(" : ").append(_authenticationErrorCode);
	}

	if (_authenticationErrorTextSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_ERRORTEXT).append(" : ").append(_authenticationErrorText);
	}

	if (_nameSet)
	{
		_toString.append("\r\n").append(ENAME_USERNAME).append(" : ").append(_name);
	}

	if (_nameTypeSet)
	{
		_toString.append("\r\n").append(ENAME_USERNAME_TYPE).append(" : ").append(_nameType);
	}

	if (_stateSet)
	{
		_toString.append("\r\n").append(ENAME_STATE).append(" : StreamState: ").append(_rsslState->streamState)
			.append(" DataState: ").append(_rsslState->dataState)
			.append(" StatusCode: ").append(_rsslState->code)
			.append(" StatusText: ").append(_stateText);
	}

	return _toString;
}

void LoginStatusImpl::encode(StatusMsg& statusMsg) const
{
	if (_pElementList == 0)
	{
		try
		{
			_pElementList = new ElementList();
		}
		catch (std::bad_alloc&)
		{
			throwMeeException("Failed to allocate memory for ElementList in LoginStatusImpl::encode().");
		}
	}
	else
	{
		_pElementList->clear();
	}

	if (_authenticationErrorCodeSet)
	{
		_pElementList->addUInt(ENAME_AUTH_ERRORCODE, _authenticationErrorCode);
	}

	if (_authenticationErrorTextSet)
	{
		_pElementList->addAscii(ENAME_AUTH_ERRORTEXT, _authenticationErrorText);
	}

	_pElementList->complete();

	statusMsg.attrib(*_pElementList);
}

void LoginStatusImpl::decode(const StatusMsg& statusMsg)
{
	_authenticationErrorCodeSet = false;
	_authenticationErrorTextSet = false;
	_stateSet = false;
	_nameSet = false;
	_nameTypeSet = false;

	if (statusMsg.hasState())
		state(statusMsg.getState());
	if (statusMsg.hasName())
		name(statusMsg.getName());
	if (statusMsg.hasNameType())
		nameType(statusMsg.getNameType());

	if ( statusMsg.getAttrib().getDataType() != DataType::ElementListEnum )
	  return;

	while (statusMsg.getAttrib().getElementList().forth())
	{
		const ElementEntry& elementEntry = statusMsg.getAttrib().getElementList().getEntry();
		const EmaString& elementName = elementEntry.getName();

		try
		{
			if (elementName == ENAME_AUTH_ERRORCODE)
			{
				authenticationErrorCode(elementEntry.getUInt());
			}
			else if (elementName == ENAME_AUTH_ERRORTEXT)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					authenticationErrorText(elementEntry.getAscii());
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
