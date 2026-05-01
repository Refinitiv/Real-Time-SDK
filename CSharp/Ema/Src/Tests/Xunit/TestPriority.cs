/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Xunit.Sdk;
using Xunit.v3;

namespace LSEG.Ema.Access.Tests;

/// <summary>
/// Assigns an execution priority to an xUnit test method.
/// Lower values run first. Tests without this attribute default to priority 0.
/// </summary>
[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public sealed class TestPriorityAttribute : Attribute
{
    public int Priority { get; }
    public TestPriorityAttribute(int priority) => Priority = priority;
}

/// <summary>
/// xUnit v3 test-case orderer that sorts test methods by <see cref="TestPriorityAttribute"/>.
/// Tests with the same (or absent) priority preserve their original relative order.
/// </summary>
public sealed class PriorityOrderer : ITestCaseOrderer
{
    IReadOnlyCollection<TTestCase> ITestCaseOrderer.OrderTestCases<TTestCase>(IReadOnlyCollection<TTestCase> testCases)
    {
        var sorted = new SortedDictionary<int, List<TTestCase>>();

        foreach (var tc in testCases)
        {
            int priority = tc is IXunitTestCase xunitTestCase
                ? xunitTestCase.TestMethod.Method.GetCustomAttribute<TestPriorityAttribute>()?.Priority ?? 0
                : 0;

            if (!sorted.TryGetValue(priority, out var bucket))
                sorted[priority] = bucket = [];
            bucket.Add(tc);
        }

        return sorted.Values.SelectMany(b => b).ToList();
    }
}

