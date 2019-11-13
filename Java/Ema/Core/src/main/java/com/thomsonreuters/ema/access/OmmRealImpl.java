///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


class OmmRealImpl extends DataImpl implements OmmReal
{
	private final static String EXPONENTNEG14_STRING 	= "Power of -14. Stringeration value is 0";
	private final static String EXPONENTNEG13_STRING 	= "Power of -13";
	private final static String EXPONENTNEG12_STRING 	= "Power of -12";
	private final static String EXPONENTNEG11_STRING 	= "Power of -11";
	private final static String EXPONENTNEG10_STRING	= "Power of -10";
	private final static String EXPONENTNEG9_STRING 	= "Power of -9";
	private final static String EXPONENTNEG8_STRING 	= "Power of -8";
	private final static String EXPONENTNEG7_STRING 	= "Power of -7";
	private final static String EXPONENTNEG6_STRING 	= "Power of -6";
	private final static String EXPONENTNEG5_STRING 	= "Power of -5";
	private final static String EXPONENTNEG4_STRING 	= "Power of -4";
	private final static String EXPONENTNEG3_STRING 	= "Power of -3";
	private final static String EXPONENTNEG2_STRING 	= "Power of -2";
	private final static String EXPONENTNEG1_STRING 	= "Power of -1";
	private final static String EXPONENT0_STRING 		= "Power of 0";
	private final static String EXPONENTPOS1_STRING 	= "Power of 1";
	private final static String EXPONENTPOS2_STRING 	= "Power of 2";
	private final static String EXPONENTPOS3_STRING 	= "Power of 3";
	private final static String EXPONENTPOS4_STRING 	= "Power of 4";
	private final static String EXPONENTPOS5_STRING 	= "Power of 5";
	private final static String EXPONENTPOS6_STRING 	= "Power of 6";
	private final static String EXPONENTPOS7_STRING 	= "Power of 7";
	private final static String DIVISOR1_STRING 		= "Divisor of 1";
	private final static String DIVISOR2_STRING 		= "Divisor of 2";
	private final static String DIVISOR4_STRING 		= "Divisor of 4";
	private final static String DIVISOR8_STRING 		= "Divisor of 8";
	private final static String DIVISOR16_STRING 		= "Divisor of 16";
	private final static String DIVISOR32_STRING 		= "Divisor of 32";
	private final static String DIVISOR64_STRING 		= "Divisor of 64";
	private final static String DIVISOR128_STRING 		= "Divisor of 128";
	private final static String DIVISOR256_STRING 		= "Divisor of 256";
	private final static String INFINITY_STRING 		= "Inf";
	private final static String NEGINFINITY_STRING 		= "-Inf";
	private final static String NAN_STRING 				= "NaN";
	private final static String DEFAULTREAL_STRING 		= "Unknown MagnitudeType value ";
	
	private com.thomsonreuters.upa.codec.Real _rsslReal = com.thomsonreuters.upa.codec.CodecFactory.createReal();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.REAL;
	}

	@Override
	public String magnitudeTypeAsString()
	{
		switch (magnitudeType())
		{
			case MagnitudeType.EXPONENT_NEG_14 :
				return EXPONENTNEG14_STRING;
			case MagnitudeType.EXPONENT_NEG_13 :
				return EXPONENTNEG13_STRING;
			case MagnitudeType.EXPONENT_NEG_12 :
				return EXPONENTNEG12_STRING;
			case MagnitudeType.EXPONENT_NEG_11 :
				return EXPONENTNEG11_STRING;
			case MagnitudeType.EXPONENT_NEG_10 :
				return EXPONENTNEG10_STRING;
			case MagnitudeType.EXPONENT_NEG_9 :
				return EXPONENTNEG9_STRING;
			case MagnitudeType.EXPONENT_NEG_8 :
				return EXPONENTNEG8_STRING;
			case MagnitudeType.EXPONENT_NEG_7 :
				return EXPONENTNEG7_STRING;
			case MagnitudeType.EXPONENT_NEG_6 :
				return EXPONENTNEG6_STRING;
			case MagnitudeType.EXPONENT_NEG_5 :
				return EXPONENTNEG5_STRING;
			case MagnitudeType.EXPONENT_NEG_4 :
				return EXPONENTNEG4_STRING;
			case MagnitudeType.EXPONENT_NEG_3 :
				return EXPONENTNEG3_STRING;
			case MagnitudeType.EXPONENT_NEG_2 :
				return EXPONENTNEG2_STRING;
			case MagnitudeType.EXPONENT_NEG_1 :
				return EXPONENTNEG1_STRING;
			case MagnitudeType.EXPONENT_0 :
				return EXPONENT0_STRING;
			case MagnitudeType.EXPONENT_POS_1 :
				return EXPONENTPOS1_STRING;
			case MagnitudeType.EXPONENT_POS_2 :
				return EXPONENTPOS2_STRING;
			case MagnitudeType.EXPONENT_POS_3 :
				return EXPONENTPOS3_STRING;
			case MagnitudeType.EXPONENT_POS_4 :
				return EXPONENTPOS4_STRING;
			case MagnitudeType.EXPONENT_POS_5 :
				return EXPONENTPOS5_STRING;
			case MagnitudeType.EXPONENT_POS_6 :
				return EXPONENTPOS6_STRING;
			case MagnitudeType.EXPONENT_POS_7 :
				return EXPONENTPOS7_STRING;
			case MagnitudeType.DIVISOR_1 :
				return DIVISOR1_STRING;
			case MagnitudeType.DIVISOR_2 :
				return DIVISOR2_STRING;
			case MagnitudeType.DIVISOR_4 :
				return DIVISOR4_STRING;
			case MagnitudeType.DIVISOR_8 :
				return DIVISOR8_STRING;
			case MagnitudeType.DIVISOR_16 :
				return DIVISOR16_STRING;
			case MagnitudeType.DIVISOR_32 :
				return DIVISOR32_STRING;
			case MagnitudeType.DIVISOR_64 :
				return DIVISOR64_STRING;
			case MagnitudeType.DIVISOR_128 :
				return DIVISOR128_STRING;
			case MagnitudeType.DIVISOR_256 :
				return DIVISOR256_STRING;
			case MagnitudeType.INFINITY :
				return INFINITY_STRING;
			case MagnitudeType.NEG_INFINITY :
				return NEGINFINITY_STRING;
			case MagnitudeType.NOT_A_NUMBER :
				return NAN_STRING;
			default :
				return (DEFAULTREAL_STRING + magnitudeType());
		}
	}
	
	@Override
	public long mantissa()
	{
		return _rsslReal.toLong();
	}

	@Override
	public int magnitudeType()
	{
		return _rsslReal.hint();
	}
	
	@Override
	public double asDouble()
	{
		return _rsslReal.toDouble();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
			return _rsslReal.toString();
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslReal.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslReal.blank();
		}
	}
}