package com.refinitiv.proxy.authentication;

public class CredentialsFactory
{
    
    /**
     * Creates the.
     *
     * @return the i credentials
     */
    public static ICredentials create()
    {
        return new CredentialsImpl();
    }
}
