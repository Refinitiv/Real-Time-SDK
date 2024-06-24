/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_REAL_H_
#define __RSSL_REAL_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"

/**
 * @addtogroup RsslRealType
 * @{
 */

/**
 * @brief  The RsslReal allows the user to represent fractional or decimal values, with 
 *	minimal conversion overhead, by using an RsslInt and a format hint.
 * 
 * Typical Use:<BR>
 * User populates value with signed value and format with \ref RsslRealHints.<BR><BR>
 * <b>Conversion</b><BR>
 * <i>When converting from RsslReal to double:</i><BR>
 * If format hint is in Exponent Range (::RSSL_RH_EXPONENT_14 to ::RSSL_RH_EXPONENT7)<BR>
 * double = RsslReal::value * 10<sup>RsslReal::hint - ::RSSL_RH_EXPONENT0</sup> <BR><BR>
 * If format hint is in Fraction Range (::RSSL_RH_FRACTION_1 - ::RSSL_RH_FRACTION_256) <BR>
 * double =  RsslReal::value / 2<sup>RsslReal::hint - ::RSSL_RH_FRACTION_1</sup><BR><BR>
 * <i>When converting from double to RsslReal:</i><BR>
 * If format hint is in Exponent Range (::RSSL_RH_EXPONENT_14 to ::RSSL_RH_EXPONENT7)<BR>
 * RsslReal::value = floor((doubleValue / 10<sup>inHint - ::RSSL_RH_EXPONENT0</sup> ) + 0.5)<BR> <BR>
 * If format hint is in Fraction Range (::RSSL_RH_FRACTION_1 - ::RSSL_RH_FRACTION_256) <BR>
 * RsslReal::value = doubleValue * 2<sup>inHint - ::RSSL_RH_FRACTION_1</sup><BR>
 * @see rsslClearReal, RSSL_INIT_REAL, RSSL_BLANK_REAL, RsslRealHints
 */
typedef struct
{
	RsslBool		isBlank;	/*!< @brief RSSL_TRUE if this real value is blank  */
	RsslUInt8		hint;		/*!< @brief Populated with an enumerated value from \ref RsslRealHints */
	RsslInt			value;		/*!< @brief raw signed value with decimal point or denominator removed and accounted for in the RsslReal::hint */
} RsslReal;


/**
 * @brief RsslReal static initializer
 * @see RsslReal, rsslClearReal
 */
#define RSSL_INIT_REAL {0, 14, 0}


/**
 * @brief Static initializer that sets RsslReal structure to blank
 * @see RsslReal, rsslBlankReal
 */
#define RSSL_BLANK_REAL {1, 0, 0}

/**
 * @brief Clears an RsslReal structure
 * @param pReal Pointer to RsslReal to clear
 * @see RsslReal, RSSL_INIT_REAL
 */
RTR_C_INLINE void rsslClearReal( RsslReal *pReal )
{
	pReal->isBlank = 0;
	pReal->hint = 14;
	pReal->value = 0;
}


/**
 * @brief Blanks an RsslReal structure
 * @param pReal Pointer to RsslReal to set to blank
 * @see RsslReal, RSSL_BLANK_REAL
 */
RTR_C_INLINE void rsslBlankReal( RsslReal *pReal )
{
	pReal->isBlank = 1;
	pReal->hint = 0;
	pReal->value = 0;
}


/**
 * @brief RsslReal format hint enumeration values
 *
 * Typical Use:<BR>
 * Conversion of RsslReal to double/float is performed by the following formula.<BR>
 * if (RsslReal.hint < ::RSSL_RH_FRACTION_1)<BR>
 * {<BR>
 *&nbsp;&nbsp; //multiply the RsslReal::value by 10<sup>scaled RsslReal::hint</sup> <BR>
 *&nbsp;&nbsp; outputValue = rsslReal.value*(pow(10,(RsslReal.hint - ::RSSL_RH_EXPONENT0)));<BR>
 * }<BR>
 * else<BR>
 * {<BR>
 *&nbsp;&nbsp; //divide the RsslReal::value by 2<sup>scaled RsslReal::hint</sup> <BR>
 *&nbsp;&nbsp; outputValue = RsslReal.value*(pow(2,(RsslReal.hint - ::RSSL_RH_FRACTION_1)));<BR>
 * }<BR>
 * <BR>
 * Conversion of double/float to RsslReal is performed by the following formula.<BR>
 * if (inputHint < ::RSSL_RH_FRACTION_1)<BR>
 * {<BR>
 *&nbsp;&nbsp; //divide the double by 10<sup>scaled RsslReal::hint</sup>, add 0.5 and take the floor <BR>
 *&nbsp;&nbsp; RsslReal.value = floor(((inputValue)/(pow(10,(inputHint - ::RSSL_RH_EXPONENT0)))) + 0.5);<BR>
 * }<BR>
 * else<BR>
 * {<BR>
 *&nbsp;&nbsp; //multiply the double by 2<sup>scaled RsslReal::hint</sup> <BR>
 *&nbsp;&nbsp; RsslReal.value = (inputValue)/(pow(2,(inputHint - ::RSSL_RH_FRACTION_1)));<BR>
 * }<BR>
 * @see RsslReal
 */
typedef enum
{
	RSSL_RH_MIN_EXP			= 0,	/*!< Minimum exponent format hint value */
	RSSL_RH_EXPONENT_14		= 0,	/*!< Value raised to the -14 power. Shifts decimal by 14 positions. */
	RSSL_RH_EXPONENT_13		= 1,	/*!< Value raised to the -13 power. Shifts decimal by 13 positions. */
	RSSL_RH_EXPONENT_12		= 2,	/*!< Value raised to the -12 power. Shifts decimal by 12 positions. */
	RSSL_RH_EXPONENT_11		= 3,	/*!< Value raised to the -11 power. Shifts decimal by 11 positions. */
	RSSL_RH_EXPONENT_10		= 4,	/*!< Value raised to the -10 power. Shifts decimal by 10 positions. */
	RSSL_RH_EXPONENT_9		= 5,	/*!< Value raised to the -9 power. Shifts decimal by 9 positions. */
	RSSL_RH_EXPONENT_8		= 6,	/*!< Value raised to the -8 power. Shifts decimal by 8 positions. */
	RSSL_RH_EXPONENT_7		= 7,	/*!< Value raised to the -7 power. Shifts decimal by 7 positions. */
	RSSL_RH_EXPONENT_6		= 8,	/*!< Value raised to the -6 power. Shifts decimal by 6 positions. */
	RSSL_RH_EXPONENT_5		= 9,	/*!< Value raised to the -5 power. Shifts decimal by 5 positions. */
	RSSL_RH_EXPONENT_4		= 10,	/*!< Value raised to the -4 power. Shifts decimal by 4 positions. */
	RSSL_RH_EXPONENT_3		= 11,	/*!< Value raised to the -3 power. Shifts decimal by 3 positions. */
	RSSL_RH_EXPONENT_2		= 12,	/*!< Value raised to the -2 power. Shifts decimal by 2 positions. */
	RSSL_RH_EXPONENT_1		= 13,	/*!< Value raised to the -1 power. Shifts decimal by 1 position. */
	RSSL_RH_EXPONENT0		= 14,	/*!< Value raised to the power 0. Value undergoes no change. */
	RSSL_RH_EXPONENT1		= 15,	/*!< Value raised to the power 1. Adds or removes 1 trailing zero. */
	RSSL_RH_EXPONENT2		= 16,	/*!< Value raised to the power 2. Adds or removes 2 trailing zeros. */
	RSSL_RH_EXPONENT3		= 17,	/*!< Value raised to the power 3. Adds or removes 3 trailing zeros. */
	RSSL_RH_EXPONENT4		= 18,	/*!< Value raised to the power 4. Adds or removes 4 trailing zeros. */
	RSSL_RH_EXPONENT5		= 19,	/*!< Value raised to the power 5. Adds or removes 5 trailing zeros. */
	RSSL_RH_EXPONENT6		= 20,	/*!< Value raised to the power 6. Adds or removes 6 trailing zeros. */
	RSSL_RH_EXPONENT7		= 21,	/*!< Value raised to the power 7. Adds or removes 7 trailing zeros. */
	RSSL_RH_MAX_EXP			= 21,	/*!< Maximum exponent format hint value */
	RSSL_RH_MIN_DIVISOR		= 22,	/*!< Minimum fraction format hint value */
	RSSL_RH_FRACTION_1		= 22,	/*!< Fractional denominator operation, equivalent to 1/1. Value undergoes no change. */
	RSSL_RH_FRACTION_2		= 23,	/*!< Fractional denominator operation, equivalent to 1/2. Adds or removes a denominator of 2. */
	RSSL_RH_FRACTION_4		= 24,	/*!< Fractional denominator operation, equivalent to 1/4. Adds or removes a denominator of 4. */
	RSSL_RH_FRACTION_8		= 25,	/*!< Fractional denominator operation, equivalent to 1/8. Adds or removes a denominator of 8. */
	RSSL_RH_FRACTION_16		= 26,	/*!< Fractional denominator operation, equivalent to 1/16. Adds or removes a denominator of 16. */
	RSSL_RH_FRACTION_32		= 27,	/*!< Fractional denominator operation, equivalent to 1/32. Adds or removes a denominator of 32. */
	RSSL_RH_FRACTION_64		= 28,	/*!< Fractional denominator operation, equivalent to 1/64. Adds or removes a denominator of 64. */
	RSSL_RH_FRACTION_128	= 29,	/*!< Fractional denominator operation, equivalent to 1/128. Adds or removes a denominator of 128. */
	RSSL_RH_FRACTION_256	= 30,	/*!< Fractional denominator operation, equivalent to 1/256. Adds or removes a denominator of 256. */
	RSSL_RH_MAX_DIVISOR		= 30,	/*!< Maximum fraction format hint value */
	RSSL_RH_INFINITY		= 33,	/*!< RsslReal represents infinity */
	RSSL_RH_NEG_INFINITY	= 34,	/*!< RsslReal representes negative infinity */
	RSSL_RH_NOT_A_NUMBER	= 35	/*!< RsslReal is not a number (NaN) */
} RsslRealHints;

/** 
 * @brief General OMM strings associated with the different real hints.
 * @see RsslRealHints, rsslRealHintToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_14 = { 4, (char*)"E-14" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_13 = { 4, (char*)"E-13" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_12 = { 4, (char*)"E-12" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_11 = { 4, (char*)"E-11" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_10 = { 4, (char*)"E-10" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_9 = { 3, (char*)"E-9" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_8 = { 3, (char*)"E-8" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_7 = { 3, (char*)"E-7" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_6 = { 3, (char*)"E-6" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_5 = { 3, (char*)"E-5" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_4 = { 3, (char*)"E-4" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_3 = { 3, (char*)"E-3" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_2 = { 3, (char*)"E-2" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT_1 = { 3, (char*)"E-1" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT0 = { 2, (char*)"E0" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT1 = { 2, (char*)"E1" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT2 = { 2, (char*)"E2" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT3 = { 2, (char*)"E3" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT4 = { 2, (char*)"E4" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT5 = { 2, (char*)"E5" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT6 = { 2, (char*)"E6" };
static const RsslBuffer RSSL_OMMSTR_RH_EXPONENT7 = { 2, (char*)"E7" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_1 = { 9, (char*)"Fraction1" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_2 = { 9, (char*)"Fraction2" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_4 = { 9, (char*)"Fraction4" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_8 = { 9, (char*)"Fraction8" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_16 = { 10, (char*)"Fraction16" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_32 = { 10, (char*)"Fraction32" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_64 = { 10, (char*)"Fraction64" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_128 = { 11, (char*)"Fraction128" };
static const RsslBuffer RSSL_OMMSTR_RH_FRACTION_256 = { 11, (char*)"Fraction256" };
static const RsslBuffer RSSL_OMMSTR_RH_INFINITY = { 8, (char*)"Infinity" };
static const RsslBuffer RSSL_OMMSTR_RH_NEG_INFINITY = { 11, (char*)"NegInfinity" };
static const RsslBuffer RSSL_OMMSTR_RH_NOT_A_NUMBER = { 10, (char*)"NotANumber" };

/**
 * @brief Provide a general OMM string representation for real hint enumeration.
 * @see RsslRealHints
 */
RSSL_API const char* rsslRealHintToOmmString(RsslUInt8 hint);

/**
 * @}
 */

/**
 *	@addtogroup RsslRealUtils
 *	@{
 */
 
/**
 * @brief Check equality of two RsslReal types.
 *
 * @param lhs Pointer to the left hand side RsslReal structure.
 * @param rhs Pointer to the right hand side RsslReal structure.
 * @return RSSL_TRUE - if equal; RSSL_FALSE - if not equal.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRealIsEqual( const RsslReal *lhs, const RsslReal *rhs )
{
	return ((lhs->value == rhs->value) && (lhs->hint == rhs->hint) &&
			(lhs->isBlank == rhs->isBlank) ? RSSL_TRUE : RSSL_FALSE);
}

 
/**
 * @brief Convert double to a RsslReal
 * @param oReal RsslReal to populate with hint and value from double
 * @param iValue double to convert to RsslReal
 * @param iHint \ref RsslRealHints enumeration hint value to use for converting double
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion occurs; ::RSSL_RET_FAILURE if unable to convert, typically due to invalie hint value
 */
RSSL_API RsslRet rsslDoubleToReal(RsslReal * oReal, RsslDouble * iValue, RsslUInt8 iHint);

/**
 * @brief Convert float to a RsslReal 
 * @param oReal RsslReal to populate with hint and value from float
 * @param iValue float to convert to RsslReal
 * @param iHint \ref RsslRealHints enumeration hint value to use for converting float
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion; ::RSSL_RET_FAILURE if unable to convert, typically due to invalid hint value
 */
RSSL_API RsslRet rsslFloatToReal(RsslReal * oReal, RsslFloat * iValue, RsslUInt8 iHint);


/**
 * @brief Convert RsslReal to a double
 * @param oValue double to convert into
 * @param iReal RsslReal to convert to double
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion; ::RSSL_RET_FAILURE if unable to convert, typically because passed in RsslReal is blank or due to invalid hint value
 */
RSSL_API RsslRet rsslRealToDouble(RsslDouble * oValue, RsslReal * iReal);

/**
 * @brief Convert numeric string to double
 * @param oValue double to convert into
 * @param iNumericString \ref RsslBuffer to convert to double, where \ref RsslBuffer::data points to memory populated with a numeric string and  \ref RsslBuffer::length indicates number of bytes pointed to
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion;  ::RSSL_RET_INVALID_DATA if numeric string is invalid 
 */
RSSL_API RsslRet rsslNumericStringToDouble(RsslDouble * oValue, RsslBuffer * iNumericString);


/**
 * @brief Converts a numeric string to an RsslReal, similar to atof, but with support for fractions and decimals.  No null termination required.
 * @param oReal RsslReal to convert string into
 * @param iNumericString Populated \ref RsslBuffer to convert into RsslReal, where \ref RsslBuffer::data points to memory populated with a numeric string and \ref RsslBuffer::length indicates number of bytes pointed to
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion;  ::RSSL_RET_INVALID_DATA if numeric string is invalid
 */
RSSL_API RsslRet rsslNumericStringToReal(RsslReal * oReal, RsslBuffer * iNumericString);

/**
 * @brief Convert RsslReal to a numeric string
 * @param buffer \ref RsslBuffer to use for string representation. \ref RsslBuffer::data should point to some amount of memory and \ref RsslBuffer::length specifies size of memory.
 * @param iReal RsslReal to convert into a numeric string
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion;  ::RSSL_RET_INVALID_DATA if numeric string is invalid; ::RSSL_RET_BUFFER_TOO_SMALL if passed in buffer does not have sufficient length for conversion
 */
RSSL_API RsslRet rsslRealToString(RsslBuffer * buffer, RsslReal * iReal);

/**
 * @}
 */





#ifdef __cplusplus
}
#endif


#endif
