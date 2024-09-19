/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access;

/// <summary>
/// OmmInvalidConfigurationException is thrown when any catastrophic errors occur while processing and creating the OmmConfig objects.
/// </summary>
public sealed class OmmInvalidConfigurationException : OmmException
{
    /// <summary>
    /// Exception type for this exception.
    /// </summary>
    public override ExceptionType Type { get => ExceptionType.OmmInvalidConfigurationException; }


    /// <summary>
    /// Default constructor for this exception.
    /// </summary>
    internal OmmInvalidConfigurationException()
    {
    }

    /// <summary>
    /// Constructor with additional message indicating the core reason for the exception.
    /// </summary>
    internal OmmInvalidConfigurationException(string message)
        : base(message)
    {
    }
}