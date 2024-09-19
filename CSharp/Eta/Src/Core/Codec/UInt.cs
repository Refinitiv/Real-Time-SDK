/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// ETA Unsigned Integer type. Can currently represent an unsigned value with precision of up to 64 bits.
    /// </summary>
    sealed public class UInt : IEquatable<UInt>
	{
        internal ulong _ulongValue;

		// for value(String) method
		private string trimmedVal;

        /// <summary>
		/// Creates <see cref="UInt"/>.
		/// </summary>
		/// <seealso cref="UInt"/>
		public UInt()
		{
		}

        /// <summary>
		/// Clears <see cref="UInt"/>.
		/// </summary>
		public void Clear()
		{
			_ulongValue = 0;
		}

		/// <summary>
		/// This method will perform a deep copy into the passed in parameter's 
		/// members from the Object calling this method.
		/// </summary>
		/// <param name="destUInt"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destUInt"/> is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Copy(UInt destUInt)
		{
			if (null == destUInt)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destUInt._ulongValue = _ulongValue;
			destUInt.IsBlank = IsBlank;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// Is UInt blank.
        /// </summary>
        public bool IsBlank { get; internal set; }

		/// <summary>
		/// Returns value as long. The value is returned as signed long. If the value
		/// is negative, the user has to convert it to an unsigned value.
		/// </summary>
		/// <returns> the value as long
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public long ToLong() => unchecked((long)_ulongValue);

        /// <summary>
        /// Returns value as ulong. The value is returned as unsigned long.
        /// </summary>
        /// <returns> the value as ulong
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ulong ToULong() => _ulongValue;

        /// <summary>
        /// Set value to uint.
        /// </summary>
        /// <param name="value"> the value to set </param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Value(uint value)
		{
			_ulongValue = value;
			IsBlank = false;
		}

        /// <summary>
        /// Set value to int.
        /// </summary>
        /// <param name="value"> the value to set </param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Value(int value)
        {
            _ulongValue = unchecked((uint)value);
            IsBlank = false;
        }

        /// <summary>
        /// Set value to ulong.
        /// </summary>
        /// <param name="value"> the value to set </param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Value(ulong value)
		{
			_ulongValue = value;
			IsBlank = false;
		}

        /// <summary>
        /// Set value to long.
        /// </summary>
        /// <param name="value"> the value to set </param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Value(long value)
        {
            _ulongValue = unchecked((ulong)value);
            IsBlank = false;
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
				return Encoders.PrimitiveEncoder.EncodeUInt(iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

		/// <summary>
		/// Decode <see cref="UInt"/>.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and appropriate
		///            version information set.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if success,
		///       <c>CodecReturnCode.INCOMPLETE_DATA</c> if failure,
		///       <c>CodecReturnCode.BLANK_DATA</c> if data is blank value
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeUInt(iter, this);
		}

        /// <summary>
        /// Convert <see cref="UInt"/> to a numeric string.
        /// </summary>
        /// <returns> the string representation of this <see cref="UInt"/>
        /// </returns>
        public override string ToString()
		{
			return Convert.ToString(_ulongValue);
		}

		/// <summary>
		/// Set value to uint.
		/// </summary>
		/// <param name="value"> the value to set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if value is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Value(string value)
		{
			if (value is null)
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
				Value(ulong.Parse(value));
				return CodecReturnCode.SUCCESS;
			}
			catch (FormatException)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
			catch(OverflowException)
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }
        }

        /// <summary>
		/// Checks equality of two UInt data.
		/// </summary>
		/// <param name="thatUInt"> the other UInt to compare to this one
		/// </param>
		/// <returns> <c>true</c> if UInts are equal, <c>false</c> otherwise
		/// </returns>
		public bool Equals(UInt thatUInt)
		{
			return (thatUInt != null) && (_ulongValue == thatUInt.ToULong()) && (IsBlank == thatUInt.IsBlank);
		}

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.
        /// </param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        ///       otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object other)
        {
            if (other is null)
                return false;

            return other is UInt uInt && Equals(uInt);
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

        internal void Blank()
        {
            Clear();
            IsBlank = true;
        }
    }
}