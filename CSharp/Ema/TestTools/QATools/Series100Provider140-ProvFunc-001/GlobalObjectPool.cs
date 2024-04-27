/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System;
using System.Linq.Expressions;
using static LSEG.Ema.Example.Traning.IProvider.GlobalObjectPool;

namespace LSEG.Ema.Example.Traning.IProvider;

/// <summary>
/// Global object pool.
/// </summary>
internal static class GlobalObjectPool
{
    public struct PooledObject<T> : IDisposable where T : class, new()
    {
        private static readonly Stack<T> objectPool = new();
        private readonly Action<T> onReturn;
        private const int MaxRetention = 5;
        public T Value { get; }
        public PooledObject(Action<T> onReturn)
        {
            this.onReturn = onReturn;
            lock (objectPool)
            {
                if (objectPool.Count > 0)
                {
                    Value = objectPool.Pop();
                    return;
                }
            }
            Value = new();
        }
        public void Dispose()
        {
            onReturn(Value);
            lock (objectPool)
            {
                if (objectPool.Count < MaxRetention)
                {
                    objectPool.Push(Value);
                    return;
                }
            }
        }
    }

    /// <summary>
    /// A shorthand for creating a pooled object.
    /// </summary>
    /// <typeparam name="T">pooled object</typeparam>
    /// <param name="obj">Retrive an object from the global pool.</param>
    /// <param name="onReturnToPool"></param>
    /// <returns>Disposable for returning the pooled object to the pool.</returns>
    public static PooledObject<T> Get<T>(out T obj, Action<T> onReturnToPool) where T : class, new()
    {
        var pooledObject = new PooledObject<T>(onReturnToPool);
        obj = pooledObject.Value;
        return pooledObject;
    }

}

internal class ClearableUtils<T> where T : class, new()
{
    /// <summary>
    /// A shorthand for creating a pooled object.
    /// </summary>
    /// <typeparam name="T">pooled object</typeparam>
    /// <param name="obj">Retrive an object from the global pool.</param>
    /// <returns>Disposable for returning the pooled object to the pool.</returns>
    public static PooledObject<T> Reuse(out T obj)
        => Get(out obj, ClearInvocation.Value);

    private static Lazy<Action<T>> ClearInvocation => new(() =>
    {
        var method = typeof(T).GetMethod("Clear") ?? throw new InvalidOperationException($"{typeof(T)} is not a reusable type as it does not have Clear() method.");
        var instanceParameter = Expression.Parameter(typeof(T));
        return new(Expression.Lambda<Action<T>>(Expression.Call(instanceParameter, method), instanceParameter).Compile());
    });
}