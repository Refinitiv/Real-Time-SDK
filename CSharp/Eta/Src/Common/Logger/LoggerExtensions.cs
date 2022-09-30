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
    /// The extenstion for logger
    /// </summary>
    public static class LoggerExtensions
    {
        /// <summary>
        /// Add <see cref="MemoryLoggerProvider"/> to the logger factor.
        /// </summary>
        /// <param name="factory">The logger factor</param>
        /// <param name="filter">The filter</param>
        /// <returns></returns>
        public static ILoggerFactory AddMemory(this ILoggerFactory factory, Func<string, LogLevel, bool> filter = null)
        {
            factory.AddProvider(new MemoryLoggerProvider(filter));
            return factory;
        }

    }
}
