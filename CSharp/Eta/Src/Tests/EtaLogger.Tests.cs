/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;

using Xunit;
using Xunit.Categories;
using Refinitiv.Common.Logger;

namespace Refinitiv.Eta.Transports.Tests
{
    public class EtaLoggerTests
    {
        static EtaLoggerTests()
        {
            EtaLogger.LogName = "ESDK.Tests.EtaLogger";
            EtaLoggerFactory.LogToMemory = true;
        }

        private static void AssertContentsContains(IMemoryLogger memoryLogger, string text, int atLeast = 1, int? atMost = null)
        {
            Stream stream = memoryLogger.Contents;
            StreamReader reader = new StreamReader(stream);
            string line = null;
            int lineCnt = 0;
            while ((line = reader.ReadLine()) != null)
            {
                if (!string.IsNullOrEmpty(line))
                {
                    if (line.Contains(text))
                        lineCnt++;
                }
            }

            Assert.True(lineCnt >= atLeast);
            Assert.True(!atMost.HasValue || lineCnt <= atMost);
        }

        private static string UniqueMessage(string text)
        {
            return $"{DateTime.UtcNow:yyyyMMdd-hhmmss.ffff}: {text}!";
        }

        [Fact]
        [Category("Unit")]
        public void LoggerIntializedOk()
        {
            var logger = EtaLogger.Instance;
            Assert.IsType<EtaLogger>(logger);
        }

        [Fact]
        [Category("Unit")]
        public void LoggerAlwaysReturnsSameInstance()
        {
            var loggerA = EtaLogger.Instance;
            var loggerB = EtaLogger.Instance;

            Assert.Same(loggerA, loggerB);
        }

        [Fact]
        [Category("Unit")]
        public void LoggerGetDefaultLoglevelOK()
        {
            var logLevel = EtaLogger.LogLevel;
            Assert.Equal(Microsoft.Extensions.Logging.LogLevel.Information, logLevel);
        }

        [Fact]
        [Category("Unit")]
        public void ChangeMinLogLevelOK()
        {
            var minLevel = EtaLogger.LogLevel;

            EtaLogger.LogLevel = Microsoft.Extensions.Logging.LogLevel.Warning;
            Assert.NotEqual(minLevel, EtaLogger.LogLevel);
            EtaLogger.LogLevel = minLevel;
        }

        [Fact]
        [Category("Integration")]
        public void LoggerWritesErrorTextWhenLogLevelBelowMin()
        {
            var memoryLogger = (IMemoryLogger)MemoryLogger.Instance;

            string errorLogText = UniqueMessage("Logger logs Nothing when Loglevel below minLevel");
            var minLevel = EtaLogger.LogLevel;
            var logger = EtaLogger.Instance;
            EtaLogger.LogLevel = Microsoft.Extensions.Logging.LogLevel.Critical;
            logger.Error(errorLogText);

            AssertContentsContains(memoryLogger, errorLogText, 0, 0);
            EtaLogger.LogLevel = minLevel;
        }
    }
}
