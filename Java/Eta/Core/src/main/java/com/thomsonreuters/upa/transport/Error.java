package com.thomsonreuters.upa.transport;

/**
 * Many of the UPA Transport Package methods take a parameter for returning
 * detailed error information. This Error object is only populated in the event
 * of an error condition and should only be inspected when a specific failure
 * code is returned from the method itself.
 * <p>
 * In several cases, positive return values reserved or have special meaning,
 * for example bytes remaining to write to the network. As a result, some
 * negative return codes may be used to indicate success - this is different
 * behavior than the Codec package where any positive value indicates success.
 * Any specific transport related success or failure error handling is described
 * along with the method that requires it.
 */
public interface Error
{
    /**
     * The {@link Channel} the error occurred on.
     * 
     * @return the channel
     */
    public Channel channel();

    /**
     * The {@link Channel} the error occurred on.
     * 
     * @param channel the channel to set
     */
    public void channel(Channel channel);

    /**
     * A UPA specific return code, used to specify the error that has occurred.
     * See the following sections for specific error conditions that may arise.
     * 
     * @return the errorId
     */
    public int errorId();

    /**
     * A UPA specific return code, used to specify the error that has occurred.
     * See the following sections for specific error conditions that may arise.
     * 
     * @param errorId the errorId to set
     */
    public void errorId(int errorId);

    /**
     * Populated with the system errno or error number associated with the
     * failure. This information is only available when the failure occurs as a
     * result of a system method, and will be populated to 0 otherwise.
     * 
     * @return the sysError
     */
    public int sysError();

    /**
     * Populated with the system errno or error number associated with the
     * failure. This information is only available when the failure occurs as a
     * result of a system method, and will be populated to 0 otherwise.
     * 
     * @param sysError the sysError to set
     */
    public void sysError(int sysError);

    /**
     * Detailed text describing the error that has occurred. This may include
     * UPA specific error information, underlying library specific error
     * information, or a combination of both.
     * 
     * @return the text
     */
    public String text();

    /**
     * Detailed text describing the error that has occurred. This may include
     * UPA specific error information, underlying library specific error
     * information, or a combination of both.
     * 
     * @param text the text to set
     */
    public void text(String text);

    /**
     * Clears Error object.
     */
    public void clear();
}