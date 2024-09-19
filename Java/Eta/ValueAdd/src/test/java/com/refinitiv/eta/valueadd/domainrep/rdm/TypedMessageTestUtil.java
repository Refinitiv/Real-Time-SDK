///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.domainrep.rdm;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;

public class TypedMessageTestUtil
{

    public static <T extends Enum<T>> List<EnumSet<T>> _createFlagCombinations(T[] baseFlags, boolean skipZero)
    {
        int skip = skipZero ? 1 : 0;
        long destFlagsSize = (long)Math.pow(2.0, baseFlags.length) - skip;
        int baseFlagsIter, destFlagsIter;

        List<EnumSet<T>> destFlags = new ArrayList<EnumSet<T>>();
        T e = baseFlags[0];
        for (destFlagsIter = skip; destFlagsIter < destFlagsSize + skip; ++destFlagsIter)
        {
            destFlags.add(destFlagsIter - skip, EnumSet.noneOf(e.getDeclaringClass()));
            for (baseFlagsIter = 0; baseFlagsIter < baseFlags.length; ++baseFlagsIter)
            {
                if (((destFlagsIter >> baseFlagsIter) & 0x1) != 0)
                {
                    destFlags.get(destFlagsIter - skip).add(baseFlags[baseFlagsIter]);
                }
            }
        }

        return destFlags;
    }

    public static int[] _createFlagCombinations(int[] baseFlags, boolean skipZero)
    {
        int skip = skipZero ? 1 : 0;
        int destFlagsSize = (int)Math.pow(2.0, baseFlags.length) - skip;
        int baseFlagsIter, destFlagsIter;

        int[] destFlags = new int[destFlagsSize + skip];
        for (destFlagsIter = skip; destFlagsIter < destFlagsSize + skip; ++destFlagsIter)
        {
            destFlags[destFlagsIter - skip] = 0;
            for (baseFlagsIter = 0; baseFlagsIter < baseFlags.length; ++baseFlagsIter)
            {
                if (((destFlagsIter >> baseFlagsIter) & 0x1) != 0)
                {
                    int flags = destFlags[destFlagsIter - skip];
                    flags |= baseFlags[baseFlagsIter];
                    destFlags[destFlagsIter - skip] = flags;
                }
            }
        }

        return destFlags;
    }
}
