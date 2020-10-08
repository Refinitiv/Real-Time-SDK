package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.rdm.ClassesOfService;

/**
 * Authentication class of service.
 * 
 * @see ClassOfService
 * @see ClassesOfService
 */
public class CosAuthentication
{
    int _type = ClassesOfService.AuthenticationTypes.NOT_REQUIRED;
    
    /**
     * Returns the type of the authentication class of service.
     *
     * @return the int
     * @see ClassesOfService
     */
    public int type()
    {
        return _type;
    }
    
    /**
     * Sets the type of the authentication class of service.
     *
     * @param type the type
     * @see ClassesOfService
     */
    public void type(int type)
    {
        _type = type;
    }

    /**
     * Clears the CosAuthentication for re-use.
     */
    public void clear()
    {
        _type = ClassesOfService.AuthenticationTypes.NOT_REQUIRED;
    }
}
