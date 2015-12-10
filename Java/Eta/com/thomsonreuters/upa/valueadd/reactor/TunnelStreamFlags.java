package com.thomsonreuters.upa.valueadd.reactor;

/**
 * The Tunnel Stream Flags.
 */
public class TunnelStreamFlags
{
    /** (0x00000) No flags set. */
    public static final int NONE = 0x0000;

    /** (0x0001) Indicates presence of Authentication. */
    public static final int AUTHENTICATION = 0x0001;
    
    /** (0x0002) Indicates presence of Flow Control. */
    public static final int FLOW_CONTROL = 0x0002;
    
    /** (0x0004) Indicates presence of Reliability. */
    public static final int RELIABILITY = 0x0004;
    
    /** (0x0008) Indicates presence of Guaranteed Messaging. */
    public static final int GUARANTEED_MESSAGING = 0x0008;
}
