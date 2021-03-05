///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.eta.codec.Hashers;

public class HashersJunit
{	
    private static final String testStrings[] =
    {
        "![ES3W3010B1",
        "CN102001101=CFXW",
        "CNHI.DY",
        "DECL4LFD.F",
        "DECP9GWL.F",
        "DEDFU0MG.F",
        "DEGA1NEQ.EW",
        "DEGF4V9S.F",
        "DEHX31RG.EW",
        "DEHZ9TQV.EW",
        "DEKE1CR3.F",
        "DEUD3MKR.EW",
        "DEVE9ZUB.EW",
        "DEVP3QE0.F",
        "DEVP79SG.F",
        "FSL6F3C74102A050F=FINR",
        "KLACA292119500.MP",
        "MSA403C4C1J1E100F=FINR",
        "PTGOV2YF12Y=R",
        "TRNLBFVDc2",
        "TWBP2YX10Y=ICSG",
        "US45950KCR95=BNTW",
        "US983919AJ06=VS",
        "WIR_u.LNX",
        "WSPL182011500.M",
        
        "zCcv1",
        "!FW1G999.DEp",
        "![1#KRKTBBMK=KFIA",
        "![BBB+CNG4Y=KFIA",
        "![S1RZ0-F1",
        "#073F8A007",
        "0#REEe*.i+C",
        "134#CHXCOM.CXC",
        "13667ZEY1=RRPS",
        "nTNF1WfW",
        "!!ECBREFDATE",
        "!!ECREFDATE",
        "!.!L!LMLS54!AAAA",
        "!.D2FMLS54/DHM",
        "!.MAUSEQDELAY",
        "!.RR65",
        "!0E1A999.DEp",
        "!0MK4999.DEp",
        "!1BOJ01AE999.DEp",
        "!1CEM01AE999.DEp"
    };
    private static final int precalculatedHashes[] =
    {
    	451,
    	631,
    	631,
    	676,
    	406,
    	451,
    	496,
    	631,
    	541,
    	316,
    	271,
    	496,
    	406,
    	676,
    	316,
    	586,
    	271,
    	676,
    	631,
    	361,
    	586,
    	631,
    	226,
    	496,
    	586,
    	
    	75,
    	115,
    	300,
    	414,
    	621,
    	266,
    	717,
    	106,
    	287,
    	703,
    	695,
    	31,
    	375,
    	21,
    	503,
    	531,
    	353,
    	491,
    	174,
    	305
    };
    
    private static final long precalculatedPolyHashValues[] =
    {
    		Long.decode("0x00000000a0000016").longValue(),
    		Long.decode("0x00000000e000009e").longValue(),
    		Long.decode("0x00000000e0000169").longValue(),
    		Long.decode("0x00000000f00001ac").longValue(),
    		Long.decode("0x00000000900000a1").longValue(),
    		Long.decode("0x00000000a000011f").longValue(),
    		Long.decode("0x00000000b0000115").longValue(),
    		Long.decode("0x00000000e0000092").longValue(),
    		Long.decode("0x00000000c0000110").longValue(),
    		Long.decode("0x0000000070000050").longValue(),
    		Long.decode("0x0000000060000074").longValue(),
    		Long.decode("0x00000000b0000074").longValue(),
    		Long.decode("0x00000000900000ad").longValue(),
    		Long.decode("0x00000000f00000bb").longValue(),
    		Long.decode("0x0000000070000050").longValue(),
    		Long.decode("0x00000000d00000d0").longValue(),
    		Long.decode("0x0000000060000070").longValue(),
    		Long.decode("0x00000000f000012c").longValue(),
    		Long.decode("0x00000000e00000b7").longValue(),
    		Long.decode("0x0000000080000009").longValue(),
    		Long.decode("0x00000000d0000091").longValue(),
    		Long.decode("0x00000000e0000130").longValue(),
    		Long.decode("0x000000005000004f").longValue(),
    		Long.decode("0x00000000b00000f7").longValue(),
    		Long.decode("0x00000000d0000122").longValue()
    };
    
    private static final int NUMBER_OF_BUCKETS = 720;

    @Test
    public void hashingEntityIdValuesTest()
    {
        for (int i = 0; i < testStrings.length; i++)
        {
            ByteBuffer buf = ByteBuffer.wrap(testStrings[i].getBytes());
            int hash = Hashers.hashingEntityId(buf, buf.position(), buf.limit(), NUMBER_OF_BUCKETS);
            assertEquals(precalculatedHashes[i], hash);
        }
    }
    
    @Test
    public void polyHashValuesTest()
    {
    	/* Test only the first 25 RICS from testStrings */
        for (int i = 0; i < 25; i++)
        {
            ByteBuffer buf = ByteBuffer.wrap(testStrings[i].getBytes());
            long hash = Hashers.polyHash(buf, buf.position(), buf.limit());
            assertEquals(precalculatedPolyHashValues[i], hash);
        }
    }
}
