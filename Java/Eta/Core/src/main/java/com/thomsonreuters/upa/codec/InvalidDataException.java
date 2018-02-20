package com.thomsonreuters.upa.codec;

class InvalidDataException extends RuntimeException
{
    private static final long serialVersionUID = 1L;

    InvalidDataException(String msg)
    {
        super(msg);
    }
}
