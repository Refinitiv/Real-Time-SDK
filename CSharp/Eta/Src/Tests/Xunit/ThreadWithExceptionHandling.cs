/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using AspectInjector.Broker;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;

namespace LSEG.Eta.Tests.Xunit;

[SkipInjection]
public class ThreadWithExceptionHandling
{
    private readonly System.Threading.Thread _thread;
    private Exception _exception; 

    public string Name { get { return _thread.Name; } set { _thread.Name = value; } }
    public bool IsAlive => _thread.IsAlive;

    public static System.Threading.Thread CurrentThread => System.Threading.Thread.CurrentThread;

    public ThreadWithExceptionHandling(ThreadStart start)
    {
        _thread = new System.Threading.Thread(new ThreadStart([DebuggerHidden] () => Invoke(start)));
    }

    public static void Sleep(TimeSpan timeSpan)
    {
        System.Threading.Thread.Sleep(timeSpan);
    }

    public static void Sleep(int millisecondsTimeout)
    {
        System.Threading.Thread.Sleep(millisecondsTimeout);
    }

    [DebuggerHidden]
    private void Invoke(ThreadStart start)
    {
        try
        {
            start.Invoke();
        }
        catch(Exception ex)
        {
            _exception = ex;
        }
    }

    public void Join()
    {
        _thread.Join();
        if(_exception is not null && !IsExceptionThreadSpecific(_exception))
        {
            throw new AggregateException(_exception);
        }
    }

    public Exception JoinAndReturnException()
    {
        _thread.Join();
        return _exception;
    }

    public static void JoinAll(IReadOnlyCollection<ThreadWithExceptionHandling> threads)
    {
        foreach (var thread in threads)
        {
            thread.Join();
        }
        var exceptions = threads
            .Select(t => t._exception)
            .Where(e => e is not null)
            .Where(e => !IsExceptionThreadSpecific(e))
            .ToArray();
        if (exceptions.Any())
        {
            throw new AggregateException(exceptions);
        }
    }

    public void Start()
    {
        _thread.Start();
    }

    private static bool IsExceptionThreadSpecific(Exception ex) 
        => ex is ThreadInterruptedException || ex is ThreadAbortException;

    public System.Threading.Thread UnderlyingThread => _thread;
}