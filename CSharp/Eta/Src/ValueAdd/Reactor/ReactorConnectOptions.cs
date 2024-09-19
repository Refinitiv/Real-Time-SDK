/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorConnectOptions to be used in the <see cref="Reactor.Connect(ReactorConnectOptions, ReactorRole,
    /// out ReactorErrorInfo?)"/> call.
    /// </summary>
    sealed public class ReactorConnectOptions
    {
        private const int DEFAULT_DELAY = 1000;

        private int m_ReconnectMaxDelay;
        private int m_ReconnectMinDelay;
        private int m_ReconnectAttemptLimit;
        /// <summary>
        /// Get a list of connections. Add at least one connection to the list.
        /// Additional connections are used as backups for fail-over in case
        /// the first connection is lost. Each connection in the list will be
        /// tried with each reconnection attempt.
        /// </summary>
        /// <value>the list of <see cref="ReactorConnectInfo"/></value>
        public List<ReactorConnectInfo> ConnectionList { get; private set; }

        /// <summary>
        /// Default constructor.
        /// </summary>
        public ReactorConnectOptions()
        {
            ConnectionList = new List<ReactorConnectInfo>(10);
            m_ReconnectMaxDelay = DEFAULT_DELAY;
            m_ReconnectMinDelay = DEFAULT_DELAY;
            m_ReconnectAttemptLimit = 0;
        }

        /// <summary>
        /// Returns the reconnectAttemptLimit value.
        /// </summary>
        /// <returns>the reconnectAttemptLimit value</returns>
        public int GetReconnectAttemptLimit()
        {
            return m_ReconnectAttemptLimit;
        }

        /// <summary>
        /// Sets the maximum number of times the Reactor will attempt to
        /// reconnect a channel. If set to -1, there is no limit.
        /// Must be in the range of -1 to <see cref="int.MaxValue"/>. Default is 0.
        /// </summary>
        /// <param name="limit">the maximum number of reconnect attempts</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if the limit is valid,
        /// otherwise <see cref="ReactorReturnCode.PARAMETER_OUT_OF_RANGE"/></returns>
        public ReactorReturnCode SetReconnectAttempLimit(int limit)
        {
            if (limit < -1)
                return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;

            m_ReconnectAttemptLimit = limit;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns the reconnectMinDelay value.
        /// </summary>
        /// <returns>the reconnectMinDelay value</returns>
        public int GetReconnectMinDelay()
        {
            return m_ReconnectMinDelay;
        }

        /// <summary>
        /// Sets the minimum time the Reactor will wait before attempting
        /// to reconnect, in milliseconds. Must be in the range of 1000 - 
        /// <see cref="int.MaxValue"/>. Default is 1000. Default value is 
        /// used if value is greater than or equal to zero and less than 1000.
        /// </summary>
        /// <param name="delay">the minimum reconnect delay</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if the limit is valid,
        /// otherwise <see cref="ReactorReturnCode.PARAMETER_OUT_OF_RANGE"/></returns>
        public ReactorReturnCode SetReconnectMinDelay(int delay)
        {
            if (delay < 0)
                return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;

            if (delay < DEFAULT_DELAY)
                m_ReconnectMinDelay = DEFAULT_DELAY;
            else
                m_ReconnectMinDelay = delay;

            // make sure m_ReconnectMaxDelay is at least m_ReconnectMinDelay
            if (m_ReconnectMaxDelay < m_ReconnectMinDelay)
                m_ReconnectMaxDelay = m_ReconnectMinDelay;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns the reconnectMaxDelay value.
        /// </summary>
        /// <returns>the reconnectMaxDelay value</returns>
        public int GetReconnectMaxDelay()
        {
            return m_ReconnectMaxDelay;
        }

        /// <summary>
        /// Sets the maximum time the Reactor will wait before attempting
        /// to reconnect, in milliseconds. Must be in the range of 1000 - 
        /// <see cref="int.MaxValue"/> and must be greater than or equal to
        /// reconnectMinDelay. Default is 1000. reconnectMinDelay value
        /// is used if value is less than reconnectMinDelay.
        /// </summary>
        /// <param name="delay">the maximum reconnect delay</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if the limit is valid,
        /// otherwise <see cref="ReactorReturnCode.PARAMETER_OUT_OF_RANGE"/></returns>
        public ReactorReturnCode SetReconnectMaxDelay(int delay)
        {
            if (delay < 0 || delay < m_ReconnectMinDelay)
                return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;

            if (m_ReconnectMaxDelay < m_ReconnectMinDelay)
                m_ReconnectMaxDelay = m_ReconnectMinDelay;
            else
                m_ReconnectMaxDelay = delay;

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Clears this object for reuse.
        /// </summary>
        public void Clear()
        {
            ConnectionList.Clear();
            m_ReconnectAttemptLimit = 0;
            m_ReconnectMinDelay = DEFAULT_DELAY;
            m_ReconnectMaxDelay = DEFAULT_DELAY;
        }

        /// <summary>
        /// This method will perform a deep copy into the passed in parameter's
        /// members from the Object calling this method.
        /// </summary>
        /// <param name="destOpts">the value getting populated with the values of the calling Object</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> on success,
        /// <see cref="ReactorReturnCode.FAILURE"/> if destOpts is null.
        /// </returns>
        public ReactorReturnCode Copy(ReactorConnectOptions destOpts)
        {
            if (destOpts == null)
                return ReactorReturnCode.FAILURE;

            destOpts.ConnectionList.Clear();
            for (int i = 0; i < ConnectionList.Count; ++i)
            {
                ReactorConnectInfo reactorConnectInfo = new ReactorConnectInfo();
                ConnectionList[i].Copy(reactorConnectInfo);
                destOpts.ConnectionList.Add(reactorConnectInfo);
            }

            destOpts.m_ReconnectAttemptLimit = m_ReconnectAttemptLimit;
            destOpts.m_ReconnectMinDelay = m_ReconnectMinDelay;
            destOpts.m_ReconnectMaxDelay = m_ReconnectMaxDelay;
            return ReactorReturnCode.SUCCESS;
        }
    }
}
