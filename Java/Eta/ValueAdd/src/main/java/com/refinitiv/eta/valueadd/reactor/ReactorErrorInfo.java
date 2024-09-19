/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;

/**
 * ErrorInfo is used by various Reactor methods to return error or warning
 * information to the user. If the {@link ReactorReturnCodes code} is
 * {@link ReactorReturnCodes#SUCCESS SUCCESS}, then there is no error.
 * Otherwise, the user should inspect the code, {@link Error} and location.
 * 
 * @see ReactorReturnCodes Error
 */
public class ReactorErrorInfo
{
    String _location = null;
    int _code = ReactorReturnCodes.SUCCESS;
    Error _error = null;

    ReactorErrorInfo()
    {
        _error = TransportFactory.createError();
    }

    /**
     * The {@link ReactorReturnCodes}. If the code is not
     * {@link ReactorReturnCodes#SUCCESS}, the user should inspect this
     * code, error and location.
     * 
     * @return {@link ReactorReturnCodes}
     */
    public int code()
    {
        return _code;
    }

    ReactorErrorInfo code(int code)
    {
        _code = code;
        return this;
    }

    /**
     * If the {@link #code()} is not {@link ReactorReturnCodes#SUCCESS SUCCESS},
     * the location will identify where the error occurred.
     * 
     * @return A String representing the location where the error occurred
     */
    public String location()
    {
        return _location;
    }

    ReactorErrorInfo location(String location)
    {
        _location = location;
        return this;
    }

    /**
     * If the {@link #code()} is not {@link ReactorReturnCodes#SUCCESS SUCCESS},
     * the {@link com.refinitiv.eta.transport.Error Error} will contain
     * information regarding the error that occurred.
     * 
     * @return An {@link Error} object
     */
    public Error error()
    {
        return _error;
    }
    
    /**
     * Returns a String representation of the ReactorErrorInfo.
     * 
     * @return a String representation of the ReactorErrorInfo
     */
    public String toString()
    {
        return "code=" + _code + ", location=" + _location + ", error.errorId=" + _error.errorId()
                + ", error.text=" + _error.text();
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _location = null;
        _code = ReactorReturnCodes.SUCCESS;
        _error.clear();
    }
}
