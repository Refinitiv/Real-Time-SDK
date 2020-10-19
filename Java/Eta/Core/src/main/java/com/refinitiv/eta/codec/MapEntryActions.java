package com.refinitiv.eta.codec;

/**
 * Action to perform on a {@link Map} for the given map entry. The entry action
 * helps to manage change processing rules and tells the consumer how to apply
 * the information contained in the entry.
 */
public class MapEntryActions
{
    // MapEntryActions class cannot be instantiated
    private MapEntryActions()
    {
        throw new AssertionError();
    }

    /**
     * Indicates that the consumer should update any previously stored or
     * displayed information with the contents of this entry. An update action
     * typically occurs when an entry has already been added and changes to the
     * contents need to be conveyed. If an update action occurs prior to the add
     * action for the same entry, the update action should be ignored.
     */
    public static final int UPDATE = 1;

    /**
     * Indicates that the consumer should add the entry. An add action typically
     * occurs when an entry is initially provided. It is possible for multiple
     * add actions to occur for the same entry. If this occurs, any previously
     * received data associated with the entry should be replaced with the newly
     * added information.
     */
    public static final int ADD = 2;

    /**
     * Indicates that the consumer should remove any stored or displayed
     * information associated with the entry. No map entry payload is included
     * when the action is delete.
     */
    public static final int DELETE = 3;
    
    /**
     * Provide string representation for a map entry action.
     * 
     * @param mapEntryActions {@link MapEntryActions} enumeration to convert to string
     * 
     * @return string representation for a map entry action.
     */
    public static String toString(int mapEntryActions)
    {
        switch (mapEntryActions)
        {
            case UPDATE:
                return "UPDATE";
            case ADD:
                return "ADD";
            case DELETE:
                return "DELETE";
            default:
                return Integer.toString(mapEntryActions);
        }
    }
}
