package com.refinitiv.eta.rdm;

/**
 * RDM View Types.
 */
public class ViewTypes
{
    // ViewTypes class cannot be instantiated
    private ViewTypes()
    {
        throw new AssertionError();
    }

    /** View Data contains a list of Field IDs */
    public static final int FIELD_ID_LIST = 1;
    /** View Data contains a list of Element Names */
    public static final int ELEMENT_NAME_LIST = 2;
}
