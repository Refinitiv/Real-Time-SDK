/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Reflection;
using System.Diagnostics;
using System.Threading;


using Microsoft.Extensions.Logging;

using Refinitiv.Common.Interfaces;

namespace Refinitiv.Common.Logger
{
    /// <summary>
    /// The implementation for EtaLogger.
    /// </summary>
    public sealed class EtaLogger : IEtaLogger
    {
        /// <summary>
        /// Gets or sets the logger name 
        /// </summary>
        public static string LogName { get; set; }

        /// <summary>
        /// Gets or sets the log level
        /// </summary>
        // sets the minimun LogLevel 
        public static LogLevel LogLevel { get; set; } = LogLevel.Information;

        // Static members will be initialized by the runtime prior to first instance
        // of the class being accessed.
        private static readonly Lazy<IEtaLogger> _instance = new Lazy<IEtaLogger>(CreateLogger);

        private static ILogger _innerLogger = null;

        private static IEtaLogger CreateLogger()
        {
            //---------------------------------------------------------
            //---------------------------------------------------------
            // Connect and configure Logging Framework.
            //---------------------------------------------------------
            var factory = EtaLoggerFactory.Instance;

            // Name the Logger.
            string name = LogName ?? Assembly.GetExecutingAssembly().GetName().Name;

            _innerLogger = factory.CreateLogger(name);
            System.Diagnostics.Debug.Assert(!(_innerLogger is null));
            var instance = new EtaLogger();
            return instance;
        }

        /// <summary>
        /// Gets the ETA logger instance
        /// </summary>
        public static IEtaLogger Instance => _instance.Value;

        private long _eventId = 0;

        private EtaLogger()
        {
        }

        /// <summary>
        /// Log a message for <see cref="LogLevel.Error"/>
        /// </summary>
        /// <param name="message">The log message</param>
        /// <returns>This object</returns>
        public IEtaLogger Error(string message)
        {
            return WriteText($"{message}", LogLevel.Error);
        }

        /// <summary>
        /// Log an <see cref="Exception"/> for <see cref="LogLevel.Error"/>
        /// </summary>
        /// <param name="exception">The <see cref="Exception"/> to log</param>
        /// <returns>This object</returns>
        public IEtaLogger Error(Exception exception)
        {
            WriteText($"{exception.Message}", LogLevel.Error);
            return WriteText($"{exception.StackTrace}", LogLevel.Error);
        }

        /// <summary>
        /// Log a message for <see cref="LogLevel.Information"/>
        /// </summary>
        /// <param name="message">The log message</param>
        /// <returns>This object</returns>
        public IEtaLogger Information(string message)
        {
            return WriteText($"{message}", LogLevel.Information);
        }

        /// <summary>
        /// Log a message for <see cref="LogLevel.Debug"/>
        /// </summary>
        /// <param name="message">The log message</param>
        /// <returns>This object</returns>
        public IEtaLogger Debug(string message)
        {
            return WriteText($"{message}", LogLevel.Debug);
        }

        /// <summary>
        /// Log a message for <see cref="LogLevel.Trace"/>
        /// </summary>
        /// <param name="message">The log message</param>
        /// <returns>This object</returns>
        public IEtaLogger Trace(string message)
        {
            return WriteText($"{message}", LogLevel.Trace);
        }

        /// <summary>
        /// Write a newline-terminated message into the log sinks for a <see cref="Microsoft.Extensions.Logging.LogLevel"/>.
        /// The default login level is <see cref="Microsoft.Extensions.Logging.LogLevel.Information"/>
        /// </summary>
        /// <param name="message">The log message</param>
        /// <param name="logLevel">The <see cref="Microsoft.Extensions.Logging.LogLevel"/></param>
        /// <returns></returns>
        public IEtaLogger Write(string message, LogLevel logLevel = LogLevel.Information)
        {
            return WriteText($"{message}\n", logLevel);
        }

        /// <summary>
        /// Write a message for a <see cref="Microsoft.Extensions.Logging.LogLevel"/>.
        /// The default login level is <see cref="Microsoft.Extensions.Logging.LogLevel.Information"/>
        /// </summary>
        /// <param name="message">The log message</param>
        /// <param name="logLevel">The <see cref="Microsoft.Extensions.Logging.LogLevel"/></param>
        /// <returns></returns>
        public IEtaLogger WriteText(string message, LogLevel logLevel = LogLevel.Information)
        {
            Interlocked.Increment(ref _eventId);

            if (logLevel >= LogLevel)
            {
                _innerLogger.Log(logLevel, (EventId)_eventId, message, null, new Func<object, Exception, string>((obj, exp) =>
                {
                    return $"{obj}";
                }));

            }

            return Instance;
        }
    }
}
