package com.refinitiv.eta.transport;

/* Compression or decompression error. */
public class CompressorException extends RuntimeException
{
    private static final long serialVersionUID = 1L;

    CompressorException(String msg, Throwable t)
    {
        super(msg, t);
    }

    CompressorException(String msg)
    {
        super(msg);
    }

    CompressorException()
    {
        super();
    }
}
