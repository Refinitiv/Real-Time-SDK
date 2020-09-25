package com.refinitiv.eta.codec;

/**
 * VectoryEntryAction helps to manage change processing rules and informs the
 * consumer of how to apply the entry's data.
 * 
 * @see VectorEntry
 */
public class VectorEntryActions
{
    // VectorEntryActions class cannot be instantiated
    private VectorEntryActions()
    {
        throw new AssertionError();
    }

    /**
     * Indicates that the consumer should update any previously stored or
     * displayed information with the contents of this entry. An update action
     * typically occurs when an entry is already set or inserted and changes to
     * the contents are required. If an update action occurs prior to the set or
     * insert action for the same entry, the update action should be ignored.
     * {@link #UPDATE} can apply to both sortable and non-sortable vectors.
     */
    public static final int UPDATE = 1;

    /**
     * Indicates that the consumer should set the entry at this index position.
     * A set action typically occurs when an entry is initially provided. It is
     * possible for multiple set actions to target the same entry. If this
     * occurs, any previously received data associated with the entry should be
     * replaced with the newly-added information. {@link #SET} can apply to both
     * sortable and non-sortable vectors.
     */
    public static final int SET = 2;

    /**
     * Indicates that the consumer should remove any stored or displayed
     * information associated with this entry's index position. {@link #CLEAR}
     * can apply to both sortable and non-sortable vectors. No entry payload is
     * included when the action is a 'clear.'
     */
    public static final int CLEAR = 3;

    /**
     * Applies only to a sortable vector. The consumer should insert this entry
     * at the index position. Any higher order index positions are incremented
     * by one (e.g., if inserting at index position 5 the existing position 5
     * becomes 6, existing position 6 becomes 7, and so on).
     */
    public static final int INSERT = 4;

    /**
     * Applies only to a sortable vector. The consumer should remove any stored
     * or displayed data associated with this entry's index position. Any higher
     * order index positions are decremented by one (e.g., if deleting at index
     * position 5 the existing position 5 is removed, position 6 becomes 5,
     * position 7 becomes 6, and so on). No entry payload is included when the
     * action is a 'delete.'
     */
    public static final int DELETE = 5;
}
