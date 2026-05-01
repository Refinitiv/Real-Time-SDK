/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor.Fallback.ConnectionInfoSelectors;
using System;
using System.Collections.Generic;
using System.Linq;
using static LSEG.Eta.Tests.ValueAddTest.Fallback.ConnectionInfoSelectors.TestUtil;

namespace LSEG.Eta.Tests.ValueAddTest.Fallback.ConnectionInfoSelectors
{
    public class PreferredHostConnectionInfoSelectorTests
    {
        private readonly ITestOutputHelper m_Output;

        public PreferredHostConnectionInfoSelectorTests(ITestOutputHelper output)
        {
            m_Output = output;
        }

        [Fact]
        public void Ctor_should_throw_on_empty_connection_list() =>
            CheckThrowOnEmptyConnectionList(x => new PreferredHostConnectionInfoSelector(x, 0));

        [Fact]
        public void Ctor_should_throw_on_null_connection_list() =>
            CheckThrowOnNullConnectionList(x => new PreferredHostConnectionInfoSelector(x, 0));

        [Theory]
        [MemberData(nameof(GetNegativePreferredConnectionIndexWithListSizes))]
        public void Ctor_should_throw_on_negative_PreferredConnectionIndex(int connectInfosCount, int preferredConnectionIndex)
        {
            var exception = Assert.Throws<ArgumentOutOfRangeException>(() =>
                new PreferredHostConnectionInfoSelector(CreateConnectInfos(connectInfosCount), preferredConnectionIndex));
            Assert.StartsWith("Preferred connection index cannot be negative", exception.Message);
        }

        public static IEnumerable<object[]> GetNegativePreferredConnectionIndexWithListSizes() =>
            from connectInfosCount in GetValidConnectionListSizes()
            from preferredConnectionIndex in new[] { -1, -2, -10 }
            select new object[] { connectInfosCount, preferredConnectionIndex };

        [Theory]
        [MemberData(nameof(GetExceedingPreferredConnectionIndexWithListSizes))]
        public void Ctor_should_throw_on_exceeding_PreferredConnectionIndex(int connectInfosCount, int preferredConnectionIndex)
        {
            var exception = Assert.Throws<ArgumentOutOfRangeException>(() =>
                new PreferredHostConnectionInfoSelector(CreateConnectInfos(connectInfosCount), preferredConnectionIndex));
            Assert.Matches("^Preferred connection index.+cannot exceed maximum connection infos index", exception.Message);
        }

        public static IEnumerable<object[]> GetExceedingPreferredConnectionIndexWithListSizes() =>
            GetValidConnectionListSizes()
                .SelectMany(connectInfosCount => new[] {
                    new object[]{connectInfosCount, connectInfosCount},
                    new object[]{connectInfosCount, connectInfosCount + 1},
                    new object[]{connectInfosCount, connectInfosCount + 2},
                    new object[]{connectInfosCount, connectInfosCount * 2},
                });

        [Theory]
        [MemberData(nameof(GetPreferredConnectionIndexWithListSizes))]
        public void Ctor_should_set_initial_connection_to_preferred_host(int connectInfosCount, int preferredConnectionIndex)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var connectInfosIndexMap = ObjectIndexMap.Of(connectInfos);
            // Act
            var selector = new PreferredHostConnectionInfoSelector(connectInfos, preferredConnectionIndex);
            // Assert
            Assert.Equal(preferredConnectionIndex, connectInfosIndexMap[selector.Current]);
        }

        public static IEnumerable<object[]> GetPreferredConnectionIndexWithListSizes() =>
            GetValidConnectionListSizes()
                .SelectMany(
                    connectInfosCount => Enumerable.Range(0, connectInfosCount),
                    (connectInfosCount, preferredConnectionIndex) => new object[] { connectInfosCount, preferredConnectionIndex });

        [Fact]
        public void SwitchToNext_should_stay_on_same_connection_when_list_is_single_item() =>
            CheckStayOnSameConnectionWhenListIsSingleItem(x => new PreferredHostConnectionInfoSelector(x, 0), m_Output);

        [Theory]
        [InlineData(2, 0, new[] { 1, 0 })]
        [InlineData(2, 1, new[] { 0, 1 })]
        [InlineData(3, 0, new[] { 1, 0, 2, 0 })]
        [InlineData(3, 1, new[] { 0, 1, 2, 1 })]
        [InlineData(3, 2, new[] { 0, 2, 1, 2 })]
        public void SwitchToNext_should_properly_switch_to_next_connection(int connectInfosCount, int preferredConnectionIndex, int[] connectIndexSequence)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var connectInfosIndexMap = ObjectIndexMap.Of(connectInfos);
            var selector = new PreferredHostConnectionInfoSelector(connectInfos, preferredConnectionIndex);

            // Act

            // Note, that connectIndexSequence doesn't contain preferred connection index because
            // we already proved that PreferredHostConnectionInfoSelector starts from it.
            // See Ctor_should_set_initial_connection_to_preferred_host test.

            var indices = connectIndexSequence
                .Select(x =>
                {
                    var nextConnInfoBeforeSwitch = selector.Next;

                    var newConnInfo = selector.SwitchToNext();

                    return new
                    {
                        SwitchToNextIndex = connectInfosIndexMap[newConnInfo],
                        CurrentIndex = connectInfosIndexMap[selector.Current],
                        NextBeforeSwitchIndex = connectInfosIndexMap[nextConnInfoBeforeSwitch],
                    };
                })
                .ToList();

            // Assert
            Assert.Equal(connectIndexSequence, indices.Select(x => x.SwitchToNextIndex).ToArray());
            Assert.Equal(connectIndexSequence, indices.Select(x => x.CurrentIndex).ToArray());
            Assert.Equal(connectIndexSequence, indices.Select(x => x.NextBeforeSwitchIndex).ToArray());
        }

        [Theory]
        [InlineData(2, 1)]
        [InlineData(3, 2)]
        [InlineData(10, 9)]
        [InlineData(11, 10)]
        public void SwitchToNext_should_switch_to_first_connection_when_list_end_is_reached(int connectInfosCount, int preferredConnectionIndex)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var connectInfosIndexMap = ObjectIndexMap.Of(connectInfos);
            var selector = new PreferredHostConnectionInfoSelector(connectInfos, preferredConnectionIndex);
            // Act
            var newConnInfo = selector.SwitchToNext();
            // Assert
            Assert.Equal(0, connectInfosIndexMap[newConnInfo]);
            Assert.Equal(0, connectInfosIndexMap[selector.Current]);
            Assert.Equal(preferredConnectionIndex, connectInfosIndexMap[selector.Next]);
        }

        [Theory]
        [InlineData(1, 0, new[] { false, false })]
        [InlineData(2, 0, new[] { false, true })]
        [InlineData(2, 1, new[] { false, true })]
        [InlineData(3, 0, new[] { false, true, false, true })]
        [InlineData(3, 1, new[] { false, true, false, true })]
        [InlineData(3, 2, new[] { false, true, false, true })]
        public void ShouldSwitchPrematurely_should_depend_on_current_and_preferred_index(int connectInfosCount, int preferredConnectionIndex, bool[] shouldSwitchPrematurely)
        {
            // Arrange
            var connectInfos = CreateConnectInfos(connectInfosCount);
            var selector = new PreferredHostConnectionInfoSelector(connectInfos, preferredConnectionIndex);
            // Act
            var actualShouldSwitchPrematurely = shouldSwitchPrematurely
                .Select(x =>
                {
                    var result = selector.ShouldSwitchPrematurely;
                    selector.SwitchToNext();
                    return result;
                })
                .ToList();
            // Assert
            Assert.Equal(shouldSwitchPrematurely, actualShouldSwitchPrematurely);
        }
    }
}
