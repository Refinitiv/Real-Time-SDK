/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Action to perform on a {@link FilterList} for the given filter item. Each
 * entry has an associated action which informs the user of how to apply the entry's contents.
 * 
 * @see FilterEntry
 */
public class FilterEntryActions
{
    // FilterEntryActions class cannot be instantiated
    private FilterEntryActions()
    {
        throw new AssertionError();
    }

    /**
     * Indicates that the consumer should update any previously stored or
     * displayed information with the contents of this entry. An update action
     * typically occurs when an entry is set and changes to the contents need to
     * be conveyed. An update action can occur prior to the set action for the
     * same entry id, in which case, the update action should be ignored.
     */
    public static final int UPDATE = 1;
    /**
     * Indicates that the consumer should set the entry corresponding to this
     * id. A set action typically occurs when an entry is initially provided.
     * Multiple set actions can occur for the same entry id, in which case, any
     * previously received data associated with the entry id should be replaced
     * with the newly-added information.
     */
    public static final int SET = 2;

    /**
     * Indicates that the consumer should remove any stored or displayed
     * information associated with this entry's id. No entry payload is included
     * when the action is a clear.
     */
    public static final int CLEAR = 3;
}
