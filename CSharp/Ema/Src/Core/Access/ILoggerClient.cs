/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using LSEG.Ema.Access;

namespace LSEG.Ema.Access
{
    internal enum LoggerLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        OFF = 5
    }

    internal enum LoggerType
    {
        FILE = 0,
        STDOUT = 1
    }

    internal interface ILoggerClient
    {
        public static readonly string CR = NewLine + "\t";
        public const string INFO_STRING = "Info";
        public const string WARNING_STRING = "Warning";
        public const string ERROR_STRING = "Error";
        public const string DEBUG_STRING = "Debug";
        public const string TRACE_STRING = "Trace";
        public const string DEFAULTSEVERITY_STRING = "Unknown Severity";

        public bool IsTraceEnabled { get; }

        public bool IsDebugEnabled { get;}

        public bool IsInfoEnabled { get;}

        public bool IsWarnEnabled { get;}

        public bool IsErrorEnabled { get;}

        public LoggerLevel Level { get; set; }

        public void Debug(string clientName, string message);

        public void Info(string clientName, string message);

        public void Warn(string clientName, string message);

        public void Error(string clientName, string message);

        public void Trace(string clientName, string message);

        public void Cleanup();

        public static string LoggerLevelAsString(LoggerLevel level)
        {
            switch (level)
            {
                case LoggerLevel.INFO:
                    return "Info";
                case LoggerLevel.WARNING:
                    return "Warning";
                case LoggerLevel.ERROR:
                    return "Error";
                case LoggerLevel.DEBUG:
                    return "Debug";
                case LoggerLevel.TRACE:
                    return "Trace";
                default:
                    return $"Unknown Severity {level}";
            }
        }

        public static string FormatLogMessage(StringBuilder strBuilder, string clientName, string message, 
            LoggerLevel level)
        {
            strBuilder.Clear();
            strBuilder.AppendLine(": loggerMsg").Append("    ClientName: ").AppendLine(clientName)
            .Append($"    Severity: {LoggerLevelAsString(level)}")
            .AppendLine($"    Text:    {message}").Append("loggerMsgEnd");

            return strBuilder.ToString();
        }
    }
}
