/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmException is base class for all exception types thrown by EMA.
/// </summary>
public abstract class OmmException : Exception
{

    /// <summary>
    /// ExceptionType represents exception type.
    /// </summary>
    public enum ExceptionType : int
    {
        /// <summary>
        /// Indicates invalid usage exception
        /// </summary>
        OmmInvalidUsageException = 1,

        /// <summary>
        /// Indicates invalid configuration exception
        /// </summary>
        OmmInvalidConfigurationException = 2,

        /// <summary>
        /// Indicates out of range exception
        /// </summary>
        OmmOutOfRangeException = 3,

        /// <summary>
        /// Indicates invalid handle exception
        /// </summary>
        OmmInvalidHandleException = 4,

        /// <summary>
        ///  Indicates unsupported domain type exception
        /// </summary>
        OmmUnsupportedDomainTypeException = 5
    }

    /// <summary>
    /// Gets exception type
    /// </summary>
    public abstract ExceptionType Type { get; }

    internal OmmException() : base() { }

    internal OmmException(string message) : base(message) { }
}
