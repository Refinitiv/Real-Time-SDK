package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.util.JsonFactory;

import java.lang.Double;
import java.lang.Float;
import java.nio.ByteBuffer;


class BasicPrimitiveConverter {

    private static final int[] intLengths = {9, 99, 999, 9999, 99999,
            999999, 9999999, 99999999, 999999999, Integer.MAX_VALUE};
    private static final long[] longLengths = {9, 99, 999, 9999, 99999, 999999, 9999999, 99999999,
            999999999, 9999999999L, 99999999999L, 999999999999L, 9999999999999L, 99999999999999L,
            999999999999999L, 9999999999999999L, 99999999999999999L, 999999999999999999L, Long.MAX_VALUE};
    private static final double log10_5 = Math.log10(5);
    private static final double log10_2 = Math.log10(2);
    private static final long double_exp_mask = 0x7ff0000000000000L;
    private static final long double_mantissa_mask = 0x000fffffffffffffL;
    private static final long double_sign_mask = 0x8000000000000000L;
    private static final int float_sign_mask = 0x80000000;
    private static final int float_exp_mask = 0x7f800000;
    private static final int float_mantissa_mask = 0x007fffff;
    private static final int[] numerator_mask = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
    private static final long DIV_MAGIC_MULT1 = 0xCCCCCCCDL;
    private static final int SHIFT1 = 32 + 3;
    private static final long DIV_MAGIC_MULT4 = 3518437209L;
    private static final int SHIFT4 = 32 + 13;
    private static final int CHUNK_LENGTH = 4;
    private static final int INT_MAX_STORED = (int) Math.pow(10, CHUNK_LENGTH) - 1;
    private static final int INT_DIVISOR = (int) Math.pow(10, CHUNK_LENGTH);
    private final static char[][] charInts;
    private static final StringBuilder sb = new StringBuilder();

    private static final RmtesDecoder decoder = CodecFactory.createRmtesDecoder();
    private static RmtesCacheBuffer cacheBuffer = CodecFactory.createRmtesCacheBuffer(2000);
    private static RmtesBuffer rmtesBuffer = CodecFactory.createRmtesBuffer(2000);

    static {
        charInts = new char[INT_MAX_STORED + 1][];
        for (int i = 0; i <= INT_MAX_STORED; i++) {
            charInts[i] = getCharRepresentation(i);
        }
    }

    private static char[] getCharRepresentation(int val) {
        int q;
        char[] res = new char[getIntLengthCompare(val)];
        for (int i = res.length - 1; i >= 0; i--) {
            q = divide1(val);
            res[i] = ConstCharArrays.digits[val - q * 10];
            val = q;
        }
        return res;
    }

    private static int divide1(int val) {
        assert(val >= 0);
        return (int) ((val * DIV_MAGIC_MULT1) >>> SHIFT1) ;
    }
    private static int divide4(int val) {
        assert (val >= 0);
        return (int) ((val * DIV_MAGIC_MULT4) >>> SHIFT4);
    }

    static int getLongLengthCompare(long value) {
        if (value == Long.MIN_VALUE)
            return 20;
        if (value < 0)
            return getPositiveLongLengthCompare(-value) + 1;
        else
            return getPositiveLongLengthCompare(value);
    }
    static int getPositiveLongLengthCompare(long value) {
        assert(value >= 0);
        int iter = 1;
        while (longLengths[iter - 1] < value) {
            iter++;
        }
        return iter;
    }
    static int getIntLengthDivide(int value) {
        if (value == Integer.MIN_VALUE)
            return 11;

        int length = 0;
        if (value < 0) {
            length++;
            value *= -1;
        }
        do {
            length++;
            value = value / 10;
        } while (value > 0);

        return length;
    }
    static int getIntLengthDivide1(int value) {
        if (value == Integer.MIN_VALUE)
            return 11;

        int length = 0;
        if (value < 0) {
            length++;
            value *= -1;
        }
        do {
            length++;
            value = divide1(value);
        } while (value > 0);

        return length;
    }
    static int getIntLengthCompare(int value) {
        if (value == Integer.MIN_VALUE)
            return 11;

        int res = 1;
        if (value < 0) {
            res++;
            value *= -1;
        }

        int iter = 0;
        while (intLengths[iter] < value) {
            iter++;
        }
        res += iter;

        return res;
    }

    static void writeInt(int value, int posLength, byte[] buffer, int start) {

        if (value == Integer.MIN_VALUE) {
            BufferHelper.copyToByteArray(ConstCharArrays.minInt, start, buffer);
        } else {
            if (value < 0) {
                buffer[start++] = '-';
                value *= -1;
            }

            int q, r;
            int end = start + posLength - 1;

            while (true) {
                if (value <= INT_MAX_STORED) {
                    BufferHelper.copyToByteArray(charInts[value], start, buffer);
                    break;
                } else {
                    q = divide4(value);
                    r = value - q * INT_DIVISOR;
                    BufferHelper.fillZeroes(CHUNK_LENGTH - charInts[r].length, buffer, end - CHUNK_LENGTH + 1);
                    BufferHelper.copyToByteArray(charInts[r], end - charInts[r].length + 1, buffer);
                    end -= CHUNK_LENGTH;
                    value = q;
                }
            }
        }

    }

    static void writeLong(long value, int posLength, int start, byte[] buffer) {

        if (value == Long.MIN_VALUE) {
            BufferHelper.copyToByteArray(ConstCharArrays.minLong, start, buffer);
        } else {
            if (value < 0) {
                buffer[start++] = '-';
                value *= -1;
            }

            long q;
            int r;
            int end = start + posLength - 1;

            while (true) {
                if (value <= Integer.MAX_VALUE) {
                    writeInt((int)value, getIntLengthCompare((int)value), buffer, start);
                    break;
                } else {
                    q = value / INT_DIVISOR;
                    r = (int)(value - q * INT_DIVISOR);
                    BufferHelper.fillZeroes(CHUNK_LENGTH - charInts[r].length, buffer, end - CHUNK_LENGTH + 1);
                    BufferHelper.copyToByteArray(charInts[r], end - charInts[r].length + 1, buffer);
                    end -= CHUNK_LENGTH;
                    value = q;
                }
            }
        }
    }

    static boolean writeLong(long value, JsonBuffer buffer, JsonConverterError error) {

        int length = getLongLengthCompare(value);
        if (BufferHelper.checkAndResize(buffer, length, error)) {
            writeLong(value, getPositiveLongLengthCompare(Math.abs(value)), buffer.position, buffer.data);
            buffer.position += length;
            return true;
        } else
            return false;
    }

    static boolean writeReal(Real value, JsonBuffer buffer, boolean asString, JsonConverterError error) {

        int length = getRealLength(value, asString);
        if (BufferHelper.checkAndResize(buffer, length, error)) {
            writeReal(value, buffer.data, buffer.position, asString);
            buffer.position += length;
            return true;
        } else {
            return false;
        }
    }

    static int getRealLength(Real real, boolean asString) {

        if (real.isBlank())
            return ConstCharArrays.nullString.length();

        if (real.hint() > RealHints.MAX_DIVISOR) {
            switch (real.hint()) {
                case RealHints.INFINITY:
                    return ConstCharArrays.inf.length  + (asString ? 2 : 0);
                case RealHints.NEG_INFINITY:
                    return ConstCharArrays.infNeg.length  + (asString ? 2 : 0);
                case RealHints.NOT_A_NUMBER:
                    return ConstCharArrays.nan.length  + (asString ? 2 : 0);
                default:
                    break;
            }
        } else {
            int length = 0;
            long value = real.toLong();
            if (value == 0) {
                return 1 + (asString ? 2 : 0);
            }
            if (value < 0) {
                value *= -1;
                length++;
            }
            if (real.hint() - RealHints.FRACTION_1 >= 0) {
                if (real.hint() - RealHints.FRACTION_1 > 0) {
                    long whole = value >> (real.hint() - RealHints.FRACTION_1);
                    int remainder = (int) (value & numerator_mask[real.hint() - RealHints.FRACTION_1]);
                    if(remainder == 0) {
                    	return length + getLongLengthCompare(whole) + (asString? 2 : 0);
                    } else {
                    	return length + getLongLengthCompare(whole) + 1 + (real.hint() - RealHints.FRACTION_1 - Long.numberOfTrailingZeros(remainder)) + (asString ? 2 : 0);
                    }
                } else {
                    return length + getLongLengthCompare(value)  + (asString ? 2 : 0);
                }
            } else {
                int exponent = real.hint() - RealHints.EXPONENT0;
                int longLength = getLongLengthCompare(value);
                if (exponent >= 0)
                    return (asString ? 2 : 0) + (value != 0 ? length + longLength + exponent : 1);
                else {
                    if (exponent == 0)
                        return length + longLength  + (asString ? 2 : 0);
                    else if (longLength + exponent > 0)
                        return length + longLength + 1  + (asString ? 2 : 0);
                    else return length + 1 + 1 - exponent  + (asString ? 2 : 0);
                }
            }
        }
        return -1;
    }

    static void writeReal(Real real, byte[] buffer, int start, boolean asString) {
        long value = real.toLong();

        if (real.isBlank()) {
            BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, start, buffer);
            return;
        }

        if (asString)
            buffer[start++] = '"';

        switch (real.hint()) {
            case RealHints.INFINITY:
                BufferHelper.copyToByteArray(ConstCharArrays.inf, start, buffer);
                return;
            case RealHints.NEG_INFINITY:
                BufferHelper.copyToByteArray(ConstCharArrays.infNeg, start, buffer);
                return;
            case RealHints.NOT_A_NUMBER:
                BufferHelper.copyToByteArray(ConstCharArrays.nan, start, buffer);
                return;
            default:
                break;
        }

        if (value == 0) {
            buffer[start++] = '0';
        } else {

            if (value < 0) {
                value *= -1;
                buffer[start++] = '-';
            }

            if (real.hint() - RealHints.FRACTION_1 >= 0) {
                if (real.hint() - RealHints.FRACTION_1 > 0) {
                    long whole = value >> (real.hint() - RealHints.FRACTION_1);
                    int remainder = (int) (value & numerator_mask[real.hint() - RealHints.FRACTION_1]);
                    if (whole == 0) {
                        buffer[start++] = '0';
                    } else {
                        int l = getLongLengthCompare(whole);
                        writeLong(whole, l, start, buffer);
                        start += l;
                    }
                    if (remainder > 0) {
                        int f = real.hint() - RealHints.FRACTION_1;
                        buffer[start++] = '.';
                        int p2 = Long.numberOfTrailingZeros(remainder);
                        int dec = (remainder >> p2) * (int) Math.pow(5, f - p2);
                        int l = f - p2 - getIntLengthCompare(dec);
                        BufferHelper.fillZeroes(f - p2 - getIntLengthCompare(dec), buffer, start);
                        writeInt(dec, getIntLengthCompare(dec), buffer, start + l);
                    }
                } else {
                    writeLong(value, getLongLengthCompare(value), start, buffer);
                }
            } else {
                int exponent = real.hint() - RealHints.EXPONENT0;
                int longLength = getLongLengthCompare(value);
                if (exponent >= 0) {
                    writeLong(value, longLength, start, buffer);
                    BufferHelper.fillZeroes(exponent, buffer, start + longLength);
                } else {
                    if (exponent == 0)
                        writeLong(value, longLength, start, buffer);
                       else if (longLength + exponent > 0) {
                        int end = start + longLength;
                        long nv = value;
                        while (exponent++ < 0) {
                            nv = value / 10;
                            buffer[end--] = (byte)(ConstCharArrays.digits[(int)(value - nv * 10)] & 0xFF);
                            value = nv;
                        }
                        buffer[end] = '.';
                        writeLong(nv, end-start, start, buffer);
                    }
                    else {
                        buffer[start++] = '0';
                        buffer[start++] = '.';
                        int x = Math.abs(longLength + exponent);
                        BufferHelper.fillZeroes(x, buffer, start);
                        writeLong(value, longLength, start + x, buffer);
                    }
                }
            }
        }

        if (asString)
            buffer[start++] = '"';
    }

    /* Floating-point numbers conversion */

    static boolean writeFloat(com.refinitiv.eta.codec.Float value, JsonBuffer buffer, boolean asString, JsonConverterError error) {

        if (value.isBlank()) {
            if (BufferHelper.checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else
                return false;
        }

        float val = value.toFloat();

        if (Float.isNaN(val))
            return BufferHelper.writeArray(ConstCharArrays.nan, buffer, asString, error);
        else if (val == Float.POSITIVE_INFINITY)
            return BufferHelper.writeArray(ConstCharArrays.inf, buffer, asString, error);
        else if (val == Float.NEGATIVE_INFINITY)
            return BufferHelper.writeArray(ConstCharArrays.infNeg, buffer, asString, error);
        else {
            sb.delete(0, sb.length());
            sb.append(val);
            if (BufferHelper.checkAndResize(buffer, asString ? sb.length() + 2 : sb.length(), error)) {
                if (asString)
                    buffer.data[buffer.position++] = '\"';
                for (int i = 0; i < sb.length(); i++) {
                    buffer.data[buffer.position++] = (byte)sb.charAt(i);
                }
                if (asString)
                    buffer.data[buffer.position++] = '\"';
                return true;
            } else
                return false;
        }
    }

    static boolean writeDouble(com.refinitiv.eta.codec.Double value, JsonBuffer buffer, boolean asString, JsonConverterError error) {

        if (value.isBlank()) {
            if (BufferHelper.checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else
                return false;
        }

        double val = value.toDouble();

        if (Double.isNaN(val))
            return BufferHelper.writeArray(ConstCharArrays.nan, buffer, asString, error);
        else if (val == Double.POSITIVE_INFINITY)
            return BufferHelper.writeArray(ConstCharArrays.inf, buffer, asString, error);
        else if (val == Double.NEGATIVE_INFINITY)
            return BufferHelper.writeArray(ConstCharArrays.infNeg, buffer, asString, error);
        else {
            sb.delete(0, sb.length());
            sb.append(val);
            if (BufferHelper.checkAndResize(buffer, asString ? sb.length() + 2 : sb.length(), error)) {
                if (asString)
                    buffer.data[buffer.position++] = '\"';
                for (int i = 0; i < sb.length(); i++) {
                    buffer.data[buffer.position++] = (byte)sb.charAt(i);
                }
                if (asString)
                    buffer.data[buffer.position++] = '\"';
                return true;
            } else
                return false;
        }
    }

    static int writeDouble0(double value, int start, byte[] buffer) {
        long bits = Double.doubleToRawLongBits(value);
        boolean positive = (bits & double_sign_mask) == 0;
        long exp = (bits & double_exp_mask) >> 52;
        long f = bits & double_mantissa_mask;

        if (exp == 2047) {
            if (f == 0) {
                if (positive) {
                    BufferHelper.copyToByteArray(ConstCharArrays.inf, start, buffer);
                    return start + ConstCharArrays.inf.length;
                } else {
                    BufferHelper.copyToByteArray(ConstCharArrays.infNeg, start, buffer);
                    return start + ConstCharArrays.infNeg.length;
                }
            } else {
                BufferHelper.copyToByteArray(ConstCharArrays.nan, start, buffer);
                return start + ConstCharArrays.nan.length;
            }
        }

        if (exp == 0 && f == 0) {
            buffer[start++] = '0';
            return start;
        }

        if (!positive) {
            buffer[start++] = '-';
        }

        int exp2 = (int)(exp - 1023);
        if (exp2 >= 0 && exp2 < 53) { //we have whole and fraction
            int shift = 52 - exp2;
            long whole = (1L << exp2) + (f >> shift);
            int whole_length = getLongLengthCompare(whole);
            writeLong(whole, whole_length, start, buffer);
            return writeFraction(f, shift, exp2, buffer, start + whole_length);
        }
        if (exp2 < 0 && exp2 != -1023) { //normal number but without integer part
            int pow10 = (-exp2) - (int) (log10_5 * (-exp2));
            double nd = Math.abs(value) * Math.pow(10, pow10);
            long whole = (long) nd;
            if (getLongLengthCompare(whole) > 1) {
                pow10--;
                nd /= 10;
            } else if (whole == 0) {
                nd *= 10;
                pow10++;
            }
            bits = Double.doubleToRawLongBits(nd);
            exp = (bits & double_exp_mask) >> 52;
            f = bits & double_mantissa_mask;
            exp2 = (int)(exp - 1023);
            if (exp2 >= 0 && exp2 < 53) {
                int shift = 52 - exp2;
                whole = (1L << exp2) + (f >> shift);
                int whole_length = getLongLengthCompare(whole);
                assert (whole_length == 1);
                writeLong(whole, whole_length, start, buffer);
                start = writeFraction(f, shift, exp2, buffer, start + whole_length);
                buffer[start++]=(byte)'e';
                buffer[start++]=(byte)'-';
                writeInt(pow10, getIntLengthCompare(pow10), buffer, start);
                return start + getIntLengthCompare(pow10);
            }
        }
        if (exp2 == -1023 && f != 0) { //subnormal numbers
            int nlz = Long.numberOfLeadingZeros(f) - 12;
            int pow10 = nlz + 1 + 1022 - (int) (log10_5 * (nlz + 1 + 1022)) - 1;
            double nd = Math.abs(value) * Math.pow(10, pow10 - 50) * 1e50; //split multiplication to avoid overflow
            long whole = (long) nd;
            if (getLongLengthCompare(whole) > 1) {
                pow10--;
                nd /= 10;
            } else if (whole == 0) {
                nd *= 10;
                pow10++;
            }
            bits = Double.doubleToRawLongBits(nd);
            exp = (bits & double_exp_mask) >> 52;
            f = bits & double_mantissa_mask;
            exp2 = (int)(exp - 1023);
            if (exp2 >= 0 && exp2 < 53) {
                int shift = 52 - exp2;
                whole = (1L << exp2) + (f >> shift);
                int whole_length = getLongLengthCompare(whole);
                assert (whole_length == 1);
                writeLong(whole, whole_length, start, buffer);
                start = writeFraction(f, shift, exp2, buffer, start + whole_length);
                buffer[start++]=(byte)'e';
                buffer[start++]=(byte)'-';
                writeInt(pow10, getIntLengthCompare(pow10), buffer, start);
                return start + getIntLengthCompare(pow10);
            }
        }
        if (exp2 > 52) {
            int pow10 = (int)(exp2 * log10_2);
            double nd = Math.abs(value) / Math.pow(10, pow10);
            long whole = (long) nd;
            if (getLongLengthCompare(whole) > 1) {
                pow10++;
                nd /= 10;
            } else if (whole == 0) {
                nd *= 10;
                pow10--;
            }
            bits = Double.doubleToRawLongBits(nd);
            exp = (bits & double_exp_mask) >> 52;
            f = bits & double_mantissa_mask;
            exp2 = (int)(exp - 1023);
            if (exp2 >= 0 && exp2 < 53) {
                int shift = 52 - exp2;
                whole = (1L << exp2) + (f >> shift);
                int whole_length = getLongLengthCompare(whole);
                assert (whole_length == 1);
                writeLong(whole, whole_length, start, buffer);
                start = writeFraction(f, shift, exp2, buffer, start + whole_length);
                buffer[start++]=(byte)'e';
                writeInt(pow10, getIntLengthCompare(pow10), buffer, start);
                return start + getIntLengthCompare(pow10);
            }
        }
        return 0;
    }

    static int writeFloat0(float value, int start, byte[] buffer) {
        int bits = Float.floatToRawIntBits(value);
        boolean positive = (bits & float_sign_mask) == 0;
        int exp = (bits & float_exp_mask) >> 23;
        int f = bits & float_mantissa_mask;

        if (exp == 255) {
            if (f == 0) {
                if (positive) {
                    BufferHelper.copyToByteArray(ConstCharArrays.inf, start, buffer);
                    return start + ConstCharArrays.inf.length;
                } else {
                    BufferHelper.copyToByteArray(ConstCharArrays.infNeg, start, buffer);
                    return start + ConstCharArrays.infNeg.length;
                }
            } else {
                BufferHelper.copyToByteArray(ConstCharArrays.nan, start, buffer);
                return start + ConstCharArrays.nan.length;
            }
        }

        if (exp == 0 && f == 0) {
            buffer[start++] = '0';
            return start;
        }

        if (!positive) {
            buffer[start++] = '-';
        }

        int exp2 = exp - 127;
        if (exp2 >= 0 && exp2 < 23) { //we have whole and possibly a fraction
            int shift = 23 - exp2;
            long whole = (1L << exp2) + (f >> shift);
            int whole_length = getLongLengthCompare(whole);
            writeLong(whole, whole_length, start, buffer);
            return writeFraction(f, shift, exp2, buffer, start + whole_length);
        }
        if (exp2 < 0 && exp2 != -127) { //normal number but without integer part
            int pow10 = (-exp2) - (int) (log10_5 * (-exp2));
            float nd = (float)(Math.abs(value) * Math.pow(10, pow10));
            long whole = (long) nd;
            if (getLongLengthCompare(whole) > 1) {
                pow10--;
                nd /= 10;
            } else if (whole == 0) {
                nd *= 10;
                pow10++;
            }
            bits = Float.floatToRawIntBits(nd);
            exp = (bits & float_exp_mask) >> 23;
            f = bits & float_mantissa_mask;
            exp2 = exp - 127;
            if (exp2 >= 0 && exp2 < 23) {
                int shift = 23 - exp2;
                whole = (1L << exp2) + (f >> shift);
                int whole_length = getLongLengthCompare(whole);
                assert (whole_length == 1);
                writeLong(whole, whole_length, start, buffer);
                start = writeFraction(f, shift, exp2, buffer, start + whole_length);
                buffer[start++]=(byte)'e';
                buffer[start++]=(byte)'-';
                writeInt(pow10, getIntLengthCompare(pow10), buffer, start);
                return start + getIntLengthCompare(pow10);
            }
        }
        if (exp2 == -127 && f != 0) { //subnormal numbers
            int nlz = Integer.numberOfLeadingZeros(f) - 9;
            int pow10 = nlz + 1 + 126 - (int) (log10_5 * (nlz + 1 + 126)) - 1;
            float nd = (float)(Math.abs(value) * Math.pow(10, pow10));
            long whole = (long) nd;
            if (getLongLengthCompare(whole) > 1) {
                pow10--;
                nd /= 10;
            } else if (whole == 0) {
                nd *= 10;
                pow10++;
            }
            bits = Float.floatToRawIntBits(nd);
            exp = (bits & float_exp_mask) >> 23;
            f = bits & float_mantissa_mask;
            exp2 = exp - 127;
            if (exp2 >= 0 && exp2 < 23) {
                int shift = 23 - exp2;
                whole = (1L << exp2) + (f >> shift);
                int whole_length = getLongLengthCompare(whole);
                assert (whole_length == 1);
                writeLong(whole, whole_length, start, buffer);
                start = writeFraction(f, shift, exp2, buffer, start + whole_length);
                buffer[start++]=(byte)'e';
                buffer[start++]=(byte)'-';
                writeInt(pow10, getIntLengthCompare(pow10), buffer, start);
                return start + getIntLengthCompare(pow10);
            }
        }

        if (exp2 >= 23) {
            int pow10 = (int)(exp2 * log10_2);
            float nd = (float)(Math.abs(value) / Math.pow(10, pow10));
            long whole = (long) nd;
            if (getLongLengthCompare(whole) > 1) {
                pow10++;
                nd /= 10;
            } else if (whole == 0) {
                nd *= 10;
                pow10--;
            }
            bits = Float.floatToRawIntBits(nd);
            exp = (bits & float_exp_mask) >> 23;
            f = bits & float_mantissa_mask;
            exp2 = exp - 127;
            if (exp2 >= 0 && exp2 < 23) {
                int shift = 23 - exp2;
                whole = (1L << exp2) + (f >> shift);
                int whole_length = getLongLengthCompare(whole);
                assert (whole_length == 1);
                writeLong(whole, whole_length, start, buffer);
                start = writeFraction(f, shift, exp2, buffer, start + whole_length);
                buffer[start++]=(byte)'e';
                writeInt(pow10, getIntLengthCompare(pow10), buffer, start);
                return start + getIntLengthCompare(pow10);
            }
        }

        return 0;
    }

    private static int writeFraction(long f, long shift, int exp2, byte[] buffer, int start) {

        long u = (f - ((f >> shift) << shift)) << 1; //get rid of the whole part in mantissa

        if (u > 0) { //Finite-Precision Fixed-Point Fraction Printout
            buffer[start++] = (byte)'.';
            shift++;
            int k = 0;
            long e = 1L;
            long one = 1L << shift;
            while (u >= e && u < one - e) {
                k++;
                long sum = (u << 3) + (u << 1);
                long digit = sum >> shift;
                buffer[start + k - 1] = (byte)(ConstCharArrays.digits[(int)digit] & 0xFF);
                u = sum - (digit << shift);
                e = (e << 3) + (e << 1);
            }
            if (((u << 3) + (u << 1)) > (1 << (shift - 1)))
                buffer[start + k - 1] = (byte)(buffer[start + k - 1] + 1);
            return start + k;
        }
        return start;
    }


    /* Date and Time conversion */

    //calculates the exact length of the given date
    static int getDateLength(Date date) {   //Iso 8601

        if (date.isBlank())
            return 4; //null
        else {
            int res = 2; //for opening and closing \" characters
            if (date.year() > 0) {
                res += 4 ;
                if (date.month() > 0) {
                    res += (1 + 2); //YYYY-MM
                    if (date.day() > 0)
                        res += (1 + 2);   //YYYY-MM-DD otherwise it'll be YYYY-MM
                } else if (date.day() > 0) {
                    res += (2 + 2);  //YYYY--DD  - is it a valid Iso 8601 concept (year-day)? paragraph 5.2.1.3 here http://lists.ebxml.org/archives/ebxml-core/200104/pdf00005.pdf doesn't have it
                }
            } else {
                if (date.month() > 0) {  //
                    res += (2 + 2);
                    if (date.day() > 0)  //   --MM-DD    else   --MM
                        res += (1 + 2);
                } else {
                    if (date.day() > 0)   //  ----DD
                        res += (2 + 2 + 2);
                }
            }
            return res;
        }
    }

    static int getTimeMaxLength(Time time) {
        if (time.isBlank())
            return 4;
        else
            return 20;
    }

    static int getDateTimeMaxLength(DateTime dateTime) {

        if (dateTime.isBlank())
            return 4;
        else {
            if (dateTime.date().isBlank())
                return 21;
            else if (dateTime.time().isBlank())
                return getDateLength(dateTime.date());
            else
                return 2 + getDateLength(dateTime.date()) + 1 + getTimeMaxLength(dateTime.time()); //might be worth to simply return maximum
        }
    }

    private static void writeNum(int number, int maxLength, JsonBuffer buffer) {

        int l = getIntLengthCompare(number);
        assert (l <= maxLength);
        for (int i = 0; i < maxLength - l; i++) {
            buffer.data[buffer.position++] = '0';
        }
        writeInt(number, l, buffer.data, buffer.position);
        buffer.position += l;
    }

    private static void writeNonemptyDate(Date date, JsonBuffer buffer) { //similar to DateImpl.toStringIso8601()

        if (date.year() > 0) {
            writeNum(date.year(), 4, buffer);
            if (date.month() > 0) {
                buffer.data[buffer.position++] = '-';
                writeNum(date.month(), 2, buffer);
                if (date.day() > 0) {    //  <0   =>    YYYY-MM
                    buffer.data[buffer.position++] = '-';
                    writeNum(date.day(), 2, buffer); // YYYY-MM-DD
                }
            } else if (date.day() > 0) {
                buffer.data[buffer.position++] = '-';
                buffer.data[buffer.position++] = '-';
                writeNum(date.day(), 2, buffer);  //YYYY--DD   - not sure whether tis is a valid Iso 8601 concept
            }
        } else { // y = 0
            if (date.month() > 0) {
                buffer.data[buffer.position++] = '-';
                buffer.data[buffer.position++] = '-';
                writeNum(date.month(), 2, buffer);
                if (date.day() > 0) {  // < 0   => --MM
                    buffer.data[buffer.position++] = '-';
                    writeNum(date.day(), 2, buffer); //--MM-DD
                }
            } else {  //y = 0, m = 0
                if (date.day() > 0) {
                    buffer.data[buffer.position++] = '-'; buffer.data[buffer.position++] = '-';
                    buffer.data[buffer.position++] = '-'; buffer.data[buffer.position++] = '-';
                    writeNum(date.day(), 2, buffer);  // ----DD
                }
            }
        }
    }

    private static void writeNonemptyTime(Time time, JsonBuffer buffer) {

        writeNum(time.hour(), 2, buffer);
        if (time.minute() != 255) {
            buffer.data[buffer.position++] = ':';
            writeNum(time.minute(), 2, buffer);

            if (time.second() != 255) {
                buffer.data[buffer.position++] = ':';
                writeNum(time.second(), 2, buffer);

                if (time.millisecond() != 65535) {
                    buffer.data[buffer.position++] = '.';
                    writeNum(time.millisecond(), 3, buffer);

                    if (time.microsecond() != 2074) {
                        writeNum(time.microsecond(), 3, buffer);

                        if (time.nanosecond() != 2047) {
                            writeNum(time.nanosecond(), 3, buffer);
                        }
                    }
                    while (buffer.data[buffer.position - 1] == '0') {
                        buffer.data[buffer.position - 1] = 0;
                        buffer.position--;
                    }
                    if (buffer.data[buffer.position - 1] == '.') {
                        buffer.data[buffer.position - 1] = 0;
                        buffer.position--;
                    }
                }
            }
        }
    }

    static boolean writeDate(Date date, JsonBuffer buffer, JsonConverterError error) {

        if (date.isValid() && BufferHelper.checkAndResize(buffer, getDateLength(date), error)) {
            if (date.isBlank()) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
            } else {
                buffer.data[buffer.position++] = '\"';
                writeNonemptyDate(date, buffer);
                buffer.data[buffer.position++] = '\"';
            }
            return true;
        }

        return false;
    }

    static boolean writeTime(Time time, JsonBuffer buffer, JsonConverterError error) {

        if (time.isValid() && BufferHelper.checkAndResize(buffer, getTimeMaxLength(time), error)) {
            if (time.isBlank()) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
            } else {
                buffer.data[buffer.position++] = '\"';
                writeNonemptyTime(time, buffer);
                buffer.data[buffer.position++] = '\"';
            }
            return true;
        }

        return false;
    }

    static boolean writeDateTime(DateTime dateTime, JsonBuffer buffer, JsonConverterError error) {

        if (dateTime.isValid() && BufferHelper.checkAndResize(buffer, getDateTimeMaxLength(dateTime), error)) {
            if (dateTime.isBlank()) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
            } else {
                buffer.data[buffer.position++] = '\"';
                if (!dateTime.date().isBlank()) {
                    writeNonemptyDate(dateTime.date(), buffer);
                }
                if (!dateTime.time().isBlank()) {
                    buffer.data[buffer.position++] = 'T';
                    writeNonemptyTime(dateTime.time(), buffer);
                }
                buffer.data[buffer.position++] = '\"';
            }
            return true;
        }

        return false;
    }


    /* String types conversion */

    static int getSafeStringMaxLength(int inLength) {
        return Math.max(inLength * 6 + 2, 4); //the worst scenario in case every character in the string has to be encoded as \\uXXXX
    }

    static boolean writeSafeString(Buffer inBuffer, JsonBuffer outBuffer, JsonConverterError error) {

        if (BufferHelper.checkAndResize(outBuffer, getSafeStringMaxLength(inBuffer.length()), error)) {

            outBuffer.data[outBuffer.position++] = '\"';
            ByteBuffer data = inBuffer.data();
            int dataLength = inBuffer.length();
            int i = 0;
            int position = inBuffer.position();
            while (i < dataLength) {
                if ((data.get(i + position) & 0xFF) > 0x1F && (data.get(i + position) & 0xFF) != 0x7F) {
                    switch (data.get(i + position)) {
                        case '\\':
                            outBuffer.data[outBuffer.position++] = '\\';
                            if (i == dataLength) {
                                outBuffer.data[outBuffer.position++] = '\\';
                            } else if (data.get(i + position + 1) == '"') {
                                outBuffer.data[outBuffer.position++] = '"';
                                i++;
                            } else if (data.get(i + position + 1) == '\\') {
                                outBuffer.data[outBuffer.position++] = '\\';
                                i++;
                            } else {
                                outBuffer.data[outBuffer.position++] = '\\';
                            }
                            break;
                        case  '"':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = '"';
                            break;
                        default:
                            outBuffer.data[outBuffer.position++] = data.get(i + position);
                            break;
                    }
                } else if ((data.get(i + position) & 0x80) != 0) {
                    outBuffer.data[outBuffer.position++] = data.get(i + position);
                } else {
                    switch (data.get(i + position)) {
                        case '\b':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'b';
                            break;
                        case '\f':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'f';
                            break;
                        case '\n':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'n';
                            break;
                        case '\r':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'r';
                            break;
                        case '\t':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 't';
                            break;
                        default:
                            BufferHelper.writeCharAsHex(data.get(i + position), outBuffer);
                    }
                }
                i++;
            }
            outBuffer.data[outBuffer.position++] = '\"';
            return true;
        }

        return false;
    }

    static int charsToEscapeCount(byte[] array) {

        if (array == null)
            return 0;

        int count = 0;
        for (int i = 0; i < array.length; i++) {
            switch (array[i]) {
                case '\"':
                case '\\':
                case '\n':
                case '\t':
                    count++;
                    break;
                default:
                    if (array[i] < ' ' || array[i] == 0x7F)
                        count += 5;
                    break;
            }
        }

        return count;
    }

    static boolean writeSafeString(byte[] array, JsonBuffer outBuffer, JsonConverterError error) {

        if (BufferHelper.checkAndResize(outBuffer, charsToEscapeCount(array) + array.length + 2, error)) {

            outBuffer.data[outBuffer.position++] = '\"';
            int i = 0;
            while (i < array.length) {
                if ((array[i] & 0xFF) > 0x1F && (array[i] & 0xFF) != 0x7F) {
                    switch (array[i]) {
                        case '\\':
                            outBuffer.data[outBuffer.position++] = '\\';
                            if (i == array.length) {
                                outBuffer.data[outBuffer.position++] = '\\';
                            } else if (array[i + 1] == '"') {
                                outBuffer.data[outBuffer.position++] = '"';
                                i++;
                            } else if (array[i + 1] == '\\') {
                                outBuffer.data[outBuffer.position++] = '\\';
                                i++;
                            } else {
                                outBuffer.data[outBuffer.position++] = '\\';
                            }
                            break;
                        case  '"':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = '"';
                            break;
                        default:
                            outBuffer.data[outBuffer.position++] = array[i];
                            break;
                    }
                } else if ((array[i] & 0x80) != 0) {
                    outBuffer.data[outBuffer.position++] = array[i];
                } else {
                    switch (array[i]) {
                        case '\b':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'b';
                            break;
                        case '\f':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'f';
                            break;
                        case '\n':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'n';
                            break;
                        case '\r':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 'r';
                            break;
                        case '\t':
                            outBuffer.data[outBuffer.position++] = '\\';
                            outBuffer.data[outBuffer.position++] = 't';
                            break;
                        default:
                            BufferHelper.writeCharAsHex(array[i], outBuffer);
                    }
                }
                i++;
            }
            outBuffer.data[outBuffer.position++] = '\"';
            return true;
        }

        return false;
    }


    static boolean writeAsciiString(Buffer ascii, JsonBuffer buffer, JsonConverterError error) {

        if (BufferHelper.checkAndResize(buffer, getSafeStringMaxLength(ascii.length()), error)) {
            if (ascii.isBlank() || ascii.length() == 0) {
                if (BufferHelper.checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                    BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
                    return true;
                } else
                    return false;
            } else {
                return writeSafeString(ascii, buffer, error);
            }
        }

        return false;
    }

    static boolean writeUTF8String(Buffer utf8, JsonBuffer buffer, JsonConverterError error) {

        if (BufferHelper.checkAndResize(buffer, getSafeStringMaxLength(utf8.length()), error)) {
            if (utf8.isBlank() || utf8.length() == 0) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else {
                return writeSafeString(utf8, buffer, error);
            }
        }

        return false;
    }

    static boolean writeRMTESString(Buffer rmtes, JsonBuffer buffer, JsonConverterError error) {

        rmtesBuffer.clear();
        cacheBuffer.clear();

        if (rmtes.isBlank() || rmtes.length() == 0) {
            if (BufferHelper.checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else
                return false;

        } else {

            if (cacheBuffer.allocatedLength() < rmtes.length() * 3) {
                cacheBuffer = CodecFactory.createRmtesCacheBuffer(rmtes.length() * 3); //in decoder.RMTESApplyToCache one byte from rmtes can turn into 3 in the cache
            }

            decoder.RMTESApplyToCache(rmtes, cacheBuffer);

            int length = rmtes.length() * 9; //in decoder.RMTESToUTF8 one byte from cacheBuffer can turn into 3
            if (length > rmtesBuffer.allocatedLength())
                reallocateRMTESBuffer(length);

            int ret = decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer);
            while (ret == CodecReturnCodes.BUFFER_TOO_SMALL) {
                reallocateRMTESBuffer(rmtesBuffer.allocatedLength() * 2);
                ret = decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer);
            }

            if (ret == CodecReturnCodes.SUCCESS && rmtesBuffer.length() != 0) {
                if (BufferHelper.checkAndResize(buffer, rmtesBuffer.length() * 6 + 2, error)) {
                    buffer.data[buffer.position++] = '\"';
                    for (int i = 0; i < rmtesBuffer.length(); i ++) {
                        writeByteChar(rmtesBuffer.byteData().get(i), buffer);
                    }
                    buffer.data[buffer.position++] = '\"';
                    return true;
                }
            }
            else  if (BufferHelper.checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                BufferHelper.copyToByteArray(ConstCharArrays.nullBytes, buffer.position, buffer.data);
                return true;
            }
        }

        return false;
    }

    static void writeByteChar(byte ch, JsonBuffer buffer) {

        if (! (((ch & 0xFF) & 0x80) == 1) ) {
            switch (ch) {
                case '\"':
                    buffer.data[buffer.position++] = '\\';
                    buffer.data[buffer.position++] = '\"';
                    break;
                case '\\':
                    buffer.data[buffer.position++] = '\\';
                    buffer.data[buffer.position++] = '\\';
                    break;
                default:
                    if ((ch & 0xFF) < ' ' || (ch & 0xFF) == 0x7F) {
                        BufferHelper.writeCharAsHex(ch, buffer);
                    } else {
                        buffer.data[buffer.position++] = ch;
                    }
            }
        }
    }

    private static boolean reallocateRMTESBuffer(int newLength) {

        try {
            JsonFactory.releaseByteArray(rmtesBuffer.byteData().array());
            byte[] newArray = JsonFactory.createByteArray(newLength);
            ByteBuffer bb = ByteBuffer.wrap(newArray);
            rmtesBuffer.clear();
            rmtesBuffer.data(bb);
            rmtesBuffer.allocatedLength(newArray.length);
            return true;
        } catch (OutOfMemoryError e) {
            return false;
        }
    }
}

