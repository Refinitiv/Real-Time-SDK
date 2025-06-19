/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmOutOfRangeException is thrown when passed in values is out of valid range.
/// </summary>
public sealed class OmmOutOfRangeException : OmmException
{
    /// <summary>
    /// Gets exception type
    /// </summary>
    public override ExceptionType Type { get => ExceptionType.OmmOutOfRangeException; }

    internal OmmOutOfRangeException() : base()
    {
    }

    internal OmmOutOfRangeException(string message) : base(message)
    {
    }
}
