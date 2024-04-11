/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


using NLog;
using NLog.Config;
using NLog.Targets;
using System.Text;
using System.Runtime.InteropServices;

namespace LSEG.Ema.Access
{
    internal class LoggerClient<T> : ILoggerClient
    {
        private readonly string LOG_MSG_LAYOUT;

        private static readonly string EMA_Console_Name = "EMA_Console";

        private static readonly string EMA_File_Name = "EMA_Logfile";

        private static readonly string EMA_Default_File_Name = "emaLog";

        private readonly OmmBaseImpl<T> m_OmmBaseImpl;

        private readonly Logger m_NLogger;

        private static readonly Logger DefaultLogger;

        public bool IsDebugEnabled => Level <= LoggerLevel.DEBUG;

        public bool IsInfoEnabled => Level <= LoggerLevel.INFO;

        public bool IsWarnEnabled => Level <= LoggerLevel.WARNING;

        public bool IsErrorEnabled => Level <= LoggerLevel.ERROR;

        public bool IsTraceEnabled => Level <= LoggerLevel.TRACE;

        public LoggerLevel Level { get; set; }

        public static readonly int ProcessId;

        private static readonly LoggingConfiguration LoggingConf;

        private readonly StringBuilder m_StrBuilder = new(1024);

        static LoggerClient()
        {
            ProcessId = System.Environment.ProcessId;

            DefaultLogger = LogManager.GetCurrentClassLogger();

            LoggingConf = new();
        }

        public LoggerClient(OmmBaseImpl<T> ommBaseImpl)
        {
            if(RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                LOG_MSG_LAYOUT = "${longdate}\r\n${level:uppercase=true}|${message}";
            }
            else
            {
                LOG_MSG_LAYOUT = "${longdate}\n${level:uppercase=true}|${message}";
            }

            m_OmmBaseImpl = ommBaseImpl;

            Clear();

            if (m_OmmBaseImpl.ConfigImpl.LoggerConfig != null)
            {
                string loggerName = $"LSEG.Ema.Access.LoggerClient.{m_OmmBaseImpl.InstanceName}";

                string messageLayout = LOG_MSG_LAYOUT;

                if(m_OmmBaseImpl.ConfigImpl.LoggerConfig.IncludeDateInLoggerOutput == 0)
                {
                    messageLayout = messageLayout.Remove(0, 11); // Remove ${longdate} from message layout.
                }

                if (m_OmmBaseImpl.ConfigImpl.LoggerConfig.LoggerType == LoggerType.STDOUT)
                {
                    var consoleTarget = new ConsoleTarget
                    {
                        Name = EMA_Console_Name,
                        Layout = messageLayout,
                        AutoFlush = true
                    };
                    LoggingConf.AddRuleForAllLevels(consoleTarget, loggerName);
                }
                else if (m_OmmBaseImpl.ConfigImpl.LoggerConfig.LoggerType == LoggerType.FILE)
                {
                    string fileName = string.IsNullOrEmpty(m_OmmBaseImpl.ConfigImpl.LoggerConfig.FileName) ?
                        EMA_Default_File_Name : m_OmmBaseImpl.ConfigImpl.LoggerConfig.FileName;
                    ulong maxArchivesFiles = m_OmmBaseImpl.ConfigImpl.LoggerConfig.NumberOfLogFiles > 0 ?
                        m_OmmBaseImpl.ConfigImpl.LoggerConfig.NumberOfLogFiles : 1;

                    var fileTarget = new FileTarget
                    {
                        Name = EMA_File_Name,
                        FileName = $"{fileName}_{ProcessId}.log",
                        Layout = messageLayout,
                        MaxArchiveFiles = (int)maxArchivesFiles,
                        ArchiveNumbering = ArchiveNumberingMode.Rolling,
                        ArchiveAboveSize = (int)m_OmmBaseImpl.ConfigImpl.LoggerConfig.MaxLogFileSize,
                        KeepFileOpen = false
                    };
                    LoggingConf.AddRuleForAllLevels(fileTarget, loggerName);
                }

                // To avoid conflicts with the rest of the components in the system obtain our own
                // logger from a LogFactory instead of the LogManager singleton
                LogFactory logFactory = new LogFactory();

                logFactory.Configuration = LoggingConf;
                logFactory.AutoShutdown = true;

                m_NLogger = logFactory.GetLogger(loggerName);

                Level = m_OmmBaseImpl.ConfigImpl.LoggerConfig.LoggerSeverity;
            }
            else
            {
                m_NLogger = DefaultLogger;
            }
        }

        public void Debug(string clientName, string message)
        {
            m_NLogger.Debug(ILoggerClient.FormatLogMessage(m_StrBuilder.Clear(), 
                clientName, message, LoggerLevel.DEBUG));
        }

        public void Error(string clientName, string message)
        {
            m_NLogger.Error(ILoggerClient.FormatLogMessage(m_StrBuilder.Clear(), 
                clientName, message, LoggerLevel.ERROR));
        }

        public void Info(string clientName, string message)
        {
            m_NLogger.Info(ILoggerClient.FormatLogMessage(m_StrBuilder.Clear(), 
                clientName, message, LoggerLevel.INFO));
        }

        public void Warn(string clientName, string message)
        {
            m_NLogger.Warn(ILoggerClient.FormatLogMessage(m_StrBuilder.Clear(), 
                clientName, message, LoggerLevel.WARNING));
        }
        public void Trace(string clientName, string message)
        {
            m_NLogger.Trace(ILoggerClient.FormatLogMessage(m_StrBuilder.Clear(), 
                clientName, message, LoggerLevel.TRACE));
        }

        public void Clear()
        {
            Level = LoggerLevel.INFO;
        }

    }
}
