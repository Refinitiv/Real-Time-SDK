package com.refinitiv.eta.transport;

import java.util.Arrays;

class EncryptionOptionsImpl implements EncryptionOptions {
    private int _connectionType;
    private String _KeystoreType;
    private String _KeystoreFile;
    private String _KeystorePasswd;
    private String _SecurityProtocol;
    private String[] _SecurityProtocolVersions;
    private String _SecurityProvider;
    private String _KeyManagerAlgorithm;
    private String _TrustManagerAlgorithm;
    
    // default values

    public static String _defaultKeystoreType = new String("JKS");
    public static String _defaultSecurityProtocol = new String("TLS");
    public static String[] _defaultSecurityProtocolVersions = {"1.3", "1.2"};
    public static String _defaultSecurityProvider = new String("SunJSSE");
    public static String _defaultKeyManagerAlgorithm = new String("SunX509");
    public static String _defaultTrustManagerAlgorithm = new String("PKIX");

    EncryptionOptionsImpl()
    {
        // for HTTP Tunneling, tunnelingType has to be set to "http" or "encrypted"
        _connectionType = ConnectionTypes.SOCKET;

        // Sun JDK defaults
        _KeystoreType = _defaultKeystoreType;
        _SecurityProtocol = _defaultSecurityProtocol;
        _SecurityProtocolVersions = _defaultSecurityProtocolVersions;
        _SecurityProvider = _defaultSecurityProvider;
        _KeyManagerAlgorithm = _defaultKeyManagerAlgorithm;
        _TrustManagerAlgorithm = _defaultTrustManagerAlgorithm;
    }

    void clear()
    {
    	_connectionType = ConnectionTypes.SOCKET;
        _KeystoreType = _defaultKeystoreType;
        _KeystoreFile = null;
        _KeystorePasswd = null;
        _SecurityProtocol = _defaultSecurityProtocol;
        _SecurityProtocolVersions = _defaultSecurityProtocolVersions;
        _SecurityProvider = _defaultSecurityProvider;
        _KeyManagerAlgorithm = _defaultKeyManagerAlgorithm;
        _TrustManagerAlgorithm = _defaultTrustManagerAlgorithm;
    }
    
    /* Make a deep copy of this object to the specified object.
     * 
     * destTunneling is the destination object.
     */
    void copy(EncryptionOptionsImpl destEncOpts)
    {
    	destEncOpts._connectionType = _connectionType;

        if (_KeystoreType != null)
        	destEncOpts._KeystoreType = new String(_KeystoreType);
        else
        	destEncOpts._KeystoreType = null;

        if (_KeystoreFile != null)
        	destEncOpts._KeystoreFile = new String(_KeystoreFile);
        else
        	destEncOpts._KeystoreFile = null;

        if (_KeystorePasswd != null)
        	destEncOpts._KeystorePasswd = new String(_KeystorePasswd);
        else
        	destEncOpts._KeystorePasswd = null;

        if (_SecurityProtocol != null)
        	destEncOpts._SecurityProtocol = new String(_SecurityProtocol);
        else
        	destEncOpts._SecurityProtocol = null;
        
        if (_SecurityProtocolVersions != null)
        	destEncOpts._SecurityProtocolVersions = _SecurityProtocolVersions.clone();
        else
        	destEncOpts._SecurityProtocolVersions = null;

        if (_SecurityProvider != null)
        	destEncOpts._SecurityProvider = new String(_SecurityProvider);
        else
        	destEncOpts._SecurityProvider = null;

        if (_KeyManagerAlgorithm != null)
        	destEncOpts._KeyManagerAlgorithm = new String(_KeyManagerAlgorithm);
        else
        	destEncOpts._KeyManagerAlgorithm = null;

        if (_TrustManagerAlgorithm != null)
        	destEncOpts._TrustManagerAlgorithm = new String(_TrustManagerAlgorithm);
        else
        	destEncOpts._TrustManagerAlgorithm = null;
    }

    @Override
    public String toString()
    {
        return "EncryptionOptions" + "\n" + 
               "\t\t\tconnectionType: " + _connectionType + "\n" + 
               "\t\t\tKeystoreType: " + _KeystoreType + "\n" + 
               "\t\t\tKeystoreFile: " + _KeystoreFile + "\n" + 
               "\t\t\tKeystorePasswd: " + _KeystorePasswd + "\n" + 
               "\t\t\tSecurityProtocol: " + _SecurityProtocol + "\n" + 
               "\t\t\tSecurityProtocolVersions: " + Arrays.toString(_SecurityProtocolVersions) + "\n" + 
               "\t\t\tSecurityProvider: " + _SecurityProvider + "\n" + 
               "\t\t\tKeyManagerAlgorithm: " + _KeyManagerAlgorithm + "\n" + 
               "\t\t\tTrustManagerAlgorithm: " + _TrustManagerAlgorithm + "\n";
    }
    
    @Override
    public void connectionType(int connectionType)
    {
    	_connectionType = connectionType;
    }
    
    @Override
    public int connectionType()
    {
    	return _connectionType;
    }

    @Override
    public void KeystoreType(String KeystoreType)
    {
        assert (KeystoreType != null) : "KeystoreType must be non-null";

        _KeystoreType = KeystoreType;
    }

    @Override
    public String KeystoreType()
    {
        return _KeystoreType;
    }

    @Override
    public void KeystoreFile(String KeystoreFile)
    {
        _KeystoreFile = KeystoreFile;
    }

    @Override
    public String KeystoreFile()
    {
        return _KeystoreFile;
    }

    @Override
    public void KeystorePasswd(String KeystorePasswd)
    {
        assert (KeystorePasswd != null) : "KeystorePasswd must be non-null";

        _KeystorePasswd = KeystorePasswd;
    }

    @Override
    public String KeystorePasswd()
    {
        return _KeystorePasswd;
    }

    @Override
    public void SecurityProtocol(String SecurityProtocol)
    {
        assert (SecurityProtocol != null) : "SecurityProtocol must be non-null";

        _SecurityProtocol = SecurityProtocol;
    }

    @Override
    public String SecurityProtocol()
    {
        return _SecurityProtocol;
    }

    @Override
    public void SecurityProvider(String SecurityProvider)
    {
        assert (SecurityProvider != null) : "SecurityProvider must be non-null";

        _SecurityProvider = SecurityProvider;
    }

    @Override
    public String SecurityProvider()
    {
        return _SecurityProvider;
    }

    @Override
    public void KeyManagerAlgorithm(String KeyManagerAlgorithm)
    {
        assert (KeyManagerAlgorithm != null) : "KeyManagerAlgorithm must be non-null";

        _KeyManagerAlgorithm = KeyManagerAlgorithm;
    }

    @Override
    public String KeyManagerAlgorithm()
    {
        return _KeyManagerAlgorithm;
    }

    @Override
    public void TrustManagerAlgorithm(String TrustManagerAlgorithm)
    {
        assert (TrustManagerAlgorithm != null) : "TrustManagerAlgorithm must be non-null";

        _TrustManagerAlgorithm = TrustManagerAlgorithm;
    }

    @Override
    public String TrustManagerAlgorithm()
    {
        return _TrustManagerAlgorithm;
    }

	@Override
	public void SecurityProtocolVersions(String[] SecurityProtocolVersions) {
		_SecurityProtocolVersions = SecurityProtocolVersions;
	}

	@Override
	public String[] SecurityProtocolVersions() {
		return _SecurityProtocolVersions;
	}

}
