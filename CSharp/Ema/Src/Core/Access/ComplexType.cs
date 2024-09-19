/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023, 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// ComplexType represents all OMM Data constructs set-able as message payload.
    /// </summary>
    public abstract class ComplexType : Data
    {
        internal Eta.Codec.DataDictionary? m_dataDictionary;

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

        /// <summary>
        /// Obtains string representation of the current ComplexType
        /// </summary>
        /// <param name="dataDictionary"><see cref="Ema.Rdm.DataDictionary"/> object used to obtain string representaiton
        /// of the complex type object</param>
        /// <returns><see cref="string"/> object representing current ComplexType instance</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual string ToString(Rdm.DataDictionary dataDictionary)
        {
            if (!dataDictionary.IsEnumTypeDefLoaded || !dataDictionary.IsFieldDictionaryLoaded)
            {
                return "\nThe provided DataDictionary is not properly loaded.";
            }

            if (Encoder != null && Encoder.m_encodeIterator != null && Encoder.m_containerComplete)
            {
                var encodedBuffer = Encoder.m_encodeIterator.Buffer();

                var tmpObject = m_objectManager.GetComplexTypeFromPool(this.DataType);
                if (tmpObject == null)
                {
                    return $"\nToString(DataDictionary) is called on an invalid DataType {DataType}";
                }

                string result = string.Empty;
                try
                {
                    CodecReturnCode ret;
                    if ((ret = tmpObject!.Decode(Codec.MajorVersion(), Codec.MinorVersion(), encodedBuffer, dataDictionary.rsslDataDictionary(), null)) != CodecReturnCode.SUCCESS)
                    {
                        return $"\nFailed to decode ComplexType of type {DataType}: {ret.GetAsString()}";
                    }
                    result = tmpObject!.ToString()!;
                }
                finally
                {
                    tmpObject!.ClearAndReturnToPool_All();
                }
                return result;
            }
            else
            {
                return $"\nComplexType instance of type {DataType} contains no valid encoded data.";
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override string ToString(int indent)
        {
            if (m_hasDecodedDataSet)
            {
                return FillString(indent);
            }

            return $"\n{GetType().Name}.ToString() method could not be used for just encoded object. Use ToString(dictionary) for just encoded object.";
        }

        internal abstract string FillString(int indent);
    }
}