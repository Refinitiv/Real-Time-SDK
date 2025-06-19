/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmOAuth2CredentialImpl.h"
#include "ExceptionTranslator.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

/* Dummy no-op OAuth2 Consumer class client for initializing handlers */
/* This should never be used */
class DummyOAuth2ConsClientImpl : public refinitiv::ema::access::OmmOAuth2ConsumerClient
{
};

static DummyOAuth2ConsClientImpl defaultOAuthConsClient;

/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal */
OmmOAuth2CredentialImpl::OmmOAuth2CredentialImpl(OAuth2Credential& credentials) :
	OAuth2Credential(credentials),
	_client(defaultOAuthConsClient),
	_oAuthArrayIndex(0)
{
	rsslClearReactorOAuthCredential(&_oAuthCredential);
	_hasOAuth2Client = false;
	_closure = NULL;
}

/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal */
OmmOAuth2CredentialImpl::OmmOAuth2CredentialImpl(OAuth2Credential& credentials, OmmOAuth2ConsumerClient& consumerClient) :
	OAuth2Credential(credentials),
	_client(consumerClient),
	_oAuthArrayIndex(0)
{
	_hasOAuth2Client = true;
	_closure = NULL;
	rsslClearReactorOAuthCredential(&_oAuthCredential);
}

/** Create an OAuth2Credential for use with OmmConsumer::SubmitOAuthCredentialRenewal */
OmmOAuth2CredentialImpl::OmmOAuth2CredentialImpl(OAuth2Credential& credentials, OmmOAuth2ConsumerClient& consumerClient, void* closure) :
	OAuth2Credential(credentials),
	_client(consumerClient),
	_oAuthArrayIndex(0)
{
	_hasOAuth2Client = true;
	_closure = closure;
	rsslClearReactorOAuthCredential(&_oAuthCredential);
}

OmmOAuth2CredentialImpl::OmmOAuth2CredentialImpl(const OmmOAuth2CredentialImpl& credentials) :
	OAuth2Credential(credentials),
	_client(credentials._client),
	_oAuthArrayIndex(credentials._oAuthArrayIndex)
{
	_hasOAuth2Client = credentials._hasOAuth2Client;
	_closure = credentials._closure;
	rsslClearReactorOAuthCredential(&_oAuthCredential);
}

/** Clear out all contained EmaString by zeroing out the memory, then free everything. */
OmmOAuth2CredentialImpl::~OmmOAuth2CredentialImpl()
{
	clear();
}

bool OmmOAuth2CredentialImpl::isOAuth2ClientSet()
{
	return _hasOAuth2Client;
}


void OmmOAuth2CredentialImpl::oAuth2ArrayIndex(UInt8 index)
{
	_oAuthArrayIndex = index;
}

UInt8 OmmOAuth2CredentialImpl::getOAuth2ArrayIndex()
{
	return _oAuthArrayIndex;
}


OmmOAuth2ConsumerClient& OmmOAuth2CredentialImpl::getClient()
{
	return _client;
}

void* OmmOAuth2CredentialImpl::getClosure()
{
	return _closure;
}
