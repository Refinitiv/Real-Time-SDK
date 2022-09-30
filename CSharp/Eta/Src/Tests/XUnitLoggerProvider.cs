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
    public class XUnitLoggerProvider : ILoggerProvider
    {
        private static readonly Lazy<XUnitLoggerProvider> _instance = new Lazy<XUnitLoggerProvider>(Create);
        public static XUnitLoggerProvider Instance => _instance.Value;

        internal ITestOutputHelper Output { get; set; }

        private static XUnitLoggerProvider Create()
        {
            return new XUnitLoggerProvider();
        }

        private XUnitLoggerProvider()
        {

        }

        public ILogger CreateLogger(string categoryName)
        {
            return new XUnitLogger(Output, categoryName);
        }

        #region IDisposable Support

        bool _isDisposed = false;
        public void Dispose()
        {
            if (!_isDisposed)
                Output = null;
            _isDisposed = true;
        }

        #endregion
    }

}
