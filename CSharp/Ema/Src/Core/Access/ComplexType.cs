/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// ComplexType represents all OMM Data constructs set-able as message payload.
    /// </summary>
    public abstract class ComplexType : Data
    {
        internal void SetEtaRate(uint rate, Qos qos)
        {
            switch (rate)
            {
                case OmmQos.Rates.TICK_BY_TICK:
                    qos.Rate(QosRates.TICK_BY_TICK);
                    break;
                case OmmQos.Rates.JUST_IN_TIME_CONFLATED:
                    qos.Rate(QosRates.JIT_CONFLATED);
                    break;
                default:
                    if (rate <= 65535)
                    {
                        qos.Rate(QosRates.TIME_CONFLATED);
                        qos.RateInfo((int)rate);
                    }
                    else
                    {
                        qos.Rate(QosRates.JIT_CONFLATED);
                    }
                    break;
            }
        }

        internal void SetEtaTimeliness(uint timeliness, Qos qos)
        {
            switch (timeliness)
            {
                case OmmQos.Timelinesses.REALTIME:
                    qos.Timeliness(QosTimeliness.REALTIME);
                    break;
                case OmmQos.Timelinesses.INEXACT_DELAYED:
                    qos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                    break;
                default:
                    if (timeliness <= 65535)
                    {
                        qos.Timeliness(QosTimeliness.DELAYED);
                        qos.TimeInfo((int)timeliness);
                    }
                    else
                    {
                        qos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                    }
                    break;
            }
        }
    }
}
