/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

///	<summary>
///	OmmQos represents Quality Of Service information in OMM.
///	</summary>
/// <remarks>
///	<para>
///	OmmQos is a read only class.</para>
///	
/// <para>
///	This class is used for extraction of OmmQos only.</para>
///	
///	<para>
///	All methods in this class are SingleThreaded.</para>
/// </remarks>
///	<seealso cref="Data"/>
///	<seealso cref="EmaBuffer"/>
///
public sealed class OmmQos : Data
{
    #region Public members

    /// <summary>
    /// Qos rate representation.
    /// </summary>
    public static class Rates
    {

        /// <summary>
        /// Indicates tick by tick rate
        /// </summary>
        public const uint TICK_BY_TICK = 0;

        /// <summary>
        /// Indicates just in time conflated rate
        /// </summary>
        public const uint JUST_IN_TIME_CONFLATED = 0xFFFFFF00;
    }

    /// <summary>
    /// Qos timeliness representation.
    /// </summary>
    public static class Timelinesses
    {

        /// <summary>
        /// Indicates real time timeliness
        /// </summary>
        public const uint REALTIME = 0;

        /// <summary>
        /// Indicates timeliness with an unknown delay value
        /// </summary>
        public const uint INEXACT_DELAYED = 0xFFFFFFFF;
    }

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.QOS; }

    /// <summary>
    /// Returns the QosRate value as a string format.
    /// </summary>
    /// <returns>string representation of this object Rate</returns>
    public string RateAsString()
    {
        m_ToString.Clear();

        switch (m_Qos.Rate())
        {
            case Eta.Codec.QosRates.TICK_BY_TICK:
                m_ToString.Append("TickByTick");
                break;
            case Eta.Codec.QosRates.JIT_CONFLATED:
                m_ToString.Append("JustInTimeConflated");
                break;
            default:
                return m_ToString.Append("Rate: ").Append(m_Qos.RateInfo()).ToString();
        }

        return m_ToString.ToString();
    }

    /// <summary>
    /// Returns the QosTimeliness value as a string format.
    /// </summary>
    /// <returns>string representation of this object timeliness</returns>
    public string TimelinessAsString()
    {
        m_ToString.Clear();

        switch (m_Qos.Timeliness())
        {
            case Eta.Codec.QosTimeliness.REALTIME:
                m_ToString.Append("RealTime");
                break;
            case Eta.Codec.QosTimeliness.DELAYED_UNKNOWN:
                m_ToString.Append("InexactDelayed");
                break;
            default:
                return m_ToString.Append("Timeliness: ").Append(m_Qos.TimeInfo()).ToString();
        }

        return m_ToString.ToString();
    }

    /// <summary>
    /// Gets Timeliness.
    /// </summary>
    public uint Timeliness
    {
        get
        {
            uint timeliness = OmmQos.Timelinesses.INEXACT_DELAYED;

            switch (m_Qos.Timeliness())
            {
                case Eta.Codec.QosTimeliness.REALTIME:
                    timeliness = OmmQos.Timelinesses.REALTIME;
                    break;
                case Eta.Codec.QosTimeliness.DELAYED_UNKNOWN:
                    timeliness = OmmQos.Timelinesses.INEXACT_DELAYED;
                    break;
                case Eta.Codec.QosTimeliness.DELAYED:
                    timeliness = (uint)m_Qos.TimeInfo();
                    break;
                default:
                    break;
            }

            return timeliness;
        }
    }

    /// <summary>
    /// Gets Rate.
    /// </summary>
    public uint Rate
    {
        get
        {
            uint rate = OmmQos.Rates.JUST_IN_TIME_CONFLATED;

            switch (m_Qos.Rate())
            {
                case Eta.Codec.QosRates.TICK_BY_TICK:
                    rate = OmmQos.Rates.TICK_BY_TICK;
                    break;
                case Eta.Codec.QosRates.JIT_CONFLATED:
                    rate = OmmQos.Rates.JUST_IN_TIME_CONFLATED;
                    break;
                case Eta.Codec.QosRates.TIME_CONFLATED:
                    rate = (uint)m_Qos.RateInfo();
                    break;
                default:
                    break;
            }

            return rate;
        }
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmQos"/> instance.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;

        m_ToString.Clear();

        switch (m_Qos.Timeliness())
        {
            case Eta.Codec.QosTimeliness.REALTIME:
                m_ToString.Append("RealTime");
                break;
            case Eta.Codec.QosTimeliness.DELAYED_UNKNOWN:
                m_ToString.Append("InexactDelayed");
                break;
            case Eta.Codec.QosTimeliness.DELAYED:
                m_ToString.Append("Timeliness: ").Append(m_Qos.TimeInfo());
                break;
            default:
                break;
        }

        m_ToString.Append('/');

        switch (m_Qos.Rate())
        {
            case Eta.Codec.QosRates.TICK_BY_TICK:
                m_ToString.Append("TickByTick");
                break;
            case Eta.Codec.QosRates.JIT_CONFLATED:
                m_ToString.Append("JustInTimeConflated");
                break;
            case Eta.Codec.QosRates.TIME_CONFLATED:
                m_ToString.Append("Rate: ").Append(m_Qos.RateInfo());
                break;
            default:
                break;
        }

        return m_ToString.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmQos() { }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        if (CodecReturnCode.SUCCESS == m_Qos.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
            Code = DataCode.BLANK;

        return CodecReturnCode.SUCCESS;
    }

    internal void Decode(Eta.Codec.Qos rsslQos)
    {
        if (rsslQos != null)
        {
            Code = DataCode.NO_CODE;

            m_Qos.Rate(rsslQos.Rate());
            m_Qos.Timeliness(rsslQos.Timeliness());
            m_Qos.IsDynamic = rsslQos.IsDynamic;
            m_Qos.TimeInfo(rsslQos.TimeInfo());
            m_Qos.RateInfo(rsslQos.RateInfo());
        }
        else
        {
            Code = DataCode.BLANK;
            m_Qos.Clear();
        }
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Qos m_Qos = new();

    #endregion

}
