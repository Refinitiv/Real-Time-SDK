package com.thomsonreuters.upa.valueadd.reactor;

/**
 * ReactorOptions to be used in the {@link ReactorFactory#createReactor(ReactorOptions,
 * ReactorErrorInfo)} call.
 */
public class ReactorOptions
{
    Object _userSpecObj = null;
    boolean _xmlTracing = false;

    ReactorOptions()
    {
        // empty constructor
    }

    /**
     * Specifies a user defined object that can be useful for identifying a
     * specific instance of a Reactor or coupling this Reactor with other user
     * created information.
     * 
     * @param userSpecObj the userSpecObj
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the userSpecObj is not
     *         null, otherwise {@link ReactorReturnCodes#PARAMETER_INVALID}.
     */
    public int userSpecObj(Object userSpecObj)
    {
        if (userSpecObj == null)
            return ReactorReturnCodes.PARAMETER_INVALID;

        _userSpecObj = userSpecObj;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the userSpecObj.
     * 
     * @return the userSpecObj.
     */
    public Object userSpecObj()
    {
        return _userSpecObj;
    }
    
    /**
     * Enables XML tracing for the Reactor. The Reactor prints the XML
     * representation of all OMM message when enabled.
     */
    public void enableXmlTracing()
    {
        _xmlTracing = true;
    }

    boolean xmlTracing()
    {
        return _xmlTracing;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _userSpecObj = null;
        _xmlTracing = false;
    }
    
    /*
     * Performs a deep copy from a specified ReactorOptions into this ReactorOptions.
     */
    void copy(ReactorOptions options)
    {
        _userSpecObj = options._userSpecObj;
        _xmlTracing =  options._xmlTracing;
    }
}
