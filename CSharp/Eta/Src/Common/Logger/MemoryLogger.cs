/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

using Microsoft.Extensions.Logging;

namespace Refinitiv.Common.Logger
{
    /// <summary>
    /// Creats logger on memory stream
    /// </summary>
    public interface IMemoryLogger : IDisposable
    {
        /// <summary>
        /// Gets the logging content
        /// </summary>
        Stream Contents { get; }

        /// <summary>
        /// Clears the logging content 
        /// </summary>
        void Clear();
    }

    /// <summary>
    /// Stash log messages in a stream; allows for later retrieval and inspection
    /// of contents of log messages during testing and such.
    /// </summary>
    public sealed class MemoryLogger : ILogger, IMemoryLogger
    {
        private static readonly Lazy<MemoryLogger> _instance = new Lazy<MemoryLogger>();

        string _categoryName;

        bool _disposed = false;

        Func<string, LogLevel, bool> _filter;

        #region _stream
        MemoryStream _stream;
        Stream Stream
        {
            get
            {
                lock (_categoryName)
                {
                    if (_stream == null)
                    {
                        _stream = new MemoryStream(1024 * 1024 * 4);
                    }

                    return _stream;
                }
            }
        }
        #endregion

        #region _writer
        StreamWriter _writer;
        StreamWriter Writer
        {
            get
            {
                lock (_categoryName)
                {
                    if (_writer == null)
                    {
                        _writer = new StreamWriter(Stream);
                    }

                    return _writer;
                }
            }
        }
        #endregion

        /// <summary>
        ///  Gets the logger instance
        /// </summary>
        public static MemoryLogger Instance => _instance.Value;

        /// <summary>
        /// Constructor
        /// </summary>
        public MemoryLogger()
        {
            _categoryName = "MEMORY";
            _filter = null;
        }


        Stream IMemoryLogger.Contents
        {
            get
            {
                lock (_categoryName)
                {
                    if (_disposed)
                        throw new ObjectDisposedException(nameof(MemoryLogger));

                    Writer.Flush();

                    long position = Stream.Position;
                    var contents = new MemoryStream((int)position);
                    Stream.Position = 0;
                    Stream.CopyTo(contents);
                    Stream.Position = position;
                    contents.Position = 0;
                    return contents;
                }
            }
        }


        /// <summary>
        /// Begins a logical operation scope.
        /// </summary>
        /// <typeparam name="TState">The type</typeparam>
        /// <param name="state">The identifier for the scope.</param>
        /// <returns></returns>
        public IDisposable BeginScope<TState>(TState state)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Checks wheter the log level is enable
        /// </summary>
        /// <param name="logLevel">A <see cref="LogLevel"/></param>
        /// <returns>Returs <true> if <see cref="LogLevel"/> is enabled; other <c>false</c></true></returns>
        public bool IsEnabled(LogLevel logLevel)
        {
            lock (_categoryName)
            {
                if (_disposed)
                    throw new ObjectDisposedException(nameof(MemoryLogger));

                return _filter == null || _filter(_categoryName, logLevel);
            }
        }

        /// <summary>
        /// Logs the content
        /// </summary>
        /// <typeparam name="TState">The type</typeparam>
        /// <param name="logLevel">The <see cref="LogLevel"/></param>
        /// <param name="eventId">The event identifer</param>
        /// <param name="state">The type</param>
        /// <param name="exception">The exception</param>
        /// <param name="formatter">The formatter</param>
        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception exception, Func<TState, Exception, string> formatter)
        {
            lock (_categoryName)
            {
                if (_disposed)
                    throw new ObjectDisposedException(nameof(MemoryLogger));

                Writer.WriteLine($"{state}");
            }
        }

        /// <summary>
        /// Clears the content
        /// </summary>
        public void Clear()
        {
            lock (_categoryName)
            {
                if (!_disposed)
                {
                    if (_writer != null)
                        _writer.Dispose();
                    _writer = null;

                    if (_stream != null)
                        _stream.Dispose();
                    _stream = null;
                }
            }
        }

        /// <summary>
        /// Disposing the content
        /// </summary>
        public void Dispose()
        {
            lock (_categoryName)
            {
                Clear();
                _disposed = true;
            }
        }
    }
}
