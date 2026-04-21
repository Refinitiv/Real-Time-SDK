/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors;
using System;
using System.Collections.Generic;
using System.Linq;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback.ConnectionInfoSelectors
{
    internal static class TestUtil
    {
        public static ReactorConnectInfo[] CreateConnectInfos(int count) =>
            Enumerable.Range(0, count).Select(x => new ReactorConnectInfo()).ToArray();

        public static void CheckThrowOnNullConnectionList(Func<IReadOnlyList<ReactorConnectInfo>, IConnectionInfoSelector> createConnectionInfoSelector) =>
            Assert.Throws<ArgumentNullException>(() => createConnectionInfoSelector(null));

        public static void CheckThrowOnEmptyConnectionList(Func<IReadOnlyList<ReactorConnectInfo>, IConnectionInfoSelector> createConnectionInfoSelector)
        {
            var exception = Assert.Throws<ArgumentException>(() => createConnectionInfoSelector(new ReactorConnectInfo[0]));
            Assert.StartsWith("Connection infos list cannot be empty", exception.Message);
        }

        public static void CheckStayOnSameConnectionWhenListIsSingleItem(
            Func<IReadOnlyList<ReactorConnectInfo>, IConnectionInfoSelector> createConnectionInfoSelector,
            ITestOutputHelper output)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(1);
            var selector = createConnectionInfoSelector(connectInfos);
            // Act/Assert
            for (int i = 1; i <= 10; i++)
            {
                output.WriteLine($"{nameof(selector.SwitchToNext)} call #{i}");
                var newConnInfo = selector.SwitchToNext();
                Assert.Same(connectInfos[0], newConnInfo);
                Assert.Same(connectInfos[0], selector.Current);
                Assert.Same(connectInfos[0], selector.Next);
            }
        }

        public static int[] GetValidConnectionListSizes() =>
            new[] { 1, 2, 3, 10, 11 };

        public static int[] GetValidConnectionListSizesExcludingOne() =>
            new[] { 2, 3, 10, 11 };
    }
}
