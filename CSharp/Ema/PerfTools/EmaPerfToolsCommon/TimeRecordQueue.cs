/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Stores a queue and pool of TimeRecord objects. This class along with
    /// <see cref="TimeRecord"/> are used to collect individual time differences
    /// for statistical calculation in a thread-safe manner -- one thread can
    /// store information by adding to the records queue and another can retrieve
    /// the information from the records queue and do any desired calculation.
    /// </summary>
    public class TimeRecordQueue
    {
        public class SimpleConcurrentQueue
        {
            private TimeRecord[] m_freeRecords;
            private TimeRecord[] m_submittedRecords;

            private int m_freeRecCurrent;
            private int m_submittedRecCurrent = -1;

            private object m_freeRecLock = new object();
            private object m_submittedRecLock = new object();

            public SimpleConcurrentQueue(int size)
            {
                m_freeRecords = new TimeRecord[size];
                for (int i = 0; i < m_freeRecords.Length; i++)
                {
                    m_freeRecords[i] = new TimeRecord();
                }

                m_submittedRecords = new TimeRecord[size];
            }

            public TimeRecord GetFreeTimeRecord()
            {
                TimeRecord res;
                Monitor.Enter(m_freeRecLock);
                if (m_freeRecCurrent < m_freeRecords.Length)
                {
                    res = m_freeRecords[m_freeRecCurrent++];
                }
                else
                {
                    m_freeRecords = new TimeRecord[(int)(m_freeRecords.Length * 1.25)]; 
                    for (int i = m_freeRecCurrent; i < m_freeRecords.Length; i++)
                    {
                        m_freeRecords[i] = new TimeRecord();
                    }
                    res = m_freeRecords[m_freeRecCurrent++];
                }
                Monitor.Exit(m_freeRecLock);
                return res;
            }

            public void ReturnTimeRecord(TimeRecord rec)
            {
                Monitor.Enter(m_freeRecLock);
                if (m_freeRecCurrent != 0)
                {
                    m_freeRecords[--m_freeRecCurrent] = rec;
                }
                Monitor.Exit(m_freeRecLock);
            }

            public void SubmitTimeRecord(long startTime, long endTime, long ticks)
            {
                Monitor.Enter(m_freeRecLock);
                TimeRecord res;
                if (m_freeRecCurrent < m_freeRecords.Length)
                {
                    res = m_freeRecords[m_freeRecCurrent++];
                }
                else
                {
                    m_freeRecords = new TimeRecord[m_freeRecords.Length + 1000];
                    for (int i = m_freeRecCurrent; i < m_freeRecords.Length; i++)
                    {
                        m_freeRecords[i] = new TimeRecord();
                    }
                    res = m_freeRecords[m_freeRecCurrent++];
                }
                res.StartTime = startTime;
                res.EndTime = endTime;
                res.Ticks = ticks;
                Monitor.Exit(m_freeRecLock);

                Monitor.Enter(m_submittedRecLock);
                if (m_submittedRecCurrent < m_submittedRecords.Length + 1)
                {
                    m_submittedRecords[++m_submittedRecCurrent] = res;
                }
                else
                {
                    m_submittedRecords = new TimeRecord[m_submittedRecords.Length + 1000];
                    m_submittedRecords[++m_submittedRecCurrent] = res;
                }
                Monitor.Exit(m_submittedRecLock);
            }

            public TimeRecord? GetNextSubmittedTimeRecord()
            {
                TimeRecord? res = null;
                Monitor.Enter(m_submittedRecLock);
                if (m_submittedRecCurrent >= 0)
                {
                    res = m_submittedRecords[m_submittedRecCurrent--];
                }
                Monitor.Exit(m_submittedRecLock);
                return res;
            }
        }

        public SimpleConcurrentQueue RecordTracker;

        public TimeRecordQueue()
        {
            RecordTracker = new SimpleConcurrentQueue(10000);
        }

        /// <summary>
        /// Cleans up TimeRecordQueue.
        /// </summary>
        public void Cleanup()
        {
            
        }
    }
}
