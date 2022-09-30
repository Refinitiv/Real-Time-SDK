/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Transports.Interfaces;
using System.IO;

namespace Refinitiv.Eta.PerfTools.Common
{
	/// <summary>
	/// Contains information associated with a ProviderThread
	/// </summary>
	public class ProviderThreadInfo
    {
		/// <summary>
		/// ID saved from thread creation
		/// </summary>
		public long ThreadId { get; set; }
		/// <summary>
		/// ETA Channel
		/// </summary>
		public IChannel? Channel { get; set; }
		/// <summary>
		/// Queue of timestamp information(for generic messages), collected periodically by the main thread
		/// </summary>
		public TimeRecordQueue GenMsgLatencyRecords { get; set; } = new TimeRecordQueue();
		/// <summary>
		/// Index into the item list at which item requests unique to this consumer start
		/// </summary>
		public int ItemListUniqueIndex { get; set; }
		/// <summary>
		/// Number of item requests to make
		/// </summary>
		public int ItemListCount { get; set; }
		/// <summary>
		/// The <see cref="DataDictionary"/> instance
		/// </summary>
		public DataDictionary? Dictionary { get; set; }
		/// <summary>
		/// Other stats, collected periodically by the main thread
		/// </summary>
		public ProviderThreadStats Stats { get; set; } = new ProviderThreadStats();
		/// <summary>
		/// File for logging stats for this connection
		/// </summary>
		public string? StatsFile { get; set; }
		/// <summary>
		/// File writer for logging stats for this connection
		/// </summary>
		public StreamWriter? StatsFileWriter { get; set; }
		/// <summary>
		/// File for logging latency for this connection
		/// </summary>
		public string? LatencyLogFile { get; set; }
		/// <summary>
		/// File writer for logging latency for this connection
		/// </summary>
		public StreamWriter? LatencyLogFileWriter { get; set; }
		/// <summary>
		/// Signals thread to shutdown
		/// </summary>
		public volatile bool Shutdown;
		/// <summary>
		/// Acknowledges thread is shutdown
		/// </summary>
		public volatile bool ShutdownAck;

		/// <summary>
		/// Submit a time record
		/// </summary>
		/// <param name="recordQueue">the record queue</param>
		/// <param name="startTime">the start time</param>
		/// <param name="endTime">the end timeparam>
		/// <param name="ticks">the number of ticks</param>
		/// <returns><see cref="CodecReturnCode"/> value indicating the status of hte operation</returns>
		public CodecReturnCode TimeRecordSubmit(TimeRecordQueue recordQueue, long startTime, long endTime, long ticks)
		{
			TimeRecord? record;
			recordQueue.Pools.TryDequeue(out record);
			if (record == null)
			{
				record = new TimeRecord();
			}
			record.Ticks = ticks;
			record.StartTime = startTime;
			record.EndTime = endTime;

			recordQueue.Records.Enqueue(record);

			return CodecReturnCode.SUCCESS;
		}
		public void Cleanup()
		{
			GenMsgLatencyRecords.Cleanup();
			if (StatsFileWriter != null)
				StatsFileWriter.Close();
			if (LatencyLogFileWriter != null)
				LatencyLogFileWriter.Close();
		}
	}
}
