package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.rdm.ClassesOfService;

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
     * @see ClassesOfService
     */
    public int type()
    {
        return _type;
    }
    
    /**
     * Sets the type of the guarantee class of service.
     * 
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
     */
    public void persistenceFilePath(String persistenceFilePath)
    {
        _persistenceFilePath = persistenceFilePath;
    }

    /**
     * Returns the persistence file path for guarantee class of service.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     */
    public String persistenceFilePath()
    {
        return _persistenceFilePath;
    }

    /**
     * Returns whether guarantee class of service will create local persistence files.
     * Use only when guarantee type is set to PERSISTENT_QUEUE.
     */
    public boolean persistLocally()
    {
        return _persistLocally;
    }

    /**
     * Enable or disable local file persistence. Default: Enabled.
     * Use only when guarantee class of service is set to PERSISTENT_QUEUE.
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
