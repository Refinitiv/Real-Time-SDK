package com.thomsonreuters.upa.codec;

/**
 * The PostUserInfo contains information that identifies the user posting this
 * information. If present on a {@link RefreshMsg}, this implies that this
 * refresh was posted to the system by the user described in postUserInfo.
 * 
 * @see PostMsg
 * @see RefreshMsg
 * @see StatusMsg
 * @see UpdateMsg
 */
public interface PostUserInfo
{
    /**
     * Clears {@link PostUserInfo}.
     */
    public void clear();

    /**
     * IP Address of user that posted this data. Must be in the range of
     * 0 - 4294967296 (2^32).
     * 
     * @param userAddr the userAddr to set
     */
    public void userAddr(long userAddr);

    /**
     * IP Address of user that posted this data.
     * 
     * Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer
     * equivalent.
     * 
     * @param userAddrString The IP address string
     */
    public void userAddr(String userAddrString);

    /**
     * IP Address of user that posted this data.
     * 
     * @return the userAddr
     */
    public long userAddr();

    /**
     * Identifier of the specific user that posted this data. Must be in the
     * range of 0 - 4294967296 (2^32).
     * 
     * @param userId the userId to set
     */
    public void userId(long userId);

    /**
     * Identifier of the specific user that posted this data.
     * 
     * @return the userId
     */
    public long userId();

    /**
     * Converts IP address in integer format to string equivalent. Must be
     * in the range of 0 - 4294967296 (2^32).
     * 
     * @param addrInt The input integer value
     * 
     * @return The IP address string
     */
    public String userAddrToString(long addrInt);
}
