package com.thomsonreuters.proxy.authentication;

public class CredentialsFactory
{
    public static ICredentials create()
    {
        return new CredentialsImpl();
    }
}
