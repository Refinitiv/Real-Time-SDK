/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * ReactorRoleTypes use with {@link ReactorFactory#createConsumerRole()},
 * {@link ReactorFactory#createProviderRole()} and
 * {@link ReactorFactory#createNIProviderRole()}.
 */
public class ReactorRoleTypes
{
	/** Indicates the ReactorChannel represents the connection of an OMM Consumer. */
    public static final int CONSUMER = 1;
    /** Indicates the ReactorChannel represents the connection of an OMM Interactive Provider. */
    public static final int PROVIDER = 2;
    /** Indicates the ReactorChannel represents the connection of an OMM Non-Interactive Provider. */
    public static final int NIPROVIDER = 3;

    /**
     * Returns a String representation of the specified ReactorRoleTypes type.
     *
     * @param type the type
     * @return String representation of the specified ReactorRoleTypes type
     */
    public static String toString(int type)
    {
        switch (type)
        {
            case 1:
                return "ReactorRoleTypes.Consumer";
            case 2:
                return "ReactorRoleTypes.Provider";
            case 3:
                return "ReactorRoleTypes.NIProvider";
            default:
                return "ReactorRoleTypes " + type + " - undefined.";
        }
    }
}
