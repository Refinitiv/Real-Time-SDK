/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.transport.ConnectionTypes;

class EncryptionConfig
{
	int					ConnectionType = ConnectionTypes.SOCKET;
	String				KeyStoreType;
	String				KeyStoreFile;
	String				KeyStorePasswd;
	String				SecurityProvider;
	String 				SecurityProtocol;
	String[]			SecurityProtocolVersions;
	String				KeyManagerAlgorithm;
	String				TrustManagerAlgorithm;


	void clear()
	{
		ConnectionType = ConnectionTypes.SOCKET;
		KeyStoreType = null;
		KeyStoreFile = null;
		KeyStorePasswd = null;
		SecurityProvider = null;
		SecurityProtocol = null;
		SecurityProtocolVersions = null;
		KeyManagerAlgorithm = null;
		TrustManagerAlgorithm = null;
	}

	void copy(EncryptionConfig source)
	{
		ConnectionType = source.ConnectionType;
		KeyStoreType = source.KeyStoreType;
		KeyStoreFile = source.KeyStoreFile;
		KeyStorePasswd = source.KeyStorePasswd;
		SecurityProvider = source.SecurityProvider;
		SecurityProtocol = source.SecurityProtocol;
		SecurityProtocolVersions = source.SecurityProtocolVersions;
		KeyManagerAlgorithm = source.KeyManagerAlgorithm;
		TrustManagerAlgorithm = source.TrustManagerAlgorithm;
	}
}
