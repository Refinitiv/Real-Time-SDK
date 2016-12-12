package com.thomsonreuters.upa.valueadd.reactor;

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
     * @param type
     * 
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
