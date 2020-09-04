package com.thomsonreuters.upa.valueadd.reactor;

/**
 * RestEventTypes used with RestEvent.
 */
class RestEventTypes
{
    public static final int COMPLETED = 0;
    public static final int FAILED = 1;
    public static final int CANCELLED = 2;
    public static final int STOPPED = 3;

    public static String toString(int type)
    {
        switch (type)
        {
            case 0:
                return "RestEventTypes.COMPLETED";
            case 1:
                return "RestEventTypes.FAILED";
            case 2:
                return "RestEventTypes.CANCELLED";
            case 3:
            	return "RestEventTypes.STOPPED";
            default:
                return "RestEventTypes " + type + " - undefined.";
        }
    }
}
