/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// State Stream States provide information on the health of the stream.
	/// </summary>
	/// <seealso cref="State"/>
	sealed public class StreamStates
	{
		// StreamStates class cannot be instantiated
		private StreamStates()
		{
		}

		/// <summary>
		/// Unspecified. Typically used as a initialization value and is not intended
		/// to be encoded or decoded.
		/// </summary>
		public const int UNSPECIFIED = 0;

		/// <summary>
		/// Stream is open. Typically implies that information will be streaming, as
		/// information changes updated information will be sent on the stream, after
		/// final <see cref="IRefreshMsg"/> or <see cref="IStatusMsg"/>.
		/// </summary>
		public const int OPEN = 1;

		/// <summary>
		/// Request was non-streaming. After final <see cref="IRefreshMsg"/> or
		/// <see cref="IStatusMsg"/> is received, the stream will be closed and no updated
		/// information will be delivered without subsequent re-request.
		/// </summary>
		public const int NON_STREAMING = 2;

		/// <summary>
		/// Closed, the applications may attempt to re-open the stream later. Can
		/// occur via either a <see cref="IRefreshMsg"/> or a <see cref="IStatusMsg"/>.
		/// </summary>
		public const int CLOSED_RECOVER = 3;

		/// <summary>
		/// Closed. Indicates that the data is not available on this
		/// service/connection and is not likely to become available.
		/// </summary>
		public const int CLOSED = 4;

		/// <summary>
		/// Closed and Redirected. Indicates that the current stream has been closed
		/// and has new identifying information, the user can issue a new request for
		/// the data using the new message key information contained in the redirect message.
		/// </summary>
		public const int REDIRECTED = 5;

		/// <summary>
		/// Provide string representation for a stream state value.
		/// </summary>
		/// <param name="streamState"> <see cref="StreamStates"/> enumeration to convert to string
		/// </param>
		/// <returns> string representation for a <paramref name="streamState"/> value
		/// </returns>
		public static string ToString(int streamState)
		{
			switch (streamState)
			{
				case UNSPECIFIED:
					return "UNSPECIFIED";
				case OPEN:
					return "OPEN";
				case NON_STREAMING:
					return "NON_STREAMING";
				case CLOSED_RECOVER:
					return "CLOSED_RECOVER";
				case CLOSED:
					return "CLOSED";
				case REDIRECTED:
					return "REDIRECTED";
				default:
					return Convert.ToString(streamState);
			}
		}

		/// <summary>
		/// Provide string representation of meaning associated with stream state.
		/// </summary>
		/// <param name="streamState"> <see cref="StreamStates"/> enumeration to get info for
		/// </param>
		/// <returns> string representation of meaning associated with <paramref name="streamState"/> value
		/// </returns>
		public static string Info(int streamState)
		{
			switch (streamState)
			{
				case StreamStates.UNSPECIFIED:
					return "Unspecified";
				case StreamStates.OPEN:
					return "Open";
				case StreamStates.NON_STREAMING:
					return "Non-streaming";
				case StreamStates.CLOSED_RECOVER:
					return "Closed, Recoverable";
				case StreamStates.CLOSED:
					return "Closed";
				case StreamStates.REDIRECTED:
					return "Redirected";
				default:
					return "Unknown Stream State";
			}
		}
	}

}