package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

/**
 * The RDM Login Close. Used by an OMM Consumer or OMM Non-Interactive Provider to close a Login.
 * stream.
 * 
 * @see LoginMsg
 */
public interface LoginClose extends LoginMsg
{
    /**
     * Performs a deep copy of {@link LoginClose} object.
     *
     * @param destCloseMsg Message to copy login close object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(LoginClose destCloseMsg);
}