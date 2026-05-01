/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors;
using System.Collections.Generic;
using System.Linq;

using static LSEG.Eta.Tests.ValueAddTest.Fallback.ConnectionInfoSelectors.TestUtil;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback.ConnectionInfoSelectors
{
    public class RoundRobinConnectionInfoSelectorTests
    {
        private readonly ITestOutputHelper m_Output;

        public RoundRobinConnectionInfoSelectorTests(ITestOutputHelper output)
        {
            m_Output = output;
        }

        public static IEnumerable<object[]> GetValidConnectInfosCount() =>
            GetValidConnectionListSizes().ToXunitMemberData();

        public static IEnumerable<object[]> GetValidConnectInfosCountExcludingOne() =>
            GetValidConnectionListSizesExcludingOne().ToXunitMemberData();

        [Fact]
        public void Ctor_should_throw_on_empty_connection_list() =>
            CheckThrowOnEmptyConnectionList(x => new RoundRobinConnectionInfoSelector(x));

        [Fact]
        public void Ctor_should_throw_on_null_connection_list() =>
            CheckThrowOnNullConnectionList(x => new RoundRobinConnectionInfoSelector(x));

        [Theory]
        [MemberData(nameof(GetValidConnectInfosCount))]
        public void Ctor_should_set_initial_connection_to_first(int connectInfosCount)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            // Act
            var selector = new RoundRobinConnectionInfoSelector(connectInfos);
            // Assert
            Assert.Same(connectInfos[0], selector.Current);
        }

        [Fact]
        public void SwitchToNext_should_stay_on_same_connection_when_list_is_single_item() =>
            CheckStayOnSameConnectionWhenListIsSingleItem(x => new RoundRobinConnectionInfoSelector(x), m_Output);

        [Theory]
        [MemberData(nameof(GetValidConnectInfosCountExcludingOne))]
        public void SwitchToNext_should_properly_switch_to_next_connection(int connectInfosCount)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var connectInfosMap = ObjectIndexMap.Of(connectInfos);
            var selector = new RoundRobinConnectionInfoSelector(connectInfos);

            // Excluding first index because we already proved that sequence starts from it. 
            var expectedConnectionIndexSequence = Enumerable.Range(1, connectInfosCount - 1).ToList();

            // Act
            var indices = expectedConnectionIndexSequence
                .Select(x =>
                {
                    var nextConnInfoBeforeSwitch = selector.Next;

                    var newConnInfo = selector.SwitchToNext();

                    return new
                    {
                        SwitchToNextIndex = connectInfosMap[newConnInfo],
                        CurrentIndex = connectInfosMap[selector.Current],
                        NextBeforeSwitchIndex = connectInfosMap[nextConnInfoBeforeSwitch],
                    };
                })
                .ToList();

            // Assert
            Assert.Equal(expectedConnectionIndexSequence, indices.Select(x => x.SwitchToNextIndex).ToArray());
            Assert.Equal(expectedConnectionIndexSequence, indices.Select(x => x.CurrentIndex).ToArray());
            Assert.Equal(expectedConnectionIndexSequence, indices.Select(x => x.NextBeforeSwitchIndex).ToArray());
        }

        [Theory]
        [MemberData(nameof(GetValidConnectInfosCountExcludingOne))]
        public void SwitchToNext_should_switch_to_first_connection_when_list_end_is_reached(int connectInfosCount)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var connectInfosIndexMap = ObjectIndexMap.Of(connectInfos);
            var selector = new RoundRobinConnectionInfoSelector(connectInfos);
            // Act
            var newConnInfo = selector.Current;
            for (int i = 0; i < connectInfosCount; i++)
                newConnInfo = selector.SwitchToNext();
            // Assert
            Assert.Equal(0, connectInfosIndexMap[newConnInfo]);
            Assert.Equal(0, connectInfosIndexMap[selector.Current]);
            Assert.Equal(1, connectInfosIndexMap[selector.Next]);
        }

        [Theory]
        [MemberData(nameof(GetValidConnectInfosCount))]
        public void ShouldSwitchPrematurely_should_always_be_false(int connectInfosCount)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var selector = new RoundRobinConnectionInfoSelector(connectInfos);
            // Act
            var actualShouldSwitchPrematurely = connectInfos
                .Select(x =>
                {
                    var result = selector.ShouldSwitchPrematurely;
                    selector.SwitchToNext();
                    return result;
                })
                .ToList();
            // Assert
            Assert.All(actualShouldSwitchPrematurely, Assert.False);
        }
    }
}
