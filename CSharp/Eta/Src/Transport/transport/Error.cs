/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Eta.Transports.Interfaces;

namespace Refinitiv.Eta.Transports
{
    /// <summary>
    /// Many of the ETA Transport Package methods take a parameter for returning
    /// detailed error information.This Error object is only populated in the event
    /// of an error condition and should only be inspected when a specific failure
    /// code is returned from the method itself.
    ///
    /// In several cases, positive return values reserved or have special meaning,
    /// for example bytes remaining to write to the network. As a result, some
    /// negative return codes may be used to indicate success - this is different
    /// behavior than the Codec package where any positive value indicates success.
    /// Any specific transport related success or failure error handling is described
    /// along with the method that requires it.
    /// </summary>
    public class Error
    {
        /// <summary>
        /// The <see cref="IChannel"/> the error occurred on.
        /// </summary>
        /// <value><see cref="IChannel"/></value>
        public IChannel Channel { get; set; }

        /// <summary>
        /// A ETA specific return code, used to specify the error that has occurred.
        /// See the following sections for specific error conditions that may arise.
        /// </summary>
        /// <value><see cref="TransportReturnCode"/></value>
        public TransportReturnCode ErrorId { get; set; }

        /// <summary>
        /// Populated with the system errno or error number associated with the
        /// failure. This information is only available when the failure occurs as a
        /// result of a system method, and will be populated to 0 otherwise.
        /// </summary>
        /// <value>The system error code</value>
        public int SysError { get; set; }

        /// <summary>
        /// Detailed text describing the error that has occurred. This may include
        /// ETA specific error information, underlying library specific error
        /// information, or a combination of both.
        /// </summary>
        /// <value>The error text</value>
        public string Text { get; set; }

        /// <summary>
        /// The default constructor
        /// </summary>
        public Error()
        {
        }

        /// <summary>
        /// The constructor
        /// </summary>
        /// <param name="errorId">The transport return code</param>
        /// <param name="text">The error text</param>
        /// <param name="channel">The <see cref="IChannel"/> the error occurred on</param>
        /// <param name="sysError">The system error Id</param>
        /// <param name="exception">The exception associated with this error</param>
        public Error(TransportReturnCode errorId, string text, IChannel channel = null, int sysError = 0, Exception exception = null)
        {
            ErrorId = errorId;
            Text = text;
            Channel = channel;
            SysError = sysError;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The stirng value</returns>
        public override string ToString()
        {
            return $"ErrorId: {ErrorId}, Text: {Text}, SysError: {SysError}, Channel: {{{Channel?.ToString()??"*null*"}}}";
        }

    }
}
