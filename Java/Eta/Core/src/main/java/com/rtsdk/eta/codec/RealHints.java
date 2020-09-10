package com.rtsdk.eta.codec;

/**
 * UPA Real Formatting Hint enumeration values.
 * <p>
 * <b>Typical Use</b>
 * <p>
 * Conversion of {@link Real} to double/float is performed by the following formula:
 *
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * {@code
 * if ({@link Real#hint()} < {@link #FRACTION_1})
 * {
 *      outputValue = Real.value()*(pow(10,(Real.hint() - {@link #EXPONENT0})));
 * }
 * else
 * {
 *      outputValue = Real.value()*(pow(2,(Real.hint() - {@link #FRACTION_1})));
 * }
 * }
 * </pre>
 * </li>
 * </ul>
 * <p>
 * Conversion of double/float to {@link Real} is performed by the following formula:
 *
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * {@code
 * if (inputHint < {@link #FRACTION_1})
 * {
 *      Real.value = (inputValue)/(pow(10,(inputHint - {@link #EXPONENT0})));
 * }
 * else
 * {
 *      Real.value = (inputValue)/(pow(2,(inputHint - {@link #FRACTION_1})));
 * }
 * }
 * </pre>
 * </li>
 * </ul>
 * @see Real
 */
public class RealHints
{
    // RealHints class cannot be instantiated
    private RealHints()
    {
        throw new AssertionError();
    }

    /** Minimum exponent format hint value */
    public static final int MIN_EXP = 0;
    
    /** Value raised to the -14 power. Shifts decimal by 14 positions */
    public static final int EXPONENT_14 = 0;
    
    /** Value raised to the -13 power. Shifts decimal by 13 positions */
    public static final int EXPONENT_13 = 1;
    
    /** Value raised to the -12 power. Shifts decimal by 12 positions */
    public static final int EXPONENT_12 = 2;
    
    /** Value raised to the -11 power. Shifts decimal by 11 positions */
    public static final int EXPONENT_11 = 3;
    
    /** Value raised to the -10 power. Shifts decimal by 10 positions */
    public static final int EXPONENT_10 = 4;
    
    /** Value raised to the -9 power. Shifts decimal by 9 positions */
    public static final int EXPONENT_9 = 5;
    
    /** Value raised to the -8 power. Shifts decimal by 8 positions */
    public static final int EXPONENT_8 = 6;
    
    /** Value raised to the -7 power. Shifts decimal by 7 positions */
    public static final int EXPONENT_7 = 7;
    
    /** Value raised to the -6 power. Shifts decimal by 6 positions */
    public static final int EXPONENT_6 = 8;
    
    /** Value raised to the -5 power. Shifts decimal by 5 positions */
    public static final int EXPONENT_5 = 9;
    
    /** Value raised to the -4 power. Shifts decimal by 4 positions */
    public static final int EXPONENT_4 = 10;
    
    /** Value raised to the -3 power. Shifts decimal by 3 positions */
    public static final int EXPONENT_3 = 11;
    
    /** Value raised to the -2 power. Shifts decimal by 2 positions */
    public static final int EXPONENT_2 = 12;
    
    /** Value raised to the -1 power. Shifts decimal by 1 position */
    public static final int EXPONENT_1 = 13;
    
    /** Value raised to the power 0. Value undergoes no change */
    public static final int EXPONENT0 = 14;
    
    /** Value raised to the power 1. Adds or removes 1 trailing zero */
    public static final int EXPONENT1 = 15;
    
    /** Value raised to the power 2. Adds or removes 2 trailing zeros */
    public static final int EXPONENT2 = 16;
    
    /** Value raised to the power 3. Adds or removes 3 trailing zeros */
    public static final int EXPONENT3 = 17;
    
    /** Value raised to the power 4. Adds or removes 4 trailing zeros */
    public static final int EXPONENT4 = 18;
    
    /** Value raised to the power 5. Adds or removes 5 trailing zeros */
    public static final int EXPONENT5 = 19;
    
    /** Value raised to the power 6. Adds or removes 6 trailing zeros */
    public static final int EXPONENT6 = 20;
    
    /** Value raised to the power 7. Adds or removes 7 trailing zeros */
    public static final int EXPONENT7 = 21;
    
    /** Maximum exponent format hint value */
    public static final int MAX_EXP = 21;
    
    /** Minimum fraction format hint value */
    public static final int MIN_DIVISOR = 22;
    
    /**
     * Fractional denominator operation, equivalent to 1/1. Value undergoes no
     * change
     */
    public static final int FRACTION_1 = 22;
    
    /**
     * Fractional denominator operation, equivalent to 1/2. Adds or removes a
     * denominator of 2
     */
    public static final int FRACTION_2 = 23;
    
    /**
     * Fractional denominator operation, equivalent to 1/4. Adds or removes a
     * denominator of 4
     */
    public static final int FRACTION_4 = 24;
    
    /**
     * Fractional denominator operation, equivalent to 1/8. Adds or removes a
     * denominator of 8
     */
    public static final int FRACTION_8 = 25;
    
    /**
     * Fractional denominator operation, equivalent to 1/16. Adds or removes a
     * denominator of 16
     */
    public static final int FRACTION_16 = 26;
    
    /**
     * Fractional denominator operation, equivalent to 1/32. Adds or removes a
     * denominator of 32
     */
    public static final int FRACTION_32 = 27;
    
    /**
     * Fractional denominator operation, equivalent to 1/64. Adds or removes a
     * denominator of 64
     */
    public static final int FRACTION_64 = 28;
    
    /**
     * Fractional denominator operation, equivalent to 1/128. Adds or removes a
     * denominator of 128
     */
    public static final int FRACTION_128 = 29;
    
    /**
     * Fractional denominator operation, equivalent to 1/256. Adds or removes a
     * denominator of 256
     */
    public static final int FRACTION_256 = 30;
    
    /** Maximum fraction format hint value */
    public static final int MAX_DIVISOR = 30;
    
    /** Indicates value of infinity */
    public static final int INFINITY = 33;
    
    /** Indicates value of negative infinity  */
    public static final int NEG_INFINITY = 34;
    
    /** Indicates value is Not A Number (NaN) */
    public static final int NOT_A_NUMBER = 35;
    
}
