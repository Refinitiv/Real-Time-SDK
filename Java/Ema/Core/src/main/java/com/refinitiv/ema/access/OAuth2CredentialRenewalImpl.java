///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2022 Refinitiv. All rights reserved.               --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorOAuthCredentialRenewal;

class OAuth2CredentialRenewalImpl implements OAuth2CredentialRenewal {

	private ReactorOAuthCredentialRenewal credentials  = ReactorFactory.createReactorOAuthCredentialRenewal();
	
	OAuth2CredentialRenewalImpl()
	{
		credentials.clear();
	}
	
	@Override
	public OAuth2CredentialRenewal clear() {
		credentials.clear();
		return this;
	}

	@Override
	public OAuth2CredentialRenewal userName(String userName) {
		credentials.userName().data(userName);
		return this;
	}

	@Override
	public OAuth2CredentialRenewal password(String password) {
		credentials.password().data(password);
		return this;
	}

	@Override
	public OAuth2CredentialRenewal newPassword(String newPassword) {
		credentials.newPassword().data(newPassword);
		return this;
	}

	@Override
	public OAuth2CredentialRenewal clientId(String clientId) {
		credentials.clientId().data(clientId);
		return this;
	}

	@Override
	public OAuth2CredentialRenewal clientSecret(String clientSecret) {
		credentials.clientSecret().data(clientSecret);
		return this;
	}
	
	@Override
	public OAuth2CredentialRenewal clientJWK(String clientJwk) {
		credentials.clientJWK().data(clientJwk);
		return this;
	}

	@Override
	public OAuth2CredentialRenewal tokenScope(String tokenScope) {
		credentials.tokenScope().data(tokenScope);
		return this;
	}
	
	@Override
	public OAuth2CredentialRenewal audience(String audience) {
		credentials.audience().data(audience);
		return this;
	}

	@Override
	public OAuth2CredentialRenewal takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl) {
		credentials.takeExclusiveSignOnControl(takeExclusiveSignOnControl);
		return this;
	}
	
	public ReactorOAuthCredentialRenewal getReactorOAuthCredentialRenewal()
	{
		return credentials;
	}
	

}
