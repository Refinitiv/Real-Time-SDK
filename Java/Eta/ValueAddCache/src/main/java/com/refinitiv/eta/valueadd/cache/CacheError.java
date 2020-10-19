package com.refinitiv.eta.valueadd.cache;

/**
 * Many of the ETA ValueAdd Cache Package methods take a parameter for returning
 * detailed error information. This Error object is only populated in the cases
 * of an error condition and should only be inspected when a specific failure
 * code is returned from the method itself.
 */
public interface CacheError
{
    /**
     * A specific error return code, used to specify the error that has occurred.
     * 
     * @return the errorId
     */
    public int errorId();

    /**
     * A specific error return code, used to specify the error that has occurred.
     * 
     * @param errorId the errorId to set
     */
    public void errorId(int errorId);
    
    /**
     * Detailed text describing the error that has occurred. This may include
     * ValueAdd Cache specific error information, underlying library specific error
     * information, or a combination of both.
     * 
     * @return the text
     */
    public String text();

    /**
     * Detailed text describing the error that has occurred. This may include
     * ValueAdd Cache specific error information, underlying library specific error
     * information, or a combination of both.
     * 
     * @param text the text to set
     */
    public void text(String text);
    
    /**
     * Clear error content set before. 
     * 
     */
    public void clear();
}