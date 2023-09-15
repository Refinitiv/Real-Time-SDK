/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// ETA 4-byte floating point value
	/// </summary>
	sealed public class Float : IEquatable<Float>
	{
		internal float _floatValue;

		// for value(String) method
		private string trimmedVal;

        /// <summary>
        /// Creates <see cref="Float"/>.
        /// </summary>
        /// <seealso cref="Float"/>
        public Float()
        {
        }

		/// <summary>
		/// Sets members in Float to 0.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_floatValue = 0;
		}

		/// <summary>
		/// Sets Float to blank.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Blank()
		{
			Clear();
            IsBlank = true;
		}

        /// <summary>
		/// Is Float blank.
		/// </summary>
		public bool IsBlank { get; internal set; }

		/// <summary>
		/// This method will perform a deep copy into the passed in parameter's 
		///          members from the Object calling this method.
		/// </summary>
		/// <param name="destFloat"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if the destFloat is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Copy(Float destFloat)
		{
			if (null == destFloat)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destFloat._floatValue = _floatValue;
			destFloat.IsBlank = IsBlank;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// Check equality of two Float types.
        /// </summary>
        /// <param name="thatFloat"> the other Float to compare to this one
        /// </param>
        /// <returns> <c>true</c> if equal, <c>false</c> otherwise
        /// </returns>
        public bool Equals(Float thatFloat)
		{
			return (thatFloat != null) && (_floatValue == thatFloat.ToFloat()) && (IsBlank == thatFloat.IsBlank);
		}

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.
        /// </param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        ///     otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Float)
                return Equals((Float)other);

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
		/// Return value as float.
		/// </summary>
		/// <returns> the value as float
		/// </returns>
        public float ToFloat()
		{
			return _floatValue;
		}

		/// <summary>
		/// Convert to a <see cref="Real"/>. Must be in the range of <see cref="RealHints.EXPONENT_14"/>
		/// - <see cref="RealHints.MAX_DIVISOR"/>.
		/// </summary>
		/// <param name="hint"> <see cref="RealHints"/> enumeration hint value to use for converting float
		/// </param>
		/// <returns> the value as <see cref="Real"/>
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Real ToReal(int hint)
		{
            Debug.Assert(hint >= RealHints.EXPONENT_14 && hint <= RealHints.MAX_DIVISOR,
                "hint is out of range. Refer to RealHints");

			Real real = new Real();

			if (_floatValue != 0)
			{
				real.Hint = hint;
				if (_floatValue > 0)
				{
					real._value = (long)(_floatValue * Real.powHintsExp[hint] + 0.5);
				}
				else
				{
					real._value = (long)(_floatValue * Real.powHintsExp[hint] - 0.5);
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
		/// Convert <see cref="Float"/> to a numeric string.
		/// </summary>
		/// <returns> the string representation of this <see cref="Float"/>
		/// </returns>
		public override string ToString()
		{
			return Convert.ToString(_floatValue, CultureInfo.InvariantCulture);
		}

		/// <summary>
		/// Set value to float.
		/// </summary>
		/// <param name="value"> the value to set </param>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Value(float value)
		{
			_floatValue = value;
			IsBlank = false;
		}

		/// <summary>
		/// Set value to string.
		/// </summary>
		/// <param name="value"> the value to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if value is invalid.
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
				Value(float.Parse(trimmedVal, CultureInfo.InvariantCulture));
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
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			if (!IsBlank)
			{
				return Encoders.PrimitiveEncoder.EncodeFloat((EncodeIterator)iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

		/// <summary>
		/// Decode Float.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c>  if success,
		///     <c>CodecReturnCode.INCOMPLETE_DATA</c> if failure,
		///     <c>CodecReturnCode.BLANK_DATA</c> if data is blank value
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeFloat(iter, this);
		}
	}
}