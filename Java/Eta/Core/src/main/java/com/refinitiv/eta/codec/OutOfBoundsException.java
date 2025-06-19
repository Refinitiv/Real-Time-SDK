/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.io.IOException;

class OutOfBoundsException extends IOException
{
    private static final long serialVersionUID = 1L;

    OutOfBoundsException(String message)
    {
        super(message);
    }

    OutOfBoundsException(Throwable cause)
    {
        initCause(cause);
    }

}
