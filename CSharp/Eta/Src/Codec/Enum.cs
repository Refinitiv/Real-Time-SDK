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
	/// Represents an enumeration type, defined as an unsigned two-byte value. Many
	/// times, this enumeration value is cross-referenced with an enumeration
	/// dictionary (e.g., enumtype.def) or a well-known enumeration definition (e.g.,
	/// those contained in RDM package).
	/// 
	/// This type allows a value ranging from 0 to 65,535.
	/// </summary>
	public class Enum : IEquatable<Enum>
	{
		internal int _enumValue;

		// for value(String) method
		private string trimmedVal;

        /// <summary>
		/// Creates <see cref="Enum"/>.
		/// </summary>
		/// <returns> Enum object
		/// </returns>
		/// <seealso cref="Enum"/>
		public Enum()
		{
		}

        /// <summary>
        /// Clears <seealso cref="Enum"/>.
        /// </summary>
        public void Clear()
		{
			_enumValue = 0;
		}

        /// <summary>
        /// Is Enum blank.
        /// </summary>
		public bool IsBlank { get; internal set; }

        /// <summary>
		/// This method will perform a deep copy into the passed in parameter's 
		///          members from the Object calling this method.
		/// </summary>
		/// <param name="destEnum"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if the destEnum is null.  </returns>
		public virtual CodecReturnCode Copy(Enum destEnum)
		{
			if (null == destEnum)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destEnum._enumValue = _enumValue;
            destEnum.IsBlank = IsBlank;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Return value as int.
		/// </summary>
		/// <returns> the value as int </returns>
		public int ToInt()
		{
			return _enumValue;
		}

        /// <summary>
		/// The enum value.
		/// </summary>
		/// <param name="value"> the value to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if value is invalid.  </returns>
		public CodecReturnCode Value(int value)
		{
			if (!(value >= 0 && value <= 65535))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_enumValue = value;
            IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Decode enum.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set.
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if success,
		///         <see cref="CodecReturnCode.INCOMPLETE_DATA"/> if failure,
		///         <see cref="CodecReturnCode.BLANK_DATA"/> if data is blank value.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeEnum(iter, this);
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
				return Encoders.PrimitiveEncoder.EncodeEnum((EncodeIterator)iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
		/// Convert <seealso cref="Enum"/> to a numeric string.
		/// </summary>
		/// <returns> the string representation of this <seealso cref="Enum"/> </returns>
		public override string ToString()
		{
			return Convert.ToString(_enumValue);
		}

        /// <summary>
		/// Set value to <seealso cref="Enum"/>.
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
				return Value(int.Parse(trimmedVal));
			}
			catch (System.FormatException)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
		/// Checks equality of two Enum data.
		/// </summary>
		/// <param name="thatEnum"> the other enum to compare to this one
		/// </param>
		/// <returns> true if enums are equal, false otherwise </returns>
		public bool Equals(Enum thatEnum)
		{
			return ((thatEnum != null) && (thatEnum.ToInt() == _enumValue) && (thatEnum.IsBlank == IsBlank));
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
            return _enumValue;
        }

        internal void Blank()
        {
            Clear();
            IsBlank = true;
        }
    }
}