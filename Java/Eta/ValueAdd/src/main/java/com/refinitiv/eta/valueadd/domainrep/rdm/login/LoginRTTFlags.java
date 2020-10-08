package com.refinitiv.eta.valueadd.domainrep.rdm.login;

/**
 * The RDM login request flags.
 *
 * @see LoginRTT
 */
public class LoginRTTFlags {

    /** (0x00) No flags set */
    public static final int NONE = 0x00;

    /** (0x01) Indicates that this message was sent by the provider */
    public static final int PROVIDER_DRIVEN = 0x01;

    /** (0x02) Indicates presence of the TCP retransmission member */
    public static final int HAS_TCP_RETRANS = 0x02;

    /** (0x04) Has RoundTrip latency */
    public static final int ROUND_TRIP_LATENCY = 0x04;

    private LoginRTTFlags()
    {
        throw new AssertionError();
    }
}
