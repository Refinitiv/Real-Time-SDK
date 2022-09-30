/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

using Microsoft.Extensions.Logging;
using Xunit.Abstractions;

namespace Refinitiv.Eta.Tests
{

    /// <summary>
    /// </summary>
    public sealed class XUnitLogger : ILogger
    {
        string _categoryName;

        bool _disposed = false;

        Func<string, LogLevel, bool> _filter;

        ITestOutputHelper _output;

        public XUnitLogger(ITestOutputHelper output, string categoryName)
        {
            _output = output;
            _categoryName = categoryName;
            _filter = null;
        }


        public IDisposable BeginScope<TState>(TState state)
        {
            throw new NotImplementedException();
        }

        public bool IsEnabled(LogLevel logLevel)
        {
            lock (_categoryName)
            {
                if (_disposed)
                    throw new ObjectDisposedException(nameof(XUnitLogger));

                return _filter == null || _filter(_categoryName, logLevel);
            }
        }

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception exception, Func<TState, Exception, string> formatter)
        {
            lock (_categoryName)
            {
                if (_disposed)
                    throw new ObjectDisposedException(nameof(XUnitLogger));

                // $KLUDGE$ The ITestOutputHElper can be in an invalid state, and
                //          there is no question we can ask it's public interface, 
                //          other than to try and use it.
                try { _output?.WriteLine($"{state}"); } catch (InvalidOperationException) { }
            }
        }

        public void Clear()
        {
            lock (_categoryName)
            {
                if (!_disposed)
                {
                    _output = null;
                }
            }
        }

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
