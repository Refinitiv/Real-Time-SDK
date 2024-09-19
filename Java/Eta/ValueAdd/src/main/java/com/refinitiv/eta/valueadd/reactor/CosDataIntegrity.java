/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.rdm.ClassesOfService;

/**
 * Data integrity class of service.
 * 
 * @see ClassOfService
 * @see ClassesOfService
 */
public class CosDataIntegrity
{
    int _type = ClassesOfService.DataIntegrityTypes.BEST_EFFORT;
    
    /**
     * Returns the type of the data integrity class of service.
     *
     * @return the int
     * @see ClassesOfService
     */
    public int type()
    {
        return _type;
    }
    
    /**
     * Sets the type of the data integrity class of service.
     *
     * @param type the type
     * @see ClassesOfService
     */
    public void type(int type)
    {
        _type = type;
    }

    /**
     * Clears the CosDataIntegrity for re-use.
     */
    public void clear()
    {
        _type = ClassesOfService.DataIntegrityTypes.BEST_EFFORT;
    }
}
