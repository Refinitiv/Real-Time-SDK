/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Console;
using NLog.Extensions.Logging;

namespace Refinitiv.Common.Logger
{
    /// <summary>
    /// Creates ETA Logger
    /// </summary>
    public sealed class EtaLoggerFactory : ILoggerFactory
    {
        /// <summary>
        /// Gets or sets log to console
        /// </summary>
        public static bool LogToConsole { get; set; }

        /// <summary>
        /// Gets or sets log to memory
        /// </summary>
        public static bool LogToMemory { get; set; }

        /// <summary>
        /// Gets or sets log to NLog
        /// </summary>
        public static bool LogToNLog { get; set; }

        /// <summary>
        /// Gets or sets log to Debug mode
        /// </summary>
        public static bool LogToDebug { get; set; }

        /// <summary>
        /// Gets to sets the log severity level
        /// </summary>
        public static LogLevel LogLevel { get; set; } = LogLevel.Information;

        // Ignore Resharper...this pattern designed to create singleton correctly.
        private static readonly Lazy<ILoggerFactory> _instance = new Lazy<ILoggerFactory>(CreateFactory);

        IList<ILoggerProvider> Registry = new List<ILoggerProvider>();

        ILoggerFactory _factory;

        /// <summary>
        /// Register the well-known providers.
        /// </summary>
        /// <returns></returns>
        private static EtaLoggerFactory CreateFactory()
        {
            var internalFactory = LoggerFactory.Create(builder =>
                {
                    if (LogToConsole)
                        builder.AddConsole();
                    if (LogToNLog)
                        builder.AddNLog();
                    if(LogToDebug)
                        builder.AddDebug();
                });

            var factory = new EtaLoggerFactory(internalFactory);

            // Alwys at least NLog is enabled.
            if (!(LogToConsole || LogToMemory || LogToNLog))
                LogToNLog = true;

            if (LogToMemory)
                internalFactory.AddMemory();

            return  factory;
        }

        /// <summary>
        /// Gets the logger instance.
        /// </summary>
        public static ILoggerFactory Instance => _instance.Value;

        private EtaLoggerFactory(ILoggerFactory factory)
        {
            _factory = factory;
        }

        ILogger ILoggerFactory.CreateLogger(string categoryName)
        {
            return _factory.CreateLogger(categoryName);
        }

        void ILoggerFactory.AddProvider(ILoggerProvider provider)
        {
            lock (Registry)
            {
                if (!Registry.Contains(provider))
                {
                    _factory.AddProvider(provider);
                    Registry.Add(provider);
                }
            }
        }

        void IDisposable.Dispose()
        {
            throw new NotImplementedException();
        }
    }
}
