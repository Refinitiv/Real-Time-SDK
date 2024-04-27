/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.PerfTools.Common
{
    public class QosWrapper
    {
        private int m_rate;
        private int m_rateInfo;
        private int m_timeliness;
        private int m_timeInfo;
        private bool m_dynamic;

        public void Rate(string? value)
        {
            m_rate = QosRateValue(value);
        }

        public void RateInfo(string? value)
        {
            if (!int.TryParse(value, out m_rateInfo))
            {
                throw new Exception($"Failed to parse RateInfo from the following string: {value}");
            }
        }

        public void Timeliness(string? value)
        {
            m_timeliness = QosTimelinessValue(value);
        }

        public void TimeInfo(string? value)
        {
            if (!int.TryParse(value, out m_timeInfo))
            {
                throw new Exception($"Failed to parse TimeInfo from the following string: {value}");
            }
        }

        public void Dynamic(string? value)
        {
            if (!int.TryParse(value, out var res))
            {
                throw new Exception($"Failed to parse Dynamic from the following string: {value}");
            }
            m_dynamic = res > 0;
        }

        public int Rate()
        {
            return m_rate;
        }

        public int RateInfo()
        {
            return m_rateInfo;
        }

        public int Timeliness()
        {
            return m_timeliness;
        }

        public int TimeInfo()
        {
            return m_timeInfo;
        }

        public bool Dynamic()
        {
            return m_dynamic;
        }

        public void Clear()
        {
            m_rate = QosTimeliness.UNSPECIFIED;
            m_rateInfo = 0;
            m_timeliness = QosTimeliness.UNSPECIFIED;
            m_timeInfo = 0;
            m_dynamic = false;
        }

        /// <summary>
        /// Converts qos timeliness string to qos timeliness value.
        /// </summary>
        /// <param name="value">string value</param>
        /// <returns>resulting int value</returns>
        private int QosTimelinessValue(string? value)
        {
            int retVal = 0;

            if (value == null || value.Equals("RSSL_QOS_TIME_UNSPECIFIED"))
            {
                retVal = QosTimeliness.UNSPECIFIED;
            }
            else if (value.Equals("RSSL_QOS_TIME_REALTIME"))
            {
                retVal = QosTimeliness.REALTIME;
            }
            else if (value.Equals("RSSL_QOS_TIME_DELAYED_UNKNOWN"))
            {
                retVal = QosTimeliness.DELAYED_UNKNOWN;
            }
            else if (value.Equals("RSSL_QOS_TIME_DELAYED"))
            {
                retVal = QosTimeliness.DELAYED;
            }

            return retVal;
        }

        /// <summary>
        /// Converts qos m_rate string to qos m_rate value.
        /// </summary>
        /// <param name="value">string value</param>
        /// <returns>resulting int value</returns>
        private int QosRateValue(string? value)
        {
            int retVal = 0;

            if (value == null ||value.Equals("RSSL_QOS_RATE_UNSPECIFIED"))
            {
                retVal = QosRates.UNSPECIFIED;
            }
            else if (value.Equals("RSSL_QOS_RATE_TICK_BY_TICK"))
            {
                retVal = QosRates.TICK_BY_TICK;
            }
            else if (value.Equals("RSSL_QOS_RATE_JIT_CONFLATED"))
            {
                retVal = QosRates.JIT_CONFLATED;
            }
            else if (value.Equals("RSSL_QOS_RATE_TIME_CONFLATED"))
            {
                retVal = QosRates.TIME_CONFLATED;
            }

            return retVal;
        }
    }
}
