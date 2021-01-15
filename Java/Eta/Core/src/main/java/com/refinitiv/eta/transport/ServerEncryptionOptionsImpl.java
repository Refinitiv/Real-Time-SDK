package com.refinitiv.eta.transport;

class ServerEncryptionOptionsImpl implements ServerEncryptionOptions {

    private String _KeystoreType;
    private String _KeystoreFile;
    private String _KeystorePasswd;
    private String _SecurityProtocol;
    private String _SecurityProvider;
    private String _KeyManagerAlgorithm;
    private String _TrustManagerAlgorithm;
    
    // default values

    public String _defaultKeystoreType = new String("JKS");
    public String _defaultSecurityProtocol = new String("TLS");
    public String _defaultSecurityProvider = new String("SunJSSE");
    public String _defaultKeyManagerAlgorithm = new String("SunX509");
    public String _defaultTrustManagerAlgorithm = new String("PKIX");

    ServerEncryptionOptionsImpl()
    {
        // Sun JDK defaults
        _KeystoreType = _defaultKeystoreType;
        _SecurityProtocol = _defaultSecurityProtocol;
        _SecurityProvider = _defaultSecurityProvider;
        _KeyManagerAlgorithm = _defaultKeyManagerAlgorithm;
        _TrustManagerAlgorithm = _defaultTrustManagerAlgorithm;
    }

    void clear()
    {
        _KeystoreType = _defaultKeystoreType;
        _KeystoreFile = null;
        _KeystorePasswd = null;
        _SecurityProtocol = _defaultSecurityProtocol;
        _SecurityProvider = _defaultSecurityProvider;
        _KeyManagerAlgorithm = _defaultKeyManagerAlgorithm;
        _TrustManagerAlgorithm = _defaultTrustManagerAlgorithm;
    }
    
    /* Make a deep copy of this object to the specified object.
     * 
     * destTunneling is the destination object.
     */
    void copy(ServerEncryptionOptionsImpl destEncOpts)
    {
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
        return "ServerEncryptionOptions" + "\n" + 
               "\t\t\tKeystoreType: " + _KeystoreType + "\n" + 
               "\t\t\tKeystoreFile: " + _KeystoreFile + "\n" + 
               "\t\t\tKeystorePasswd: " + _KeystorePasswd + "\n" + 
               "\t\t\tSecurityProtocol: " + _SecurityProtocol + "\n" + 
               "\t\t\tSecurityProvider: " + _SecurityProvider + "\n" + 
               "\t\t\tKeyManagerAlgorithm: " + _KeyManagerAlgorithm + "\n" + 
               "\t\t\tTrustManagerAlgorithm: " + _TrustManagerAlgorithm + "\n";
    }
    

    @Override
    public void keystoreType(String KeystoreType)
    {
        assert (KeystoreType != null) : "KeystoreType must be non-null";

        _KeystoreType = KeystoreType;
    }

    @Override
    public String keystoreType()
    {
        return _KeystoreType;
    }

    @Override
    public void keystoreFile(String KeystoreFile)
    {
        assert (KeystoreFile != null) : "KeystoreFile must be non-null";

        _KeystoreFile = KeystoreFile;
    }

    @Override
    public String keystoreFile()
    {
        return _KeystoreFile;
    }

    @Override
    public void keystorePasswd(String KeystorePasswd)
    {
        assert (KeystorePasswd != null) : "KeystorePasswd must be non-null";

        _KeystorePasswd = KeystorePasswd;
    }

    @Override
    public String keystorePasswd()
    {
        return _KeystorePasswd;
    }

    @Override
    public void securityProtocol(String SecurityProtocol)
    {
        assert (SecurityProtocol != null) : "SecurityProtocol must be non-null";

        _SecurityProtocol = SecurityProtocol;
    }

    @Override
    public String securityProtocol()
    {
        return _SecurityProtocol;
    }

    @Override
    public void securityProvider(String SecurityProvider)
    {
        assert (SecurityProvider != null) : "SecurityProvider must be non-null";

        _SecurityProvider = SecurityProvider;
    }

    @Override
    public String securityProvider()
    {
        return _SecurityProvider;
    }

    @Override
    public void keyManagerAlgorithm(String KeyManagerAlgorithm)
    {
        assert (KeyManagerAlgorithm != null) : "KeyManagerAlgorithm must be non-null";

        _KeyManagerAlgorithm = KeyManagerAlgorithm;
    }

    @Override
    public String keyManagerAlgorithm()
    {
        return _KeyManagerAlgorithm;
    }

    @Override
    public void trustManagerAlgorithm(String TrustManagerAlgorithm)
    {
        assert (TrustManagerAlgorithm != null) : "TrustManagerAlgorithm must be non-null";

        _TrustManagerAlgorithm = TrustManagerAlgorithm;
    }

    @Override
    public String trustManagerAlgorithm()
    {
        return _TrustManagerAlgorithm;
    }

}
