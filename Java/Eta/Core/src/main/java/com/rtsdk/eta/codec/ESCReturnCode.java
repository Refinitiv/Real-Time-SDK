package com.rtsdk.eta.codec;

/* Used to know the Return Code based on the ESC sequence used in RMTES. */

class ESCReturnCode
{
    static final int ESC_ERROR = -1;
    static final int ESC_SUCCESS = 0;
    static final int UTF_ENC = 1;
    static final int RHPA_CMD = 2;
    static final int RREP_CMD = 3;
    static final int END_CHAR = 4;
}
