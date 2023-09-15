/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System.Globalization;
using System.Text;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Xunit;
using Xunit.Categories;

namespace CodecTestProject
{
    [Category("ByteBuffer")]
    public class PrimitiveDataTest
    {
        EncodeIterator _encIter = new EncodeIterator();
        DecodeIterator _decIter = new DecodeIterator();
        Buffer _buffer = new Buffer();

        [Fact]
        [Category("Unit")]
        public void UIntEncodeDecodeBlankTest()
        {
            UInt uintv = new UInt();

            Assert.False(uintv.IsBlank);
            uintv.Blank();
            Assert.True(uintv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, uintv.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.UINT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            UInt uint1 = new UInt();
            Assert.False(uint1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, uint1.Decode(_decIter));

            Assert.True(uint1.IsBlank);
            Assert.Equal(uintv.ToLong(), uint1.ToLong());
        }

        [Fact]
        [Category("Unit")]
        public void UIntSetTest()
        {
            UInt thisUInt = new UInt();

            thisUInt.Value(11223344);
            Assert.Equal(11223344, thisUInt.ToLong());
        }

        [Fact]
        [Category("Unit")]
        public void UIntStringTest()
        {
            UInt testUInt = new UInt();
            Assert.Equal(CodecReturnCode.SUCCESS, testUInt.Value("    "));
            Assert.True(testUInt.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testUInt.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, testUInt.Value(""));
            Assert.True(testUInt.IsBlank);

            Assert.Equal(CodecReturnCode.SUCCESS, testUInt.Value("11112222"));
            Assert.Equal("11112222", testUInt.ToString());
            Assert.False(testUInt.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void CopyUIntTest()
        {
            UInt Left = new UInt();
            UInt Right = new UInt();

            Left.Value(13);

            // Make sure they are NOT equal, by asserting if they ARE equal
            Assert.True(Left._ulongValue != Right._ulongValue);

            Left.Copy(Right);
            //  Make sure they are Equal
            Assert.Equal<bool>(Left.IsBlank, Right.IsBlank);
            Assert.Equal(Left._ulongValue, Right._ulongValue);

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            Left.Value(Right._ulongValue + 1);
            Assert.True(Left._ulongValue != Right._ulongValue);
        }

        [Fact]
        [Category("Unit")]
        public void CopyEnumTest()
        {
            Enum Left = new Enum();
            Enum Right = new Enum();

            Left.Value(13);

            // Make sure they are NOT equal, by asserting if they ARE equal
            Assert.True(Left._enumValue != Right._enumValue);

            Left.Copy(Right);
            //  Make sure they are Equal
            Assert.Equal(Left.IsBlank, Right.IsBlank);
            Assert.Equal(Left._enumValue, Right._enumValue);

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            if (65535 < Right._enumValue) Left.Value(0); else Left.Value(Right._enumValue + 1);
            Assert.True(Left._enumValue != Right._enumValue);
        }


        [Fact]
        [Category("Unit")]
        public void CopyFloatTest()
        {
            Float Left = new Float();
            Float Right = new Float();

            Left._floatValue = 13;

            // Make sure they are NOT equal, by asserting if they ARE equal
            Assert.True(Left._floatValue != Right._floatValue);

            Left.Copy(Right);
            //  Make sure they are Equal
            Assert.Equal(Left.IsBlank, Right.IsBlank);
            Assert.True(Left._floatValue == Right._floatValue);

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            Left.Value(Right._floatValue + 1);
            Assert.True(Left._floatValue != Right._floatValue);
        }


        [Fact]
        [Category("Unit")]
        public void CopyDoubleTest()
        {
            Double Left = new Double();
            Double Right = new Double();

            Left.Value(13);

            // Make sure they are NOT equal, by asserting if they ARE equal
            Assert.True(Left._doubleValue != Right._doubleValue);

            Left.Copy(Right);
            //  Make sure they are Equal
            Assert.Equal(Left.IsBlank, Right.IsBlank);
            Assert.True(Left._doubleValue == Right._doubleValue);

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            Left.Value(Right._doubleValue + 1);
            Assert.True(Left._doubleValue != Right._doubleValue);
        }

        [Fact]
        [Category("Unit")]
        public void CopyRealTest()
        {
            Real Left = new Real();
            Real Right = new Real();

            Left._value = 13;

            // Make sure they are NOT equal, by asserting if they ARE equal
            Assert.True(Left._value != Right._value);

            Left.Copy(Right);
            //  Make sure they are Equal
            Assert.Equal(Left.IsBlank, Right.IsBlank);
            Assert.Equal(Left._value, Right._value);

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            Left._value = Right._value + 1;
            Assert.True(Left._value != Right._value);
        }

        [Fact]
        [Category("Unit")]
        public void CopyIntTest()
        {
            Int iLeft = new Int();
            Int iRight = new Int();

            iLeft.Value(100);
            Assert.True(iLeft.ToLong() != iRight.ToLong());

            iLeft.Copy(iRight);
            Assert.Equal(iLeft.ToLong(), iRight.ToLong());

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            iLeft.Value(9);
            Assert.True(iLeft.ToLong() != iRight.ToLong());
        }

        [Fact]
        [Category("Unit")]
        public void UIntEncodeDecodeTest()
        {
            UInt unint = new ();

            {//long
                long val;

                /* leading 0's */
                for (val = 6148914691236517205; val != 0; val >>= 1)
                {
                    unint.Value(val);
                    UIntED(unint);
                }

                for (val = -6148914691236517206; val != -1; val >>= 1)
                {
                    unint.Value(val);
                    UIntED(unint);
                }

                /* trailing 0's */
                for (val = -6148914691236517206; val != 0; val <<= 1)
                {
                    unint.Value(val);
                    UIntED(unint);
                }
                /* Highest hex number */
                val = -1;
                unint.Value(val);
                UIntED(unint);

                /* 0 */
                val = 0;
                unint.Value(val);
                UIntED(unint);
            }

            {//ulong
                ulong val;
                /* leading 0's */
                for (val = 6148914691236517205; val != 0; val >>= 1)
                {
                    unint.Value(val);
                    UIntED(unint);
                }

                for (val = 0b_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010;
                    val != 0b_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111_1111;
                    val = (val >> 1) | 0b_1000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000)
                {
                    unint.Value(val);
                    UIntED(unint);
                }

                /* trailing 0's */
                for (val = 0b_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010_1010;
                     val != 0;
                     val <<= 1)
                {
                    unint.Value(val);
                    UIntED(unint);
                }
            }

            /* Highest hex number */
            unint.Value("18446744073709551615");
            UIntED(unint);

            /* 0 */
            unint.Value("0");
            UIntED(unint);
        }

        [Fact]
        [Category("Unit")]
        public void Int64EncodeDecodeTest()
        {
            Int intv = new Int();
            long val;

            /* leading 0's */
            for (val = 6148914691236517205; val != 0; val >>= 1)
            {
                intv.Value(val);
                IntED(intv);
            }

            /* Negative numbers with leading 1's */
            for (val = -6148914691236517206; val != -1; val >>= 1)
            {
                intv.Value(val);
                IntED(intv);
            }

            /* Numbers with trailing zeros */
            for (val = -6148914691236517206; val != 0; val <<= 1)
            {
                intv.Value(val);
                IntED(intv);
            }

            /* Highest hex number */
            val = -1;
            intv.Value(val);
            IntED(intv);

            /* 0 */
            val = 0;
            intv.Value(val);
            IntED(intv);
        }

        [Fact]
        [Category("Unit")]
        public void IntEncodeDecodeBlankTest()
        {
            Int intv = new Int();

            Assert.False(intv.IsBlank);
            intv.Blank();
            Assert.True(intv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, intv.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.INT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Int int1 = new Int();
            Assert.False(int1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, int1.Decode(_decIter));

            Assert.True(int1.IsBlank);
            Assert.Equal<long>(intv.ToLong(), int1.ToLong());
        }

        [Fact]
        [Category("Unit")]
        public void IntSetTest()
        {
            Int thisInt = new Int();

            thisInt.Value(111222);
            Assert.Equal<long>(111222, thisInt.ToLong());
        }

        [Fact]
        [Category("Unit")]
        public void IntStringTest()
        {
            Int testInt = new Int();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testInt.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, testInt.Value("    "));
            Assert.True(testInt.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testInt.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, testInt.Value(""));
            Assert.True(testInt.IsBlank);

            Assert.Equal(CodecReturnCode.SUCCESS, testInt.Value("1122"));
            Assert.Equal("1122", testInt.ToString());
            Assert.False(testInt.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void dateEncodeDecodeTest()
        {
            Date date = new Date();

            for (int i = 4095; i != 0; i >>= 1)
            {
                date.Year(i);
                for (int j = 0; j <= 12; ++j)
                {
                    date.Month(j);
                    for (int k = 0; k <= 31; ++k)
                        DateED(date);
                }
            }
        }

        [Fact]
        [Category("Unit")]
        public void dateEncodeDecodeBlankTest()
        {
            Date date = new Date();

            date.Blank();
            Assert.True(date.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, date.Encode(_encIter));

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Date date1 = new Date();
            date1.Decode(_decIter);

            Assert.Equal(date.Day(), date1.Day());
            Assert.Equal(date.Month(), date1.Month());
            Assert.Equal(date.Year(), date1.Year());
            Assert.True(date1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void dateStringTest()
        {
            Date date = new Date();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, date.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, date.Value("    "));
            Assert.True(date.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, date.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, date.Value(""));
            Assert.True(date.IsBlank);

            //1974/04/14
            date.Value("1974/04/14");
            Assert.Equal(1974, date.Year());
            Assert.Equal(4, date.Month());
            Assert.Equal(14, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04/14/74
            date.Value("04/14/74");
            Assert.Equal(1974, date.Year());
            Assert.Equal(4, date.Month());
            Assert.Equal(14, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04/14/1974
            date.Value("04/14/1974");
            Assert.Equal(1974, date.Year());
            Assert.Equal(4, date.Month());
            Assert.Equal(14, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //1974 04 14
            date.Value("1974 04 14");
            Assert.Equal(1974, date.Year());
            Assert.Equal(4, date.Month());
            Assert.Equal(14, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04 14 74
            date.Value("04 14 74");
            Assert.Equal(1974, date.Year());
            Assert.Equal(4, date.Month());
            Assert.Equal(14, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04 14 1974
            date.Value("04 14 1974");
            Assert.Equal(1974, date.Year());
            Assert.Equal(4, date.Month());
            Assert.Equal(14, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04 JAN 74
            date.Value("04 JAN 74");
            Assert.Equal(1974, date.Year());
            Assert.Equal(1, date.Month());
            Assert.Equal(4, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04 JAN 1974
            date.Value("04 JAN 1974");
            Assert.Equal(1974, date.Year());
            Assert.Equal(1, date.Month());
            Assert.Equal(4, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04 jan 74
            date.Value("04 jan 74");
            Assert.Equal(1974, date.Year());
            Assert.Equal(1, date.Month());
            Assert.Equal(4, date.Day());
            Assert.False(date.IsBlank);

            date.Blank();

            //04 jan 1974
            date.Value("04 jan 1974");
            Assert.Equal(1974, date.Year());
            Assert.Equal(1, date.Month());
            Assert.Equal(4, date.Day());
            Assert.False(date.IsBlank);

        }

        [Fact]
        [Category("Unit")]
        public void dateEqualsTest()
        {
            Date thisDate = new Date();
            Date thatDate = new Date();
            thisDate.Blank();
            thatDate.Blank();
            Assert.True(thisDate.Equals(thatDate));
            thisDate.Year(2012);
            thisDate.Month(10);
            thisDate.Day(15);
            thatDate.Year(2012);
            thatDate.Month(10);
            thatDate.Day(15);
            Assert.True(thisDate.Equals(thatDate));
            thatDate.Blank();
            Assert.False(thisDate.Equals(thatDate));
        }

        [Fact]
        [Category("Unit")]
        public void dateSetTest()
        {
            Date thisDate = new Date();

            Assert.Equal(CodecReturnCode.SUCCESS, thisDate.Year(0));
            Assert.Equal(0, thisDate.Year());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDate.Month(0));
            Assert.Equal(0, thisDate.Month());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDate.Day(0));
            Assert.Equal(0, thisDate.Day());
            Assert.True(thisDate.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDate.Year(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDate.Month(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDate.Day(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDate.Year(65536));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDate.Month(13));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDate.Day(32));
            Assert.Equal(CodecReturnCode.SUCCESS, thisDate.Year(65535));
            Assert.Equal(65535, thisDate.Year());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDate.Month(12));
            Assert.Equal(12, thisDate.Month());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDate.Day(31));
            Assert.Equal(31, thisDate.Day());
        }

        [Fact]
        [Category("Unit")]
        public void dateTimeisValidTest()
        {
            DateTime dateTime = new DateTime();

            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Time().Blank();
            Assert.True(dateTime.IsValid);

            dateTime.Date().Blank();
            dateTime.Time().Blank();
            Assert.True(dateTime.IsValid);

            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Hour(11);
            dateTime.Minute(12);
            dateTime.Second(2);
            dateTime.Millisecond(2);
            Assert.True(dateTime.IsValid);

        }

        [Fact]
        [Category("Unit")]
        public void DateTimeToStringTest()
        {
            DateTime dateTime = new DateTime();

            //blank
            dateTime.Blank();
            Assert.Equal("", dateTime.ToString());

            //invalid
            dateTime.Year(2012);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, dateTime.Month(13)); //invalid
            dateTime.Day(2);

            //valid date, blank time
            dateTime.Clear();
            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Time().Blank();
            Assert.Equal("02 DEC 2012", dateTime.ToString());


            //valid date, valid time
            dateTime.Clear();
            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Hour(2);
            dateTime.Minute(2);
            dateTime.Second(2);
            dateTime.Millisecond(2);
            dateTime.Microsecond(3);
            dateTime.Nanosecond(4);
            Assert.Equal("02 DEC 2012 02:02:02:002:003:004", dateTime.ToString());

            //valid date, valid time
            dateTime.Clear();
            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Hour(2);
            dateTime.Minute(2);
            dateTime.Second(2);
            dateTime.Millisecond(2);
            dateTime.Microsecond(3);
            Assert.Equal("02 DEC 2012 02:02:02:002:003:000", dateTime.ToString());

            //valid date, valid time
            dateTime.Clear();
            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Hour(2);
            dateTime.Minute(2);
            dateTime.Second(2);
            dateTime.Millisecond(2);
            Assert.Equal("02 DEC 2012 02:02:02:002:000:000", dateTime.ToString());

            //valid date, valid time
            dateTime.Clear();
            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Hour(2);
            dateTime.Minute(2);
            dateTime.Second(2);
            Assert.Equal("02 DEC 2012 02:02:02:000:000:000", dateTime.ToString());

            //valid date, valid time
            dateTime.Clear();
            dateTime.Year(2012);
            dateTime.Month(12);
            dateTime.Day(2);
            dateTime.Hour(2);
            dateTime.Minute(2);
            Assert.Equal("02 DEC 2012 02:02:00:000:000:000", dateTime.ToString());

            //blank date, valid time
            dateTime.Date().Blank();
            dateTime.Hour(2);
            dateTime.Minute(2);
            dateTime.Second(2);
            dateTime.Millisecond(2);
            dateTime.Microsecond(3);
            dateTime.Nanosecond(4);
            Assert.Equal("02:02:02:002:003:004", dateTime.ToString());

            // this will fail intentionally because of the blank in the middle
            dateTime.Blank();
            dateTime.Hour(2);
            dateTime.Minute(255);
            dateTime.Second(2);
            dateTime.Millisecond(2);
            dateTime.Microsecond(3);
            dateTime.Nanosecond(4);
            Assert.False(dateTime.IsValid);

            // this will work and should only output up to Second.
            dateTime.Blank();
            dateTime.Hour(2);
            dateTime.Minute(3);
            dateTime.Second(4);
            dateTime.Millisecond(65535);
            dateTime.Microsecond(2047);
            dateTime.Nanosecond(2047);
            Assert.True(dateTime.IsValid);
            Assert.Equal("02:03:04", dateTime.ToString());
        }

        [Fact]
        [Category("Unit")]
        public void dateTimeSetTest()
        {
            DateTime thisDateTime = new DateTime();

            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Year(2012));
            Assert.Equal(2012, thisDateTime.Year());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Month(10));
            Assert.Equal(10, thisDateTime.Month());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Day(15));
            Assert.Equal(15, thisDateTime.Day());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Hour(8));
            Assert.Equal(8, thisDateTime.Hour());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Minute(47));
            Assert.Equal(47, thisDateTime.Minute());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Second(43));
            Assert.Equal(43, thisDateTime.Second());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Millisecond(123));
            Assert.Equal(123, thisDateTime.Millisecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Microsecond(345));
            Assert.Equal(345, thisDateTime.Microsecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Nanosecond(223));
            Assert.Equal(223, thisDateTime.Nanosecond());
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Year(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Month(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Day(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Hour(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Minute(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Second(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Millisecond(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Year(65536));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Month(13));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Day(32));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Hour(24));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Minute(60));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Second(61));
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Second(60)); // leap Second
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Millisecond(1000));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Microsecond(1000));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisDateTime.Nanosecond(1000));
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Year(65535));
            Assert.Equal(65535, thisDateTime.Year());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Month(12));
            Assert.Equal(12, thisDateTime.Month());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Day(31));
            Assert.Equal(31, thisDateTime.Day());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Hour(23));
            Assert.Equal(23, thisDateTime.Hour());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Minute(59));
            Assert.Equal(59, thisDateTime.Minute());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Second(59));
            Assert.Equal(59, thisDateTime.Second());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Millisecond(999));
            Assert.Equal(999, thisDateTime.Millisecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Microsecond(999));
            Assert.Equal(999, thisDateTime.Microsecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisDateTime.Nanosecond(999));
            Assert.Equal(999, thisDateTime.Nanosecond());
        }

        [Fact]
        [Category("Unit")]
        public void dateTimeStringTest()
        {
            DateTime dateTime = new DateTime();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, dateTime.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, dateTime.Value("    "));
            Assert.True(dateTime.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, dateTime.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, dateTime.Value(""));
            Assert.True(dateTime.IsBlank);

            //1974/04/14 02:02:02:200
            dateTime.Value("1974/04/14 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/74 02:02:02:200
            dateTime.Value("04/14/74 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/1974 02:02:02:200
            dateTime.Value("04/14/1974 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974/04/14 02 02 02 200
            dateTime.Value("1974/04/14 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/74 02 02 02 200
            dateTime.Value("04/14/74 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/1974 02 02 02 200
            dateTime.Value("04/14/1974 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974 04 14 02:02:02:200
            dateTime.Value("1974 04 14 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 74 02:02:02:200
            dateTime.Value("04 14 74 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 1974 02:02:02:200
            dateTime.Value("04 14 1974 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 74 02:02:02:200
            dateTime.Value("04 JAN 74 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 1974 02:02:02:200
            dateTime.Value("04 JAN 1974 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 74 02:02:02:200
            dateTime.Value("04 jan 74 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 1974 02:02:02:200
            dateTime.Value("04 jan 1974 02:02:02:200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974 04 14 02 02 02 200
            dateTime.Value("1974 04 14 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 74 02 02 02 200
            dateTime.Value("04 14 74 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 1974 02 02 02 200
            dateTime.Value("04 14 1974 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974 04 14 02 02 02 200
            dateTime.Value("1974 04 14 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 74 02 02 02 200
            dateTime.Value("04 14 74 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 1974 02 02 02 200
            dateTime.Value("04 14 1974 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 74 02 02 02 200
            dateTime.Value("04 JAN 74 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 1974 02 02 02 200
            dateTime.Value("04 JAN 1974 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 74 02 02 02 200
            dateTime.Value("04 jan 74 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 1974 02 02 02 200
            dateTime.Value("04 jan 1974 02 02 02 200");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(0, dateTime.Microsecond());
            Assert.Equal(0, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            // round 2
            //1974/04/14 02:02:02:200:300:400
            dateTime.Value("1974/04/14 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/74 02:02:02:200:300:400
            dateTime.Value("04/14/74 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/1974 02:02:02:200
            dateTime.Value("04/14/1974 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974/04/14 02 02 02 200
            dateTime.Value("1974/04/14 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/74 02 02 02 200
            dateTime.Value("04/14/74 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04/14/1974 02 02 02 200
            dateTime.Value("04/14/1974 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974 04 14 02:02:02:200
            dateTime.Value("1974 04 14 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 74 02:02:02:200
            dateTime.Value("04 14 74 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 1974 02:02:02:200
            dateTime.Value("04 14 1974 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 74 02:02:02:200
            dateTime.Value("04 JAN 74 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 1974 02:02:02:200
            dateTime.Value("04 JAN 1974 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 74 02:02:02:200
            dateTime.Value("04 jan 74 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 1974 02:02:02:200
            dateTime.Value("04 jan 1974 02:02:02:200:300:400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974 04 14 02 02 02 200
            dateTime.Value("1974 04 14 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 74 02 02 02 200
            dateTime.Value("04 14 74 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 1974 02 02 02 200
            dateTime.Value("04 14 1974 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //1974 04 14 02 02 02 200
            dateTime.Value("1974 04 14 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 74 02 02 02 200
            dateTime.Value("04 14 74 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 14 1974 02 02 02 200
            dateTime.Value("04 14 1974 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(4, dateTime.Month());
            Assert.Equal(14, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 74 02 02 02 200
            dateTime.Value("04 JAN 74 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 JAN 1974 02 02 02 200
            dateTime.Value("04 JAN 1974 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 74 02 02 02 200
            dateTime.Value("04 jan 74 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);

            dateTime.Blank();

            //04 jan 1974 02 02 02 200
            dateTime.Value("04 jan 1974 02 02 02 200 300 400");
            Assert.Equal(1974, dateTime.Year());
            Assert.Equal(1, dateTime.Month());
            Assert.Equal(4, dateTime.Day());
            Assert.Equal(2, dateTime.Hour());
            Assert.Equal(2, dateTime.Minute());
            Assert.Equal(2, dateTime.Second());
            Assert.Equal(200, dateTime.Millisecond());
            Assert.Equal(300, dateTime.Microsecond());
            Assert.Equal(400, dateTime.Nanosecond());
            Assert.False(dateTime.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void dateTimeEncodeDecodeBlankTest()
        {
            DateTime dateTime = new DateTime();

            dateTime.Blank();
            Assert.True(dateTime.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecReturnCode ret = dateTime.Encode(_encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            DateTime dateTime1 = new DateTime();
            ret = dateTime1.Decode(_decIter);
            Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
            Assert.True(dateTime1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void CopyDateTimeTest()
        {
            DateTime Left = new DateTime();
            DateTime Right = new DateTime();
            Left.LocalTime();

            Time tempTime = new Time();
            Date tempDate = new Date();
            Left.Time().Copy(tempTime);
            Left.Date().Copy(tempDate);

            addToTime(tempTime, 2, 2, 2, 2, 4, 5);
            addToDate(tempDate, 3, 3, 3);

            Right.Nanosecond(tempTime.Nanosecond());
            Right.Microsecond(tempTime.Microsecond());
            Right.Millisecond(tempTime.Millisecond());
            Right.Second(tempTime.Second());
            Right.Minute(tempTime.Minute());
            Right.Hour(tempTime.Hour());
            Right.Day(tempDate.Day());
            Right.Month(tempDate.Month());
            Right.Year(tempDate.Year());

            // test that they are NOT equal!
            Assert.True(Left.Day() != Right.Day());
            Assert.True(Left.Month() != Right.Month());
            Assert.True(Left.Year() != Right.Year());
            Assert.True(Left.Hour() != Right.Hour());
            Assert.True(Left.Minute() != Right.Minute());
            Assert.True(Left.Second() != Right.Second());
            Assert.True(Left.Millisecond() != Right.Millisecond());
            Assert.True(Left.Microsecond() != Right.Microsecond());
            Assert.True(Left.Nanosecond() != Right.Nanosecond());

            Left.Copy(Right);  // Testing this method

            Assert.Equal(Left.Day(), Right.Day());
            Assert.Equal(Left.Month(), Right.Month());
            Assert.Equal(Left.Year(), Right.Year());
            Assert.Equal(Left.Hour(), Right.Hour());
            Assert.Equal(Left.Minute(), Right.Minute());
            Assert.Equal(Left.Second(), Right.Second());
            Assert.Equal(Left.Millisecond(), Right.Millisecond());
            Assert.Equal(Left.Microsecond(), Right.Microsecond());
            Assert.Equal(Left.Nanosecond(), Right.Nanosecond());

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            addToTime(tempTime, 5, 5, 5, 5, 6, 7);
            addToDate(tempDate, 5, 11, 15);

            Right.Nanosecond(tempTime.Nanosecond());
            Right.Microsecond(tempTime.Microsecond());
            Right.Millisecond(tempTime.Millisecond());
            Right.Second(tempTime.Second());
            Right.Minute(tempTime.Minute());
            Right.Hour(tempTime.Hour());
            Right.Day(tempDate.Day());
            Right.Month(tempDate.Month());
            Right.Year(tempDate.Year());

            Assert.True(Left.Day() != Right.Day());
            Assert.True(Left.Month() != Right.Month());
            Assert.True(Left.Year() != Right.Year());
            Assert.True(Left.Hour() != Right.Hour());
            Assert.True(Left.Minute() != Right.Minute());
            Assert.True(Left.Second() != Right.Second());
            Assert.True(Left.Millisecond() != Right.Millisecond());
            Assert.True(Left.Microsecond() != Right.Microsecond());
            Assert.True(Left.Nanosecond() != Right.Nanosecond());
        }

        [Fact]
        [Category("Unit")]
        public void CopyTimeTest()
        {
            Time Left = new Time();
            Time Right = new Time();
            DateTime temp = new DateTime();
            temp.LocalTime();
            Left = temp._time;

            // test that they are NOT equal!
            Left.Copy(Right);  // Testing this method
            addToTime(Right, 2, 2, 2, 2, 3, 4);

            Assert.True(Left.Hour() != Right.Hour());
            Assert.True(Left.Minute() != Right.Minute());
            Assert.True(Left.Second() != Right.Second());
            Assert.True(Left.Millisecond() != Right.Millisecond());
            Assert.True(Left.Microsecond() != Right.Microsecond());
            Assert.True(Left.Nanosecond() != Right.Nanosecond());

            Left.Copy(Right);  // Testing this method


            Assert.Equal(Left.Hour(), Right.Hour());
            Assert.Equal(Left.Minute(), Right.Minute());
            Assert.Equal(Left.Second(), Right.Second());
            Assert.Equal(Left.Millisecond(), Right.Millisecond());
            Assert.Equal(Left.Microsecond(), Right.Microsecond());
            Assert.Equal(Left.Nanosecond(), Right.Nanosecond());

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            addToTime(Right, 4, 4, 4, 400, 500, 600);

            Assert.True(Left.Hour() != Right.Hour());
            Assert.True(Left.Minute() != Right.Minute());
            Assert.True(Left.Second() != Right.Second());
            Assert.True(Left.Millisecond() != Right.Millisecond());
            Assert.True(Left.Microsecond() != Right.Microsecond());
            Assert.True(Left.Nanosecond() != Right.Nanosecond());
        }

        [Fact]
        [Category("Unit")]
        public void CopyDateTest()
        {
            Date Left = new Date();
            Date Right = new Date();

            Left.Value("3/17/1967");

            // test that they are NOT equal!
            Assert.True(Left.Day() != Right.Day());
            Assert.True(Left.Month() != Right.Month());
            Assert.True(Left.Year() != Right.Year());


            Left.Copy(Right);  // Testing this method

            Assert.Equal(Left.Day(), Right.Day());
            Assert.Equal(Left.Month(), Right.Month());
            Assert.Equal(Left.Year(), Right.Year());

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            int idt_day = Left.Day();
            int idt_month = Left.Month();
            int idt_year = Left.Year();

            if (1 == idt_day) idt_day = 15; else idt_day--;
            if (1 == idt_month) idt_month = 6; else idt_month--;
            if (1 == idt_year) idt_year = 1967; else idt_year--;

            Right.Day(idt_day);
            Right.Month(idt_month);
            Right.Year(idt_year);


            Assert.True(Left.Day() != Right.Day());
            Assert.True(Left.Month() != Right.Month());
            Assert.True(Left.Year() != Right.Year());
        }

        [Fact]
        [Category("Unit")]
        public void CopyQosTest()
        {
            Qos Left = new Qos();
            Qos Right = new Qos();

            Left.Blank();
            Right._dynamic = true;
            Right.IsBlank = false;
            Right._rate = QosRates.JIT_CONFLATED;
            Right._timeliness = QosTimeliness.DELAYED;
            Right._timeInfo = 1;
            Right._rateInfo = 1;

            // Make sure they are NOT equal, by asserting if they ARE equal
            Assert.True(Left._dynamic != Right._dynamic);
            Assert.True(Left.IsBlank != Right.IsBlank);
            Assert.True(Left._rate != Right._rate);
            Assert.True(Left._timeliness != Right._timeliness);
            Assert.True(Left._timeInfo != Right._timeInfo);
            Assert.True(Left._rateInfo != Right._rateInfo);

            Left.Copy(Right);
            //  Make sure they are Equal
            Assert.Equal(Left._dynamic, Right._dynamic);
            Assert.Equal(Left.IsBlank, Right.IsBlank);
            Assert.Equal(Left._rate, Right._rate);
            Assert.Equal(Left._timeliness, Right._timeliness);
            Assert.Equal(Left._timeInfo, Right._timeInfo);
            Assert.Equal(Left._rateInfo, Right._rateInfo);

            // Change it again to make sure they are now NOT equal 
            // ( this ensures we are doing a DEEP copy and not just copying references)
            Right._dynamic = true;
            Right.IsBlank = false;
            Right._rate = QosRates.JIT_CONFLATED;
            Right._timeliness = QosTimeliness.DELAYED;
            Right._timeInfo = 1;
            Right._rateInfo = 1;
            Assert.True(Left._dynamic != Right._dynamic);
            Assert.True(Left.IsBlank != Right.IsBlank);
            Assert.True(Left._rate != Right._rate);
            Assert.True(Left._timeliness != Right._timeliness);
            Assert.True(Left._timeInfo != Right._timeInfo);
            Assert.True(Left._rateInfo != Right._rateInfo);
        }

        [Fact]
        [Category("Unit")]
        public void TimeBlankTest()
        {
            Time time = new Time();
            time.Blank();
            Assert.True(time.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void TimeEqualsTest()
        {
            Time thisTime = new Time();
            Time thatTime = new Time();
            thisTime.Blank();
            thatTime.Blank();
            Assert.True(thisTime.Equals(thatTime));
            thisTime.Hour(23);
            thisTime.Minute(59);
            thisTime.Second(59);
            thisTime.Millisecond(999);
            thatTime.Hour(23);
            thatTime.Minute(59);
            thatTime.Second(59);
            thatTime.Millisecond(999);
            Assert.True(thisTime.Equals(thatTime));
            thatTime.Blank();
            Assert.False(thisTime.Equals(thatTime));
        }

        [Fact]
        [Category("Unit")]
        public void TimeSetTest()
        {
            Time thisTime = new Time();

            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Hour(0));
            Assert.Equal(0, thisTime.Hour());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Minute(0));
            Assert.Equal(0, thisTime.Minute());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Second(0));
            Assert.Equal(0, thisTime.Second());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Millisecond(0));
            Assert.Equal(0, thisTime.Millisecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Microsecond(0));
            Assert.Equal(0, thisTime.Microsecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Nanosecond(0));
            Assert.Equal(0, thisTime.Nanosecond());
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Hour(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Minute(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Second(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Millisecond(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Hour(24));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Minute(60));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Second(61));
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Second(60)); // leap Second
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Millisecond(1000));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Microsecond(1000));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisTime.Nanosecond(1000));
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Hour(23));
            Assert.Equal(23, thisTime.Hour());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Minute(59));
            Assert.Equal(59, thisTime.Minute());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Second(59));
            Assert.Equal(59, thisTime.Second());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Millisecond(999));
            Assert.Equal(999, thisTime.Millisecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Microsecond(999));
            Assert.Equal(999, thisTime.Microsecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Nanosecond(999));
            Assert.Equal(999, thisTime.Nanosecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Hour(255));
            Assert.Equal(255, thisTime.Hour());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Minute(255));
            Assert.Equal(255, thisTime.Minute());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Second(255));
            Assert.Equal(255, thisTime.Second());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Millisecond(65535));
            Assert.Equal(65535, thisTime.Millisecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Microsecond(2047));
            Assert.Equal(2047, thisTime.Microsecond());
            Assert.Equal(CodecReturnCode.SUCCESS, thisTime.Nanosecond(2047));
            Assert.Equal(2047, thisTime.Nanosecond());
            Assert.True(thisTime.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void TimeStringTest()
        {
            Time time = new Time();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, time.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, time.Value("    "));
            Assert.True(time.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, time.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, time.Value(""));
            Assert.True(time.IsBlank);

            //02:02:02:200
            time.Value("02:02:02:200:300:400");
            Assert.Equal(2, time.Hour());
            Assert.Equal(2, time.Minute());
            Assert.Equal(2, time.Second());
            Assert.Equal(200, time.Millisecond());
            Assert.Equal(300, time.Microsecond());
            Assert.Equal(400, time.Nanosecond());
            Assert.False(time.IsBlank);

            time.Blank();

            //02 02 02 200
            time.Value("02 02 02 200 300 400");
            Assert.Equal(2, time.Hour());
            Assert.Equal(2, time.Minute());
            Assert.Equal(2, time.Second());
            Assert.Equal(200, time.Millisecond());
            Assert.Equal(300, time.Microsecond());
            Assert.Equal(400, time.Nanosecond());
            Assert.False(time.IsBlank);

            time.Blank();

            //02:02:02
            time.Value("02:02:02");
            Assert.Equal(2, time.Hour());
            Assert.Equal(2, time.Minute());
            Assert.Equal(2, time.Second());
            Assert.False(200 == time.Millisecond());
            Assert.False(300 == time.Microsecond());
            Assert.False(400 == time.Nanosecond());
            Assert.False(time.IsBlank);

            time.Blank();

            //02:02
            time.Value("02:02");
            Assert.Equal(2, time.Hour());
            Assert.Equal(2, time.Minute());
            Assert.False(2 == time.Second());
            Assert.False(200 == time.Millisecond());
            Assert.False(300 == time.Microsecond());
            Assert.False(400 == time.Nanosecond());
            Assert.False(time.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void TimeEncodeDecodeTest()
        {
            Time time = new Time();

            for (int hour = 23; hour >= 0; hour = hour - 2)
            {
                time.Hour(hour);
                for (int min = 59; min >= 0; min = min - 10)
                {
                    time.Minute(min);
                    for (int sec = 59; sec >= 0; sec = sec - 10)
                    {
                        time.Second(sec);
                        for (int msec = 59; msec >= 0; msec = msec - 10)
                        {
                            time.Millisecond(msec);
                            for (int usec = 59; usec >= 0; usec = usec - 10)
                            {
                                time.Microsecond(usec);
                                for (int nsec = 59; nsec >= 0; nsec = nsec - 10)
                                {
                                    time.Nanosecond(nsec);
                                    timeED(time);
                                }
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        public void TimeEncodeDecodeBlankTest()
        {
            Time time = new Time();

            time.Blank();
            Assert.True(time.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, time.Encode(_encIter));

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Time time1 = new Time();
            time1.Decode(_decIter);

            Assert.Equal(time.Hour(), time1.Hour());
            Assert.Equal(time.Minute(), time1.Minute());
            Assert.Equal(time.Second(), time1.Second());
            Assert.Equal(time.Millisecond(), time1.Millisecond());
            Assert.Equal(time.Microsecond(), time1.Microsecond());
            Assert.Equal(time.Nanosecond(), time1.Nanosecond());
        }

        [Fact]
        [Category("Unit")]
        public void EnumEncodeDecodeTest()
        {
            Enum enumv = new Enum();

            for (short e = 0; e <= 0xFF; e++)
            {
                enumv.Value(e);

                _encIter.Clear();
                _buffer.Data(new ByteBuffer(15));
                _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

                enumv.Encode(_encIter);

                _decIter.Clear();
                _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

                Enum enumv1 = new Enum();
                enumv1.Decode(_decIter);

                Assert.Equal(enumv.ToInt(), enumv1.ToInt());
            }
        }

        [Fact]
        [Category("Unit")]
        public void EnumEncodeDecodeBlankTest()
        {
            Enum enumv = new Enum();

            Assert.False(enumv.IsBlank);
            enumv.Blank();
            Assert.True(enumv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, enumv.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.INT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Enum enum1 = new Enum();
            Assert.False(enum1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, enum1.Decode(_decIter));

            Assert.True(enum1.IsBlank);
            Assert.Equal(enumv.ToInt(), enum1.ToInt());
        }

        [Fact]
        [Category("Unit")]
        public void enumSetTest()
        {
            Enum thisEnum = new Enum();

            Assert.Equal(CodecReturnCode.SUCCESS, thisEnum.Value(0));
            Assert.Equal(0, thisEnum.ToInt());
            Assert.Equal(CodecReturnCode.SUCCESS, thisEnum.Value(65535));
            Assert.Equal(65535, thisEnum.ToInt());
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisEnum.Value(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisEnum.Value(65536));
        }

        [Fact]
        [Category("Unit")]
        public void enumStringTest()
        {
            string enumStr1 = "0";
            string enumStr2 = "65535";
            string enumStr3 = "32000";
            Enum testEnum = new Enum();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testEnum.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, testEnum.Value("    "));
            Assert.True(testEnum.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testEnum.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, testEnum.Value(""));
            Assert.True(testEnum.IsBlank);

            Assert.Equal(CodecReturnCode.SUCCESS, testEnum.Value(enumStr1));
            Assert.Equal(enumStr1, testEnum.ToString());
            Assert.False(testEnum.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, testEnum.Value(enumStr2));
            Assert.Equal(enumStr2, testEnum.ToString());
            Assert.False(testEnum.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, testEnum.Value(enumStr3));
            Assert.Equal(enumStr3, testEnum.ToString());
            Assert.False(testEnum.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void RealEncodeDecodeTest()
        {
            Real real = new Real();

            for (int hint = 0; hint <= 30; ++hint)
            {
                long val;
                /* leading 0's */
                for (val = 6148914691236517205; val != 0; val >>= 1)
                {
                    real.Value(val, hint);
                    RealED(real);
                }

                for (val = -6148914691236517206; val != -1; val >>= 1)
                {
                    real.Value(val, hint);
                    RealED(real);
                }

                /* trailing 0's */
                for (val = -6148914691236517206; val != 0; val <<= 1)
                {
                    real.Value(val, hint);
                    RealED(real);
                }

                real.Value(0xFFFFFFFFFFFFFFFFL, hint);
                RealED(real);

                real.Value(0, RealHints.INFINITY);
                RealED(real);

                real.Value(0, RealHints.NEG_INFINITY);
                RealED(real);

                real.Value(0, RealHints.NOT_A_NUMBER);
                RealED(real);


                real.Value(0, hint);
                RealED(real);
            }
        }

        [Fact]
        [Category("Unit")]
        public void realEncodeDecodeBlankTest()
        {
            Real real = new Real();

            real.Blank();
            Assert.True(real.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, real.Encode(_encIter));

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Real real1 = new Real();
            real1.Decode(_decIter);

            Assert.Equal(real.Hint, real1.Hint);
            Assert.Equal(real.ToLong(), real1.ToLong());
            Assert.True(real.Equals(real1));
            Assert.True(real1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void realSetTest()
        {
            Real thisReal = new Real();

            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(-1000000000, RealHints.EXPONENT_14));
            Assert.Equal(-1000000000, thisReal.ToLong());
            Assert.Equal(RealHints.EXPONENT_14, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(1000000000, RealHints.FRACTION_256));
            Assert.Equal(1000000000, thisReal.ToLong());
            Assert.Equal(RealHints.FRACTION_256, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(-111222.333444, RealHints.EXPONENT_6));
            Assert.Equal(-111222.333444, thisReal.ToDouble(), 0);
            Assert.Equal(RealHints.EXPONENT_6, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(111222.333444, RealHints.EXPONENT_6));
            Assert.Equal(111222.333444, thisReal.ToDouble(), 0);
            Assert.Equal(RealHints.EXPONENT_6, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((float)-111222.333444, RealHints.EXPONENT_6));
            Assert.Equal((float)-111222.333444, (float)thisReal.ToDouble(), 0);
            Assert.Equal(RealHints.EXPONENT_6, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((float)111222.333444, RealHints.EXPONENT_6));
            Assert.Equal((float)111222.333444, (float)thisReal.ToDouble(), 0);
            Assert.Equal(RealHints.EXPONENT_6, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(0, RealHints.EXPONENT_2));
            Assert.Equal(RealHints.EXPONENT_2, thisReal.Hint);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(0, RealHints.INFINITY));
            Assert.Equal(RealHints.INFINITY, thisReal.Hint);
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(0, RealHints.NEG_INFINITY));
            Assert.Equal(RealHints.NEG_INFINITY, thisReal.Hint);
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(0, RealHints.EXPONENT7));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(0, RealHints.EXPONENT_12));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value(0, RealHints.EXPONENT_14));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((double)0, RealHints.EXPONENT_2));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((double)0, RealHints.EXPONENT7));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((double)0, RealHints.EXPONENT_12));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((double)0, RealHints.EXPONENT_14));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((float)0, RealHints.EXPONENT_2));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((float)0, RealHints.EXPONENT7));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((float)0, RealHints.EXPONENT_12));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value((float)0, RealHints.EXPONENT_14));
            Assert.Equal(0, thisReal.ToLong());
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value("Inf"));
            Assert.Equal(0, thisReal.ToLong());
            Assert.Equal(RealHints.INFINITY, thisReal.Hint);
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value("-Inf"));
            Assert.Equal(0, thisReal.ToLong());
            Assert.Equal(RealHints.NEG_INFINITY, thisReal.Hint);
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.SUCCESS, thisReal.Value("NaN"));
            Assert.Equal(0, thisReal.ToLong());
            Assert.Equal(RealHints.NOT_A_NUMBER, thisReal.Hint);
            Assert.False(thisReal.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisReal.Value(-1000000000, -1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisReal.Value(1000000000, 31));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisReal.Value(-111222.333444, -1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisReal.Value(111222.333444, 31));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisReal.Value((float)-111222.333444, -1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisReal.Value((float)111222.333444, 31));
        }

        [Fact]
        [Category("Unit")]
        public void RealStringTest()
        {
            string realStr1 = "0.0";
            string realStr2 = "1.1234567";
            string realStr3 = "100000.0000001";
            string realStr4 = "1";
            string realStr5 = "0.0000001";
            string realStr6 = "+0";
            string realStr7 = "+0.0";
            string realStr7a = "+0.0000000000000";
            string realStr8 = "-123456.7654321";
            string realStr9 = "-0.0";
            string realStr10 = "-10";
            string realStr11 = "1/4";
            string realStr12 = "-1/4";
            string realStr13 = "100 1/4";
            string realStr14 = "-1000 1/4";
            string realStr15 = "  -0.9999999             ";
            string realStr16 = "-9223372036854775808";
            string realStr17 = "9223372036854775807";
            string realStr18 = "922337203685477.5807";
            string realStr19 = "-922337203685477.5808";
            string realStr20 = "92233720368547757 1/4";
            string realStr21 = "-92233720368547758 1/4";
            string realStr22 = "Inf";
            string realStr23 = "-Inf";
            string realStr24 = "NaN";
            Real testReal = new Real();
            double tempDouble;

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testReal.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, testReal.Value("   "));
            Assert.True(testReal.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testReal.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, testReal.Value(""));
            Assert.True(testReal.IsBlank);

            testReal.Value(realStr1);
            tempDouble = System.Convert.ToDouble(realStr1, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr2);
            tempDouble = System.Convert.ToDouble(realStr2, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr3);
            tempDouble = System.Convert.ToDouble(realStr3, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr4);
            tempDouble = System.Convert.ToDouble(realStr4, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr5);
            tempDouble = System.Convert.ToDouble(realStr5, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr6);
            Assert.True(testReal.IsBlank);
            testReal.Value(realStr7);
            Assert.True(testReal.IsBlank);
            testReal.Value(realStr7a);
            Assert.True(testReal.IsBlank);
            testReal.Value(realStr8);
            tempDouble = System.Convert.ToDouble(realStr8, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr9);
            tempDouble = System.Convert.ToDouble(realStr9, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr10);
            tempDouble = System.Convert.ToDouble(realStr10, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr11);
            tempDouble = 0.25;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr12);
            tempDouble = -0.25;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr13);
            tempDouble = 100.25;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr14);
            tempDouble = -1000.25;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr15);
            tempDouble = System.Convert.ToDouble(realStr15, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr16);
            Assert.Equal(-9223372036854775808L, testReal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, testReal.Hint);
            tempDouble = System.Convert.ToDouble(realStr16, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr17);
            Assert.Equal(9223372036854775807L, testReal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, testReal.Hint);
            tempDouble = System.Convert.ToDouble(realStr17, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr18);
            Assert.Equal(9223372036854775807L, testReal.ToLong());
            Assert.Equal(RealHints.EXPONENT_4, testReal.Hint);
            tempDouble = System.Convert.ToDouble(realStr18, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr19);
            Assert.Equal(-9223372036854775808L, testReal.ToLong());
            Assert.Equal(RealHints.EXPONENT_4, testReal.Hint);
            tempDouble = System.Convert.ToDouble(realStr19, CultureInfo.InvariantCulture);
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr20);
            tempDouble = 92233720368547757.25;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr21);
            tempDouble = -92233720368547758.25;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr22);
            tempDouble = double.PositiveInfinity;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr23);
            tempDouble = double.NegativeInfinity;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
            testReal.Value(realStr24);
            tempDouble = double.NaN;
            Assert.Equal(testReal.ToDouble(), tempDouble, 0);
            Assert.False(testReal.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void QosEncodeDecodeBlankTest()
        {
            Qos qosv = new Qos();

            Assert.False(qosv.IsBlank);
            qosv.Blank();
            Assert.True(qosv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, qosv.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.INT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Qos qos1 = new Qos();
            Assert.False(qos1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, qos1.Decode(_decIter));

            Assert.True(qos1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void QosSetTest()
        {
            Qos thisQos = new Qos();

            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.Rate(QosRates.UNSPECIFIED));
            Assert.Equal(QosRates.UNSPECIFIED, thisQos.Rate());
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.Timeliness(QosTimeliness.UNSPECIFIED));
            Assert.Equal(QosTimeliness.UNSPECIFIED, thisQos.Timeliness());
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.RateInfo(0));
            Assert.Equal(0, thisQos.RateInfo());
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.TimeInfo(0));
            Assert.Equal(0, thisQos.TimeInfo());
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.Rate(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.Timeliness(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.RateInfo(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.TimeInfo(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.Rate(16));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.Timeliness(8));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.RateInfo(65536));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisQos.TimeInfo(65536));
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.Rate(QosRates.TIME_CONFLATED));
            Assert.Equal(QosRates.TIME_CONFLATED, thisQos.Rate());
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.Timeliness(QosTimeliness.DELAYED));
            Assert.Equal(QosTimeliness.DELAYED, thisQos.Timeliness());
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.RateInfo(65535));
            Assert.Equal(65535, thisQos.RateInfo());
            Assert.Equal(CodecReturnCode.SUCCESS, thisQos.TimeInfo(65535));
            Assert.Equal(65535, thisQos.TimeInfo());
        }

        [Fact]
        [Category("Unit")]
        public void QosEncodeDecodeTest()
        {
            int[] rates = { QosRates.UNSPECIFIED,
                QosRates.TICK_BY_TICK,
                QosRates.JIT_CONFLATED,
                QosRates.TIME_CONFLATED };
            int[] timeliness = { QosTimeliness.UNSPECIFIED,
                QosTimeliness.REALTIME,
                QosTimeliness.DELAYED_UNKNOWN,
                QosTimeliness.DELAYED };
            Qos qos = new Qos();
            int rateInfo = 0;
            int timeInfo = 0;

            for (int rate = 0; rate < rates.Length; rate++)
            {
                for (int tm = 0; tm < timeliness.Length; tm++)
                {
                    if (rates[rate] == QosRates.TIME_CONFLATED)
                    {
                        for (rateInfo = 0x007f; rateInfo == 0; rateInfo >>= 1)
                        {
                            if (timeliness[tm] == QosTimeliness.DELAYED)
                            {
                                for (timeInfo = 0x007f; timeInfo == 0; timeInfo >>= 1)
                                {
                                    qos.Rate(rates[rate]);
                                    qos.Timeliness(timeliness[tm]);
                                    qos.RateInfo(rateInfo);
                                    qos.TimeInfo(timeInfo);
                                    qos.IsDynamic = false;
                                    QosED(qos);

                                    qos.Rate(rates[rate]);
                                    qos.Timeliness(timeliness[tm]);
                                    qos.RateInfo(rateInfo);
                                    qos.TimeInfo(timeInfo);
                                    qos.IsDynamic = true;
                                    QosED(qos);
                                }
                            }
                        }
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        public void QosEqualsTest()
        {
            Qos qos1 = new Qos();
            Qos qos2 = new Qos();

            Assert.True(qos1.Equals(qos2));
            qos1.Rate(QosRates.TICK_BY_TICK);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.Rate(QosRates.JIT_CONFLATED);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.Rate(QosRates.TIME_CONFLATED);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.Timeliness(QosTimeliness.REALTIME);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.Timeliness(QosTimeliness.DELAYED);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.RateInfo(1000);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
            qos1.TimeInfo(2000);
            Assert.False(qos1.Equals(qos2));
            qos1.Clear();
            Assert.True(qos1.Equals(qos2));
        }

        [Fact]
        [Category("Unit")]
        public void QosIsBetterTest()
        {
            Qos qos1 = new Qos();
            Qos qos2 = new Qos();

            Assert.False(qos1.IsBetter(qos2));
            qos1.Rate(QosRates.TICK_BY_TICK);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Rate(QosRates.JIT_CONFLATED);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Rate(QosRates.TIME_CONFLATED);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Timeliness(QosTimeliness.REALTIME);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Timeliness(QosTimeliness.DELAYED);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Rate(QosRates.TIME_CONFLATED);
            qos1.RateInfo(1000);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
            qos1.Timeliness(QosTimeliness.DELAYED);
            qos1.TimeInfo(2000);
            Assert.True(qos1.IsBetter(qos2));
            qos1.Clear();
            Assert.False(qos1.IsBetter(qos2));
        }

        [Fact]
        [Category("Unit")]
        public void QosIsInRangeTest()
        {
            Qos qos = new Qos();
            Qos bestQos = new Qos();
            Qos worstQos = new Qos();

            bestQos._rate = QosRates.TIME_CONFLATED;
            bestQos._rateInfo = 65532;
            bestQos._timeliness = QosTimeliness.DELAYED;
            bestQos._timeInfo = 65533;
            worstQos._rate = QosRates.TIME_CONFLATED;
            worstQos._rateInfo = 65534;
            worstQos._timeliness = QosTimeliness.DELAYED;
            worstQos._timeInfo = 65535;

            Assert.False(qos.IsInRange(bestQos, worstQos));
            qos._rate = QosRates.TIME_CONFLATED;
            qos._rateInfo = 65531;
            qos._timeliness = QosTimeliness.DELAYED;
            qos._timeInfo = 65532;
            Assert.False(qos.IsInRange(bestQos, worstQos));
            qos._rate = QosRates.TIME_CONFLATED;
            qos._rateInfo = 65535;
            qos._timeliness = QosTimeliness.DELAYED;
            qos._timeInfo = 65536;
            Assert.False(qos.IsInRange(bestQos, worstQos));
            qos.Clear();
            Assert.False(qos.IsInRange(bestQos, worstQos));
            qos._rate = QosRates.TIME_CONFLATED;
            qos._rateInfo = 65532;
            qos._timeliness = QosTimeliness.DELAYED;
            qos._timeInfo = 65533;
            Assert.True(qos.IsInRange(bestQos, worstQos));
            qos.Clear();
            Assert.False(qos.IsInRange(bestQos, worstQos));
            qos._rate = QosRates.TIME_CONFLATED;
            qos._rateInfo = 65533;
            qos._timeliness = QosTimeliness.DELAYED;
            qos._timeInfo = 65534;
            Assert.True(qos.IsInRange(bestQos, worstQos));
            qos.Clear();
            Assert.False(qos.IsInRange(bestQos, worstQos));
            qos._rate = QosRates.TIME_CONFLATED;
            qos._rateInfo = 65534;
            qos._timeliness = QosTimeliness.DELAYED;
            qos._timeInfo = 65535;
            Assert.True(qos.IsInRange(bestQos, worstQos));
        }

        [Fact]
        [Category("Unit")]
        public void StateEncodeDecodeBlankTest()
        {
            State statev = new State();

            Assert.False(statev.IsBlank);
            statev.Blank();
            Assert.True(statev.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, statev.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.INT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            State state1 = new State();
            Assert.False(state1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, state1.Decode(_decIter));

            Assert.True(state1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void StateSetTest()
        {
            State thisState = new State();
            Buffer buffer = new Buffer();

            Assert.Equal(CodecReturnCode.SUCCESS, thisState.Code(StateCodes.NONE));
            Assert.Equal(StateCodes.NONE, thisState.Code());
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.DataState(DataStates.NO_CHANGE));
            Assert.Equal(DataStates.NO_CHANGE, thisState.DataState());
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.StreamState(StreamStates.UNSPECIFIED));
            Assert.Equal(StreamStates.UNSPECIFIED, thisState.StreamState());
            buffer.Data("");
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.Text(buffer));
            Assert.Equal("", thisState.Text().ToString());
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.Code(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.DataState(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.StreamState(-1));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.Text(null));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.Code(128));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.DataState(8));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, thisState.StreamState(32));
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.Code(StateCodes.UNABLE_TO_REQUEST_AS_BATCH));
            Assert.Equal(StateCodes.UNABLE_TO_REQUEST_AS_BATCH, thisState.Code());
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.DataState(DataStates.SUSPECT));
            Assert.Equal(DataStates.SUSPECT, thisState.DataState());
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.StreamState(StreamStates.REDIRECTED));
            Assert.Equal(StreamStates.REDIRECTED, thisState.StreamState());
            buffer.Data("1234567890");
            Assert.Equal(CodecReturnCode.SUCCESS, thisState.Text(buffer));
            Assert.Equal("1234567890", thisState.Text().ToString());
        }

        [Fact]
        [Category("Unit")]
        public void StateEncodeDecodeTest()
        {
            int[] ss = { StreamStates.UNSPECIFIED,
                StreamStates.OPEN,
                StreamStates.NON_STREAMING,
                StreamStates.CLOSED_RECOVER,
                StreamStates.CLOSED,
                StreamStates.REDIRECTED };
            int[] ds = { DataStates.NO_CHANGE,
                DataStates.OK,
                DataStates.SUSPECT };

            State state = new State();
            int code; // -34 - 19
            Buffer s1 = new Buffer();
            s1.Data("abc");
            string s2 = "qwertyu";
            Buffer s = new Buffer();
            s.Data(s2);

            for (int i = 0; i < ss.Length; i++)
            {
                for (int j = 0; j < ds.Length; j++)
                {
                    for (code = -34; code == 19; code++)
                    {
                        state.Code(code);
                        state.DataState(ds[j]);
                        state.StreamState(ss[i]);
                        StateED(state);

                        state.Text(s1);
                        StateED(state);

                        state.Text(s);
                        StateED(state);
                    }
                }
            }
        }

        [Fact]
        [Category("Unit")]
        public void StateCopyTest()
        {
            State testState = new State();
            State copiedState = new State();
            testState.StreamState(StreamStates.REDIRECTED);
            testState.DataState(DataStates.SUSPECT);
            testState.Text().Data("testString");
            testState.Code(StateCodes.FULL_VIEW_PROVIDED);
            Assert.Equal(StreamStates.REDIRECTED, testState.StreamState());
            Assert.Equal(StateCodes.FULL_VIEW_PROVIDED, testState.Code());
            Assert.Equal(DataStates.SUSPECT, testState.DataState());

            testState.Copy(copiedState);
            Assert.Equal(StreamStates.REDIRECTED, copiedState.StreamState());
            Assert.Equal(DataStates.SUSPECT, copiedState.DataState());
            Assert.Equal(StateCodes.FULL_VIEW_PROVIDED, copiedState.Code());
            Assert.Equal(testState.Text().ToString(), copiedState.Text().ToString());
            Assert.NotSame(testState, copiedState);

            testState.Text().Data("testString123");
            testState.Copy(copiedState);
            Assert.Equal(StreamStates.REDIRECTED, copiedState.StreamState());
            Assert.Equal(DataStates.SUSPECT, copiedState.DataState());
            Assert.Equal(StateCodes.FULL_VIEW_PROVIDED, copiedState.Code());
            Assert.Equal(testState.Text().ToString(), copiedState.Text().ToString());
        }

        [Fact]
        [Category("Unit")]
        public void BufEncodeDecodeBlankTest()
        {
            Buffer bufv = new Buffer();

            Assert.False(bufv.IsBlank);
            bufv.Blank();
            Assert.True(bufv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, bufv.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.INT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Buffer buf1 = new Buffer();
            Assert.False(buf1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, buf1.Decode(_decIter));

            Assert.True(buf1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        [Category("ByteBuffer")]
        public void BufferEncodeDecodeTest()
        {
            byte[] data = { 0, 1, 2, 3, 4, 5, 6, 7 };
            Buffer buf = new Buffer();
            buf.Data(ByteBuffer.Wrap(data));
            BufferED(buf);

            byte[] data1 = { 0, 1, 2 };
            buf.Data(ByteBuffer.Wrap(data1));
            BufferED(buf);

            buf = new Buffer();
            buf.Data("abhytgcdty");
            BufferED(buf);

            buf = new Buffer();
            buf.Data("abhytgcdty");
            BufferED(buf);
        }

        [Fact]
        [Category("Unit")]
        public void FloatingPointTests()
        {
            float maxFloatConstant = 340282346638528860000000000000000000000.0f;
            float minFloatConstant = 0.0000000000000000000000000000000000000000000014012984643248171f;
            Float maxFloat = new Float();
            maxFloat.Value(340282346638528860000000000000000000000.0f);
            Float minFloat = new Float();
            minFloat.Value(0.0000000000000000000000000000000000000000000014012984643248171f);
            Float testFloat32 = new Float();
            testFloat32.Value(160141100000000000000000000000000000000.0f);
            Float testFloat32min = new Float();
            testFloat32min.Value(0.0000000000000000000000000000000000000000000024012984643248171f);
            Double testFloat64 = new Double();
            testFloat64.Value(440282346638528860000000000000000000000.0);

            Double testFloat64min = new Double();
            testFloat64min.Value(0.00000000000000000000000000000000000000000000040129846432481707);

            Double testCast64a = new Double();
            testCast64a.Value(160141100000000000000000000000000000000.0f);

            Double testCast64b = new Double();
            testCast64b.Value(440282346638528860000000000000000000000.0);

            Float testFloat = new Float();
            testFloat.Value((float)testCast64b.ToDouble()); //This is from a float that is too big
            Float testFloat2 = new Float();
            testFloat2.Value((float)testCast64a.ToDouble()); //This is from a valid 32 bit float

            Assert.True(maxFloat.ToFloat() > testFloat32.ToFloat());
            Assert.True(maxFloatConstant > testFloat32.ToFloat());
            Assert.True(testFloat32.ToFloat() < maxFloat.ToFloat());

            Assert.True(minFloat.ToFloat() < testFloat32min.ToFloat());
            Assert.True(minFloat.ToFloat() < testFloat32.ToFloat());
            Assert.True(minFloatConstant < testFloat32min.ToFloat());
            Assert.True(minFloat.ToFloat() > testFloat64min.ToDouble());

            Assert.True(testFloat64.ToDouble() > maxFloat.ToFloat());
            Assert.True(minFloat.ToFloat() < testFloat64.ToDouble());

            Assert.True(testCast64a.ToDouble() == testFloat2.ToFloat());
            Assert.False(testCast64b.ToDouble() == testFloat.ToFloat());
        }

        [Fact]
        [Category("Unit")]
        [Category("ByteBuffer")]
        public void StringCompareTests()
        {
            ByteBuffer buf1;
            buf1 = PutString("Test\0garbage");
            Buffer buffer1 = new Buffer();
            buffer1.Data(buf1, 0, 0);
            ByteBuffer buf2;
            buf2 = PutString("tEST\0moreGarbage");
            Buffer buffer2 = new Buffer();
            buffer2.Data(buf2, 0, 0);
            ByteBuffer buf3;
            buf3 = PutString("tESTer");
            Buffer buffer3 = new Buffer();
            buffer3.Data(buf3, 0, 0);

            /* All zero length */
            Assert.True(buffer1.Equals(buffer2));
            Assert.True(buffer1.Equals(buffer3));
            Assert.True(buffer2.Equals(buffer3));

            /* Length vs. No length */
            buffer1.Data(buf1, 0, 1);
            Assert.False(buffer1.Equals(buffer2));
            Assert.True(!buffer1.Equals(buffer3));
            Assert.True(!buffer2.Equals(buffer1));
            Assert.True(!buffer3.Equals(buffer1));

            /* Test vs. teST vs. teST */
            buffer1.Data(buf1, 0, 4);
            buffer2.Data(buf2, 0, 4);
            buffer3.Data(buf3, 0, 4);

            Assert.True(buffer1.Equals(buffer1));
            Assert.True(buffer2.Equals(buffer2));
            Assert.True(buffer3.Equals(buffer3));

            Assert.True(!buffer1.Equals(buffer2));
            Assert.True(!buffer1.Equals(buffer3));
            Assert.True(buffer2.Equals(buffer3));

            /* Test\0 vs. teST\0 vs. teSTe */
            buffer1.Data(buf1, 0, 5);
            buffer2.Data(buf2, 0, 5);
            buffer3.Data(buf3, 0, 5);

            Assert.True(buffer1.Equals(buffer1));
            Assert.True(buffer2.Equals(buffer2));
            Assert.True(buffer3.Equals(buffer3));

            Assert.True(!buffer1.Equals(buffer2));
            Assert.True(!buffer2.Equals(buffer1));
            Assert.True(!buffer1.Equals(buffer3));
            Assert.True(!buffer2.Equals(buffer3));
            Assert.True(!buffer3.Equals(buffer1));
            Assert.True(!buffer3.Equals(buffer2));

            /* Test\0g vs. teST\0m vs. teSTer */
            buffer1.Data(buf1, 0, 6);
            buffer2.Data(buf2, 0, 6);
            buffer3.Data(buf3, 0, 6);

            Assert.True(buffer1.Equals(buffer1));
            Assert.True(buffer2.Equals(buffer2));
            Assert.True(buffer3.Equals(buffer3));

            Assert.True(!buffer1.Equals(buffer2));
            Assert.True(!buffer2.Equals(buffer1));
            Assert.True(!buffer1.Equals(buffer3));
            Assert.True(!buffer2.Equals(buffer3));
            Assert.True(!buffer3.Equals(buffer1));
            Assert.True(!buffer3.Equals(buffer2));

        }

        [Fact]
        [Category("Unit")]
        public void DoubleEncodeDecodeBlankTest()
        {
            Double doublev = new Double();

            Assert.False(doublev.IsBlank);
            doublev.Blank();
            Assert.True(doublev.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, doublev.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.UINT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Double double1 = new Double();
            Assert.False(double1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, double1.Decode(_decIter));

            Assert.True(double1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void DoubleSetTest()
        {
            Double thisDouble = new Double();

            thisDouble.Value(111222.333444);
            Assert.Equal(111222.333444, thisDouble.ToDouble(), 0);
        }

        [Fact]
        [Category("Unit")]
        public void DoubleStringTest()
        {
            Double testDouble = new Double();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testDouble.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, testDouble.Value("    "));
            Assert.True(testDouble.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testDouble.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, testDouble.Value(""));
            Assert.True(testDouble.IsBlank);

            Assert.Equal(CodecReturnCode.SUCCESS, testDouble.Value("11.22"));
            Assert.Equal("11.22", testDouble.ToString());
            Assert.False(testDouble.IsBlank);
        }

        [Fact(Skip = "Take 3 minute to run")]
        public void FloatEncDecTest()
        {
            Double ii, jj;
            ii = new Double();
            jj = new Double();

            Float putVal32 = new Float();
            Double putVal = new Double();

            for (ii.Value(-50); ii.ToDouble() <= 50; ii.Value(ii.ToDouble() + 1))
            {
                for (jj.Value(1.0); jj.ToDouble() < 10.0; jj.Value(jj.ToDouble() + 0.0001111111))
                {
                    // positive
                    putVal32.Value((float)(jj.ToDouble() * System.Math.Pow(10, ii.ToDouble())));
                    FloatED(putVal32);

                    // negative
                    putVal32.Value((float)(-jj.ToDouble() * System.Math.Pow(10, ii.ToDouble())));
                    FloatED(putVal32);
                }
            }

            // test effective exponent range
            for (ii.Value(-308); ii.ToDouble() <= 307; ii.Value(ii.ToDouble() + 1))
            {
                for (jj.Value(1.0); jj.ToDouble() < 10.0; jj.Value(jj.ToDouble() + 0.0001111111))
                {
                    // positive
                    putVal.Value(jj.ToDouble() * System.Math.Pow(10, ii.ToDouble()));
                    DoubleED(putVal);

                    // negative
                    putVal.Value(-jj.ToDouble() * System.Math.Pow(10, ii.ToDouble()));
                    DoubleED(putVal);
                }
            }
        }

        [Fact]
        [Category("Unit")]
        public void FloatEncodeDecodeBlankTest()
        {
            Float floatv = new Float();

            Assert.False(floatv.IsBlank);
            floatv.Blank();
            Assert.True(floatv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, floatv.Encode(_encIter));

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.UINT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Float float1 = new Float();
            Assert.False(float1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, float1.Decode(_decIter));

            Assert.True(float1.IsBlank);
        }

        [Fact]
        [Category("Unit")]
        public void floatSetTest()
        {
            Float thisFloat = new Float();

            thisFloat.Value((float)111222.333444);
            Assert.Equal((float)111222.333444, thisFloat.ToFloat(), 0);
        }

        [Fact]
        [Category("Unit")]
        public void floatStringTest()
        {
            Float testFloat = new Float();

            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testFloat.Value(null));
            Assert.Equal(CodecReturnCode.SUCCESS, testFloat.Value("    "));
            Assert.True(testFloat.IsBlank);
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, testFloat.Value("sfbgksj"));
            Assert.Equal(CodecReturnCode.SUCCESS, testFloat.Value(""));
            Assert.True(testFloat.IsBlank);

            Assert.Equal(CodecReturnCode.SUCCESS, testFloat.Value("11.22"));
            Assert.Equal("11.22", testFloat.ToString());
            Assert.False(testFloat.IsBlank);
        }


        private void UIntED(UInt unint)
        {
            _encIter.Clear();
            _buffer.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            unint.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            UInt uint1 = new UInt();
            uint1.Decode(_decIter);

            Assert.Equal(unint.ToLong(), uint1.ToLong());
            Assert.Equal(unint.ToULong(), uint1.ToULong());
        }
        private void IntED(Int intv)
        {
            _encIter.Clear();
            _buffer.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            intv.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Int int1 = new Int();
            int1.Decode(_decIter);

            Assert.Equal<long>(intv.ToLong(), int1.ToLong());
        }

        private void DateED(Date date)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            date.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Date date1 = new Date();
            date1.Decode(_decIter);

            Assert.Equal(date.Day(), date1.Day());
            Assert.Equal(date.Month(), date1.Month());
            Assert.Equal(date.Year(), date1.Year());
        }

        private void timeED(Time time)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            time.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Time time1 = new Time();
            time1.Decode(_decIter);

            Assert.Equal(time.Hour(), time1.Hour());
            Assert.Equal(time.Minute(), time1.Minute());
            Assert.Equal(time.Second(), time1.Second());
            Assert.Equal(time.Millisecond(), time1.Millisecond());
            Assert.Equal(time.Microsecond(), time1.Microsecond());
            Assert.Equal(time.Nanosecond(), time1.Nanosecond());
        }

        private void dateTimeED(DateTime dateTime)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecReturnCode ret = dateTime.Encode(_encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            DateTime dateTime1 = new DateTime();
            ret = dateTime1.Decode(_decIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(dateTime.Day(), dateTime1.Day());
            Assert.Equal(dateTime.Month(), dateTime1.Month());
            Assert.Equal(dateTime.Year(), dateTime1.Year());
            Assert.Equal(dateTime.Hour(), dateTime1.Hour());
            Assert.Equal(dateTime.Minute(), dateTime1.Minute());
            Assert.Equal(dateTime.Second(), dateTime1.Second());
            Assert.Equal(dateTime.Millisecond(), dateTime1.Millisecond());
            Assert.Equal(dateTime.Microsecond(), dateTime1.Microsecond());
            Assert.Equal(dateTime.Nanosecond(), dateTime1.Nanosecond());
        }

        private void addToDate(Date date, int year, int month, int day)
        {
            // This function does NOT take into account adding very large numbers like if you want to increment day by 360 
            //  It also assumes every month has ONLY 27 days

            int tmpDay = date.Day();
            if (27 < tmpDay + day)
            {
                date.Day(tmpDay + day - 27);
                month++;
            }
            else
                date.Day(tmpDay + day);


            int tmpMonth = date.Month();
            if (12 < tmpMonth + month)
            {
                date.Month(tmpMonth + month - 12);
                year++;
            }
            else
                date.Month(tmpMonth + month);

            date.Year(date.Year() + year);


        }
        private void addToTime(Time time, int Hour, int min, int sec, int mSec, int usec, int nsec)
        {
            // This function does NOT take into account adding numbers greater than 59 to the to the minutes or Seconds,
            // 999 to the mSec, or 23 to the hours
            //   


            //  add Nanosecond
            int temp_nsec = time.Nanosecond();
            if (999 <= temp_nsec + nsec)
            {
                time.Nanosecond(temp_nsec + nsec - 999);
                usec++;
            }
            else
                time.Nanosecond(temp_nsec + nsec);

            //  add Microsecond
            int temp_usec = time.Microsecond();
            if (999 <= temp_usec + usec)
            {
                time.Microsecond(temp_usec + usec - 999);
                mSec++;
            }
            else
                time.Microsecond(temp_usec + usec);

            //  add milliseconds
            int temp_mSec = time.Millisecond();
            if (999 <= temp_mSec + mSec)
            {
                time.Millisecond(temp_mSec + mSec - 999);
                sec++;
            }
            else
                time.Millisecond(temp_mSec + mSec);


            //  add seconds
            int temp_sec = time.Second();
            if (59 <= temp_sec + sec)
            {
                time.Second(temp_sec + sec - 59);
                min++;
            }
            else
                time.Second(temp_sec + sec);

            //  add minutes
            int temp_min = time.Minute();
            if (59 <= temp_min + min)
            {
                time.Minute(temp_min + min - 59);
                Hour++;
            }
            else
                time.Minute(temp_min + min);

            //  add hours
            int temp_hour = time.Hour();
            if (23 < temp_hour + Hour)
            {
                time.Hour(temp_hour + Hour - 23);
            }
            else
                time.Hour(temp_hour + Hour);
        }

        private void RealED(Real real)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            real.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Real real1 = new Real();
            real1.Decode(_decIter);

            Assert.Equal(real.Hint, real1.Hint);
            Assert.Equal(real.ToLong(), real1.ToLong());
            Assert.True(real.Equals(real1));
        }

        private void QosED(Qos qos)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            qos.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Qos qos1 = new Qos();
            qos1.Decode(_decIter);

            Assert.Equal(qos.Rate(), qos1.Rate());
            Assert.Equal(qos.RateInfo(), qos1.RateInfo());
            Assert.Equal(qos.TimeInfo(), qos1.TimeInfo());
            Assert.Equal(qos.Timeliness(), qos1.Timeliness());
            Assert.Equal(qos.IsDynamic, qos1.IsDynamic);
        }

        private void StateED(State state)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            state.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            State state1 = new State();
            state1.Decode(_decIter);

            Assert.Equal(state.Code(), state1.Code());
            Assert.Equal(state.DataState(), state1.DataState());
            Assert.Equal(state.StreamState(), state1.StreamState());
            Assert.Equal(state.Text(), state1.Text());
        }

        private void BufferED(Buffer buf)
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            buf.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Buffer buf1 = new Buffer();
            buf1.Data(new ByteBuffer(15));
            buf1.Decode(_decIter);

            Assert.Equal(buf.ToString(), buf1.ToString());
            Assert.True(buf.Equals(buf1));
        }

        private ByteBuffer PutString(string s)
        {
            ByteBuffer tmp = new ByteBuffer(127);
            tmp.Put(Encoding.ASCII.GetBytes(s));
            tmp.Flip();

            return tmp;
        }

        private void FloatED(Float putVal)
        {
            Float getVal = new Float();

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            putVal.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, getVal.Decode(_decIter));

            Assert.True(putVal.Equals(getVal));
        }

        private void DoubleED(Double putVal)
        {
            Double getVal = new Double();

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            putVal.Encode(_encIter);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.MajorVersion(), Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, getVal.Decode(_decIter));

            Assert.True(putVal.Equals(getVal));
        }

    }
}
