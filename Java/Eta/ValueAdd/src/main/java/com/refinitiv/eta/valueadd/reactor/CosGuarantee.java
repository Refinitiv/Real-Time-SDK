/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.rdm.ClassesOfService;

/**
 * Guarantee class of service.
 * 
 * @see ClassOfService
 * @see ClassesOfService
*/
public class CosGuarantee
{
    int _type = ClassesOfService.GuaranteeTypes.NONE;
    String _persistenceFilePath = null;
    boolean _persistLocally = true;
    
    /**
     * Returns the type of the guarantee class of service.
     *
     * @return the int
     * @see ClassesOfService
     */
    public int type()
    {
        return _type;
    }
    
    /**
     * Sets the type of the guarantee class of service.
     *
     * @param type the type
     * @see ClassesOfService
     */
    public void type(int type)
    {
        _type = type;
    }


    /**
     * Set the path for the guarantee class of service persistence file.
     * If not specified, the current working directory is used.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     *
     * @param persistenceFilePath the persistence file path
     */
    public void persistenceFilePath(String persistenceFilePath)
    {
        _persistenceFilePath = persistenceFilePath;
    }

    /**
     * Returns the persistence file path for guarantee class of service.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     *
     * @return the string
     */
    public String persistenceFilePath()
    {
        return _persistenceFilePath;
    }

    /**
     * Returns whether guarantee class of service will create local persistence files.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     *
     * @return true, if successful
     */
    public boolean persistLocally()
    {
        return _persistLocally;
    }

    /**
     * Enable or disable local file persistence. Default: Enabled.
     * Use only when guarantee class of service is set to PERSISTENT_QUEUE.
     *
     * @param persistLocally the persist locally
     */
    public void persistLocally(boolean persistLocally)
    {
        _persistLocally = persistLocally;
    }

    /**
     * Clears the CosGuarantee for re-use.
     */
    public void clear()
    {
        _type = ClassesOfService.GuaranteeTypes.NONE;
        _persistenceFilePath = null;
        _persistLocally = true;
    }
}
