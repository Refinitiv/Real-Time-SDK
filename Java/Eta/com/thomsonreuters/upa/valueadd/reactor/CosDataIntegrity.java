package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.rdm.ClassesOfService;

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
     * @see ClassesOfService
     */
    public int type()
    {
        return _type;
    }
    
    /**
     * Sets the type of the data integrity class of service.
     * 
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
