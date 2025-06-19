/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

public interface OmmOAuth2ConsumerClient {

	/**
	 * Invoked when new credentials are required
	 * @param event Renewal event information
	 */ 
	public void onOAuth2CredentialRenewal(OmmConsumerEvent event);
}
