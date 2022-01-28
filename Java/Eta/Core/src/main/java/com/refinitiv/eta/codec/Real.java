package com.refinitiv.eta.codec;

import java.math.BigDecimal;

/**
 * This type allows the user to represent fractional or decimal values, with
 * minimal conversion overhead, by using a number and a format hint.
 * 
 * Typical Use:<BR>
 * User populates value with signed value and format with {@link RealHints}.<BR>
 * If format hint is in Exponent Range (0 - 14) value * 10^(format - 14)<BR>
 * If format hint is in Fraction Range (22 - 30) value * (1/2)^(format - 22)<BR>
 * 
 * An application can convert between a Real and a Java float or double as needed.
 * Converting a Real to a double or float is typically done to perform calculations 
 * or display data after receiving it. 
 * The conversion process adds or removes decimal or denominator information from the 
 * value to optimize transmission sizes. In a Real type, the decimal or denominator 
 * information is indicated by the Real.hint and the Real.toLong indicates the value, 
 * less any decimal or denominator. If the Real.isBlank method is true, this is handled 
 * as blank regardless of information contained in the Real.hint and Real.toLong methods.

 * @see RealHints
 */
public interface Real
{
    /**
     * Clears {@link Real}.
     */
    public void clear();

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destReal the value getting populated with the values of the calling Object
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if the destReal is null. 
     */
    public int copy(Real destReal);
    
    /**
     * Used to encode a Real into a buffer.
     * 
     * @param iter {@link EncodeIterator} with buffer to encode into. Iterator
     *            should also have appropriate version information set
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Decode Real.
     * 
     * @param iter DecodeIterator with buffer to decode from and appropriate
     *            version information set
     * 
     * @return {@link CodecReturnCodes#SUCCESS} if success,
     *         {@link CodecReturnCodes#INCOMPLETE_DATA} if failure,
     *         {@link CodecReturnCodes#BLANK_DATA} if data is blank value.
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Sets Real to blank.
     */
    public void blank();

    /**
     * Returns a Boolean value. Indicates whether data is considered blank. If true,
     * value and hint should be ignored, if false value and hint determine the resultant
     * value. This allows Real to be represented as blank when used as either a primitive
     * type or a set-defined primitive type.
     *
     * @return true, if is blank
     */
    public boolean isBlank();

    /**
     * Check equality of two Real types.
     * 
     * @param thatReal the other Real to compare to this one
     * 
     * @return true if equal, false otherwise
     */
    public boolean equals(Real thatReal);

    /**
     * Convert Real to a double.
     * 
     * @return the double representation of this {@link Real}
     */
    public double toDouble();

    /**
     * Convert Real to a BigDecimal.
     *
     * @return the BigDecimal representation of this {@link Real}.
     * If hint is {@link RealHints#NOT_A_NUMBER}, {@link RealHints#INFINITY} or {@link RealHints#NEG_INFINITY}
     * the returned value will be null.
     * If {@link Real} is blank the returned value will be null.
     */
    public BigDecimal toBigDecimal();

    /**
     * Convert Real to a numeric string.
     * 
     * @return the string representation of this {@link Real}
     */
    public String toString();

    /**
     * Convert double to a {@link Real}. Hint must be in the range of
     * {@link RealHints#EXPONENT_14} - {@link RealHints#MAX_DIVISOR}.
     * 
     * @param value double to convert to {@link Real}
     * @param hint {@link RealHints} enumeration hint value to use for converting double
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value or hint is invalid. 
     */
    public int value(double value, int hint);

    /**
     * Convert float to a {@link Real}. Hint must be in the range of
     * {@link RealHints#EXPONENT_14} - {@link RealHints#MAX_DIVISOR}.
     * 
     * @param value float to convert to {@link Real}
     * @param hint  {@link RealHints} enumeration hint value to use for converting float
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value or hint is invalid. 
     */
    public int value(float value, int hint);

    /**
     * Convert String to a {@link Real}.
     * 
     * @param value String to convert to {@link Real}
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value is invalid. 
     */
    public int value(String value);

    /**
     * Set raw signed value and hint. Hint must be in the range of
     * {@link RealHints#EXPONENT_14} - {@link RealHints#MAX_DIVISOR}.
     * 
     * @param value raw signed value
     * @param hint {@link RealHints} enumeration hint
     * 
     * @return {@link CodecReturnCodes#SUCCESS} on success,
     *         {@link CodecReturnCodes#INVALID_ARGUMENT} if value or hint is invalid. 
     */
    public int value(long value, int hint);

    /**
     * The raw value represented by the Real (omitting any decimal or denominator).
     * Typically requires application of the hint before interpreting or performing
     * any calculations. This member can currently represent up to 63 bits and a one-bit
     * sign (positive or negative).
     * 
     * @return the value as long
     */
    public long toLong();

    /**
     * Populated with an enumerated value from {@link RealHints}.
     * raw value contained in Real. Hint values can add or remove up to seven trailing
     * zeros, 14 decimal places, or fractional denominators up to 256.
     * 
     * @return the hint
     */
    public int hint();
}