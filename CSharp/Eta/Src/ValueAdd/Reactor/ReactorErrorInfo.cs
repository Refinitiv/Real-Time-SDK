/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorErrorInfo is used by various Reactor methods to return error or warning
    /// information to the user. If the <see cref="ReactorReturnCode"/> is <see cref="ReactorReturnCode.SUCCESS"/>,
    /// then there is no error. Otherwise, the user should inspect the code, <see cref="Transports.Error"/> and location.
    /// </summary>
    public class ReactorErrorInfo
    {
        /// <summary>
        /// Create <see cref="ReactorErrorInfo"/>.
        /// </summary>
        public ReactorErrorInfo()
        {
            Error = new Error();
        }

        /// <summary>
        /// Gets <see cref="ReactorReturnCode"/>.
        /// If the code is not <see cref="ReactorReturnCode.SUCCESS"/>,
        /// the user should inspect this code, error and location.
        /// </summary>
        public ReactorReturnCode Code { get; internal set; }

        /// <summary>
        /// Gets the location to identify where the error occurred when the <see cref="Code"/>
        /// is not <see cref="ReactorReturnCode.SUCCESS"/>.
        /// </summary>
        public string? Location { get; internal set; }

        /// <summary>
        /// Gets the <see cref="Transports.Error"/> when the <see cref="Code"/> is not <see cref="ReactorReturnCode.SUCCESS"/>
        /// to get information regarding the error that occurred.
        /// </summary>
        public Error Error { get; internal set; }

        /// <summary>
        /// Returns a String representation of the <see cref="ReactorErrorInfo"/>.
        /// </summary>
        /// <returns>a string representation of this object.</returns>
        public override string ToString()
        {
            return $"code={Code}, location={Location}, Error.ErrorId={Error?.ErrorId}, Error.Text={Error?.Text}";
        }
    }
}
