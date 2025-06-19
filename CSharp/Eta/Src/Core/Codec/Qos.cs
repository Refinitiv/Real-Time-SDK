/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// ETA Quality of Service class contains information rate and/or timeliness
	/// information. Timeliness conveys information about the age of data. Rate
	/// conveys information about the data's period of change. Some timeliness or
	/// rate values may allow for additional time or rate information to be provided.
	/// </summary>
	///
	/// <remarks>
	/// <para>
	/// A consumer can use <see cref="IRequestMsg"/> to indicate the desired QoS for its
	/// streams. This can be a request for a specific QoS or a range of qualities of
	/// service, where any value within the range will satisfy the request. The
	/// <see cref="IRefreshMsg"/> includes QoS used to indicate the QoS being provided for a
	/// stream. When issuing a request, the QoS specified on the request typically
	/// matches the advertised QoS of the service, as conveyed via the Source
	/// Directory domain model.</para>
	/// 
	/// <para>
	/// <ul>
	/// <li>
	/// An initial request containing only <see cref="IRequestMsg.Qos"/> indicates a
	/// request for the specified QoS. If a provider cannot satisfy this QoS, the
	/// request should be rejected.</li>
	/// <li>An initial request containing both <see cref="IRequestMsg.Qos"/> and
	/// <see cref="IRequestMsg.WorstQos"/> sets the range of acceptable QoSs.</li>
	/// </ul>
	/// </para>
	/// <para>
	/// Any QoS within the range, inclusive of the specified qos and worstQos, will
	/// satisfy the request. If a provider cannot provide a QoS within the range, the
	/// provider should reject the request. When a provider responds to an initial
	/// request, the <see cref="IRefreshMsg.Qos"/> should contain the actual QoS being
	/// provided to the stream. Subsequent requests should not specify a range as the
	/// QoS has been established for the stream. Because QoS information is mostly
	/// optional on a <see cref="IRequestMsg"/> (Some components may require qos on initial
	/// request and reissue messages):
	/// </para>
    ///
	/// <list type="bullet">
	/// <item>If neither qos nor worstQos are specified on an initial request to open a
	/// stream, it is assumed that any QoS will satisfy the request. Additionally, it
	/// will have a timeliness of <see cref="QosTimeliness.REALTIME"/> and a rate of
	/// <see cref="QosRates.TICK_BY_TICK"/></item>
	/// <item>If QoS information is absent on a subsequent reissue request, it is
	/// assumed that QoS, timeliness, and rate conform to the stream's currently
	/// established settings</item>
	/// </list>.
	/// 
	/// </remarks>
	/// <seealso cref="QosRates"/>
	/// <seealso cref="QosTimeliness"/>
	sealed public class Qos : IEquatable<Qos>
	{
		internal int _timeliness;
		internal int _rate;
		internal bool _dynamic;
		internal int _timeInfo;
		internal int _rateInfo;
		internal readonly int SEED = 23;
		internal readonly int PRIME = 31;

        /// <summary>
		/// Creates <see cref="Qos"/>.
		/// </summary>
		/// <seealso cref="Qos"/>
        public Qos()
        {
        }

		/// <summary>
		/// Clears <see cref="Qos"/>. Useful for object reuse.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_timeliness = QosTimeliness.UNSPECIFIED;
			_rate = QosRates.UNSPECIFIED;
			_dynamic = false;
			_timeInfo = 0;
			_rateInfo = 0;
		}

        /// <summary>
        /// Is Qos blank.
        /// </summary>
        public bool IsBlank { get; internal set; }

		/// <summary>
		/// This method will perform a deep copy into the passed in parameter's 
		/// members from the Object calling this method.
		/// </summary>
		/// <param name="destQos"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destQos"/> is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Copy(Qos destQos)
		{
			if (null == destQos)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

            destQos._timeliness = _timeliness;
            destQos._rate = _rate;
            destQos._dynamic = _dynamic;
            destQos._timeInfo = _timeInfo;
            destQos._rateInfo = _rateInfo;
            destQos.IsBlank = IsBlank;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Used to encode Qos into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into. Iterator
		///            should also have appropriate version information set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			if (!IsBlank)
			{
				return Encoders.PrimitiveEncoder.EncodeQos((EncodeIterator)iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

		/// <summary>
		/// Decode Qos.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if success,
		///         <c>CodecReturnCode.INCOMPLETE_DATA</c> if failure,
		///         <c>CodecReturnCode.BLANK_DATA</c> if data is blank value.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeQos(iter, this);
		}

        /// <summary>
		/// Provide string representation for a <see cref="Qos"/> value.
		/// </summary>
		/// <returns> string representation for a <see cref="Qos"/> value
		/// </returns>
		public override string ToString()
		{
			return "Qos: " + QosTimeliness.ToString(_timeliness) + "/" + QosRates.ToString(_rate) + "/" + ((IsDynamic) ? "Dynamic" : "Static") + " - timeInfo: " + TimeInfo() + " - rateInfo: " + RateInfo();
		}

        /// <summary>
		/// Checks if the two Qos values are equal.
		/// </summary>
		/// <param name="thatQos"> the other Qos to compare to this one
		/// </param>
		/// <returns> <c>true</c> if the two Qos values are equal, <c>false</c> otherwise
		/// </returns>
		public bool Equals(Qos thatQos)
		{
			return ((thatQos != null) && (_rate == thatQos._rate) && (_timeliness == thatQos._timeliness) && (_rateInfo == thatQos._rateInfo) && (_timeInfo == thatQos._timeInfo));
		}

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.
        /// </param>
        /// <returns> <c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        ///       otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Qos)
                return Equals((Qos)other);

            return false;
        }

		/// <summary>
		/// Checks if Qos is better than another Qos.
		/// </summary>
		/// <param name="thatQos"> the other Qos to compare to this one
		/// </param>
		/// <returns> <c>true</c> if Qos is better, <c>false</c> otherwise
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool IsBetter(Qos thatQos)
		{
			// make UNSPECIFIED qos rate/timeliness worse than others
			int unspecifiedValue = 65535 * 2 + 3;

			int newAdjustedRate = AdjustRateQos(unspecifiedValue);
			int oldAdjustedRate = thatQos.AdjustRateQos(unspecifiedValue);

			int newAdjustedTimeliness = AdjustTimeQos(unspecifiedValue);
			int oldAdjustedTimeliness = thatQos.AdjustTimeQos(unspecifiedValue);

			if (newAdjustedTimeliness < oldAdjustedTimeliness)
			{
				return true;
			}
			if (newAdjustedTimeliness > oldAdjustedTimeliness)
			{
				return false;
			}

			if (newAdjustedRate < oldAdjustedRate)
			{
				return true;
			}

			return false;
		}

		/// <summary>
		/// Checks if Qos is in the range of best and worse.
		/// </summary>
		/// <param name="bestQos"> the best Qos to compare to this one </param>
		/// <param name="worstQos"> the worst Qos to compare to this one
		/// </param>
		/// <returns> <c>true</c> if Qos is in range, <c>false</c> otherwise
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool IsInRange(Qos bestQos, Qos worstQos)
		{
			// make UNSPECIFIED worstQos rate/timeliness worse than others
			int unspecifiedValueWorst = 65535 * 2 + 3;
			// make UNSPECIFIED bestQos rate/timeliness better than others
			int unspecifiedValueBest = -1;

			int bestAdjustedRate = bestQos.AdjustRateQos(unspecifiedValueBest);
			int worstAdjustedRate = worstQos.AdjustRateQos(unspecifiedValueWorst);
			int qosAdjustedRate = AdjustRateQos(unspecifiedValueBest);
			int bestAdjustedTimeliness = bestQos.AdjustTimeQos(unspecifiedValueBest);
			int worstAdjustedTimeliness = worstQos.AdjustTimeQos(unspecifiedValueWorst);
			int qosAdjustedTimeliness = AdjustTimeQos(unspecifiedValueBest);

			// in range if best and worst qos rate/timeliness UNSPECIFIED
			if (  bestQos._rate == QosRates.UNSPECIFIED && bestQos._timeliness == QosTimeliness.UNSPECIFIED && worstQos._rate == QosRates.UNSPECIFIED && worstQos._timeliness == QosTimeliness.UNSPECIFIED)
			{
				return true;
			}

			// in range if best and worst qos and this qos are all equal
			if (bestQos.Equals(worstQos))
			{
				return bestQos.Equals(this);
			}

			// out of range if this qos rate is UNSPECIFIED but best or worst qos rate is not
			if (_rate == QosRates.UNSPECIFIED)
			{
				if ((bestQos._rate != QosRates.UNSPECIFIED) || ( worstQos._rate != QosRates.UNSPECIFIED))
				{
					return false;
				}
			}

			// out of range if this qos timeliness is UNSPECIFIED but best or worst qos timeliness is not
			if (_timeliness == QosTimeliness.UNSPECIFIED)
			{
				if ((bestQos._timeliness != QosTimeliness.UNSPECIFIED) || (  worstQos._timeliness != QosTimeliness.UNSPECIFIED))
				{
					return false;
				}
			}

			return ((bestAdjustedRate <= qosAdjustedRate) && (worstAdjustedRate >= qosAdjustedRate) && (bestAdjustedTimeliness <= qosAdjustedTimeliness) && (worstAdjustedTimeliness >= qosAdjustedTimeliness));
		}

		/// <summary>
		/// Information timeliness enum, from <see cref="QosTimeliness"/>. Used to convey
		/// information about the age of the data. Must be in the range of 0 - 7.
		/// </summary>
		/// <param name="timeliness"> the timeliness to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="timeliness"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Timeliness(int timeliness)
		{
			if (!(timeliness >= 0 && timeliness <= 7))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_timeliness = timeliness;
            IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Information timeliness enum, from <see cref="QosTimeliness"/>. Used to convey
		/// information about the age of the data.
		/// </summary>
		/// <returns> the timeliness
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int Timeliness()
		{
			return _timeliness;
		}

		/// <summary>
		/// Information rate enum, from <see cref="QosRates"/>. Used to convey information
		/// about the data's period of change. Must be in the range of 0 - 15.
		/// </summary>
		/// <param name="rate"> the rate to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if rate is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Rate(int rate)
		{
			if (!(rate >= 0 && rate <= 15))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_rate = rate;
            IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Information rate enum, from <see cref="QosRates"/>. Used to convey information
		/// about the data's period of change.
		/// </summary>
		/// <returns> the rate
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int Rate()
		{
			return _rate;
		}

        /// <summary>
        /// If <c>true</c>, Qos is dynamic. Used to describe the changeability of the
        /// quality of service, typically over the life of a data stream.
        /// </summary>
        public bool IsDynamic
		{
            get => _dynamic;

            set
            {
                _dynamic = value;
                IsBlank = false;
            }
		}

		/// <summary>
		/// Specific timeliness information. Only present when timeliness is set to
		/// <see cref="QosTimeliness.DELAYED"/>. Must be in the range of 0 - 65535.
		/// </summary>
		/// <param name="timeInfo"> the timeInfo to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="timeInfo"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode TimeInfo(int timeInfo)
		{
			if (!(timeInfo >= 0 && timeInfo <= 65535))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_timeInfo = timeInfo;
            IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Specific timeliness information. Only present when timeliness is set to
		/// <see cref="QosTimeliness.DELAYED"/>.
		/// </summary>
		/// <returns> the timeInfo
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int TimeInfo()
		{
			return _timeInfo;
		}

		/// <summary>
		/// Specific rate information in milliseconds. Only present when rate is set
		/// to <see cref="QosRates.TIME_CONFLATED"/>. Must be in the range of 0 - 65535.
		/// </summary>
		/// <param name="rateInfo"> the rateInfo to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="rateInfo"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode RateInfo(int rateInfo)
		{
			if (!(rateInfo >= 0 && rateInfo <= 65535))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_rateInfo = rateInfo;
            IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Specific rate information in milliseconds. Only present when rate is set
		/// to <see cref="QosRates.TIME_CONFLATED"/>.
		/// </summary>
		/// <returns> the rateInfo
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public int RateInfo()
		{
			return _rateInfo;
		}

		/// <summary>
		/// The hash value of this object
		/// </summary>
		/// <returns>The hash code
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public override int GetHashCode()
		{
			int result = SEED;

			result = PRIME * result ^ (_timeliness + 1);
			result = PRIME * result ^ (_rate + 2);
			result = PRIME * result ^ (_timeInfo + 3);
			result = PRIME * result ^ (_rateInfo + 4);

			return result;
		}

		// Used to determine better/worse qos rates.
		// Smaller returned values mean better qos rates.
		// unspecifiedValue is returned for qos rate of UNSPECIFIED.
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		private int AdjustRateQos(int unspecifiedValue)
		{
			if (_rate == QosRates.TICK_BY_TICK)
			{
				return 0;
			}
			if (_rate == QosRates.JIT_CONFLATED)
			{
				return 65535 * 2;
			}
			if (_rate == QosRates.TIME_CONFLATED)
			{
				if (_rateInfo == 65535)
				{
					return 65535 * 2 + 1; // max time conflated is worse than JIT conflated
				}
				return _rateInfo * 2 - 1;
			}

			return unspecifiedValue;
		}

		// Used to determine better/worse qos timeliness values.
		// Smaller returned values mean better qos timeliness values.
		// unspecifiedValue is returned for qos timeliness value of UNSPECIFIED.
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		private int AdjustTimeQos(int unspecifiedValue)
		{
			if (_timeliness == QosTimeliness.REALTIME)
			{
				return 0;
			}
			if (_timeliness == QosTimeliness.DELAYED_UNKNOWN)
			{
				return 65536;
			}
			if (_timeliness == QosTimeliness.DELAYED)
			{
				return _timeInfo;
			}

			return unspecifiedValue;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal void Blank()
        {
            Clear();
            IsBlank = true;
        }
    }

}