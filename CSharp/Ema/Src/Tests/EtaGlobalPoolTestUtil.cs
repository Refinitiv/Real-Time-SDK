/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using AspectInjector.Broker;
using System;
using System.Collections.Generic;
using System.Threading;

namespace LSEG.Ema.Access.Tests;

[SkipInjection]
public static class EtaGlobalPoolTestUtil
{
    private static ThreadLocal<Stack<ClearableSection>> _threadSections =
        new(() => new Stack<ClearableSection>());
    private static int _activeConsumers = 0;
    private static readonly ManualResetEventSlim _allSectionsCleared = new (true);

    public static IDisposable CreateClearableSection()
    {
        var section = new ClearableSection();
        _threadSections!.Value!.Push(section);
        _allSectionsCleared.Reset();
        Interlocked.Increment(ref _activeConsumers);
        return section;
    }

    public static T MarkForClear<T>(this T clearable) where T : Data
    {
        if (_threadSections.Value == null || _threadSections.Value.Count == 0)
        {
            throw new InvalidOperationException("No active ClearableSection found. Use CreateSection() construct first.");
        }

        _threadSections.Value.Peek().AddClearable(clearable);
        return clearable;
    }

    public static void ClearAll()
    {
        var sections = _threadSections.Value;
        while (sections!.Count > 0)
        {
            var section = sections.Pop();
            section.Dispose();
        }
    }

    private class ClearableSection : IDisposable
    {
        private readonly List<Data> _clearables = new();
        private bool _isDisposed = false;

        public void AddClearable(Data clearable)
        {
            if (_isDisposed)
                throw new ObjectDisposedException(nameof(ClearableSection));

            _clearables.Add(clearable);
        }

        public void Dispose()
        {
            if (_isDisposed)
                return;

            if (Interlocked.Decrement(ref _activeConsumers) == 0)
            {
                _allSectionsCleared.Set();
            }

            _isDisposed = true;

            foreach (var clearable in _clearables)
            {
                clearable.Clear_All();
            }

            _clearables.Clear();

            if (_threadSections.Value != null && _threadSections.Value.Count > 0)
            {
                var poppedSection = _threadSections.Value.Pop();

                if (poppedSection != this)
                {
                    throw new InvalidOperationException("Nested sections were disposed in the wrong order.");
                }
            }
        }
    }

    public static void CheckPoolSizes()
    {
        _allSectionsCleared.Wait();
        var pool = EtaObjectGlobalPool.Instance;
        Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaBufferPool.Count);
        Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaEncodeIteratorPool.Count);
        foreach (var keyVal in pool.m_etaByteBufferBySizePool)
        {
            Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, keyVal.Value.Count);
        }
    }
}