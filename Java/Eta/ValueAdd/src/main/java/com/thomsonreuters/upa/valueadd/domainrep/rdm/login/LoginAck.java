package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;




/**
 * The RDM Login Ack.  Used for an off-stream Post Acknowledgment
 * 
 * @see LoginMsg
 * 
 * @deprecated use {@link com.thomsonreuters.upa.codec.AckMsg} instead
 */
@Deprecated
public interface LoginAck extends LoginMsg
{
    /**
     * Performs a deep copy of {@link LoginAck} object.
     *
     * @param destAckMsg Message to copy login ack object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(LoginAck destAckMsg);
}