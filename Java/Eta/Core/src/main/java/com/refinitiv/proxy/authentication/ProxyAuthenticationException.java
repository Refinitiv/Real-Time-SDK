package com.refinitiv.proxy.authentication;

public class ProxyAuthenticationException extends Exception
{
    private static final long serialVersionUID = 5209362189686162178L;

    /**
     * Instantiates a new proxy authentication exception.
     *
     * @param message the message
     */
    public ProxyAuthenticationException(String message)
    {
        super(message);
    }

    /**
     * Instantiates a new proxy authentication exception.
     *
     * @param message the message
     * @param cause the cause
     */
    public ProxyAuthenticationException(String message, Throwable cause)
    {
        super(message, cause);
    }
}
