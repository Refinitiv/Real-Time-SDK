/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Linq;

using NLog;
using NLog.Targets;
using NLog.Config;

using Xunit;
using Xunit.Categories;

namespace Refinitiv.Eta.Transports.Tests
{
    /// <summary>
    /// Verify that NLog has been configured to write a log file.
    /// </summary>
    [Collection("Sequential")]
    public class NLogTests
    {
        // Must match a NLog.config::rules/logger[name]
        const string LOG_NAME = "ESDK.Tests.EtaLogger";

        static Logger logger = LogManager.GetLogger(LOG_NAME);

        private static void AssertContentsContains(Stream stream, string text, int atLeast = 1, int? atMost = null)
        {
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

        private static NLog.Targets.Target FindTargetOfNamedRule(string ruleName)
        {
            var targetName = (from rule in LogManager.Configuration.LoggingRules
                              where string.Compare(rule.LoggerNamePattern, ruleName, true) == 0
                              select rule.Targets.First().Name)
              .FirstOrDefault();

            var target = LogManager.Configuration.FindTargetByName(targetName);
            return target;
        }

        private static string FindNLogTargetFileName()
        {
            var target = FindTargetOfNamedRule(LOG_NAME) as NLog.Targets.FileTarget;

            Assert.NotNull(target);

            return ((NLog.Layouts.SimpleLayout)target.FileName).Text;

        }

        /// <summary>
        /// Create a FileTarget to be used for testing
        /// of log archive file creation in the archive 
        /// directory, with an archiveFilename configured at runtime
        /// the archive directory should not already contain this
        /// filename
        /// 
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="archiveFilename"></param>
        /// <param name="loggerName"></param>
        /// <returns>Logger</returns>
        private static Logger CreateArchiveFileTarget(string filename, string archiveFilename, string loggerName)
        {
            var fileTarget = new FileTarget
            {
                Name = "logfile2",
                Layout = "${longdate} ${logger} ${level} ${message}",
                MaxArchiveFiles = 20
            };
            fileTarget.FileName = filename;
            fileTarget.ArchiveFileName = archiveFilename.Replace("Log.", "Log.${date:yyyy-MM-dd}.");
            fileTarget.ArchiveEvery = FileArchivePeriod.Minute;
            fileTarget.ArchiveNumbering = ArchiveNumberingMode.DateAndSequence;
            var logRule = new LoggingRule(loggerName, LogLevel.Trace, fileTarget);
            var config = new LoggingConfiguration();
            config.AddTarget("file", fileTarget);
            config.LoggingRules.Add(logRule);
            LogManager.Configuration = config;
            Logger logger = LogManager.GetLogger(loggerName);

            return logger;
        }

        [Fact]
        [Category("Unit")]
        public void NLogConfigDefaultRuleFound()
        {
            var rules = LogManager.Configuration.LoggingRules;
            Assert.Contains(LOG_NAME, rules.Select(r => r.LoggerNamePattern));
        }

        [Fact]
        [Category("Integration")]
        public void EtaArchiveLogFile()
        {
            System.Collections.ObjectModel.ReadOnlyCollection<Target> oTargets = LogManager.Configuration.AllTargets;
            var rules = LogManager.Configuration.LoggingRules;

            var target = LogManager.Configuration.FindTargetByName("logfile.archived") as FileTarget;
            Assert.NotNull(target);

            var targetFileName = target.FileName;
            var filename = targetFileName.ToString().Replace("'", "");
            filename = filename.Replace("2", "A");
            var archiveFilename = filename.Replace("logs", "archives");
            var logger = CreateArchiveFileTarget(filename, archiveFilename, "NLogLogger");

            logger.Debug("debug log happened");
            logger.Trace("Trace log happened");
            logger.Warn("Warn log happened");
            logger.Info("Info log happened");
            logger.Error("Error log happened");
            logger.Info("Info other log forms are also available as well (exception type logs)");

            var archiveDate = DateTime.Now.ToLocalTime().ToString("yyyy-MM-dd");
            var archiveFileCreated = archiveFilename.Replace(".txt", "." + archiveDate + ".txt");
            Assert.True(File.Exists(archiveFileCreated));

            //reset targets
            var oldConfig = new LoggingConfiguration();
            int i = 0;
            foreach (Target item in oTargets)
            {
                oldConfig.AddTarget(item);
                LogLevel ll = rules[i].Levels[0];
                int z = rules[i].Levels.Count - 1;
                oldConfig.AddRule(ll, rules[i].Levels[z], item, rules[i].LoggerNamePattern);
                i++;
            }

            LogManager.Configuration = oldConfig;
        }
    }
}
