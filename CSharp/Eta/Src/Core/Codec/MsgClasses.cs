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
	/// Identifies the specific type of a message (For example, <see cref="IUpdateMsg"/>, <see cref="IRequestMsg"/> etc).
	/// </summary>
	/// <seealso cref="IMsg"/>
	sealed public class MsgClasses
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private MsgClasses()
		{
            throw new System.NotImplementedException();
        }

		/// <summary>
		/// Consumers use <see cref="IRequestMsg"/> to express interest in a new stream or
		/// modify some parameters on an existing stream; typically results in the
		/// delivery of a <see cref="IRefreshMsg"/> or <see cref="IStatusMsg"/>.
		/// </summary>
		public const int REQUEST = 1;

		/// <summary>
		/// The Interactive Provider can use this class to respond to a consumer's
		/// request for information (solicited) or provide a data resynchronization
		/// point (unsolicited). The NIP can use this class to initiate a data flow
		/// on a new item stream. Conveys state information, QoS, stream
		/// permissioning information, and group information in addition to payload.
		/// </summary>
		public const int REFRESH = 2;

		/// <summary>
		/// Indicates changes to the stream or data properties. A provider uses
		/// <see cref="IStatusMsg"/> to close streams and to indicate successful
		/// establishment of a stream when there is no data to convey.
		/// This message can indicate changes:
		/// <ul>
		/// <li>In streamState or dataState</li>
		/// <li>In a stream's permissioning information</li>
		/// <li>To the item group to which the stream belongs</li>
		/// </ul>
		/// </summary>
		public const int STATUS = 3;

		/// <summary>
		/// Interactive or NIPs use <see cref="IUpdateMsg"/> to convey changes to information on a stream.
		/// Update messages typically flow on a stream after delivery of a refresh
		/// </summary>
		public const int UPDATE = 4;

		/// <summary>
		/// A consumer uses <see cref="ICloseMsg"/> to indicate no further interest in a
		/// stream. As a result, the stream should be closed.
		/// </summary>
		public const int CLOSE = 5;

		/// <summary>
		/// A provider uses <see cref="IAckMsg"/> to inform a consumer of success or failure
		/// for a specific <see cref="IPostMsg"/> or <see cref="ICloseMsg"/>.
		/// </summary>
		public const int ACK = 6;

		/// <summary>
		/// A bi-directional message that does not have any implicit interaction
		/// semantics associated with it, thus the name generic. After a stream is
		/// established via a request-refresh/status interaction:
		/// <ul>
		/// <li>A consumer can send this message to a provider.</li>
		/// <li>A provider can send this message to a consumer.</li>
		/// <li>NIPs can send this message to the ADH/</li>
		/// </ul>
		/// </summary>
		public const int GENERIC = 7;

		/// <summary>
		/// A consumer uses <see cref="IPostMsg"/> to push content upstream. This information
		/// can be applied to an Enterprise Platform cache or routed further upstream
		/// to a data source. After receiving posted data, upstream components can
		/// republish it to downstream consumers.
		/// </summary>
		public const int POST = 8;

		/// <summary>
		/// String representation of a message class.
		/// </summary>
		/// <param name="msgClass"> message class
		/// </param>
		/// <returns> the string representation of a message class
		/// </returns>
		public static string ToString(int msgClass)
		{
			string ret = "";

			switch (msgClass)
			{
				case UPDATE:
					ret = "UPDATE";
					break;
				case GENERIC:
					ret = "GENERIC";
					break;
				case REFRESH:
					ret = "REFRESH";
					break;
				case REQUEST:
					ret = "REQUEST";
					break;
				case POST:
					ret = "POST";
					break;
				case STATUS:
					ret = "STATUS";
					break;
				case CLOSE:
					ret = "CLOSE";
					break;
				case ACK:
					ret = "ACK";
					break;
				default:
					ret = Convert.ToString(msgClass);
					break;
			}

			return ret;
		}
	}
}