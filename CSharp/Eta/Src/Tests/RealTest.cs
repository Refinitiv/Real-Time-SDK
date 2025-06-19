/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Xunit;
using Xunit.Categories;

namespace CodecTestProject
{
    public class RealTest
    {
        private void TestRealToString(double val, int hint)
        {
            Real real = new Real();
            real.Value(val, hint);
            string s = System.Convert.ToString(val);
            Assert.Equal(s, real.ToString());
        }

        private void TestRealToDouble(string s)
        {
            Real real = new Real();
            real.Value(s);
            Double tst = new Double();
            tst.Value(real.ToDouble());
            Assert.True(real.ToDouble() == tst.ToDouble());
        }

        private void TestDoubleToReal(double val, int hint)
        {
            Double dbl = new Double();
            dbl.Value(val);
            Real real = new Real();
            real = dbl.ToReal(hint);
            Assert.True(val == real.ToDouble());
        }

        private void TestDoubleToRealFraction(double val, int hint)
        {
            Real real = new Real();
            real.Value(val, hint);
            double tst = real.ToDouble();
            Assert.True(val == tst);
        }

        private void TestDoubleToRealDecimal(double val, int hint)
        {
            Real real = new Real();
            real.Value(val, hint);
            double tst = real.ToDouble();
            Assert.True(val == tst);
        }

        private void TestDoubleRealCompare(double dblVal, long val, int hint)
        {
            Real real = new Real();
            real.Value(dblVal, hint);
            Assert.True(val == real.ToLong());
            Assert.Equal(hint, real.Hint);
        }

        [Fact]
        [Category("Unit")]
        [Category("ByteBuffer")]
        private void TestRealField()
        {
            FieldEntry entry = new FieldEntry();
            FieldList fieldList = new FieldList();
            Real real = new Real();
            Real decReal = new Real();
            Real encReal = new Real();
            Real outReal = new Real();
            EncodeIterator encIter = new EncodeIterator();
            DecodeIterator decIter = new DecodeIterator();
            int majorVersion = Codec.MajorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
            int minorVersion = Codec.MinorVersion();  // This should be initialized to the MINOR version of RWF being encoded
            Buffer _buffer = new Buffer();
            _buffer.Data(new ByteBuffer(50));

            fieldList.Clear();
            fieldList.Flags = FieldListFlags.HAS_STANDARD_DATA;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(_buffer, majorVersion, minorVersion);

            /* init */
            entry.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeFieldListInit(encIter, fieldList, null, 0));
            entry.FieldId = 1;
            entry.DataType = DataTypes.REAL;

            /* 32-bit 64-bit real */
            real.Blank();
            real.Value(65535, RealHints.EXPONENT_2);
            Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeFieldEntry(encIter, entry, real));

            /* 64-bit real */
            entry.Clear();
            entry.FieldId = 2;
            entry.DataType = DataTypes.REAL;
            real.Blank();
            real.Value(68719476735L, RealHints.EXPONENT_2);

            Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeFieldEntry(encIter, entry, real));

            entry.Clear();
            entry.FieldId = 3;
            entry.DataType = DataTypes.REAL;

            /* 32-bit 64-bit real */
            encReal.Blank();
            encReal.Value(65535, RealHints.EXPONENT_2);

            Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeFieldEntry(encIter, entry, encReal));

            /* finish encoding */
            Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeFieldListComplete(encIter, true));

            // Setup Container
            fieldList.Clear();
            entry.Clear();
            decIter.Clear();
            decIter.SetBufferAndRWFVersion(_buffer, majorVersion, minorVersion);

            // Begin container decoding
            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeFieldList(decIter, fieldList, null));

            Assert.Equal(FieldListFlags.HAS_STANDARD_DATA, fieldList.Flags);

            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeFieldEntry(decIter, entry));

            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeReal(decIter, decReal));

            bool correctDecReal = false;
            if (!decReal.IsBlank && decReal.Hint == RealHints.EXPONENT_2 && decReal.ToLong() == 65535)
                correctDecReal = true;
            Assert.True(correctDecReal);

            entry.Clear();

            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeFieldEntry(decIter, entry));

            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeReal(decIter, outReal));

            correctDecReal = false;
            if (!outReal.IsBlank && outReal.Hint == RealHints.EXPONENT_2 && outReal.ToLong() == 68719476735L)
                correctDecReal = true;
            Assert.True(correctDecReal);

            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeFieldEntry(decIter, entry));

            Assert.Equal(CodecReturnCode.SUCCESS, Decoders.DecodeReal(decIter, decReal));

            correctDecReal = false;
            if (!decReal.IsBlank && decReal.Hint == RealHints.EXPONENT_2 && decReal.ToLong() == 65535)
                correctDecReal = true;
            Assert.True(correctDecReal);

        }

        [Fact]
        [Category("Unit")]
        public void RealTests()
        {
            //Real Related Conversion test
            TestRealToString((double)20 / 4, RealHints.FRACTION_4);
            TestRealToString((double)100 / 256, RealHints.FRACTION_256);
            TestRealToString((double)-20 / 128, RealHints.FRACTION_128);
            TestRealToString((double)123456 / 64, RealHints.FRACTION_64);
            TestRealToString(1.5, RealHints.FRACTION_2);
            TestRealToString(-1.5, RealHints.FRACTION_2);
            TestRealToString(0.5, RealHints.FRACTION_2);
            TestRealToString(5, RealHints.FRACTION_2);
            TestRealToString(1.5, RealHints.FRACTION_4);
            TestRealToString(-1.5, RealHints.FRACTION_4);
            TestRealToString(0.5, RealHints.FRACTION_4);
            TestRealToString(12.25, RealHints.FRACTION_4);
            TestRealToString(-1.75, RealHints.FRACTION_4);
            TestRealToString(0.75, RealHints.FRACTION_4);
            TestRealToString(5, RealHints.FRACTION_4);
            TestRealToString(1.125, RealHints.FRACTION_8);
            TestRealToString(1.0 + 1.0 / 256.0, RealHints.FRACTION_256);

            TestRealToString(210, RealHints.EXPONENT1);
            TestRealToString(2100, RealHints.EXPONENT2);
            TestRealToString(21000, RealHints.EXPONENT3);
            TestRealToString(210000, RealHints.EXPONENT4);
            TestRealToString(2100000, RealHints.EXPONENT5);
            TestRealToString(21000000, RealHints.EXPONENT6);
            TestRealToString(210000000, RealHints.EXPONENT7);
            TestRealToString(20, RealHints.EXPONENT_1);
            TestRealToString(-20, RealHints.EXPONENT_1);
            TestRealToString(123456, RealHints.EXPONENT_1);
            TestRealToString(1.5, RealHints.EXPONENT_1);
            TestRealToString(-1.5, RealHints.EXPONENT_1);
            TestRealToString(0.5, RealHints.EXPONENT_1);
            TestRealToString(5, RealHints.EXPONENT_1);
            TestRealToString(1.5, RealHints.EXPONENT_2);
            TestRealToString(-1.5, RealHints.EXPONENT_2);
            TestRealToString(0.5, RealHints.EXPONENT_2);
            TestRealToString(12.25, RealHints.EXPONENT_2);
            TestRealToString(-1.75, RealHints.EXPONENT_2);
            TestRealToString(0.75, RealHints.EXPONENT_2);
            TestRealToString(5, RealHints.EXPONENT_2);
            TestRealToString(1.125, RealHints.EXPONENT_3);

            TestRealToDouble("-12");
            TestRealToDouble("-12.");
            TestRealToDouble("-12.0");
            TestRealToDouble("-12.1");
            TestRealToDouble("-12.12");
            TestRealToDouble("-12.123");
            TestRealToDouble("-12.1234");
            TestRealToDouble("-12.12345");
            TestRealToDouble("-12.123456");
            TestRealToDouble("-12.1234567");
            TestRealToDouble("-12.12345678");
            TestRealToDouble("-1.123456789");
            TestRealToDouble("12");
            TestRealToDouble("12.");
            TestRealToDouble("12.0");
            TestRealToDouble("12.1");
            TestRealToDouble("12.12");
            TestRealToDouble("12.123");
            TestRealToDouble("12.1234");
            TestRealToDouble("12.12345");
            TestRealToDouble("12.123456");
            TestRealToDouble("12.1234567");
            TestRealToDouble("12.12345678");
            TestRealToDouble("1.123456789");
            TestRealToDouble("-12.123456789");
            TestRealToDouble("12.123456789");
            TestRealToDouble("12.000000");
            TestRealToDouble("0.000000");
            TestRealToDouble("0.000001");
            TestRealToDouble("-0.000001");
            TestRealToDouble("-12.000000");
            TestRealToDouble(".1");
            TestRealToDouble("0");
            TestRealToDouble("12 1/2");
            TestRealToDouble("12 1/4");
            TestRealToDouble("12 2/8");
            TestRealToDouble("12 2/16");
            TestRealToDouble("12 2/32");
            TestRealToDouble("12 2/64");
            TestRealToDouble("12 2/128");
            TestRealToDouble("12 2/256");
            TestRealToDouble("-12 1/2");
            TestRealToDouble("-12 1/4");
            TestRealToDouble("-12 2/8");
            TestRealToDouble("-12 2/16");
            TestRealToDouble("-12 2/32");
            TestRealToDouble("-12 2/64");
            TestRealToDouble("-12 2/128");
            TestRealToDouble("-12 2/256");
            TestRealToDouble("-12 0/2");
            TestRealToDouble("12.1234");

            TestDoubleToReal((double)20 / 4, RealHints.FRACTION_4);
            TestDoubleToReal((double)100 / 256, RealHints.FRACTION_256);
            TestDoubleToReal((double)-20 / 128, RealHints.FRACTION_128);
            TestDoubleToReal((double)123456 / 64, RealHints.FRACTION_64);
            TestDoubleToReal(1.5, RealHints.FRACTION_2);
            TestDoubleToReal(-1.5, RealHints.FRACTION_2);
            TestDoubleToReal(0.5, RealHints.FRACTION_2);
            TestDoubleToReal(5, RealHints.FRACTION_2);
            TestDoubleToReal(1.5, RealHints.FRACTION_4);
            TestDoubleToReal(-1.5, RealHints.FRACTION_4);
            TestDoubleToReal(0.5, RealHints.FRACTION_4);
            TestDoubleToReal(12.25, RealHints.FRACTION_4);
            TestDoubleToReal(-1.75, RealHints.FRACTION_4);
            TestDoubleToReal(0.75, RealHints.FRACTION_4);
            TestDoubleToReal(5, RealHints.FRACTION_4);
            TestDoubleToReal(1.125, RealHints.FRACTION_8);
            TestDoubleToReal(1.0 + 1.0 / 256.0, RealHints.FRACTION_256);

            TestDoubleToReal(210, RealHints.EXPONENT1);
            TestDoubleToReal(2100, RealHints.EXPONENT2);
            TestDoubleToReal(21000, RealHints.EXPONENT3);
            TestDoubleToReal(210000, RealHints.EXPONENT4);
            TestDoubleToReal(2100000, RealHints.EXPONENT5);
            TestDoubleToReal(21000000, RealHints.EXPONENT6);
            TestDoubleToReal(210000000, RealHints.EXPONENT7);
            TestDoubleToReal(20, RealHints.EXPONENT_1);
            TestDoubleToReal(0, RealHints.EXPONENT1);
            TestDoubleToReal(-20, RealHints.EXPONENT_1);
            TestDoubleToReal(123456, RealHints.EXPONENT_1);
            TestDoubleToReal(1.5, RealHints.EXPONENT_1);
            TestDoubleToReal(-1.5, RealHints.EXPONENT_1);
            TestDoubleToReal(0.5, RealHints.EXPONENT_1);
            TestDoubleToReal(5, RealHints.EXPONENT_1);
            TestDoubleToReal(1.5, RealHints.EXPONENT_2);
            TestDoubleToReal(-1.5, RealHints.EXPONENT_2);
            TestDoubleToReal(0.5, RealHints.EXPONENT_2);
            TestDoubleToReal(12.25, RealHints.EXPONENT_2);
            TestDoubleToReal(-1.75, RealHints.EXPONENT_2);
            TestDoubleToReal(0.75, RealHints.EXPONENT_2);
            TestDoubleToReal(5, RealHints.EXPONENT_2);
            TestDoubleToReal(1.125, RealHints.EXPONENT_3);

            TestDoubleToRealFraction((double)20 / 4, RealHints.FRACTION_4);
            TestDoubleToRealFraction((double)100 / 256, RealHints.FRACTION_256);
            TestDoubleToRealFraction((double)-20 / 128, RealHints.FRACTION_128);
            TestDoubleToRealFraction((double)123456 / 64, RealHints.FRACTION_64);
            TestDoubleToRealFraction(1.5, RealHints.FRACTION_2);
            TestDoubleToRealFraction(-1.5, RealHints.FRACTION_2);
            TestDoubleToRealFraction(0.5, RealHints.FRACTION_2);
            TestDoubleToRealFraction(5, RealHints.FRACTION_2);
            TestDoubleToRealFraction(1.5, RealHints.FRACTION_4);
            TestDoubleToRealFraction(-1.5, RealHints.FRACTION_4);
            TestDoubleToRealFraction(0.5, RealHints.FRACTION_4);
            TestDoubleToRealFraction(12.25, RealHints.FRACTION_4);
            TestDoubleToRealFraction(-1.75, RealHints.FRACTION_4);
            TestDoubleToRealFraction(0.75, RealHints.FRACTION_4);
            TestDoubleToRealFraction(5, RealHints.FRACTION_4);
            TestDoubleToRealFraction(1.125, RealHints.FRACTION_8);
            TestDoubleToRealFraction(1.0 + 1.0 / 256.0, RealHints.FRACTION_256);

            TestDoubleToRealDecimal(210, RealHints.EXPONENT1);
            TestDoubleToRealDecimal(2100, RealHints.EXPONENT2);
            TestDoubleToRealDecimal(21000, RealHints.EXPONENT3);
            TestDoubleToRealDecimal(210000, RealHints.EXPONENT4);
            TestDoubleToRealDecimal(2100000, RealHints.EXPONENT5);
            TestDoubleToRealDecimal(21000000, RealHints.EXPONENT6);
            TestDoubleToRealDecimal(210000000, RealHints.EXPONENT7);
            TestDoubleToRealDecimal(20, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(0, RealHints.EXPONENT1);
            TestDoubleToRealDecimal(-20, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(123456, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(1.5, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(-1.5, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(0.5, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(5, RealHints.EXPONENT_1);
            TestDoubleToRealDecimal(1.5, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(-1.5, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(0.5, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(12.25, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(-1.75, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(0.75, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(5, RealHints.EXPONENT_2);
            TestDoubleToRealDecimal(1.125, RealHints.EXPONENT_3);

            TestDoubleRealCompare(.35, 35, RealHints.EXPONENT_2);
            TestDoubleRealCompare(100000000.1, 1000000001, RealHints.EXPONENT_1);
            TestDoubleRealCompare(100000001.1, 1000000011, RealHints.EXPONENT_1);
            TestDoubleRealCompare(900000000.1, 9000000001L, RealHints.EXPONENT_1);
            TestDoubleRealCompare(100010000.1, 1000100001, RealHints.EXPONENT_1);
            TestDoubleRealCompare(111111111.1, 1111111111, RealHints.EXPONENT_1);
            TestDoubleRealCompare(11111111.1, 111111111, RealHints.EXPONENT_1);
            TestDoubleRealCompare(1.1, 11, RealHints.EXPONENT_1);
            TestDoubleRealCompare(111.1, 1111, RealHints.EXPONENT_1);
            TestDoubleRealCompare(11.1, 111, RealHints.EXPONENT_1);
            TestDoubleRealCompare(0.00001, 1, RealHints.EXPONENT_5);
            TestDoubleRealCompare(100000000.2, 1000000002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(2000.2, 20002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(20000.2, 200002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(200000.2, 2000002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(2000000.2, 20000002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(20000000.2, 200000002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(200000000.2, 2000000002, RealHints.EXPONENT_1);
            TestDoubleRealCompare(987.0123456789012, 9870123456789012L, RealHints.EXPONENT_13);

            //extensive double/real test
            for (int i = 0; i < 13; i++)
            {
                for (int j = 0; j < 10000; j++)
                {
                    switch (i)
                    {
                        case (12):
                            TestDoubleRealCompare((double)((.000000000001) + (.000000000001) * j), (long)System.Math.Floor(((double)((.000000000001 + .000000000001 * j) * (1000000000000L))) + 0.5), RealHints.EXPONENT_12);
                            break;
                        case (11):
                            TestDoubleRealCompare((double)((.00000000001) + (.00000000001) * j), (long)System.Math.Floor(((double)((.00000000001 + .00000000001 * j) * (100000000000L))) + 0.5), RealHints.EXPONENT_11);
                            break;
                        case (10):
                            TestDoubleRealCompare((double)((.0000000001) + (.0000000001) * j), (long)System.Math.Floor(((double)((.0000000001 + .0000000001 * j) * (10000000000L))) + 0.5), RealHints.EXPONENT_10);
                            break;
                        case (9):
                            TestDoubleRealCompare((double)((.000000001) + (.000000001) * j), (long)System.Math.Floor(((double)((.000000001 + .000000001 * j) * (1000000000))) + 0.5), RealHints.EXPONENT_9);
                            break;
                        case (8):
                            TestDoubleRealCompare((double)((.00000001) + (.00000001) * j), (long)System.Math.Floor(((double)((.00000001 + .00000001 * j) * (100000000))) + 0.5), RealHints.EXPONENT_8);
                            break;
                        case (7):
                            TestDoubleRealCompare((double)((.0000001) + (.0000001) * j), (long)System.Math.Floor(((double)((.0000001 + .0000001 * j) * (10000000))) + 0.5), RealHints.EXPONENT_7);
                            break;
                        case (6):
                            TestDoubleRealCompare((double)((.000001) + (.000001) * j), (long)System.Math.Floor(((double)((.000001 + .000001 * j) * (1000000))) + 0.5), RealHints.EXPONENT_6);
                            break;
                        case (5):
                            TestDoubleRealCompare((double)((.00001) + (.00001) * j), (long)System.Math.Floor(((double)((.00001 + .00001 * j) * (100000))) + 0.5), RealHints.EXPONENT_5);
                            break;
                        case (4):
                            TestDoubleRealCompare((double)((.0001) + (.0001) * j), (long)System.Math.Floor(((double)((.0001 + .0001 * j) * (10000)) + 0.5)), RealHints.EXPONENT_4);
                            break;
                        case (3):
                            TestDoubleRealCompare((double)((.001) + (.001) * j), (long)System.Math.Floor(((double)((.001 + .001 * j) * (1000))) + 0.5), RealHints.EXPONENT_3);
                            break;
                        case (2):
                            TestDoubleRealCompare((double)((.01) + (.01) * j), (long)System.Math.Floor(((double)((.01 + .01 * j) * (100))) + 0.5), RealHints.EXPONENT_2);
                            break;
                        case (1):
                            TestDoubleRealCompare((double)((.1) + (.1) * j), (long)System.Math.Floor(((double)((.1 + .1 * j) * (10))) + 0.5), RealHints.EXPONENT_1);
                            break;
                        case (0):
                            TestDoubleRealCompare((double)(1 + 1 * j), (long)System.Math.Floor(((double)(1 + 1 * j)) + 0.5), RealHints.EXPONENT0);
                            break;
                    }
                }
            }

            TestDoubleToRealDecimal(0.123456789, RealHints.EXPONENT_9);
            TestDoubleToRealDecimal(1.23456789, RealHints.EXPONENT_9);
            TestDoubleToRealDecimal(1.2, RealHints.EXPONENT_9);

        }

    }
}
