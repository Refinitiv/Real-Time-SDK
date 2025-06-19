/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmReal represents Real number in Omm.
/// </summary>
/// <remarks>
/// OmmReal encapsulates magnitude type and mantissa information.<br/>
/// OmmReal is a read only class.<br/>
/// This class is used for extraction of Real info only.<br/>
/// All methods in this class are single threaded.<br/>
/// <example>
/// The following code snippet shows setting of Real in FieldList:<br/>
///
/// <code>
/// FieldList fList = new FieldList();
///
/// List.AddReal(321, 245, OmmReal.MagnitudeTypes.EXPONENT_NEG_8)
/// 	.AddRealFromDouble(345, 245.234, OmmReal.MagnitudeTypes.EXPONENT_NEG_3)
/// 	.Complete();
/// </code>
///
/// The following code snippet shows extraction of OmmReal from FieldList:<br/>
///
/// <code>
/// void DecodeFieldList( FieldList fList )
/// {
///     var fListIt = fList.GetEnumerator();
/// 	while (fListIt.MoveNext())
/// 	{
/// 		FieldEntry fEntry = fListIt.Current;
///
/// 		if ( fEntry.Code != Data.DataCode.BLANK )
/// 			switch ( fEntry.LoadType )
/// 			{
/// 				case DataType.DataTypes.REAL:
/// 					OmmReal ommReal = fEntry.OmmRealValue();
/// 					long mantissa = ommReal.Mantissa;
/// 				break;
/// 			}
/// 	}
/// }
/// </code>
/// </example>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmReal : Data
{
    #region Public members

    /// <summary>
    /// MagnitudeType represents item stream data state.
    /// </summary>
    public static class MagnitudeTypes
    {
        /// <summary>
        /// Power of -14.
        /// </summary>
        public const int EXPONENT_NEG_14 = 0;

        /// <summary>
        /// Power of -13.
        /// </summary>
        public const int EXPONENT_NEG_13 = 1;

        /// <summary>
        /// Power of -12.
        /// </summary>
        public const int EXPONENT_NEG_12 = 2;

        /// <summary>
        /// Power of -11.
        /// </summary>
        public const int EXPONENT_NEG_11 = 3;

        /// <summary>
        /// Power of -10.
        /// </summary>
        public const int EXPONENT_NEG_10 = 4;

        /// <summary>
        /// Power of -9.
        /// </summary>
        public const int EXPONENT_NEG_9 = 5;

        /// <summary>
        /// Power of -8.
        /// </summary>
        public const int EXPONENT_NEG_8 = 6;

        /// <summary>
        /// Power of -7.
        /// </summary>
        public const int EXPONENT_NEG_7 = 7;

        /// <summary>
        /// Power of -6.
        /// </summary>
        public const int EXPONENT_NEG_6 = 8;

        /// <summary>
        /// Power of -5.
        /// </summary>
        public const int EXPONENT_NEG_5 = 9;

        /// <summary>
        /// Power of -4.
        /// </summary>
        public const int EXPONENT_NEG_4 = 10;

        /// <summary>
        /// Power of -3.
        /// </summary>
        public const int EXPONENT_NEG_3 = 11;

        /// <summary>
        /// Power of -2.
        /// </summary>
        public const int EXPONENT_NEG_2 = 12;

        /// <summary>
        /// Power of -1.
        /// </summary>
        public const int EXPONENT_NEG_1 = 13;

        /// <summary>
        /// Power of 0.
        /// </summary>
        public const int EXPONENT_0 = 14;

        /// <summary>
        /// Power of 1.
        /// </summary>
        public const int EXPONENT_POS_1 = 15;

        /// <summary>
        /// Power of 2.
        /// </summary>
        public const int EXPONENT_POS_2 = 16;

        /// <summary>
        /// Power of 3.
        /// </summary>
        public const int EXPONENT_POS_3 = 17;

        /// <summary>
        /// Power of 4.
        /// </summary>
        public const int EXPONENT_POS_4 = 18;

        /// <summary>
        /// Power of 5.
        /// </summary>
        public const int EXPONENT_POS_5 = 19;

        /// <summary>
        /// Power of 6.
        /// </summary>
        public const int EXPONENT_POS_6 = 20;

        /// <summary>
        /// Power of 7.
        /// </summary>
        public const int EXPONENT_POS_7 = 21;

        /// <summary>
        /// Divisor of 1.
        /// </summary>
        public const int DIVISOR_1 = 22;

        /// <summary>
        /// Divisor of 2.
        /// </summary>
        public const int DIVISOR_2 = 23;

        /// <summary>
        /// Divisor of 4.
        /// </summary>
        public const int DIVISOR_4 = 24;

        /// <summary>
        /// Divisor of 8.
        /// </summary>
        public const int DIVISOR_8 = 25;

        /// <summary>
        /// Divisor of 16.
        /// </summary>
        public const int DIVISOR_16 = 26;

        /// <summary>
        /// Divisor of 32.
        /// </summary>
        public const int DIVISOR_32 = 27;

        /// <summary>
        /// Divisor of 64.
        /// </summary>
        public const int DIVISOR_64 = 28;

        /// <summary>
        /// Divisor of 128.
        /// </summary>
        public const int DIVISOR_128 = 29;

        /// <summary>
        /// Divisor of 256.
        /// </summary>
        public const int DIVISOR_256 = 30;

        /// <summary>
        /// Represents infinity.
        /// </summary>
        public const int INFINITY = 33;

        /// <summary>
        /// Represents negative infinity.
        /// </summary>
        public const int NEG_INFINITY = 34;

        /// <summary>
        /// Represents not a number (NaN).
        /// </summary>
        public const int NOT_A_NUMBER = 35;
    }

    /// <summary>
    /// Returns the MagnitudeType value as a string format.
    /// </summary>
    /// <param name="magnitudeType">the magnitude type</param>
    /// <returns>string representation of the passed in MagnitudeType</returns>
    internal static string MagnitudeTypeAsString(int magnitudeType)
    {
        return magnitudeType switch
        {
            MagnitudeTypes.EXPONENT_NEG_14 => EXPONENTNEG14_STRING,
            MagnitudeTypes.EXPONENT_NEG_13 => EXPONENTNEG13_STRING,
            MagnitudeTypes.EXPONENT_NEG_12 => EXPONENTNEG12_STRING,
            MagnitudeTypes.EXPONENT_NEG_11 => EXPONENTNEG11_STRING,
            MagnitudeTypes.EXPONENT_NEG_10 => EXPONENTNEG10_STRING,
            MagnitudeTypes.EXPONENT_NEG_9 => EXPONENTNEG9_STRING,
            MagnitudeTypes.EXPONENT_NEG_8 => EXPONENTNEG8_STRING,
            MagnitudeTypes.EXPONENT_NEG_7 => EXPONENTNEG7_STRING,
            MagnitudeTypes.EXPONENT_NEG_6 => EXPONENTNEG6_STRING,
            MagnitudeTypes.EXPONENT_NEG_5 => EXPONENTNEG5_STRING,
            MagnitudeTypes.EXPONENT_NEG_4 => EXPONENTNEG4_STRING,
            MagnitudeTypes.EXPONENT_NEG_3 => EXPONENTNEG3_STRING,
            MagnitudeTypes.EXPONENT_NEG_2 => EXPONENTNEG2_STRING,
            MagnitudeTypes.EXPONENT_NEG_1 => EXPONENTNEG1_STRING,
            MagnitudeTypes.EXPONENT_0 => EXPONENT0_STRING,
            MagnitudeTypes.EXPONENT_POS_1 => EXPONENTPOS1_STRING,
            MagnitudeTypes.EXPONENT_POS_2 => EXPONENTPOS2_STRING,
            MagnitudeTypes.EXPONENT_POS_3 => EXPONENTPOS3_STRING,
            MagnitudeTypes.EXPONENT_POS_4 => EXPONENTPOS4_STRING,
            MagnitudeTypes.EXPONENT_POS_5 => EXPONENTPOS5_STRING,
            MagnitudeTypes.EXPONENT_POS_6 => EXPONENTPOS6_STRING,
            MagnitudeTypes.EXPONENT_POS_7 => EXPONENTPOS7_STRING,
            MagnitudeTypes.DIVISOR_1 => DIVISOR1_STRING,
            MagnitudeTypes.DIVISOR_2 => DIVISOR2_STRING,
            MagnitudeTypes.DIVISOR_4 => DIVISOR4_STRING,
            MagnitudeTypes.DIVISOR_8 => DIVISOR8_STRING,
            MagnitudeTypes.DIVISOR_16 => DIVISOR16_STRING,
            MagnitudeTypes.DIVISOR_32 => DIVISOR32_STRING,
            MagnitudeTypes.DIVISOR_64 => DIVISOR64_STRING,
            MagnitudeTypes.DIVISOR_128 => DIVISOR128_STRING,
            MagnitudeTypes.DIVISOR_256 => DIVISOR256_STRING,
            MagnitudeTypes.INFINITY => INFINITY_STRING,
            MagnitudeTypes.NEG_INFINITY => NEGINFINITY_STRING,
            MagnitudeTypes.NOT_A_NUMBER => NAN_STRING,
            _ => (DEFAULTREAL_STRING + magnitudeType)
        };
    }

    /// <summary>
    /// Returns the MagnitudeType value as a string format.
    /// </summary>
    /// <returns>string representation of this object MagnitudeType</returns>
    public string MagnitudeTypeAsString()
    {
        return MagnitudeTypeAsString(MagnitudeType);
    }

    /// <summary>
    /// Returns Mantissa.
    /// </summary>
    public long Mantissa { get => m_Real.ToLong(); }

    /// <summary>
    /// Returns MagnitudeType.
    /// </summary>
    public int MagnitudeType { get => m_Real.Hint; }

    /// <summary>
    /// Returns as double value.
    /// </summary>
    /// <returns>value of Real as double</returns>
    public double AsDouble() { return m_Real.ToDouble(); }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmReal"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
            return m_Real.ToString();
    }

    #endregion

    #region Implementation details

    internal OmmReal() 
    { 
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmReal;
        m_dataType = Access.DataType.DataTypes.REAL;
    }

    [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
    internal CodecReturnCode DecodeOmmReal(DecodeIterator dIter)
    {
        CodecReturnCode decodeRetValue = m_Real.Decode(dIter);
        if (decodeRetValue == CodecReturnCode.SUCCESS)
        {
            Code = DataCode.NO_CODE;
            return CodecReturnCode.SUCCESS;
        }       
        else if (CodecReturnCode.INVALID_ARGUMENT == decodeRetValue)
        {
            return CodecReturnCode.INVALID_ARGUMENT;
        }

        Code = DataCode.BLANK;
        m_Real.Blank();
        return CodecReturnCode.SUCCESS;
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    private LSEG.Eta.Codec.Real m_Real = new();

    private const string EXPONENTNEG14_STRING = "Power of -14. Stringeration value is 0";
    private const string EXPONENTNEG13_STRING = "Power of -13";
    private const string EXPONENTNEG12_STRING = "Power of -12";
    private const string EXPONENTNEG11_STRING = "Power of -11";
    private const string EXPONENTNEG10_STRING = "Power of -10";
    private const string EXPONENTNEG9_STRING = "Power of -9";
    private const string EXPONENTNEG8_STRING = "Power of -8";
    private const string EXPONENTNEG7_STRING = "Power of -7";
    private const string EXPONENTNEG6_STRING = "Power of -6";
    private const string EXPONENTNEG5_STRING = "Power of -5";
    private const string EXPONENTNEG4_STRING = "Power of -4";
    private const string EXPONENTNEG3_STRING = "Power of -3";
    private const string EXPONENTNEG2_STRING = "Power of -2";
    private const string EXPONENTNEG1_STRING = "Power of -1";
    private const string EXPONENT0_STRING = "Power of 0";
    private const string EXPONENTPOS1_STRING = "Power of 1";
    private const string EXPONENTPOS2_STRING = "Power of 2";
    private const string EXPONENTPOS3_STRING = "Power of 3";
    private const string EXPONENTPOS4_STRING = "Power of 4";
    private const string EXPONENTPOS5_STRING = "Power of 5";
    private const string EXPONENTPOS6_STRING = "Power of 6";
    private const string EXPONENTPOS7_STRING = "Power of 7";
    private const string DIVISOR1_STRING = "Divisor of 1";
    private const string DIVISOR2_STRING = "Divisor of 2";
    private const string DIVISOR4_STRING = "Divisor of 4";
    private const string DIVISOR8_STRING = "Divisor of 8";
    private const string DIVISOR16_STRING = "Divisor of 16";
    private const string DIVISOR32_STRING = "Divisor of 32";
    private const string DIVISOR64_STRING = "Divisor of 64";
    private const string DIVISOR128_STRING = "Divisor of 128";
    private const string DIVISOR256_STRING = "Divisor of 256";
    private const string INFINITY_STRING = "Inf";
    private const string NEGINFINITY_STRING = "-Inf";
    private const string NAN_STRING = "NaN";
    private const string DEFAULTREAL_STRING = "Unknown MagnitudeType value ";

    #endregion

}
