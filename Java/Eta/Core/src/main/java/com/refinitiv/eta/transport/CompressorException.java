/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
