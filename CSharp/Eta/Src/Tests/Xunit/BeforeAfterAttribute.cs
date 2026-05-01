/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using AspectInjector.Broker;
using System.Diagnostics;
using System.Reflection;
using Xunit.v3;

namespace LSEG.Eta.Tests.Xunit;

[SkipInjection]
public class BeforeAfterAttribute : BeforeAfterTestAttribute
{
    private ExceptionDiscoverer exceptionDiscoverer;

    [DebuggerHidden]
    public override void Before(MethodInfo methodUnderTest, IXunitTest test)
    {
        var testRunnerThreadId = Thread.CurrentThread.ManagedThreadId;
        exceptionDiscoverer = new ExceptionDiscoverer(exceptionThread => exceptionThread != testRunnerThreadId);
    }

    public override void After(MethodInfo methodUnderTest, IXunitTest test)
    {
        try
        {
            exceptionDiscoverer?.ThrowIfAny();
        }
        finally
        {
            exceptionDiscoverer?.Dispose();
        }
    }
}
