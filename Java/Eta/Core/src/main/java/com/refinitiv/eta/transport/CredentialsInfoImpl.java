package com.refinitiv.eta.transport;

class CredentialsInfoImpl implements CredentialsInfo
{
    private String _HTTPproxyUsername;
    private String _HTTPproxyPasswd;
    private String _HTTPproxyDomain;
    private String _HTTPproxyLocalHostname;
    private String _HTTPproxyKRB5configFile;	

    CredentialsInfoImpl()
    {
    }

    /* Make a deep copy of this object to the specified object.
     * 
     * destCredentials is the destination object.
     */
    void copy(CredentialsInfoImpl destCredentials)
    {
        if (_HTTPproxyUsername != null)
            destCredentials._HTTPproxyUsername = new String(_HTTPproxyUsername);
        else
            destCredentials._HTTPproxyUsername = null;

        if (_HTTPproxyPasswd != null)
            destCredentials._HTTPproxyPasswd = new String(_HTTPproxyPasswd);
        else
            destCredentials._HTTPproxyPasswd = null;

        if (_HTTPproxyDomain != null)
            destCredentials._HTTPproxyDomain = new String(_HTTPproxyDomain);
        else
            destCredentials._HTTPproxyDomain = null;

        if (_HTTPproxyLocalHostname != null)
            destCredentials._HTTPproxyLocalHostname = new String(_HTTPproxyLocalHostname);
        else
            destCredentials._HTTPproxyLocalHostname = null;

        if (_HTTPproxyKRB5configFile != null)
            destCredentials._HTTPproxyKRB5configFile = new String(_HTTPproxyKRB5configFile);
        else
            destCredentials._HTTPproxyKRB5configFile = null;
    }

    @Override
    public String toString()
    {
        return "CredentialsInfo" + "\n" + 
               "\t\t\tHTTPproxyUsername: " + _HTTPproxyUsername + "\n" + 
               "\t\t\tHTTPproxyPasswd: " + _HTTPproxyPasswd + "\n" + 
               "\t\t\tHTTPproxyDomain: " + _HTTPproxyDomain + "\n" + 
               "\t\t\tHTTPproxyLocalHostname: " + _HTTPproxyLocalHostname + "\n" + 
               "\t\t\tHTTPproxyKRB5configFile: " + _HTTPproxyKRB5configFile;
    }

    void clear()
    {
        _HTTPproxyUsername = null;
        _HTTPproxyPasswd = null;
        _HTTPproxyDomain = null;
        _HTTPproxyLocalHostname = null;
        _HTTPproxyKRB5configFile = null;
    }

    public void HTTPproxyUsername(String HTTPproxyUsername)
    {
        assert (HTTPproxyUsername != null) : "HTTPproxyUsername must be non-null";

        _HTTPproxyUsername = HTTPproxyUsername;
    }

    @Override
    public String HTTPproxyUsername()
    {
        return _HTTPproxyUsername;
    }

    public void HTTPproxyPasswd(String HTTPproxyPasswd)
    {
        assert (HTTPproxyPasswd != null) : "tunnelingHTTPproxyPasswd must be non-null";

        _HTTPproxyPasswd = HTTPproxyPasswd;
    }

    @Override
    public String HTTPproxyPasswd()
    {
        return _HTTPproxyPasswd;
    }

    public void HTTPproxyDomain(String HTTPproxyDomain)
    {
        assert (HTTPproxyDomain != null) : "HTTPproxyDomain must be non-null";

        _HTTPproxyDomain = HTTPproxyDomain;
    }

    @Override
    public String HTTPproxyDomain()
    {
        return _HTTPproxyDomain;
    }

    public void HTTPproxyLocalHostname(String HTTPproxyLocalHostname)
    {
        assert (HTTPproxyLocalHostname != null) : "HTTPproxyLocalHostname must be non-null";

        _HTTPproxyLocalHostname = HTTPproxyLocalHostname;
    }

    @Override
    public String HTTPproxyLocalHostname()
    {
        return _HTTPproxyLocalHostname;
    }

    public void HTTPproxyKRB5configFile(String HTTPproxyKRB5configFile)
    {
        assert (HTTPproxyKRB5configFile != null) : "HTTPproxyKRB5configFile must be non-null";

        _HTTPproxyKRB5configFile = HTTPproxyKRB5configFile;
    }

    @Override
    public String HTTPproxyKRB5configFile()
    {
        return _HTTPproxyKRB5configFile;
    }
}
