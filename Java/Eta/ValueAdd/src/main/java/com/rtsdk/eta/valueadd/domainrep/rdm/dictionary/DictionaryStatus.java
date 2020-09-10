package com.rtsdk.eta.valueadd.domainrep.rdm.dictionary;

import com.rtsdk.eta.codec.State;

/**
 * The RDM Dictionary Status. Used by an OMM Provider to indicate changes to the Dictionary stream.
 * 
 * @see DictionaryMsg
 */
public interface DictionaryStatus extends DictionaryMsg
{
    
    /**
     * The RDM Dictionary status flags. Populated by {@link DictionaryStatusFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * The RDM Dictionary status flags. Populated by {@link DictionaryStatusFlags}.
     * 
     * @return flags - RDM Dictionary status flags
     */
    public int flags();

    /** 
     * Checks the presence of state field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if state field is present, false - if not.
     */
    public boolean checkHasState();
    
    /** 
     * Applies state presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasState();
    
    /**
     * state - The current state of the stream.
     * 
     * @return  state
     */
    public State state();
    
    /**
     * Sets the state field for the dictionary status message.
     *
     * @param state the state
     */
    public void state(State state);

    /**
     * Performs a deep copy of {@link DictionaryStatus} object.
     *
     * @param destStatusMsg Message to copy dictionary status object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(DictionaryStatus destStatusMsg);
    
    /**
     * Checks the presence of clear cache flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkClearCache();

    /**
     * Applies clear cache flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyClearCache();
}