package com.thomsonreuters.upa.valueadd.examples.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;

/**
 * Market price item data.
 */
public class MarketPriceItem
{
    public static final int RDNDISPLAY_FID = 2;
    public static final int RDN_EXCHID_FID = 4;
    public static final int DIVPAYDATE_FID = 38;
    public static final int TRDPRC_1_FID = 6;
    public static final int BID_FID = 22;
    public static final int ASK_FID = 25;
    public static final int ACVOL_1_FID = 32;
    public static final int NETCHNG_1_FID = 11;
    public static final int ASK_TIME_FID = 267;
    public static final int PERATIO_FID = 36;
    public static final int SALTIME_FID = 379;

    public String itemName;
    public boolean isInUse;
    public int RDNDISPLAY;
    public int RDN_EXCHID;
    public Date DIVPAYDATE;
    public double TRDPRC_1;
    public double BID;
    public double ASK;
    public double ACVOL_1;
    public double NETCHNG_1;
    public DateTime ASK_TIME;
    public double PERATIO;

    public DateTime SALTIME;
    private StringBuilder stringBuf = new StringBuilder();
    private final static String eolChar = "\n";
    private final static String tabChar = "\t";

    
    public MarketPriceItem()
    {
        DIVPAYDATE = CodecFactory.createDate();
        ASK_TIME = CodecFactory.createDateTime();
        SALTIME = CodecFactory.createDateTime();
    }

    /**
     * clears market price item fields.
     */
    public void clear()
    {
        isInUse = false;
        RDNDISPLAY = 0;
        RDN_EXCHID = 0;
        DIVPAYDATE.clear();
        TRDPRC_1 = 0;
        BID = 0;
        ASK = 0;
        ACVOL_1 = 0;
        NETCHNG_1 = 0;
        ASK_TIME.localTime();
        PERATIO = 0;
        setSALTIME();    
        }
    
    
    /**
     * Initializes market price item fields.
     */
    public void initFields()
    {
        isInUse = true;
        RDNDISPLAY = 100;
        RDN_EXCHID = 155;
        DIVPAYDATE.value("10/22/2010");
        TRDPRC_1 = 1.00;
        BID = 0.99;
        ASK = 1.03;
        ACVOL_1 = 100000;
        NETCHNG_1 = 2.15;
        ASK_TIME.localTime();
        setSALTIME();        
        PERATIO = 5.00;
    }

    /**
     * Updates item that's currently in use.
     */
    public void updateFields()
    {
        TRDPRC_1 += 0.01;
        BID += 0.01;
        ASK += 0.01;
    
        ASK_TIME.localTime();
        PERATIO += 0.01;
        setSALTIME();
    }
    
    /*
     * This methods set the SAL time to be one second less than the ASK Time
     */
    private void setSALTIME()
    {
    	ASK_TIME.copy(SALTIME);
    	int tempSec = SALTIME.second();
    	if (0==tempSec)
    		SALTIME.second(59);
    	else
    		SALTIME.second(tempSec -1 );
    }
    
    
    public String toString()
    {
        stringBuf.setLength(0);
        stringBuf.append(tabChar);
        stringBuf.append("RDMDISPLAY: ");
        stringBuf.append(RDNDISPLAY);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("RDN_EXCHID: ");
        stringBuf.append(RDN_EXCHID);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("DIVPAYDATE: ");
        stringBuf.append(DIVPAYDATE);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("TRDPRC_1: ");
        stringBuf.append(TRDPRC_1);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("BID: ");
        stringBuf.append(BID);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("ASK: ");
        stringBuf.append(ASK);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("ACVOL_1: ");
        stringBuf.append(ACVOL_1);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("NETCHNG_1: ");
        stringBuf.append(NETCHNG_1);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("ASK_TIME: ");
        stringBuf.append(ASK_TIME);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("PERATIO: ");
        stringBuf.append(PERATIO);
        stringBuf.append(eolChar);
        stringBuf.append(tabChar);
        stringBuf.append("SALTIME: ");
        stringBuf.append(SALTIME);
        stringBuf.append(eolChar);
        return stringBuf.toString();
    }
}
