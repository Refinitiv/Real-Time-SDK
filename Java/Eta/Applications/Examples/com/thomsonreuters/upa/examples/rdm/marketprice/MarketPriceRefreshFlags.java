package com.thomsonreuters.upa.examples.rdm.marketprice;

/**
 * Market price refresh flags. 
 */
public class MarketPriceRefreshFlags
{
    /** (0x00) No flags set. */
    public static final int NONE = 0x00;
    
    /** (0x01) Indicates presence of the service id member. */
    public static final int HAS_SERVICE_ID = 0x01;
    
    /** (0x02) Indicates presence of the qos member. */
    public static final int HAS_QOS = 0x02;
  
    /** (0x04) Indicates presence of the solicited flag. */
    public static final int SOLICITED = 0x04;
    
    /** (0x08) Indicates presence of the refresh complete flag. */
    public static final int REFRESH_COMPLETE = 0x08;
    
    /** (0x10) Indicates presence of the private stream flag. */
    public static final int PRIVATE_STREAM = 0x10;

    /** (0x20) Indicates presence of the clear cache flag. */
    public static final int CLEAR_CACHE = 0x20;
    
    private MarketPriceRefreshFlags()
    {
        throw new AssertionError();
    }
}
