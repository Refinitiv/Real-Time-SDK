package com.rtsdk.eta.codec;

/* Contains RMTES Parse States based on initial character sequences
 * including all ESC possibilities in order to change working and current states.
 */

class RMTESParseState
{
    static final int ERROR = -1;
    static final int NORMAL = 0;
    static final int ESC = 1;
    static final int LBRKT = 3;
    static final int RHPA = 4;
    static final int RREP = 5;
    static final int ESC_21 = 6;
    static final int ESC_22 = 7;
    static final int ESC_24 = 8;
    static final int ESC_24_28 = 9;
    static final int ESC_24_29 = 10;
    static final int ESC_24_2A = 11;
    static final int ESC_24_2B = 12;
    static final int ESC_25 = 13;
    static final int ESC_26 = 14;
    static final int ESC_26_40 = 15;
    static final int ESC_26_40_ESC = 16;
    static final int ESC_26_40_ESC_24 = 17;
    static final int ESC_26_40_ESC_24_29 = 18;
    static final int ESC_26_40_ESC_24_2A = 19;
    static final int ESC_26_40_ESC_24_2B = 20;
    static final int ESC_28 = 21;
    static final int ESC_29 = 22;
    static final int ESC_2A = 23;
    static final int ESC_2B = 24;
}
