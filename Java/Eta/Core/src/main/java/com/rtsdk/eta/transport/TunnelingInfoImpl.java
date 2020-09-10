package com.rtsdk.eta.transport;

class TunnelingInfoImpl implements TunnelingInfo
{
    private String _tunnelingType;
    private boolean _HTTPproxy;
    private String _HTTPproxyHostName;
    private int _HTTPproxyPort;
    private String _objectName;
    private String _KeystoreType;
    private String _KeystoreFile;
    private String _KeystorePasswd;
    private String _SecurityProtocol;
    private String _SecurityProvider;
    private String _KeyManagerAlgorithm;
    private String _TrustManagerAlgorithm;
    
    // default values
    private String _defaultTunnelingType = new String("None");
    private String _defaultObjectName = new String("");
    private String _defaultKeystoreType = new String("JKS");
    private String _defaultSecurityProtocol = new String("TLS");
    private String _defaultSecurityProvider = new String("SunJSSE");
    private String _defaultKeyManagerAlgorithm = new String("SunX509");
    private String _defaultTrustManagerAlgorithm = new String("PKIX");

    TunnelingInfoImpl()
    {
        // for HTTP Tunneling, tunnelingType has to be set to "http" or "encrypted"
        _tunnelingType = _defaultTunnelingType;

        _objectName = _defaultObjectName;

        // Sun JDK defaults
        _KeystoreType = _defaultKeystoreType;
        _SecurityProtocol = _defaultSecurityProtocol;
        _SecurityProvider = _defaultSecurityProvider;
        _KeyManagerAlgorithm = _defaultKeyManagerAlgorithm;
        _TrustManagerAlgorithm = _defaultTrustManagerAlgorithm;
    }

    /* Make a deep copy of this object to the specified object.
     * 
     * destTunneling is the destination object.
     */
    void copy(TunnelingInfoImpl destTunneling)
    {
        if (_tunnelingType != null)
            destTunneling._tunnelingType = new String(_tunnelingType);
        else
            destTunneling._tunnelingType = null;

        destTunneling._HTTPproxy = _HTTPproxy;

        if (_HTTPproxyHostName != null)
            destTunneling._HTTPproxyHostName = new String(_HTTPproxyHostName);
        else
            destTunneling._HTTPproxyHostName = null;

        destTunneling._HTTPproxyPort = _HTTPproxyPort;

        if (_objectName != null)
            destTunneling._objectName = new String(_objectName);
        else
            destTunneling._objectName = null;

        if (_KeystoreType != null)
            destTunneling._KeystoreType = new String(_KeystoreType);
        else
            destTunneling._KeystoreType = null;

        if (_KeystoreFile != null)
            destTunneling._KeystoreFile = new String(_KeystoreFile);
        else
            destTunneling._KeystoreFile = null;

        if (_KeystorePasswd != null)
            destTunneling._KeystorePasswd = new String(_KeystorePasswd);
        else
            destTunneling._KeystorePasswd = null;

        if (_SecurityProtocol != null)
            destTunneling._SecurityProtocol = new String(_SecurityProtocol);
        else
            destTunneling._SecurityProtocol = null;

        if (_SecurityProvider != null)
            destTunneling._SecurityProvider = new String(_SecurityProvider);
        else
            destTunneling._SecurityProvider = null;

        if (_KeyManagerAlgorithm != null)
            destTunneling._KeyManagerAlgorithm = new String(_KeyManagerAlgorithm);
        else
            destTunneling._KeyManagerAlgorithm = null;

        if (_TrustManagerAlgorithm != null)
            destTunneling._TrustManagerAlgorithm = new String(_TrustManagerAlgorithm);
        else
            destTunneling._TrustManagerAlgorithm = null;
    }

    @Override
    public String toString()
    {
        return "TunnelingInfo" + "\n" + 
               "\t\t\ttunnelingType: " + _tunnelingType + "\n" + 
               "\t\t\tHTTPproxy: " + _HTTPproxy + "\n" + 
               "\t\t\tHTTPproxyHostName: " + _HTTPproxyHostName + "\n" + 
               "\t\t\tHTTPproxyPort: " + _HTTPproxyPort + "\n" + 
               "\t\t\tobjectName: " + _objectName + "\n" + 
               "\t\t\tKeystoreType: " + _KeystoreType + "\n" + 
               "\t\t\tKeystoreFile: " + _KeystoreFile + "\n" + 
               "\t\t\tKeystorePasswd: " + _KeystorePasswd + "\n" + 
               "\t\t\tSecurityProtocol: " + _SecurityProtocol + "\n" + 
               "\t\t\tSecurityProvider: " + _SecurityProvider + "\n" + 
               "\t\t\tKeyManagerAlgorithm: " + _KeyManagerAlgorithm + "\n" + 
               "\t\t\tTrustManagerAlgorithm: " + _TrustManagerAlgorithm + "\n";
    }

    @Override
    public void tunnelingType(String tunnelingType)
    {
        assert (tunnelingType != null) : "tunnelingType must be non-null";

        _tunnelingType = tunnelingType;
    }

    @Override
    public String tunnelingType()
    {
        return _tunnelingType;
    }

    @Override
    public void HTTPproxy(boolean HTTPproxy)
    {
        _HTTPproxy = HTTPproxy;
    }

    @Override
    public boolean HTTPproxy()
    {
        return _HTTPproxy;
    }

    @Override
    public void HTTPproxyHostName(String HTTPproxyHostName)
    {
        assert (HTTPproxyHostName != null) : "HTTPproxyHostName must be non-null";

        _HTTPproxyHostName = HTTPproxyHostName;
    }

    @Override
    public String HTTPproxyHostName()
    {
        return _HTTPproxyHostName;
    }

    @Override
    public void HTTPproxyPort(int HTTPproxyPort)
    {
        assert (HTTPproxyPort >= 0 && HTTPproxyPort <= 65535) : "HTTPproxyPort is out of range (0-65535)"; // uint16

        _HTTPproxyPort = HTTPproxyPort;
    }

    @Override
    public int HTTPproxyPort()
    {
        return _HTTPproxyPort;
    }

    @Override
    public void objectName(String objectName)
    {
        assert (objectName != null) : "objectName must be non-null";

        _objectName = objectName;
    }

    @Override
    public String objectName()
    {
        return _objectName;
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
        assert (KeystoreFile != null) : "KeystoreFile must be non-null";

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

    void clear()
    {
        _tunnelingType = _defaultTunnelingType;
        _HTTPproxyHostName = null;
        _objectName = _defaultObjectName;
        _KeystoreType = _defaultKeystoreType;
        _KeystoreFile = null;
        _KeystorePasswd = null;
        _SecurityProtocol = _defaultSecurityProtocol;
        _SecurityProvider = _defaultSecurityProvider;
        _KeyManagerAlgorithm = _defaultKeyManagerAlgorithm;
        _TrustManagerAlgorithm = _defaultTrustManagerAlgorithm;
    }

}
