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
}
