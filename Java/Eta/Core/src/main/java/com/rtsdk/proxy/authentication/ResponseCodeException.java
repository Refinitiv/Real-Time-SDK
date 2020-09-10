package com.rtsdk.proxy.authentication;

public class ResponseCodeException extends ProxyAuthenticationException
{
    private static final long serialVersionUID = -6455982052563237834L;

    /**
     * Instantiates a new response code exception.
     *
     * @param message the message
     */
    public ResponseCodeException(String message)
    {
        super(message);
    }

    /**
     * Instantiates a new response code exception.
     *
     * @param message the message
     * @param cause the cause
     */
    public ResponseCodeException(String message, Throwable cause)
    {
        super(message, cause);
    }

}
