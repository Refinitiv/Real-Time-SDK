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

namespace Refinitiv.Common.Interfaces
{
    /// <summary>
    /// The interface contacts for ETA logger
    /// </summary>
    public interface IEtaLogger
    {
        /// <summary>
        /// Write a newline-terminated message into the log sinks.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger Write(string message, LogLevel logLevel = LogLevel.Information);

        /// <summary>
        /// Write a newline-terminated message into the log sinks.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger Error(string message);

        /// <summary>
        /// Write a newline-terminated message into the log sinks.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger Error(Exception exception);

        /// <summary>
        /// Write a newline-terminated message into the log sinks.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger Debug(string message);

        /// <summary>
        /// Write a newline-terminated message into the log sinks.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger Trace(string message);

        /// <summary>
        /// Write a newline-terminated message into the log sinks.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger Information(string message);

        /// <summary>
        /// Write a message into the log sinks; not terminated by newline.
        /// </summary>
        /// <returns>The logger instance.</returns>
        IEtaLogger WriteText(string message, LogLevel logLevel = LogLevel.Information);
    }
}
