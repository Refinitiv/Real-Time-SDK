package com.thomsonreuters.upa.valueadd.examples.common;

/**
 * Symbol list request flags. 
 */
public class SymbolListRequestFlags
{
    
    /** (0x00) No flags set. */
    public static final int NONE = 0x0000;
    
    /** (0x0001) Indicates presence of the qos member. */
    public static final int HAS_QOS = 0x0001;
    
    /** (0x0002) Indicates presence of the qos member. */
    public static final int HAS_PRIORITY = 0x0002;
    
    /** (0x0004) Indicates presence of the service id member. */
    public static final int HAS_SERVICE_ID = 0x0004;
    
    /** (0x0008) Indicates presence of the streaming flag. */
    public static final int STREAMING = 0x0008;
    
    private SymbolListRequestFlags()
    {
        throw new AssertionError();
    }
    
    
}
