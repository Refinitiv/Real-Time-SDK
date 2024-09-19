/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// This type allows the user to represent fractional or decimal values, with
	/// minimal conversion overhead, by using a number and a format hint.
	/// </summary>
	///
	/// <remarks>
	/// <para>Typical Use:</para>
	/// <list type="bullets">
	/// <item>User populates value with signed value and format with <see cref="RealHints"/>.</item>
	/// <item>If format hint is in Exponent Range (0 - 14) value * 10<sup>(format - 14)</sup></item>
	/// <item>If format hint is in Fraction Range (22 - 30) value * (1/2)<sup>(format - 22)</sup></item>
	/// </list>
	///
	/// <para>
	/// An application can convert between a Real and a float or double as needed.
	/// Converting a Real to a double or float is typically done to perform calculations 
	/// or display data after receiving it.</para>
	///
	/// <para>
	/// The conversion process adds or removes decimal or denominator information from the 
	/// value to optimize transmission sizes. In a Real type, the decimal or denominator 
	/// information is indicated by the Real.hint and the Real.toLong indicates the value, 
	/// less any decimal or denominator. If the Real.isBlank method is true, this is handled 
	/// as blank regardless of information contained in the Real.hint and Real.toLong methods.</para>
	///
	/// </remarks>
	/// <seealso cref="RealHints"/>
	sealed public class Real
	{
		internal const byte BLANK_REAL = 0x20;
		internal static readonly double[] powHintsExp = new double[] {100000000000000.0, 10000000000000.0, 1000000000000.0, 100000000000.0, 10000000000.0, 1000000000.0, 100000000.0, 10000000.0, 1000000.0, 100000.0, 10000.0, 1000.0, 100.0, 10.0, 1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001, 1, 2, 4, 8, 16, 32, 64, 128, 256};
		internal static readonly string[] zeroDisplayStrings = new string[] {"0.0E-14", "0.0E-13", "0.0E-12", "0.0E-11", "0.0E-10", "0.0E-9", "0.0E-8", "0.0E-7", "0.0E-6", "0.0E-5", "0.0E-4", "0.000", "0.00", "0.0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};

		internal long _value;

		// for toString() method so don't have to re-convert if set by string
		private string _stringVal;

		// for value(String) method
		private string trimmedVal;
		private static string infinity = "Inf";
		private static string negInfinity = "-Inf";
		private static string notANumber = "NaN";
		private readonly int MAX_STRLEN = 20;
		internal UInt valueUInt =new UInt();
		internal UInt tempValue =new UInt();
		internal Int trailzerovalue = new Int();
		internal Int trailzerocount = new Int();
		internal Int foundDigit = new Int();
		internal Int nextDigit = new Int();
		internal UInt denominator =new UInt();
		internal Int expdiff = new Int();
		internal UInt numerator =new UInt();

        /// <summary>
		/// Creates <see cref="Real"/>.
		/// </summary>
		/// <seealso cref="Real"/>
        public Real()
        {
        }

		/// <summary>
		/// Clears <see cref="Real"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			IsBlank = false;
			Hint = RealHints.EXPONENT0;
			_value = 0;
			_stringVal = null;
		}

		/// <summary>
		/// Sets Real to blank.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Blank()
		{
            IsBlank = true;
            Hint = RealHints.EXPONENT_14;
			_value = 0;
			_stringVal = null;
		}

		/// <summary>
		/// This method will perform a deep copy into the passed in parameter's 
		/// members from the Object calling this method.
		/// </summary>
		/// <param name="destReal"> the value getting populated with the values of the calling Object
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if the <paramref name="destReal"/> is <c>null</c>.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Copy(Real destReal)
		{
			if (null == destReal)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destReal._value = _value;
            destReal.Hint = Hint;
            destReal.IsBlank = IsBlank;

            destReal._stringVal = null;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Convert double to a <see cref="Real"/>. Hint must be in the range of
		/// <see cref="RealHints.EXPONENT_14"/> - <see cref="RealHints.MAX_DIVISOR"/>.
		/// </summary>
		/// <param name="value"> double to convert to <see cref="Real"/> </param>
		/// <param name="hint"> <see cref="RealHints"/> enumeration hint value to use for converting double
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="value"/> or <paramref name="hint"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Value(double value, int hint)
		{
			if (!(hint >= RealHints.EXPONENT_14 && hint <= RealHints.NOT_A_NUMBER && hint != 31)) // 31 is 'reserved'
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			if (value == double.NaN)
			{
				_value = 0;
                Hint = RealHints.NOT_A_NUMBER;
			}
			else if (value == double.PositiveInfinity)
			{
				_value = 0;
                Hint = RealHints.INFINITY;
			}
			else if (value == double.NegativeInfinity)
			{
				_value = 0;
                Hint = RealHints.NEG_INFINITY;
			}
			else
			{
                Hint = hint;
				if (value > 0)
				{
					_value = (long)(value * powHintsExp[hint] + 0.5);
				}
				else
				{
					_value = (long)(value * powHintsExp[hint] - 0.5);
				}
			}
			IsBlank = false;
			_stringVal = null;
			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Convert float to a <see cref="Real"/>. Hint must be in the range of
		/// <see cref="RealHints.EXPONENT_14"/> - <see cref="RealHints.MAX_DIVISOR"/>.
		/// </summary>
		/// <param name="value"> float to convert to <see cref="Real"/> </param>
		/// <param name="hint">  <see cref="RealHints"/> enumeration hint value to use for converting float
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="value"/> or <paramref name="hint"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Value(float value, int hint)
		{
			if (!(hint >= RealHints.EXPONENT_14 && hint <= RealHints.NOT_A_NUMBER && hint != 31)) // 31 is 'reserved'
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			if (value == float.NaN)
			{
				_value = 0;
				Hint = RealHints.NOT_A_NUMBER;
			}
			else if (value == float.PositiveInfinity)
			{
				_value = 0;
				Hint = RealHints.INFINITY;
			}
			else if (value == float.NegativeInfinity)
			{
				_value = 0;
				Hint = RealHints.NEG_INFINITY;
			}
			else
			{
				Hint = hint;
				if (value > 0)
				{
					_value = (long)(value * powHintsExp[hint] + 0.5);
				}
				else
				{
					_value = (long)(value * powHintsExp[hint] - 0.5);
				}
			}
			IsBlank = false;
			_stringVal = null;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Set raw signed value and hint. Hint must be in the range of
		/// <see cref="RealHints.EXPONENT_14"/> - <see cref="RealHints.MAX_DIVISOR"/>.
		/// </summary>
		/// <param name="value"> raw signed value </param>
		/// <param name="hint"> <see cref="RealHints"/> enumeration hint
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> on success,
		///         <c>CodecReturnCode.INVALID_ARGUMENT</c> if <paramref name="value"/> or <paramref name="hint"/> is invalid.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Value(long value, int hint)
		{
			if (!(hint >= RealHints.EXPONENT_14 && hint <= RealHints.NOT_A_NUMBER && hint != 31 && hint != 32)) // 31 is 'reserved'
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_value = value;
			Hint = hint;
			IsBlank = false;
			_stringVal = null;

			return CodecReturnCode.SUCCESS;
		}

		/// <summary>
		/// Converts to double value
		/// </summary>
		/// <returns>The double value
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public double ToDouble()
		{
			if (IsBlank == true)
			{
				return 0;
			}
			switch (Hint)
			{
				case RealHints.NOT_A_NUMBER:
					return double.NaN;
				case RealHints.INFINITY:
					return double.PositiveInfinity;
				case RealHints.NEG_INFINITY:
					return double.NegativeInfinity;
			}

			double v = _value / powHintsExp[Hint];
			return v;
		}

		/// <summary>
		/// The raw value represented by the Real (omitting any decimal or denominator).
		/// Typically requires application of the hint before interpreting or performing
		/// any calculations. This member can currently represent up to 63 bits and a one-bit
		/// sign (positive or negative).
		/// </summary>
		/// <returns> the value as long
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public long ToLong()
		{
			return _value;
		}

        /// <summary>
		/// Populated with an enumerated value from <see cref="RealHints"/>.
		/// raw value contained in Real. Hint values can add or remove up to seven trailing
		/// zeros, 14 decimal places, or fractional denominators up to 256.
		/// </summary>
		/// <value> the hint </value>
		public int Hint { get; internal set; }

        /// <summary>
        /// Returns a Boolean value. Indicates whether data is considered blank. If <c>true</c>,
        /// value and hint should be ignored, if <c>false</c> value and hint determine the resultant
        /// value. This allows Real to be represented as blank when used as either a primitive
        /// type or a set-defined primitive type.
        /// </summary>
        public bool IsBlank { get; internal set; }

        /// <summary>
		/// Check equality of two Real types.
		/// </summary>
		/// <param name="thatReal"> the other Real to compare to this one
		/// </param>
		/// <returns> <c>true</c> if equal, <c>false</c> otherwise
		/// </returns>
		public bool Equals(Real thatReal)
		{
			if (thatReal != null)
			{
				if ((_value == thatReal.ToLong()) && (Hint == thatReal.Hint) && (IsBlank == thatReal.IsBlank))
				{
					return true;
				}
				else
				{
					return ((ToDouble() == thatReal.ToDouble()) && (IsBlank == thatReal.IsBlank));
				}
			}
			else
			{
				return false;
			}
		}

        /// <summary>
        /// Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.
        /// </summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.
        /// </param>
        /// <returns> <c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        ///     otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is Real)
                return Equals((Real)other);

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
		/// Used to encode a Real into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into. Iterator
		///            should also have appropriate version information set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <see cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.PrimitiveEncoder.EncodeReal(iter, this);
		}

		/// <summary>
		/// Decode Real.
		/// </summary>
		/// <param name="iter"> DecodeIterator with buffer to decode from and appropriate
		///            version information set
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if success,
		///         <c>CodecReturnCode.INCOMPLETE_DATA</c> if failure,
		///         <c>CodecReturnCode.BLANK_DATA</c> if data is blank value.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeReal(iter, this);
		}

        /// <summary>
		/// Convert Real to a numeric string.
		/// </summary>
		/// <returns> the string representation of this <see cref="Real"/>
		/// </returns>
		public override string ToString()
		{
			if (string.ReferenceEquals(_stringVal, null))
			{
				switch (Hint)
				{
					case RealHints.INFINITY:
					{
						_stringVal = infinity;
						return "Inf";
					}
					case RealHints.NEG_INFINITY:
					{
						_stringVal = negInfinity;
						return "-Inf";
					}
					case RealHints.NOT_A_NUMBER:
					{
						_stringVal = notANumber;
						return "NaN";
					}
				}

				if (_value != 0)
				{
					return Convert.ToString(ToDouble());
				}
				else
				{
					return zeroDisplayStrings[Hint];
				}
			}
			else
			{
				return _stringVal;
			}
		}

		/// <summary>
		/// Convert String to a <see cref="Real"/>.
		/// </summary>
		/// <param name="value"> String to convert to <see cref="Real"/>
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

			int maxStringLen = MAX_STRLEN;
			bool isNeg = false;
			int valIdx = 0;
			_stringVal = null;

			valueUInt.Clear();
			tempValue.Clear();
			trailzerovalue.Clear();
			trailzerocount.Clear();
			foundDigit.Clear();
			nextDigit.Clear();
			denominator.Clear();
			expdiff.Clear();
			numerator.Clear();

			trimmedVal = value.Trim();

			if (valIdx == trimmedVal.Length)
			{
				Blank();
				_stringVal = trimmedVal;
				return CodecReturnCode.SUCCESS;
			}

			if (trimmedVal.Equals(infinity, StringComparison.CurrentCultureIgnoreCase))
			{
				IsBlank = false;
				_value = 0;
				_stringVal = infinity;
				Hint = RealHints.INFINITY;
				return CodecReturnCode.SUCCESS;
			}
			else if (trimmedVal.Equals(negInfinity, StringComparison.CurrentCultureIgnoreCase))
			{
				IsBlank = false;
				_value = 0;
				_stringVal = negInfinity;
				Hint = RealHints.NEG_INFINITY;
				return CodecReturnCode.SUCCESS;
			}
			else if (trimmedVal.Equals(notANumber, StringComparison.CurrentCultureIgnoreCase))
			{
				IsBlank = false;
				_value = 0;
				_stringVal = notANumber;
				Hint = RealHints.NOT_A_NUMBER;
				return CodecReturnCode.SUCCESS;
			}

			// check if blank
			if (trimmedVal[valIdx] == '+')
			{
				bool isAllZeroes = true;
				for (int i = valIdx + 1; i < trimmedVal.Length; i++)
				{
					if (trimmedVal[i] != '0' && trimmedVal[i] != '.')
					{
						isAllZeroes = false;
						break;
					}
				}
				if (isAllZeroes)
				{
					Blank();
					_stringVal = trimmedVal;
					return CodecReturnCode.SUCCESS;
				}
			}

			// check if negative
			if (trimmedVal[valIdx] == '-')
			{
				isNeg = true;
				maxStringLen++;
				valIdx++;
			}
			else if (trimmedVal[valIdx] == '+')
			{
				valIdx++;
			}

			valIdx = Rwf_atonumber_end_trailzero(trimmedVal, valIdx, valueUInt, foundDigit, trailzerovalue, trailzerocount, nextDigit, tempValue);

			if (valIdx == trimmedVal.Length)
			{
				// number must be no bigger than max string length
				if (trimmedVal.Length <= maxStringLen)
				{
					Value(!isNeg ? valueUInt.ToLong() : -valueUInt.ToLong(), RealHints.EXPONENT0);
					IsBlank = false;
				}
				else
				{
					// error
					return CodecReturnCode.INVALID_ARGUMENT;
				}
				_stringVal = trimmedVal;
				return CodecReturnCode.SUCCESS;
			}

			/* Check for decimal value */
			if (trimmedVal[valIdx] == '.')
			{
				/* It is a decimal value */
				int startdec = ++valIdx;
				int exponent;
				maxStringLen++;

				if (trimmedVal.Length > maxStringLen)
				{
					// error
					return CodecReturnCode.INVALID_ARGUMENT;
				}

				valIdx = Rwf_atonumber_end(trimmedVal, valIdx, valueUInt, foundDigit, nextDigit, tempValue);

				exponent = valIdx - startdec;

				if (exponent == 0)
				{
					// error
					return CodecReturnCode.INVALID_ARGUMENT;
				}

				Value(!isNeg ? valueUInt.ToLong() : -valueUInt.ToLong(), RealHints.EXPONENT0 - exponent);
				if (trimmedVal[0] != '+')
				{
					IsBlank = false;
				}
			}
			else if (trimmedVal[valIdx] == ' ')
			{
				valIdx++;
				maxStringLen++;

				/* Check for another digit. Then it might be a fraction. */
				if ((trimmedVal[valIdx] >= '0' && trimmedVal[valIdx] <= '9'))
				{
					tempValue.Clear();
					denominator.Clear();

					valIdx = Rwf_atonumber_end(trimmedVal, valIdx, numerator, foundDigit, nextDigit, tempValue);

					/* Verify fraction */
					if (trimmedVal[valIdx] != '/')
					{
						// error
						return CodecReturnCode.INVALID_ARGUMENT;
					}

					maxStringLen++;
					if (trimmedVal.Length >= maxStringLen)
					{
						// error
						return CodecReturnCode.INVALID_ARGUMENT;
					}

					valIdx++;
					valIdx = Rwf_atonumber_end(trimmedVal, valIdx, denominator, foundDigit, nextDigit, tempValue);

					int hint = Rwf_SetFractionHint((int)denominator.ToLong());
					if (hint == 0)
					{
						// error
						return CodecReturnCode.INVALID_ARGUMENT;
					}

					valueUInt.Value((valueUInt.ToLong() * denominator.ToLong()) + numerator.ToLong());
					Value(!isNeg ? valueUInt.ToLong() : -valueUInt.ToLong(), hint);
					IsBlank = false;
				}
				else if (valIdx == trimmedVal.Length)
				{
					Value(!isNeg ? valueUInt.ToLong() : -valueUInt.ToLong(), RealHints.EXPONENT0);
					IsBlank = false;
				}
				else
				{
					// error
					return CodecReturnCode.INVALID_ARGUMENT;
				}
			}
			else if (trimmedVal[valIdx] == '/')
			{
				tempValue.Clear();
				denominator.Clear();

				valIdx++;
				valIdx = Rwf_atonumber_end(trimmedVal, valIdx, denominator, foundDigit, nextDigit, tempValue);

				int hint = Rwf_SetFractionHint((int)denominator.ToLong());
				if (hint == 0)
				{
					// error
					return CodecReturnCode.INVALID_ARGUMENT;
				}

				/* value stays as value */
				Value(!isNeg ? valueUInt.ToLong() : -valueUInt.ToLong(), hint);
				IsBlank = false;
			}
			else
			{
				// error
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			return CodecReturnCode.SUCCESS;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		private int Rwf_atonumber_end_trailzero(string str, int index, UInt result, Int foundDigit, Int trailzerovalue, Int trailzerocount, Int nextDigit, UInt tempValue)
		{
			while ((index < str.Length) && (str[index] >= '0' && str[index] <= '9'))
			{
				tempValue.Value(result.ToLong() * 10);
				nextDigit.Value((str[index] - 0x30));
				if (str[index] == '0')
				{
					if (trailzerocount.ToLong() == 0)
					{
						trailzerovalue.Value(result.ToLong());
					}
					trailzerocount.Value(trailzerocount.ToLong() + 1);
				}
				else
				{
					trailzerocount.Value(0);
				}
				foundDigit.Value(1);
				result.Value(tempValue.ToLong() + nextDigit.ToLong());
				index++;
			}

			return index;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		private int Rwf_atonumber_end(string str, int index, UInt result, Int foundDigit, Int nextDigit, UInt tempValue)
		{
			while ((index < str.Length) && (str[index] >= '0' && str[index] <= '9'))
			{
				foundDigit.Value(1);
				tempValue.Value(result.ToLong() * 10);
				nextDigit.Value((str[index] - 0x30));
				result.Value(tempValue.ToLong() + nextDigit.ToLong());
				index++;
			}

			return index;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		private int Rwf_SetFractionHint(int denom)
		{
			int retval = 0;
			switch (denom)
			{
				case 1:
					retval = RealHints.FRACTION_1;
					break;
				case 2:
					retval = RealHints.FRACTION_2;
					break;
				case 4:
					retval = RealHints.FRACTION_4;
					break;
				case 8:
					retval = RealHints.FRACTION_8;
					break;
				case 16:
					retval = RealHints.FRACTION_16;
					break;
				case 32:
					retval = RealHints.FRACTION_32;
					break;
				case 64:
					retval = RealHints.FRACTION_64;
					break;
				case 128:
					retval = RealHints.FRACTION_128;
					break;
				case 256:
					retval = RealHints.FRACTION_256;
					break;
			}
			return retval;
		}

        internal bool HintBlank
        {
            get
            {
                return (Hint & BLANK_REAL) > 0 ? true : false;
            }
        }
    }

}