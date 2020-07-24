package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.transport.ConnectionTypes;

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
	String				location;
	boolean				enableSessionMgnt;

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
		location = ActiveConfig.DEFAULT_REGION_LOCATION;
		enableSessionMgnt = ActiveConfig.DEFAULT_ENABLE_SESSION_MGNT;
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
		location = source.location;
		enableSessionMgnt = source.enableSessionMgnt;
	}
}
