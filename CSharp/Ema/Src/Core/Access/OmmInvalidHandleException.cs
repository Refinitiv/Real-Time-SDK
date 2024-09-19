/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmInvalidHandleException is thrown when application passes in an invalid
    /// handle to <see cref="OmmConsumer"/>. OmmConsumer uses long values, called
    /// handles to identify individual item streams.<br/>
    /// OmmConsumer validates each passed in handle against all open and known handles.
    /// </summary>
    public class OmmInvalidHandleException : OmmException
    {
        /// <summary>
        /// Gets exception type
        /// </summary>
        public override ExceptionType Type => ExceptionType.OmmInvalidHandleException;

        /// <summary>
        /// Gets the invalid handle.
        /// </summary>
        public long Handle { get; private set; }


        internal OmmInvalidHandleException(long handle, string message)
            : base(message)
        {
            Handle = handle;
        }
    }
}
