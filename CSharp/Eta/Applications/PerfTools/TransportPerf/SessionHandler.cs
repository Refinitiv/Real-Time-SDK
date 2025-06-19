/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Threading;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;

namespace LSEG.Eta.PerfTools.TransportPerf
{
	/// <summary>
	/// High level handler for multiple transport session threads.
	/// Enables a transport thread to handle multiple channels.
	/// </summary>
	public class SessionHandler
    {
		/// <summary>
		/// Gets Handler lock.
		/// </summary>
		public ReaderWriterLockSlim HandlerLock { get; private set; } =
			new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

		/// <summary>
		/// Gets new channels accepted from the main thread.
		/// </summary>
		public List<IChannel> NewChannelsList { get; private set; } = new List<IChannel>(10000);

		/// <summary>
		/// Gets or sets total number of channels this thread is currently handling.
		/// </summary>
		public int OpenChannelsCount { get; set; }

		/// <summary>
		/// Gets or sets whether connections are active.
		/// </summary>
		public bool Active { get; set; }

		/// <summary>
		/// Gets latency records. Updated from transport thread.
		/// </summary>
		public TimeRecordQueue LatencyRecords { get; private set; } = new TimeRecordQueue();

		/// <summary>
		/// Gets or sets a thread associated with this handler.
		/// </summary>
		public TransportThread? TransportThread { get; set; }
		
		/// <summary>
		/// Gets or sets a role of this handler.
		/// </summary>
		public TransportTestRole Role { get; set; }

		/// <summary>
		/// Initializes the handler for a transport perf thread.
		/// </summary>
		public void Init()
		{
			Active = false;
			Role = TransportTestRole.READER | TransportTestRole.WRITER;
		}
	}
}
