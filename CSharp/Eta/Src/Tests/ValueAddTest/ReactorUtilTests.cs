/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using Xunit;

using static LSEG.Eta.ValueAdd.Reactor.ReactorUtil;

namespace LSEG.Eta.Tests.ValueAddTest
{
    public class ReactorUtilTests
    {
        static ReactorUtilTests()
        {
            // enforce ReactorUtil static constructor to be called before any of test
            var _ = TicksPerMilliSecond;
        }

        [Fact]
        public void ConvertDateTimeToMilliSecondTime_should_perform_conversion_properly()
        {
            // Arrange
            var now = DateTime.Now;
            var nowMs = GetCurrentTimeMilliSecond();
            // Act
            var result = ConvertDateTimeToMilliSecondTime(now);
            // Assert
            Assert.Equal(nowMs, result, 10.0);
        }

        [Fact]
        public void ConvertMilliSecondTimeToDateTime_should_perform_conversion_properly()
        {
            // Arrange
            var now = DateTime.Now;
            var nowMs = GetCurrentTimeMilliSecond();
            // Act
            var result = ConvertMilliSecondTimeToDateTime(nowMs);
            // Assert
            Assert.Equal(now, result, TimeSpan.FromMilliseconds(10));
        }
    }
}
