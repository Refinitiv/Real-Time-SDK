/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access.Tests;

/// <summary>
/// Tests for the <see cref="EmaObjectManager"/> accessed via <see cref="EmaGlobalObjectPool.Instance"/>.
/// </summary>
public class EmaObjectPoolTests
{
    [Fact]
    public void ObjectPrimitivePoolTest()
    {
        var objectPool = EmaGlobalObjectPool.Instance;
        var arrays = new EmaObjectManager.DataArray[EmaObjectManager.INITIAL_POOL_SIZE * 2];

        // no objects were returned from the pool yet
        Assert.Equal(0, objectPool.primitivePool.current);

        for (int i = 0; i < arrays.Length; i++)
        {
            arrays[i] = objectPool.GetPrimitiveDataArrayFromPool();
        }

        // pool has been drained dry to the limit
        Assert.Equal(objectPool.primitivePool.limit, objectPool.primitivePool.current);

        Array.ForEach(arrays, (array) =>
        {
            if (array.OwnedByPool)
                objectPool.ReturnPrimitiveDataArrayToPool(array);
        });

        Assert.Equal(0, objectPool.primitivePool.current);

        // test has passed when no exceptions were thrown
    }

    [Fact]
    public void ObjectComplexPoolTest()
    {
        var objectPool = EmaGlobalObjectPool.Instance;
        var arrays = new EmaObjectManager.ComplexTypeArray[EmaObjectManager.INITIAL_POOL_SIZE * 2];

        // no objects were returned from the pool yet
        Assert.Equal(0, objectPool.complexTypePool.current);

        for (int i = 0; i < arrays.Length; i++)
        {
            arrays[i] = objectPool.GetComplexTypeArrayFromPool();
        }

        // pool has been drained dry to the limit
        Assert.Equal(objectPool.complexTypePool.limit, objectPool.complexTypePool.current);

        Array.ForEach(arrays, (array) =>
        {
            if (array.OwnedByPool)
                objectPool.ReturnComplexTypeArrayToPool(array);
        });

        Assert.Equal(0, objectPool.complexTypePool.current);

        // test has passed when no exceptions were thrown
    }

    [Fact]
    public void ObjectMsgTypePoolTest()
    {
        var objectPool = EmaGlobalObjectPool.Instance;
        var arrays = new EmaObjectManager.MsgTypeArray[EmaObjectManager.INITIAL_POOL_SIZE * 2];

        // no objects were returned from the pool yet
        Assert.Equal(0, objectPool.msgTypePool.current);

        for (int i = 0; i < arrays.Length; i++)
        {
            arrays[i] = objectPool.GetMsgTypeArrayFromPool();
        }

        // pool has been drained dry to the limit
        Assert.Equal(objectPool.msgTypePool.limit, objectPool.msgTypePool.current);

        Array.ForEach(arrays, (array) =>
        {
            if (array.OwnedByPool)
                objectPool.ReturnMsgTypeArrayToPool(array);
        });

        Assert.Equal(0, objectPool.msgTypePool.current);

        // test has passed when no exceptions were thrown
    }
}