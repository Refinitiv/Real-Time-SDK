/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// ETA Real Formatting Hint enumeration values.
    /// </summary>
    /// <remarks>
    ///
    /// <para>
    /// <b>Typical Use</b>
    /// </para>
    /// <para>
    /// Conversion of <see cref="Real"/> to double/float is performed by the following formula:
    /// </para>
    /// 
    /// <code>
    /// if (<see cref="Real.Hint"/> lt; <see cref="FRACTION_1"/>)
    /// {
    ///      outputValue = Real.Value*(pow(10,(Real.Hint - <see cref="EXPONENT0"/>)));
    /// }
    /// else
    /// {
    ///      outputValue = Real.Value*(pow(2,(Real.Hint - <see cref="FRACTION_1"/>)));
    /// }
    /// </code>
    ///
    /// <para>
    /// Conversion of double/float to <see cref="Real"/> is performed by the following formula:
    /// </para>
    /// 
    /// <code>
    /// if (inputHint lt; <see cref="FRACTION_1"/>)
    /// {
    ///      Real.Value = (inputValue)/(pow(10,(inputHint - <see cref="EXPONENT0"/>)));
    /// }
    /// else
    /// {
    ///      Real.Value = (inputValue)/(pow(2,(inputHint - <see cref="FRACTION_1"/>)));
    /// }
    /// </code>
    /// </remarks>
    /// <seealso cref="Real"/>
    sealed public class RealHints
	{
		// RealHints class cannot be instantiated
		private RealHints()
		{
		}

		/// <summary>
		/// Minimum exponent format hint value </summary>
		public const int MIN_EXP = 0;

		/// <summary>
		/// Value raised to the -14 power. Shifts decimal by 14 positions </summary>
		public const int EXPONENT_14 = 0;

		/// <summary>
		/// Value raised to the -13 power. Shifts decimal by 13 positions </summary>
		public const int EXPONENT_13 = 1;

		/// <summary>
		/// Value raised to the -12 power. Shifts decimal by 12 positions </summary>
		public const int EXPONENT_12 = 2;

		/// <summary>
		/// Value raised to the -11 power. Shifts decimal by 11 positions </summary>
		public const int EXPONENT_11 = 3;

		/// <summary>
		/// Value raised to the -10 power. Shifts decimal by 10 positions </summary>
		public const int EXPONENT_10 = 4;

		/// <summary>
		/// Value raised to the -9 power. Shifts decimal by 9 positions </summary>
		public const int EXPONENT_9 = 5;

		/// <summary>
		/// Value raised to the -8 power. Shifts decimal by 8 positions </summary>
		public const int EXPONENT_8 = 6;

		/// <summary>
		/// Value raised to the -7 power. Shifts decimal by 7 positions </summary>
		public const int EXPONENT_7 = 7;

		/// <summary>
		/// Value raised to the -6 power. Shifts decimal by 6 positions </summary>
		public const int EXPONENT_6 = 8;

		/// <summary>
		/// Value raised to the -5 power. Shifts decimal by 5 positions </summary>
		public const int EXPONENT_5 = 9;

		/// <summary>
		/// Value raised to the -4 power. Shifts decimal by 4 positions </summary>
		public const int EXPONENT_4 = 10;

		/// <summary>
		/// Value raised to the -3 power. Shifts decimal by 3 positions </summary>
		public const int EXPONENT_3 = 11;

		/// <summary>
		/// Value raised to the -2 power. Shifts decimal by 2 positions </summary>
		public const int EXPONENT_2 = 12;

		/// <summary>
		/// Value raised to the -1 power. Shifts decimal by 1 position </summary>
		public const int EXPONENT_1 = 13;

		/// <summary>
		/// Value raised to the power 0. Value undergoes no change </summary>
		public const int EXPONENT0 = 14;

		/// <summary>
		/// Value raised to the power 1. Adds or removes 1 trailing zero </summary>
		public const int EXPONENT1 = 15;

		/// <summary>
		/// Value raised to the power 2. Adds or removes 2 trailing zeros </summary>
		public const int EXPONENT2 = 16;

		/// <summary>
		/// Value raised to the power 3. Adds or removes 3 trailing zeros </summary>
		public const int EXPONENT3 = 17;

		/// <summary>
		/// Value raised to the power 4. Adds or removes 4 trailing zeros </summary>
		public const int EXPONENT4 = 18;

		/// <summary>
		/// Value raised to the power 5. Adds or removes 5 trailing zeros </summary>
		public const int EXPONENT5 = 19;

		/// <summary>
		/// Value raised to the power 6. Adds or removes 6 trailing zeros </summary>
		public const int EXPONENT6 = 20;

		/// <summary>
		/// Value raised to the power 7. Adds or removes 7 trailing zeros </summary>
		public const int EXPONENT7 = 21;

		/// <summary>
		/// Maximum exponent format hint value </summary>
		public const int MAX_EXP = 21;

		/// <summary>
		/// Minimum fraction format hint value </summary>
		public const int MIN_DIVISOR = 22;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/1. Value undergoes no
		/// change
		/// </summary>
		public const int FRACTION_1 = 22;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/2. Adds or removes a
		/// denominator of 2
		/// </summary>
		public const int FRACTION_2 = 23;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/4. Adds or removes a
		/// denominator of 4
		/// </summary>
		public const int FRACTION_4 = 24;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/8. Adds or removes a
		/// denominator of 8
		/// </summary>
		public const int FRACTION_8 = 25;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/16. Adds or removes a
		/// denominator of 16
		/// </summary>
		public const int FRACTION_16 = 26;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/32. Adds or removes a
		/// denominator of 32
		/// </summary>
		public const int FRACTION_32 = 27;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/64. Adds or removes a
		/// denominator of 64
		/// </summary>
		public const int FRACTION_64 = 28;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/128. Adds or removes a
		/// denominator of 128
		/// </summary>
		public const int FRACTION_128 = 29;

		/// <summary>
		/// Fractional denominator operation, equivalent to 1/256. Adds or removes a
		/// denominator of 256
		/// </summary>
		public const int FRACTION_256 = 30;

		/// <summary>
		/// Maximum fraction format hint value </summary>
		public const int MAX_DIVISOR = 30;

		/// <summary>
		/// Indicates value of infinity </summary>
		public const int INFINITY = 33;

		/// <summary>
		/// Indicates value of negative infinity </summary>
		public const int NEG_INFINITY = 34;

		/// <summary>
		/// Indicates value is Not A Number (NaN) </summary>
		public const int NOT_A_NUMBER = 35;

	}

}