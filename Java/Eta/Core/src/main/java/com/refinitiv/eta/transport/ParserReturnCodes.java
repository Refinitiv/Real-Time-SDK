/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

class ParserReturnCodes {
    private ParserReturnCodes()
    {
        throw new AssertionError();
    }

    public static final int INVALID_HEADER_GROUP = -5;
    public static final int INVALID_COOKIES = -4;
    public static final int INVALID_CONNECTION_LINE = -3;
    public static final int HEADERS_NOT_PRESENTED = -2;
    public static final int FAILURE = -1;
    public static final int SUCCESS = 0;
}
