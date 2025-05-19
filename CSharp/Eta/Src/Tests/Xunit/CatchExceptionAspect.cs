/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using AspectInjector.Broker;
using System;
using System.Diagnostics;
using System.Runtime.ExceptionServices;

namespace LSEG.Eta.Tests.Xunit;

/// <summary>
/// Injects try-catch code to the attributed entities allowing caught exceptions to be passed to subscribers in the underlying thread.
/// </summary>
[Aspect(Scope.Global)]
[Injection(typeof(CatchExceptionAspect))]
[SkipInjection]
[AttributeUsage(AttributeTargets.Class | AttributeTargets.Method | AttributeTargets.Assembly)]
public class CatchExceptionAspect : Attribute
{
    public class EventArg
    {
        public Exception Exception { get; init; }
        public bool Handled { get; set; }
    }

    public static event EventHandler<EventArg> ExceptionCaught;

    [Advice(Kind.Around, Targets = Target.Method)]
    [DebuggerHidden]
    public object InterceptMethods(
        [Argument(Source.Target)] Func<object[], object> method,
        [Argument(Source.Instance)] object instance,
        [Argument(Source.Arguments)] object[] args,
        [Argument(Source.ReturnType)] Type returnType)
    {
        try
        {
            return method(args);
        }
        catch (Exception exception)
        {
            var arg = new EventArg { Exception = exception };
            ExceptionCaught?.Invoke(instance, arg);
            if (!arg.Handled)
            {
                ExceptionDispatchInfo.Capture(exception).Throw();
            }
            return GetDefaultValue(returnType);
        }
    }

    private static object GetDefaultValue(Type type)
        => type.IsValueType && type != typeof(void) ? Activator.CreateInstance(type) : null;
}