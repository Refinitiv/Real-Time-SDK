/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Extensions.Logging;

namespace Refinitiv.Common.Logger
{
    /// <summary>
    /// The provider for logging to memory stream.
    /// </summary>
    public class MemoryLoggerProvider : ILoggerProvider
    {
        Func<string, LogLevel, bool> _filter;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="filter">The filter</param>
        public MemoryLoggerProvider(Func<string, LogLevel, bool> filter)
        {
            _filter = filter;
        }

        /// <summary>
        /// Creates a memory logger.
        /// </summary>
        /// <param name="categoryName">The catagory name</param>
        /// <returns>The <see cref="MemoryLogger"/> instance</returns>
        public ILogger CreateLogger(string categoryName)
        {
            return MemoryLogger.Instance;
        }

        #region IDisposable Support

        /// <summary>
        /// Disposes the unmanaged resources.
        /// </summary>
        public void Dispose()
        {
        }

        #endregion
    }

}
