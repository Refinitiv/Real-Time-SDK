/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Library Version Information to be populated with {@link Transport} library version info.
 * 
 * @see Transport
 */
public interface LibraryVersionInfo
{
    /**
     * Product Release and Load information.
     * 
     * @return the productVersion
     */
    public String productVersion();

    /**
     * Internal Node information, useful for raising questions or reporting issues.
     * 
     * @return the internalVersion
     */
    public String productInternalVersion();

    /**
     * Date library was produced for product release.
     * 
     * @return the productDate
     */
    public String productDate();
}