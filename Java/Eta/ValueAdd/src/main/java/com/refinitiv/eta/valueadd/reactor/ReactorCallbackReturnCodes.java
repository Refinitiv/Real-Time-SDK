/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Information about the state of the ReactorChannel connection as well as any
 * messages for that channel are returned to the application via a series of
 * callback functions. There are several values that can be returned from a
 * callback function implementation. These can trigger specific Reactor behavior
 * based on the outcome of the callback function. This class defines the
 * callback return codes that applications can use.
 */
public class ReactorCallbackReturnCodes
{
    /**
     * Indicates that the callback function was successful and the message or
     * event has been handled.
     */
    public static final int SUCCESS = 0;

    /**
     * Indicates that the message or event has failed to be handled. Returning
     * this code from any callback function will cause the Reactor to shutdown.
     */
    public static final int FAILURE = -1;

    /**
     * Can be returned from any domain-specific callback (e.g.,
     * RDMLoginMsgCallback). This will cause the Reactor to invoke the
     * DefaultMsgCallback for this message upon the domain-specific callbacks
     * return.
     */
    public static final int RAISE = -2;

    /**
     * Returns a String representation of the specified
     * ReactorCallbackReturnCodes type.
     *
     * @param type the type
     * @return String representation of the specified
     *         ReactorCallbackReturnCodes type
     */
    public static String toString(int type)
    {
        switch (type)
        {
            case 0:
                return "ReactorCallbackReturnCodes.SUCCESS";
            case -1:
                return "ReactorCallbackReturnCodes.FAILURE";
            case -2:
                return "ReactorCallbackReturnCodes.RAISE";
            default:
                return "ReactorCallbackReturnCodes " + type + " - undefined.";
        }
    }
}
