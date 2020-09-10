package com.rtsdk.ema.access;

import com.rtsdk.eta.transport.ConnectionTypes;

class EncryptionConfig
{
	int					ConnectionType;
	String				KeyStoreType;
	String				KeyStoreFile;
	String				KeyStorePasswd;
	String				SecurityProvider;
	String 				SecurityProtocol;
	String				KeyManagerAlgorithm;
	String				TrustManagerAlgorithm;

	EncryptionConfig()
	{
		clear();
	}

	void clear()
	{
		ConnectionType = ConnectionTypes.HTTP;
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
