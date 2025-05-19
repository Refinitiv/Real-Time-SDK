/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using AspectInjector.Broker;
using LSEG.Eta.Tests.Xunit;
using System;
using System.Reflection;
using Xunit.Sdk;

namespace LSEG.Ema.Access.Tests.Xunit;

[SkipInjection]
public class BeforeAfterAttribute : BeforeAfterTestAttribute
{
    private IDisposable? clearEtaPoolSection;
    private ExceptionDiscoverer? exceptionDiscoverer;
    public override void Before(MethodInfo _)
    {
        clearEtaPoolSection = EtaGlobalPoolTestUtil.CreateClearableSection();
        var testRunnerThreadId = Thread.CurrentThread.ManagedThreadId;
        exceptionDiscoverer = new ExceptionDiscoverer(exceptionThread => exceptionThread != testRunnerThreadId);
    }

    public override void After(MethodInfo _)
    {
        clearEtaPoolSection?.Dispose();
        EtaGlobalPoolTestUtil.CheckPoolSizes();
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
