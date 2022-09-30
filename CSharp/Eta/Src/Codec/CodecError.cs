/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    /// <summary>
    /// Many of the ETA Codec Package methods take a parameter for returning
    /// detailed error information. This Error object is only populated in the event
    /// of an error condition and should only be inspected when a specific failure
    /// code is returned from the method itself.
    /// /// </summary>
    public sealed class CodecError
    {
        /// <summary>
		/// A ETA specific return code, used to specify the error that has occurred.
		/// See the following sections for specific error conditions that may arise.
		/// </summary>
		/// <returns> the errorId </returns>
        public CodecReturnCode ErrorId { get; set; }

        /// <summary>
		/// Detailed text describing the error that has occurred.
		/// </summary>
		/// <returns> the text </returns>
        public string Text { get; set; }

        /// <summary>
        /// Returns string representation 
        /// </summary>
        public override string ToString()
        {
            return $"{{ErrorId: {ErrorId}, Text: {Text}}}";
        }
    }
}
