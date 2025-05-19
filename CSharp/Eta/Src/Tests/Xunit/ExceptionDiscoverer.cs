/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using AspectInjector.Broker;
using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Linq;

namespace LSEG.Eta.Tests.Xunit;

/// <summary>
/// Allows capturing exceptions:
/// -handled by CatchExceptionAspect (in classes where the AOP aspect was used on) that occur in certain threads
/// -domain unhandled exceptions
/// -ignores exceptions handled by ThreadWithExceptionHandling
/// </summary>
[SkipInjection]
public class ExceptionDiscoverer : IDisposable
{
    private readonly ConcurrentBag<Exception> _capturedExceptions = new();
    private readonly Predicate<int> _threadIdPredicate;
    private bool _disposed = false;

    public ExceptionDiscoverer(Predicate<int> threadIdPredicate)
    {
        AppDomain.CurrentDomain.UnhandledException += HandleUnhandledException;
        CatchExceptionAspect.ExceptionCaught += HandleException;
        _threadIdPredicate = threadIdPredicate;
    }

    private void HandleException(object sender, CatchExceptionAspect.EventArg e)
    {
        if (_threadIdPredicate(Thread.CurrentThread.ManagedThreadId) && !IsHandledByThreadWithExceptionHandling())
        {
            _capturedExceptions.Add(e.Exception);
            e.Handled = true;
        }
    }

    private static bool IsHandledByThreadWithExceptionHandling()
        => new StackTrace()
            .GetFrames()
            .Select(frame => frame.GetMethod())
            .Any(method => method.DeclaringType == typeof(ThreadWithExceptionHandling));

    private void HandleUnhandledException(object sender, UnhandledExceptionEventArgs e)
    {
        if (e.ExceptionObject is Exception ex)
        {
            _capturedExceptions.Add(ex);
        }
    }

    public void ThrowIfAny()
    {
        if (!_capturedExceptions.IsEmpty)
        {
            throw new AggregateException(_capturedExceptions);
        }
    }

    public void Dispose()
    {
        if (_disposed) return;

        _disposed = true;
        AppDomain.CurrentDomain.UnhandledException -= HandleUnhandledException;
        CatchExceptionAspect.ExceptionCaught -= HandleException;
    }
}
