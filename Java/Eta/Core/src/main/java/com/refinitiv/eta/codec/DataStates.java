/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Provides information on the health of the data flowing within a stream.
 * 
 * @see State
 */
public class DataStates
{
    // DataStates class cannot be instantiated
    private DataStates()
    {
        throw new AssertionError();
    }

    /**
     * Indicates there is no change in the current state of the data. When
     * available, it is preferable to send more concrete state information (such
     * as OK or SUSPECT) instead of NO_CHANGE. This typically conveys code or
     * text information associated with an item group, but no change to the
     * group's previous data and stream state has occurred.
     */
    public static final int NO_CHANGE = 0;

    /**
     * Data is Ok. All data associated with the stream is healthy and current
     */
    public static final int OK = 1;
    
    /**
     * Data is Suspect. Similar to a stale data state, indicates that the health
     * of some or all data associated with the stream is out of date or cannot
     * be confirmed that it is current.
     */
    public static final int SUSPECT = 2;

    /**
     * Provides string representation for a data state value.
     * 
     * @param dataState {@link DataStates} enumeration to convert to string
     * 
     * @return string representation for a data state value
     */
    public static String toString(int dataState)
    {
        switch (dataState)
        {
            case NO_CHANGE:
                return "NO_CHANGE";
            case OK:
                return "OK";
            case SUSPECT:
                return "SUSPECT";
            default:
                return "Unknown Data State";
        }
    }

    /**
     * Provides string representation of meaning associated with data state.
     * 
     * @param dataState {@link DataStates} enumeration to get info for
     * 
     * @return string representation of meaning associated with data state
     */
    public static String info(int dataState)
    {
        switch (dataState)
        {
            case DataStates.NO_CHANGE:
                return "No Change";
            case DataStates.OK:
                return "Ok";
            case DataStates.SUSPECT:
                return "Suspect";
            default:
                return "Unknown Data State";
        }
    }
}
