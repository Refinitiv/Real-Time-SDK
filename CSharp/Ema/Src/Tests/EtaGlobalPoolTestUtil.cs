/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

namespace LSEG.Ema.Access.Tests;

internal static class EtaGlobalPoolTestUtil
{
    public static readonly List<Data> MarkedForClear = new ();

    public static T MarkForClear<T>(this T data) where T : Data
    {
        MarkedForClear.Add(data);
        return data;
    }

    public static void Clear()
    {
        MarkedForClear.ForEach(d => d.Clear_All());
        MarkedForClear.Clear();
    }

    public static void CheckEtaGlobalPoolSizes()
    {
        var pool = EtaObjectGlobalPool.Instance;
        Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaBufferPool.Count);
        Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaEncodeIteratorPool.Count);
        foreach (var keyVal in pool.m_etaByteBufferBySizePool)
        {
            Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, keyVal.Value.Count);
        }
    }
}