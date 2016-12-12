package com.thomsonreuters.proxy.authentication;

public class ProxyAuthenticationException extends Exception
{
    private static final long serialVersionUID = 5209362189686162178L;

    public ProxyAuthenticationException(String message)
    {
        super(message);
    }

    public ProxyAuthenticationException(String message, Throwable cause)
    {
        super(message, cause);
    }
}
