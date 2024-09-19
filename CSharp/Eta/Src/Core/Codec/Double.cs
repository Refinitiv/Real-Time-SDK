/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// ETA 8-byte double value
	/// </summary>
	sealed public class Double : IEquatable<Double>
	{
		internal double _doubleValue;

		// for value(String) method
		private string trimmedVal;

        /// <summary>
		/// Creates <see cref="Double"/>.
		/// </summary>
		/// <seealso cref="Double"/>
        public Double()
        {
        }

		/// <summary>
		/// Sets members in Double to 0.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_doubleValue = 0;
		}

		/// <summary>
		/// Sets Double to blank.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Blank()
		{
			Clear();
            IsBlank = true;
		}

        /// <summary>
        /// Is Double blank.
        /// </summary>
		public bool IsBlank { get; internal set; }

		/// <summary>
		/// This method will perform a deep copy into the passed in parameter's members
		/// from the Object calling this method.
		/// </summary>
		/// <param name="destDouble"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destDouble"/> is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Copy(Double destDouble)
		{
			if (null == destDouble)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destDouble._doubleValue = _doubleValue;
			destDouble.IsBlank = IsBlank;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Check equality of two Double types.
		/// </summary>
		/// <param name="thatDouble"> the other Double to compare to this one
		/// </param>
		/// <returns> <c>true</c> if equal, <c>false</c> otherwise
		/// </returns>
		public bool Equals(Double thatDouble)
		{
			return ((thatDouble != null) && (_doubleValue == thatDouble.ToDouble()) && (IsBlank == thatDouble.IsBlank));
		}

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.
        /// </param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        ///    otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Double)
                return Equals((Double)other);

            return false;
        }

        /// <summary>
        /// Serves as a hash function for a particular type.
        /// </summary>
        /// <returns>A hash code for the current <c>Object</c>.
        /// </returns>
        public override int GetHashCode()
        {
            return ToString().GetHashCode();
        }

		/// <summary>
		/// Return value as double.
		/// </summary>
		/// <returns> the value as double
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public double ToDouble()
		{
			return _doubleValue;
		}

		/// <summary>
		/// Convert to a <see cref="Real"/> 
		/// </summary>
		/// <param name="hint"> <see cref="RealHints"/> enumeration hint value to use for converting double.
		///   Must be in the range of <see cref="RealHints.EXPONENT_14"/> - <see cref="RealHints.MAX_DIVISOR"/>.
		/// </param>
		/// <returns> the value as <see cref="Real"/>
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Real ToReal(int hint)
		{
            Debug.Assert(hint >= RealHints.EXPONENT_14 && hint <= RealHints.MAX_DIVISOR, 
                "hint is out of range. Refer to RealHints");

			Real real = new Real();

			if (_doubleValue != 0)
			{
				real.Hint = hint;
				if (_doubleValue > 0)
				{
					real._value = (long)(_doubleValue * Real.powHintsExp[hint] + 0.5);
				}
				else
				{
					real._value = (long)(_doubleValue * Real.powHintsExp[hint] - 0.5);
				}
				real.IsBlank = false;
			}
			else
			{
				real.Blank();
			}

			return real;
		}

		/// <summary>
		/// Convert <see cref="Double"/> to a numeric string.
		/// </summary>
		/// <returns> the string representation of this <see cref="Double"/> in Invariant culture
		/// </returns>
		public override string ToString()
		{
			return Convert.ToString(_doubleValue, CultureInfo.InvariantCulture);
		}

		/// <summary>
		/// Set value to double.
		/// </summary>
		/// <param name="value"> the value to set </param>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Value(double value)
		{
			_doubleValue = value;
            IsBlank = false;
		}

		/// <summary>
		/// Set value to string.
		/// </summary>
		/// <remarks>Provided string is parsed using Invariant culture</remarks>
		/// <param name="value"> the value to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="value"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Value(string value)
		{
			if (string.ReferenceEquals(value, null))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			trimmedVal = value.Trim();
			if (trimmedVal.Length == 0)
			{
				// blank
				Blank();
				return CodecReturnCode.SUCCESS;
			}

			try
			{
				Value(double.Parse(trimmedVal, CultureInfo.InvariantCulture));
				return CodecReturnCode.SUCCESS;
			}
			catch (System.FormatException)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

		/// <summary>
		/// Used to encode into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into. Iterator
		///            should also have appropriate version information set.
		/// </param>
		/// <returns> Operation status code
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			if (!IsBlank)
			{
				return Encoders.PrimitiveEncoder.EncodeDouble(iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

		/// <summary>
		/// Decode Double.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if success,
		///     <c>CodecReturnCode.INCOMPLETE_DATA</c> if failure,
		///     <c>CodecReturnCode.BLANK_DATA</c> if data is blank value
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeDouble(iter, this);
		}
	}

}