/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// Many of the ETA Codec Package methods take a parameter for returning
    /// detailed error information.
    /// </summary>
    ///
    /// <remarks>
    /// This Error object is only populated in the event of an error condition
    /// and should only be inspected when a specific failure code is returned
    /// from the method itself.
    /// </remarks>
    public sealed class CodecError
    {
        /// <summary>
		/// A ETA specific return code, used to specify the error that has occurred.
		/// See the following sections for specific error conditions that may arise.
		/// </summary>
		/// <value> the errorId </value>
        public CodecReturnCode ErrorId { get; set; }

        /// <summary>
		/// Detailed text describing the error that has occurred.
		/// </summary>
		/// <value> the text </value>
        public string Text { get; set; }

        /// <summary>
        /// Returns string representation 
        /// </summary>
        /// <returns>string representation of this error.
        /// </returns>
        public override string ToString()
        {
            return $"{{ErrorId: {ErrorId}, Text: {Text}}}";
        }
    }
}
