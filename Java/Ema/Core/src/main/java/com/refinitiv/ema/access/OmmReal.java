///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * OmmReal represents Real number in Omm.
 * <p>OmmReal encapsulates magnitude type and mantissa information.</p>
 * 
 * OmmReal is a read only class.<br>
 * This class is used for extraction of Real only.
 *
 * @see Data
 */
public interface OmmReal extends Data
{
	/**
	 * MagnitudeType represents item stream data state.
	 */
	public class MagnitudeType
	{
		/** Power of -14. */
		public final static int EXPONENT_NEG_14 = 0;

		/** Power of -13. */
		public final static int EXPONENT_NEG_13 = 1;

		/** Power of -12. */
		public final static int EXPONENT_NEG_12 = 2;

		/** Power of -11. */
		public final static int EXPONENT_NEG_11 = 3;

		/** Power of -10. */
		public final static int EXPONENT_NEG_10 = 4;

		/** Power of -9. */
		public final static int EXPONENT_NEG_9 = 5;

		/** Power of -8. */
		public final static int EXPONENT_NEG_8 = 6;

		/** Power of -7. */
		public final static int EXPONENT_NEG_7 = 7;

		/** Power of -6. */
		public final static int EXPONENT_NEG_6 = 8;

		/** Power of -5. */
		public final static int EXPONENT_NEG_5 = 9;

		/** Power of -4. */
		public final static int EXPONENT_NEG_4 = 10;

		/** Power of -3. */
		public final static int EXPONENT_NEG_3 = 11;

		/** Power of -2. */
		public final static int EXPONENT_NEG_2 = 12;

		/** Power of -1. */
		public final static int EXPONENT_NEG_1 = 13;

		/** Power of 0. */
		public final static int EXPONENT_0 = 14;

		/** Power of 1. */
		public final static int EXPONENT_POS_1 = 15;

		/** Power of 2. */
		public final static int EXPONENT_POS_2 = 16;

		/** Power of 3. */
		public final static int EXPONENT_POS_3 = 17;

		/** Power of 4. */
		public final static int EXPONENT_POS_4 = 18;

		/** Power of 5. */
		public final static int EXPONENT_POS_5 = 19;

		/** Power of 6. */
		public final static int EXPONENT_POS_6 = 20;

		/** Power of 7. */
		public final static int EXPONENT_POS_7 = 21;

		/** Divisor of 1. */
		public final static int DIVISOR_1 = 22;

		/** Divisor of 2. */
		public final static int DIVISOR_2 = 23;

		/** Divisor of 4.  */
		public final static int DIVISOR_4 = 24;

		/** Divisor of 8.  */
		public final static int DIVISOR_8 = 25;

		/** Divisor of 16.  */
		public final static int DIVISOR_16 = 26;

		/** Divisor of 32.  */
		public final static int DIVISOR_32 = 27;

		/** Divisor of 64.  */
		public final static int DIVISOR_64 = 28;

		/** Divisor of 128.  */
		public final static int DIVISOR_128 = 29;

		/** Divisor of 256.  */
		public final static int DIVISOR_256 = 30;

		/** Represents infinity. */
		public final static int INFINITY = 33;

		/** Represents negative infinity. */
		public final static int NEG_INFINITY = 34;
		
		/** Represents not a number (NaN). */
		public final static int NOT_A_NUMBER = 35;
	}

	/**
	 * Returns the MagnitudeType value as a string format.
	 * 
	 * @return string representation of this object MagnitudeType
	 */
	public String magnitudeTypeAsString();

	/**
	 * Returns Mantissa.
	 * 
	 * @return value of OmmReal Mantissa
	 */
	public long mantissa();

	/**
	 * Returns MagnitudeType.
	 * 
	 * @return value of OmmReal MagnitudeType
	 */
	public int magnitudeType();

	/**
	 * Returns AsDouble.
	 * 
	 * @return value of Real as double
	 */
	public double asDouble();
}