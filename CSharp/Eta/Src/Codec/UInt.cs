/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
    /// ETA Unsigned Integer type. Can currently represent an unsigned value with precision of up to 64 bits.
    /// </summary>

    sealed public class UInt : IEquatable<UInt>
	{
        internal long _longValue;
		internal byte[] _bytes = new byte[9];

		// for value(String) method
		private string trimmedVal;

        /// <summary>
		/// Creates <see cref="UInt"/>.
		/// </summary>
		/// <returns> UInt object
		/// </returns>
		/// <seealso cref="UInt"/>
		public UInt()
		{
		}

        /// <summary>
		/// Clears <seealso cref="UInt"/>.
		/// </summary>
		public void Clear()
		{
			_longValue = 0;
		}

        /// <summary>
		/// This method will perform a deep copy into the passed in parameter's 
		///          members from the Object calling this method.
		/// </summary>
		/// <param name="destUInt"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if the destUInt is null.  </returns>
		public CodecReturnCode Copy(UInt destUInt)
		{
			if (null == destUInt)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destUInt._longValue = _longValue;
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
        /// <returns> the value as long </returns>
        public long ToLong()
		{
			return _longValue;
		}

        /// <summary>
		/// Set value to int.
		/// </summary>
		/// <param name="value"> the value to set </param>
		public void Value(int value)
		{
			_longValue = value;
			IsBlank = false;
		}


        /// <summary>
        /// Set value to long.
        /// </summary>
        /// <param name="value"> the value to set </param>
        public void Value(long value)
		{
			_longValue = value;
			IsBlank = false;
		}

        /// <summary>
		/// Used to encode into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into. Iterator
		///            should also have appropriate version information set.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			if (!IsBlank)
			{
				return Encoders.PrimitiveEncoder.EncodeUInt((EncodeIterator)iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
		/// Decode <seealso cref="UInt"/>.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and appropriate
		///            version information set.
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if success, <see cref="CodecReturnCode.INCOMPLETE_DATA"/> if
		///         failure, <see cref="CodecReturnCode.BLANK_DATA"/> if data is blank value
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeUInt(iter, this);
		}

        /// <summary>
        /// Convert <seealso cref="UInt"/> to a numeric string.
        /// </summary>
        /// <returns> the string representation of this <seealso cref="UInt"/> </returns>
        public override string ToString()
		{
			return Convert.ToString(_longValue);
		}

        /// <summary>
        /// Set value to uint.
        /// </summary>
        /// <param name="value"> the value to set
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
        ///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if value is invalid.  </returns>
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
				Value(long.Parse(value));
				return CodecReturnCode.SUCCESS;
			}
			catch (System.FormatException)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
		/// Checks equality of two UInt data.
		/// </summary>
		/// <param name="thatUInt"> the other UInt to compare to this one
		/// </param>
		/// <returns> true if UInts are equal, false otherwise </returns>
		public bool Equals(UInt thatUInt)
		{
			return ((thatUInt != null) && (_longValue == thatUInt.ToLong()) && (IsBlank == thatUInt.IsBlank));
		}

        /// <summary>Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.</summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.</param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        /// otherwise, <c>false</c>.</returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Enum)
                return Equals((Enum)other);

            return false;
        }

        /// <summary>Serves as a hash function for a particular type.</summary>
        /// <returns>A hash code for the current <c>Object</c>.</returns>
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