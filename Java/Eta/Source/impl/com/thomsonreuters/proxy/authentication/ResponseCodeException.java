package com.thomsonreuters.proxy.authentication;

public class ResponseCodeException extends ProxyAuthenticationException
{
    private static final long serialVersionUID = -6455982052563237834L;

    public ResponseCodeException(String message)
    {
        super(message);
    }

    public ResponseCodeException(String message, Throwable cause)
    {
        super(message, cause);
    }

}
