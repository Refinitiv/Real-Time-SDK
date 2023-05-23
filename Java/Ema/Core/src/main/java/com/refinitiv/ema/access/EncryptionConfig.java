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
		KeyManagerAlgorithm = source.KeyManagerAlgorithm;
		TrustManagerAlgorithm = source.TrustManagerAlgorithm;
	}
}
