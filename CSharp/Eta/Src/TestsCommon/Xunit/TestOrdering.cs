/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using Xunit.Sdk;
using Xunit.v3;

namespace LSEG.Eta.Tests.Common.Xunit;

/// <summary>
/// Allows to execute tests in particular order according to <see cref="Priority"/> property value.
/// Could be applied either to test method, test class or both. If both test class &amp; test methods have this
/// attribute, Priority value is taken from method.
/// </summary>
[Obsolete("Don't commit usage of this attribute to VCS. It's intended for debugging only.")]
[AttributeUsage(AttributeTargets.Method | AttributeTargets.Class, AllowMultiple = false)]
public class TestPriorityAttribute : Attribute, ITraitAttribute
{
    public uint Priority { get; private set; }

    public TestPriorityAttribute(uint priority) => Priority = priority;

    public IReadOnlyCollection<KeyValuePair<string, string>> GetTraits() =>
        [ new(TraitName, Priority.ToString()) ];

    public const string TraitName = "Priority";
}

/// <summary>
/// In order to make <see cref="TestPriorityAttribute"/> work you should "apply"
/// this entity to particular test class via <see cref="TestCaseOrdererAttribute"/> as following:
/// <code>
/// [TestCaseOrderer(
///     ordererTypeName: "LSEG.Eta.Tests.Xunit.PriorityOrderer",
///     ordererAssemblyName: "ESDK.Tests")]
/// public class MyTests
/// { ...
/// </code>
/// Or to whole test assembly:
/// <code>
/// [assembly: TestCaseOrderer(
///     ordererTypeName: "LSEG.Eta.Tests.Xunit.PriorityOrderer",
///     ordererAssemblyName: "ESDK.Tests")]
/// </code>
/// </summary>
[Obsolete("Don't commit usage of this attribute to VCS. It's intended for debugging only.")]
public class PriorityOrderer : ITestCaseOrderer
{
    public IReadOnlyCollection<TTestCase> OrderTestCases<TTestCase>(IReadOnlyCollection<TTestCase> testCases)
        where TTestCase : notnull, ITestCase
    {
        string assemblyQualifiedName = typeof(TestPriorityAttribute).AssemblyQualifiedName!;
        var sortedMethods = new SortedDictionary<uint, List<TTestCase>>();
        foreach (TTestCase testCase in testCases)
        {
            var priority =
                GetPriorityOrDefault(testCase.TestMethod?.Traits)
                ?? GetPriorityOrDefault(testCase.TestClass?.Traits)
                ?? uint.MaxValue;

            GetOrCreate(sortedMethods, priority).Add(testCase);
        }

        return sortedMethods.Keys
            .SelectMany(
                priority => sortedMethods[priority]
                    .OrderBy(testCase => testCase.TestMethod?.MethodName ?? "")
                    .ThenBy(testCase => testCase.TestCaseDisplayName))
            .ToList();
    }

    private static uint? GetPriorityOrDefault(IReadOnlyDictionary<string, IReadOnlyCollection<string>>? traits)
    {
        if (traits == null)
            return null;
        if (!traits.TryGetValue(TestPriorityAttribute.TraitName, out var traitValues))
            return null;
        if (traitValues.Count == 0)
            return null;
        return uint.Parse(traitValues.First());
    }

    private static TValue GetOrCreate<TKey, TValue>(
        IDictionary<TKey, TValue> dictionary, TKey key)
        where TKey : struct
        where TValue : new() =>
        dictionary.TryGetValue(key, out TValue? result)
            ? result!
            : (dictionary[key] = new TValue());
}
