package com.rtsdk.eta.examples.common;

/**
 * Status flags used by the UPA Java Generic Consumer and Provider applications.
 */
public class GenericResponseStatusFlags
{
    // GenericResponseStatusFlags class cannot be instantiated
    private GenericResponseStatusFlags()
    {
        throw new AssertionError();
    }

    /** (0x00) Generic response successful. */
    public static final int SUCCESS = 0x00;

    /** (0x01) Generic response failure. */
    public static final int FAILURE = 0x01;
}
