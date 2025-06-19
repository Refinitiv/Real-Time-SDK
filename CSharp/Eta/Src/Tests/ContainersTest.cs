/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Xunit;
using Xunit.Categories;

namespace LSEG.Eta.Transports.Tests
{
    [Category("ByteBuffer")]
    public class ContainersTest
    {
        EncodeIterator _encIter = new EncodeIterator();
        DecodeIterator _decIter = new DecodeIterator();
        Buffer _buffer = new Buffer();

        void testBufferArray(Buffer[] testBuffers, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */
            for (int type = DataTypes.BUFFER; type <= DataTypes.RMTES_STRING; type++)
            {
                _encIter.Clear();
                _buffer.Data(new ByteBuffer(100));
                _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                Array array = new Array();
                ArrayEntry ae = new ArrayEntry();
                array.ItemLength = itemLength;
                array.PrimitiveType = type;
                array.EncodeInit(_encIter);
                int i;
                for (i = 0; i < count; i++)
                {
                    if (!blank)
                        ae.Encode(_encIter, testBuffers[i]);
                    else
                        ae.EncodeBlank(_encIter);
                }
                array.EncodeComplete(_encIter, true);

                _decIter.Clear();
                _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                Array array1 = new Array();
                array1.Decode(_decIter);
                Assert.Equal(type, array.PrimitiveType);
                Assert.Equal(itemLength, array.ItemLength);

                CodecReturnCode ret = 0;
                i = 0;
                Buffer tmpBuffer = new Buffer();
                while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        tmpBuffer.Clear();
                        ret = tmpBuffer.Decode(_decIter);
                        if (ret == CodecReturnCode.SUCCESS)
                        {
                            Assert.Equal(testBuffers[i].ToString(), tmpBuffer.ToString());
                        }
                        else if (ret == CodecReturnCode.BLANK_DATA)
                        {
                            Assert.True(tmpBuffer.IsBlank);
                            if (!blank)
                                Assert.True(testBuffers[i].IsBlank);
                        }
                        if (blank)
                            Assert.Equal(CodecReturnCode.BLANK_DATA,ret);
                    }
                    i++;
                }
                Assert.Equal(count, i);
            }
        }

        void testEnumArray(Enum[] testEnums, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength = itemLength;
            array.PrimitiveType = DataTypes.ENUM;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                    ae.Encode(_encIter, testEnums[i]);
                else
                    ae.EncodeBlank(_encIter);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.ENUM, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Enum tmpEnum = new Enum();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    tmpEnum.Clear();
                    ret = tmpEnum.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testEnums[i].ToInt(), tmpEnum.ToInt());
                        Assert.Equal(testEnums[i].ToString(), tmpEnum.ToString());
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpEnum.IsBlank);
                        if (!blank)
                            Assert.True(testEnums[i].IsBlank);
                    }
                    if (blank)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testStateArray(State[] testStates, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.STATE;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                    ae.Encode(_encIter, testStates[i]);
                else
                    ae.EncodeBlank(_encIter);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.STATE, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            State tmpState = new State();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    tmpState.Clear();
                    ret = tmpState.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testStates[i].Code(), tmpState.Code());
                        Assert.Equal(testStates[i].DataState(), tmpState.DataState());
                        Assert.Equal(testStates[i].StreamState(), tmpState.StreamState());
                        Assert.True(testStates[i].Text().Equals(tmpState.Text()));
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpState.IsBlank);
                        if (!blank)
                            Assert.True(testStates[i].IsBlank);
                    }
                    if (blank)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testQosArray(Qos[] testQoss, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.QOS;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                    ae.Encode(_encIter, testQoss[i]);
                else
                    ae.EncodeBlank(_encIter);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.QOS, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Qos tmpQos = new Qos();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpQos.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testQoss[i].Rate(), tmpQos.Rate());
                        Assert.Equal(testQoss[i].RateInfo(), tmpQos.RateInfo());
                        Assert.Equal(testQoss[i].Timeliness(), tmpQos.Timeliness());
                        Assert.Equal(testQoss[i].TimeInfo(), tmpQos.TimeInfo());
                        Assert.Equal(testQoss[i].IsDynamic, tmpQos.IsDynamic);
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpQos.IsBlank);
                        if (!blank)
                            Assert.True(testQoss[i].IsBlank);
                    }
                    if (blank)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testDateArray(Date[] testDates, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.DATE;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                    ae.Encode(_encIter, testDates[i]);
                else
                    ae.EncodeBlank(_encIter);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.DATE, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Date tmpDate = new Date();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpDate.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testDates[i].Year(), tmpDate.Year());
                        Assert.Equal(testDates[i].Month(), tmpDate.Month());
                        Assert.Equal(testDates[i].Day(), tmpDate.Day());
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpDate.IsBlank);
                        if (!blank)
                            Assert.True(testDates[i].IsBlank);
                    }
                    if (blank)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testTimeArray(Time[] testTimes, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.TIME;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                    ae.Encode(_encIter, testTimes[i]);
                else
                    ae.EncodeBlank(_encIter);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.TIME, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Time tmpTime = new Time();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpTime.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testTimes[i].Hour(), tmpTime.Hour());
                        Assert.Equal(testTimes[i].Minute(), tmpTime.Minute());
                        Assert.Equal(testTimes[i].Second(), tmpTime.Second());
                        Assert.Equal(testTimes[i].Millisecond(), tmpTime.Millisecond());
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpTime.IsBlank);
                        if (!blank)
                            Assert.True(testTimes[i].IsBlank);
                    }
                    if (blank)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testDateTimeArray(DateTime[] testDateTimes, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.DATETIME;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                    ae.Encode(_encIter, testDateTimes[i]);
                else
                    ae.EncodeBlank(_encIter);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.DATETIME, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            DateTime tmpDateTime = new DateTime();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpDateTime.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testDateTimes[i].Year(), tmpDateTime.Year());
                        Assert.Equal(testDateTimes[i].Month(), tmpDateTime.Month());
                        Assert.Equal(testDateTimes[i].Day(), tmpDateTime.Day());
                        Assert.Equal(testDateTimes[i].Hour(), tmpDateTime.Hour());
                        Assert.Equal(testDateTimes[i].Minute(), tmpDateTime.Minute());
                        Assert.Equal(testDateTimes[i].Second(), tmpDateTime.Second());
                        Assert.Equal(testDateTimes[i].Millisecond(), tmpDateTime.Millisecond());
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpDateTime.IsBlank);
                        if (!blank)
                            Assert.True(testDateTimes[i].IsBlank);
                    }
                    if (blank)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testRealArray(Real[] testReals, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.REAL;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                {
                    ae.Encode(_encIter, testReals[i]);
                }
                else
                {
                    ae.EncodeBlank(_encIter);
                }
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.REAL, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Real tmpReal = new Real();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpReal.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.Equal(testReals[i].ToLong(), tmpReal.ToLong());
                        Assert.Equal(testReals[i].Hint, tmpReal.Hint);
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpReal.IsBlank);
                        if (!blank)
                            Assert.True(testReals[i].IsBlank);
                    }
                    if (blank)
                    {
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                    }
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testFloatArray(Float[] testFloats, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.FLOAT;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                {
                    ae.Encode(_encIter, testFloats[i]);
                }
                else
                {
                    ae.EncodeBlank(_encIter);
                }
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.FLOAT, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Float tmpFloat = new Float();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpFloat.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.True(testFloats[i].ToFloat() == tmpFloat.ToFloat());
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpFloat.IsBlank);
                        if (!blank)
                            Assert.True(testFloats[i].IsBlank);
                    }
                    if (blank)
                    {
                        Assert.Equal(CodecReturnCode.BLANK_DATA,ret);
                    }
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ingored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void testDoubleArray(Double[] testDoubles, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.DOUBLE;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                {
                    ae.Encode(_encIter, testDoubles[i]);
                }
                else
                {
                    ae.EncodeBlank(_encIter);
                }
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.DOUBLE, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Double tmpDouble = new Double();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = tmpDouble.Decode(_decIter);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        Assert.True(testDoubles[i].ToDouble() == tmpDouble.ToDouble());
                    }
                    else if (ret == CodecReturnCode.BLANK_DATA)
                    {
                        Assert.True(tmpDouble.IsBlank);
                        if (!blank)
                            Assert.True(testDoubles[i].IsBlank);
                    }
                    if (blank)
                    {
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                    }
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    // ignored
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void TestUIntArray(long[] testUInts, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.UINT;
            array.EncodeInit(_encIter);
            UInt data = new UInt();
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                {
                    data.Clear();
                    data.Value(testUInts[i]);
                    ae.Encode(_encIter, data);
                }
                else
                {
                    if (testUInts[i] == 0)
                        ae.EncodeBlank(_encIter);
                    else
                    {
                        data.Clear();
                        data.Value(testUInts[i]);
                        ae.Encode(_encIter, data);
                    }
                }
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.UINT, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            UInt tmpUInt = new UInt();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS,ret);

                ret = tmpUInt.Decode(_decIter);
                if (!blank)
                {
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(testUInts[i], tmpUInt.ToLong());
                }
                else
                {
                    if (testUInts[i] == 0)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                    else
                    {
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);
                        Assert.Equal(testUInts[i], tmpUInt.ToLong());
                    }
                }
                i++;
            }
            Assert.Equal(count, i);
        }

        void TestIntArray(long[] testInts, int count, int itemLength, bool blank)
        {
            /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength =itemLength;
            array.PrimitiveType =DataTypes.INT;
            array.EncodeInit(_encIter);
            Int data = new Int();
            int i;
            for (i = 0; i < count; i++)
            {
                if (!blank)
                {
                    data.Clear();
                    data.Value(testInts[i]);
                    ae.Encode(_encIter, data);
                }
                else
                {
                    // When the blank option on this test is true,
                    // zero values in the testInts[] will be encoded
                    // as blank, while non-zero values will be encoded
                    // as non-blank data. This allows mixing blank
                    // and non-blank in a test, depending on the contents
                    // of testInts[].
                    if (testInts[i] == 0)
                        ae.EncodeBlank(_encIter);
                    else
                    {
                        data.Clear();
                        data.Value(testInts[i]);
                        ae.Encode(_encIter, data);
                    }
                }
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.INT, array.PrimitiveType);
            Assert.Equal(itemLength, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Int tmpInt = new Int();
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS,ret);

                ret = tmpInt.Decode(_decIter);
                if (!blank)
                {
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(testInts[i], tmpInt.ToLong());
                }
                else
                {
                    // If test using blank option: expect blank entry
                    // if the testInts[] value was zero. Else expect non-blank.
                    if (testInts[i] == 0)
                        Assert.Equal(CodecReturnCode.BLANK_DATA, ret);
                    else
                    {
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);
                        Assert.Equal(testInts[i], tmpInt.ToLong());
                    }
                }

                i++;
            }
            Assert.Equal(count, i);
        }

        [Fact]
        [Category("Unit")]
        public void ArrayUnitTests()
        {
            /* Tests encoding arrays of different types and itemLengths, ensuring that it succeeds or fails as appropriate */

            /* Indexed by encoded size(e.g. testInts[4] is a 4-byte value) */
            long[] testInts = { 0x0, 0x7f, 0x7fff, 0x7fffff, 0x7fffffff, 0x7fffffffffL, 0x7fffffffffffL, 0x7fffffffffffffL, 0x7fffffffffffffffL };
            TestIntArray(testInts, 9, 8, false);
            TestIntArray(testInts, 9, 0, false);
            TestIntArray(testInts, 9, 0, true /* blank */);
            TestIntArray(testInts, 3, 2, false);
            TestIntArray(testInts, 5, 4, false);

            // when blank == true, test will encode 0x0 Ints as blank
            long[] testInts2 = { 0x0, 0x0, 0x7fff, 0x7fffff, 0x0, 0x7fffffffffL, 0x0, 0x7fffffffffffffL, 0x0 };
            TestIntArray(testInts2, 9, 8, false);
            TestIntArray(testInts2, 9, 0, false);
            TestIntArray(testInts2, 9, 0, true /* blank */);
            TestIntArray(testInts2, 3, 2, false);
            TestIntArray(testInts2, 5, 4, false);

            long[] testUInts = { 0x0, 0xff, 0xffff, 0xffffff, 0xffffffffL, 0xffffffffffL, 0xffffffffffffL, 0xffffffffffffffL};
            TestUIntArray(testUInts, 8, 8, false);
            TestUIntArray(testUInts, 8, 0, false);
            TestUIntArray(testUInts, 8, 0, true /* blank */);
            TestUIntArray(testUInts, 3, 2, false);
            TestUIntArray(testUInts, 5, 4, false);

            long[] testUInts2 = { 0x0, 0x0, 0xffff, 0xffffff, 0x0, 0xffffffffffL, 0x0, 0xffffffffffffffL, 0x0 };
            TestUIntArray(testUInts2, 9, 0, false);
            TestUIntArray(testUInts2, 9, 0, true);
            TestUIntArray(testUInts2, 9, 8, false);
            TestUIntArray(testUInts2, 3, 2, false);
            TestUIntArray(testUInts2, 5, 4, false);

            Real[] testReals = new Real[testInts.Length];
            int hint = RealHints.EXPONENT5;
            for (int i = 0; i < testInts.Length; i++)
            {
                testReals[i] = new Real();
                testReals[i].Value(testInts[i], hint);
            }
            testRealArray(testReals, 9, 0, false);
            testRealArray(testReals, 9, 0, true);

            int[][] testDT = new int[3][]; // test for 3 entries

            for(int i =0; i < 3; i++ )
            {
                testDT[i] = new int[7];
            }

		    DateTime[] testDateTime = new DateTime[3];
            Date[] testDate = new Date[3];
            Time[] testTime = new Time[3];

		    for (int i=0; i<3; i++)
		    {
			    testDT[i][0] = 1990 + 2*i;	// year
			    testDT[i][1] = 1 + 3*i;		// month
			    testDT[i][2] = 7*i+2;			// day
			    testDT[i][3] = 11 - 2*i;	// hour
			    testDT[i][4] = 60 - 10*i;	// minute
			    testDT[i][5] = 60 - 11*i;	// second
			    testDT[i][6] = 1000 - 60*i;	// milisecond

			    testDateTime[i] = new DateTime();
			    testDateTime[i].Year(testDT[i][0]);
                testDateTime[i].Month(testDT[i][1]);
                testDateTime[i].Day(testDT[i][2]);
                testDateTime[i].Hour(testDT[i][3]);
                testDateTime[i].Minute(testDT[i][4]);
                testDateTime[i].Second(testDT[i][5]);
                testDateTime[i].Millisecond(testDT[i][6]);

                testDate[i] = new Date();
			    testDate[i].Year(testDT[i][0]);
                testDate[i].Month(testDT[i][1]);
                testDate[i].Day(testDT[i][2]);

                testTime[i] = new Time();
			    testTime[i].Hour(testDT[i][3]);
                testTime[i].Minute(testDT[i][4]);
                testTime[i].Second(testDT[i][5]);
                testTime[i].Millisecond(testDT[i][6]);
             }

            testDateArray(testDate, 3, 0, false);

            testDateArray(testDate, 3, 0, true);

            testDateArray(testDate, 3, 4, false);

            testTimeArray(testTime, 3, 0, false);

            testTimeArray(testTime, 3, 0, true);

            testTimeArray(testTime, 1, 3, false);

            testTimeArray(testTime, 3, 5, false);

            testDateTimeArray(testDateTime, 3, 0, false);
            testDateTimeArray(testDateTime, 3, 0, true);
            testDateTimeArray(testDateTime, 1, 7, false);
            testDateTimeArray(testDateTime, 3, 9, false);

            Qos testQos3 = new Qos();
            testQos3.IsDynamic = false;
		    testQos3.Rate(QosRates.TICK_BY_TICK);
		    testQos3.Timeliness(QosTimeliness.REALTIME);

		    Qos testQos1 = new Qos();
            testQos1.IsDynamic = true;
		    testQos1.Rate(QosRates.TICK_BY_TICK);
		    testQos1.Timeliness(QosTimeliness.DELAYED);
		    testQos1.TimeInfo(567);

		    Qos testQos2 = new Qos();
            testQos2.IsDynamic = false;
		    testQos2.Rate(QosRates.TIME_CONFLATED);
		    testQos2.RateInfo(897);
		    testQos2.Timeliness(QosTimeliness.REALTIME);

		    Qos[] testQos = { testQos2, testQos1, testQos3 };

            testQosArray(testQos, 3, 0, false);

            testQosArray(testQos, 3, 0, true);

            State testState1 = new State();
            testState1.Code(StateCodes.INVALID_VIEW);
		    testState1.DataState(DataStates.NO_CHANGE);
		    testState1.StreamState(StreamStates.OPEN);
		    Buffer stateText = new Buffer();
            stateText.Data("Illinois");
		    testState1.Text(stateText);

		    State testState2 = new State();
            testState2.Code(StateCodes.NONE);
		    testState2.DataState(DataStates.NO_CHANGE);
		    testState2.StreamState(StreamStates.OPEN);

		    State testState3 = new State();
            testState3.Code(StateCodes.INVALID_ARGUMENT);
		    testState3.DataState(DataStates.NO_CHANGE);
		    testState3.StreamState(StreamStates.NON_STREAMING);
		    Buffer stateText3 = new Buffer();
            stateText3.Data("bla bla");
		    testState3.Text(stateText3);

		    State[] testState = { testState1, testState2, testState3 };

            testStateArray(testState, 3, 0, false);

            testStateArray(testState, 3, 0, true);


            Enum[] testEnum = new Enum[3];
		    for (int i =0; i<3; i++)
		    {
			    testEnum[i] = new Enum();
			    testEnum[i].Value((ushort) testInts[i]);
            }

            testEnumArray(testEnum, 3, 0, false);

            testEnumArray(testEnum, 3, 0, true);

            testEnumArray(testEnum, 2, 1, false);
            testEnumArray(testEnum, 3, 2, false);

            Buffer testBuffer1 = new Buffer();
            ByteBuffer bb = new ByteBuffer(7);
            byte[] bts = { 1, 2, 0, 10, 57, 0x7F, 4 };
            bb.Put(bts);
		    testBuffer1.Data(bb,0,7);

		    Buffer testBuffer2 = new Buffer();
            bb = new ByteBuffer(5);
		    byte[] bts1 = { 27, 48, 0, 10, 57 };
            bb.Put(bts1);
		    testBuffer2.Data(bb,0,5);

		    Buffer testBuffer3 = new Buffer();
            bb = new ByteBuffer(4);
		    byte[] bts2 = { 10, 57, 0x7F, 4 };
            bb.Put(bts2);
		    testBuffer3.Data(bb,0,4);

		    Buffer[] testBuffer = { testBuffer1, testBuffer2, testBuffer3 };

            testBufferArray(testBuffer, 3, 0, false);

            testBufferArray(testBuffer, 3, 0, true);

    // FLOAT
            Float[] testFloats = new Float[10];
		    for (int i=0; i< 10; i++)
		    {
			    testFloats[i] = new Float();
			    testFloats[i].Value((float)3.3 * i);
		    }

            testFloatArray(testFloats, 10, 0, false);

            testFloatArray(testFloats, 10, 0, true);

            testFloatArray(testFloats, 10, 4, false);

    // DOUBLE
            Double[] testDoubles = new Double[10];
		    for (int i=0; i< 10; i++)
		    {
			    testDoubles[i] = new Double();
			    testDoubles[i].Value((double)4.321 * i);
		    }

            testDoubleArray(testDoubles, 10, 0, false);

            testDoubleArray(testDoubles, 10, 0, true);

            testDoubleArray(testDoubles, 10, 8, false);
	    }

        [Fact]
        public void ArrayEncodeDecodeBlankTest()
        {
            Array arrayv = new Array();

            Assert.False(arrayv.IsBlank);
            arrayv.Blank();
            Assert.True(arrayv.IsBlank);

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(15));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            FieldList list = new FieldList();
            list.ApplyHasStandardData();
            list.EncodeInit(_encIter, null, 0);
            FieldEntry entry = new FieldEntry();
            entry.FieldId = 1;
            entry.DataType = DataTypes.INT;
            entry.EncodeBlank(_encIter);
            list.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            list.Decode(_decIter, null);
            entry.Decode(_decIter);
            Array array1 = new Array();
            Assert.False(array1.IsBlank);
            Assert.Equal(CodecReturnCode.BLANK_DATA, array1.Decode(_decIter));

            Assert.True(array1.IsBlank);
        }

        [Fact]
        public void ArrayBufferEntryTest()
        {
            Buffer buffer1 = new Buffer();
            ByteBuffer bb = new ByteBuffer(7);
            byte[] bts = { 41, 42, 40, 50, 57, 32, 33 };
            bb.Put(bts);
            buffer1.Data(bb, 0, 7);

            Buffer buffer2 = new Buffer();
            bb = new ByteBuffer(6);
            byte[] bts1 = { 37, 48, 40, 50, 57, 32 };
            bb.Put(bts1);
            buffer2.Data(bb, 0, 6);

            Buffer buffer3 = new Buffer();
            bb = new ByteBuffer(4);
            byte[] bts2 = { 50, 57, 48, 34 };
            bb.Put(bts2);
            buffer3.Data(bb, 0, 4);

            Buffer buffer4 = new Buffer();
            bb = new ByteBuffer(4);
            byte[] bts3 = { 10, 57, 0x7F, 4 };
            bb.Put(bts3);
            buffer4.Data(bb, 0, 0);

            Buffer[] buffers = { buffer1, buffer2, buffer3, buffer4 };

            _encIter.Clear();
            _buffer.Data(new ByteBuffer(100));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength = 6;
            array.PrimitiveType = DataTypes.BUFFER;
            array.EncodeInit(_encIter);
            int i;
            for (i = 0; i < 4; i++)
            {
                ae.Encode(_encIter, buffers[i]);
            }
            array.EncodeComplete(_encIter, true);

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array1 = new Array();
            array1.Decode(_decIter);
            Assert.Equal(DataTypes.BUFFER, array.PrimitiveType);
            Assert.Equal(6, array.ItemLength);

            CodecReturnCode ret = 0;
            i = 0;
            Buffer tmpBuffer = new Buffer();
            ae.Clear();
            byte[] bytes;
            byte[] testBytes;
            while ((ret = ae.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCode.SUCCESS)
                {
                    tmpBuffer.Clear();
                    ret = tmpBuffer.Decode(_decIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(6, tmpBuffer.Length);
                    bytes = tmpBuffer.Data().Contents;
                    testBytes = buffers[i].Data().Contents;
                    if (array.ItemLength <= buffers[i].Length)
                    {
                        for (int j = 0; j < array.ItemLength; j++)
                        {
                            Assert.Equal(testBytes[j], bytes[tmpBuffer.Position + j]);
                        }
                    }
                    else
                    {
                        for (int j = 0; j < buffers[i].Length; j++)
                        {
                            Assert.Equal(testBytes[j], bytes[tmpBuffer.Position + j]);
                        }
                        for (int j = buffers[i].Length; j < array.ItemLength; j++)
                        {
                            Assert.Equal(0x00, bytes[tmpBuffer.Position + j]);
                        }
                    }
                }
                else if (ret == CodecReturnCode.BLANK_DATA)
                {
                    //
                }
                i++;
            }
            Assert.Equal(4, i);

        }

        private void DecodeElementList(int elements, int fi)
        {
            ElementListFlags[] flags = { ElementListFlags.HAS_STANDARD_DATA, ElementListFlags.HAS_ELEMENT_LIST_INFO | ElementListFlags.HAS_STANDARD_DATA };
            // These arrays for fids and dataTypes make up our "Dictionary"

            ElementList decElementList = new ElementList();
            ElementEntry decEntry = new ElementEntry();
            Buffer[] names = {new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer()
                };
            names[0].Data("INT");
            names[1].Data("UINT");
            names[2].Data("REAL");
            names[3].Data("DATE");
            names[4].Data("TIME");

            int[] dataTypes =
                {
                DataTypes.INT,
                DataTypes.UINT,
                DataTypes.REAL,
                DataTypes.DATE,
                DataTypes.TIME
            };

            Int decInt = new Int();
            UInt decUInt = new UInt();
            Real decReal = new Real();
            Date decDate = new Date();
            Time decTime = new Time();
            DateTime decDateTime = new DateTime();
            Array decArray = new Array();
            ArrayEntry decArrayEntry = new ArrayEntry();

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            decElementList.Decode(_decIter, null);
            Assert.Equal(decElementList.Flags, flags[fi]);
            if (flags[fi] == ElementListFlags.HAS_ELEMENT_LIST_INFO)
                Assert.Equal(256, decElementList.SetId);
            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = decEntry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.True(decEntry.Name.Equals(names[eCount]));
                Assert.Equal(dataTypes[eCount], decEntry.DataType);
                switch (decEntry.DataType)
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decInt.Decode(_decIter));
                        Assert.Equal(-2049, decInt.ToLong());
                        break;
                    case DataTypes.UINT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decUInt.Decode(_decIter));
                        Assert.Equal(2049L, decUInt.ToLong());
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, decReal.Decode(_decIter));
                        Assert.Equal(0xFFFFF, decReal.ToLong());
                        Assert.Equal(1, decReal.Hint);
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDate.Decode(_decIter));
                        Assert.Equal(3, decDate.Day());
                        Assert.Equal(8, decDate.Month());
                        Assert.Equal(1892, decDate.Year());
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decTime.Decode(_decIter));
                        Assert.Equal(23, decTime.Hour());
                        Assert.Equal(59, decTime.Minute());
                        Assert.Equal(59, decTime.Second());
                        Assert.Equal(999, decTime.Millisecond());
                        break;
                    case DataTypes.DATETIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDateTime.Decode(_decIter));
                        Assert.Equal(3, decDateTime.Day());
                        Assert.Equal(8, decDateTime.Month());
                        Assert.Equal(1892, decDateTime.Year());
                        Assert.Equal(23, decDateTime.Hour());
                        Assert.Equal(59, decDateTime.Minute());
                        Assert.Equal(59, decDateTime.Second());
                        Assert.Equal(999, decDateTime.Millisecond());
                        break;
                    case DataTypes.ARRAY:
                        Assert.Equal(CodecReturnCode.SUCCESS, decArray.Decode(_decIter));
                        Assert.Equal(DataTypes.INT, decArray.PrimitiveType);
                        Assert.Equal(0, decArray.ItemLength);

                        int i = 0;
                        while ((ret = decArrayEntry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            Assert.Equal(CodecReturnCode.SUCCESS, ret);
                            ret = decInt.Decode(_decIter);
                            Assert.Equal(CodecReturnCode.SUCCESS, ret);
                            Assert.Equal(0xDEEEDEEE, decInt.ToLong());
                            i++;
                            decArrayEntry.Clear();
                        }
                        Assert.Equal(3, i);
                        break;
                }
                decEntry.Clear();
                eCount++;
            }
            Assert.Equal(elements, eCount);

        }

        [Fact]
        public void ElementListUnitTests()
        {
            ElementList container = new ElementList();
            ElementEntry entry = new ElementEntry();

            ElementListFlags[] flags = { ElementListFlags.HAS_STANDARD_DATA, ElementListFlags.HAS_ELEMENT_LIST_INFO | ElementListFlags.HAS_STANDARD_DATA };

            Buffer[] names = {new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer()
                };
            names[0].Data("INT");
            names[1].Data("UINT");
            names[2].Data("REAL");
            names[3].Data("DATE");
            names[4].Data("TIME");


            int[] dataTypes =
                {
                DataTypes.INT,
                DataTypes.UINT,
                DataTypes.REAL,
                DataTypes.DATE,
                DataTypes.TIME,
            };

            Int paylInt = new Int();
            paylInt.Value(-2049);

            UInt paylUInt = new UInt();
            paylUInt.Value(2049);

            Real paylReal = new Real();
            paylReal.Value(0xFFFFF, 1);

            Date paylDate = new Date();
            paylDate.Day(3);
            paylDate.Month(8);
            paylDate.Year(1892);

            Time paylTime = new Time();
            paylTime.Hour(23);
            paylTime.Minute(59);
            paylTime.Second(59);
            paylTime.Millisecond(999);

            DateTime paylDateTime = new DateTime();
            paylDateTime.Day(3);
            paylDateTime.Month(8);
            paylDateTime.Year(1892);
            paylDateTime.Hour(23);
            paylDateTime.Minute(59);
            paylDateTime.Second(59);
            paylDateTime.Millisecond(999);

            /* Pre-encode array */
            Buffer preencodedArray = new Buffer();
            preencodedArray.Data(new ByteBuffer(20));

            _encIter.Clear();
            _encIter.SetBufferAndRWFVersion(preencodedArray, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength = 0;
            array.PrimitiveType = DataTypes.INT;
            array.EncodeInit(_encIter);
            Int data = new Int();
            data.Value(0xDEEEDEEE);
            for (int i = 0; i < 3; i++)
            {
                ae.Encode(_encIter, data);
            }
            array.EncodeComplete(_encIter, true);

            for (int i = 0; i < dataTypes.Length; i++)
            {
                for (int k = 0; k < flags.Length; k++)
                {
                    _encIter.Clear();
                    _buffer.Data(new ByteBuffer(100));
                    _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    container.Flags = flags[k];
                    if ((flags[k] & ElementListFlags.HAS_ELEMENT_LIST_INFO) > 0)
                        container.SetId = 256;
                    container.EncodeInit(_encIter, null, 0);
                    for (int j = 0; j <= i; j++)
                    {
                        entry.Clear();
                        entry.Name = names[j];
                        entry.DataType = dataTypes[j];
                        switch (dataTypes[j])
                        {
                            case DataTypes.INT:
                                entry.Encode(_encIter, paylInt);
                                break;
                            case DataTypes.UINT:
                                entry.Encode(_encIter, paylUInt);
                                break;
                            case DataTypes.REAL:
                                entry.Encode(_encIter, paylReal);
                                break;
                            case DataTypes.DATE:
                                entry.Encode(_encIter, paylDate);
                                break;
                            case DataTypes.TIME:
                                entry.Encode(_encIter, paylTime);
                                break;
                            case DataTypes.DATETIME:
                                entry.Encode(_encIter, paylDateTime);
                                break;
                            case DataTypes.ARRAY:
                                // encode two entries with the same array but different way of encoding
                                entry.Encode(_encIter, preencodedArray);

                                entry.Clear();
                                entry.Name = names[j - 1];
                                entry.DataType = DataTypes.ARRAY;
                                entry.EncodeInit(_encIter, 0);
                                array.ItemLength = 0;
                                array.PrimitiveType = DataTypes.INT;
                                array.EncodeInit(_encIter);
                                for (int l = 0; l < 3; l++)
                                {
                                    ae.Encode(_encIter, data);
                                }
                                array.EncodeComplete(_encIter, true);
                                entry.EncodeComplete(_encIter, true);
                                break;
                        }
                    }
                    container.EncodeComplete(_encIter, true);

                    // decode encoded element list and compare
                    DecodeElementList(i + 1, k);
                }
            }
        }

        private void DecodeFieldList(int elements, int fi)
        {
            FieldListFlags[] flags = { FieldListFlags.HAS_STANDARD_DATA, FieldListFlags.HAS_FIELD_LIST_INFO | FieldListFlags.HAS_STANDARD_DATA };

            int[] fids = { 1, 2, 3, 4, 5 };

            FieldList decFieldList = new FieldList();
            FieldEntry decEntry = new FieldEntry();

            Int decInt = new Int();
            UInt decUInt = new UInt();
            Real decReal = new Real();
            Date decDate = new Date();
            Time decTime = new Time();
            DateTime decDateTime = new DateTime();
            Array decArray = new Array();
            ArrayEntry decArrayEntry = new ArrayEntry();

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            decFieldList.Decode(_decIter, null);
            Assert.Equal(decFieldList.Flags, flags[fi]);
            if ((flags[fi] & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
            {
                Assert.Equal(256, decFieldList.FieldListNum);
                Assert.Equal(257, decFieldList.DictionaryId);
            }
            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = decEntry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(fids[eCount], decEntry.FieldId);
                switch (decEntry.DataType)
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decInt.Decode(_decIter));
                        Assert.Equal(-2049, decInt.ToLong());
                        break;
                    case DataTypes.UINT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decUInt.Decode(_decIter));
                        Assert.Equal(2049L, decUInt.ToLong());
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, decReal.Decode(_decIter));
                        Assert.Equal(0xFFFFF, decReal.ToLong());
                        Assert.Equal(1, decReal.Hint);
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDate.Decode(_decIter));
                        Assert.Equal(3, decDate.Day());
                        Assert.Equal(8, decDate.Month());
                        Assert.Equal(1892, decDate.Year());
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decTime.Decode(_decIter));
                        Assert.Equal(23, decTime.Hour());
                        Assert.Equal(59, decTime.Minute());
                        Assert.Equal(59, decTime.Second());
                        Assert.Equal(999, decTime.Millisecond());
                        break;
                    case DataTypes.DATETIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDateTime.Decode(_decIter));
                        Assert.Equal(3, decDateTime.Day());
                        Assert.Equal(8, decDateTime.Month());
                        Assert.Equal(1892, decDateTime.Year());
                        Assert.Equal(23, decDateTime.Hour());
                        Assert.Equal(59, decDateTime.Minute());
                        Assert.Equal(59, decDateTime.Second());
                        Assert.Equal(999, decDateTime.Millisecond());
                        break;
                    case DataTypes.ARRAY:
                        Assert.Equal(CodecReturnCode.SUCCESS, decArray.Decode(_decIter));
                        Assert.Equal(DataTypes.INT, decArray.PrimitiveType);
                        Assert.Equal(0, decArray.ItemLength);

                        int i = 0;
                        while ((ret = decArrayEntry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            Assert.Equal(CodecReturnCode.SUCCESS, ret);
                            ret = decInt.Decode(_decIter);
                            Assert.Equal(CodecReturnCode.SUCCESS, ret);
                            Assert.Equal(0xDEEEDEEE, decInt.ToLong());
                            i++;
                            decArrayEntry.Clear();
                        }
                        Assert.Equal(3, i);
                        break;
                }
                decEntry.Clear();
                eCount++;
            }
            Assert.Equal(elements, eCount);

        }

        [Fact]
        public void FieldListUnitTests()
        {
            FieldList container = new FieldList();
            FieldEntry entry = new FieldEntry();

            FieldListFlags[] flags = { FieldListFlags.HAS_STANDARD_DATA, FieldListFlags.HAS_FIELD_LIST_INFO | FieldListFlags.HAS_STANDARD_DATA };


            int[] fids = { 1, 2, 3, 4, 5 };

            int[] dataTypes =
                {
                DataTypes.INT,
                DataTypes.UINT,
                DataTypes.REAL,
                DataTypes.DATE,
                DataTypes.TIME,
            };

            Int paylInt = new Int();
            paylInt.Value(-2049);

            UInt paylUInt = new UInt();
            paylUInt.Value(2049);

            Real paylReal = new Real();
            paylReal.Value(0xFFFFF, 1);

            Date paylDate = new Date();
            paylDate.Day(3);
            paylDate.Month(8);
            paylDate.Year(1892);

            Time paylTime = new Time();
            paylTime.Hour(23);
            paylTime.Minute(59);
            paylTime.Second(59);
            paylTime.Millisecond(999);

            DateTime paylDateTime = new DateTime();
            paylDateTime.Day(3);
            paylDateTime.Month(8);
            paylDateTime.Year(1892);
            paylDateTime.Hour(23);
            paylDateTime.Minute(59);
            paylDateTime.Second(59);
            paylDateTime.Millisecond(999);

            /* Pre-encode array */
            Buffer preencodedArray = new Buffer();
            preencodedArray.Data(new ByteBuffer(20));

            _encIter.Clear();
            _encIter.SetBufferAndRWFVersion(preencodedArray, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Array array = new Array();
            ArrayEntry ae = new ArrayEntry();
            array.ItemLength = 0;
            array.PrimitiveType = DataTypes.INT;
            array.EncodeInit(_encIter);
            Int data = new Int();
            data.Value(0xDEEEDEEE);
            for (int i = 0; i < 3; i++)
            {
                ae.Encode(_encIter, data);
            }
            array.EncodeComplete(_encIter, true);

            for (int i = 0; i < dataTypes.Length; i++)
            {
                for (int k = 0; k < flags.Length; k++)
                {
                    _encIter.Clear();
                    _buffer.Data(new ByteBuffer(100));
                    _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    container.Flags = flags[k];
                    if ((flags[k] & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
                    {
                        container.DictionaryId = 257;
                        container.FieldListNum = 256;
                    }
                    container.EncodeInit(_encIter, null, 0);
                    for (int j = 0; j <= i; j++)
                    {
                        entry.Clear();
                        entry.FieldId = fids[j];
                        entry.DataType = dataTypes[j];

                        switch (dataTypes[j])
                        {
                            case DataTypes.INT:
                                entry.Encode(_encIter, paylInt);
                                break;
                            case DataTypes.UINT:
                                entry.Encode(_encIter, paylUInt);
                                break;
                            case DataTypes.REAL:
                                entry.Encode(_encIter, paylReal);
                                break;
                            case DataTypes.DATE:
                                entry.Encode(_encIter, paylDate);
                                break;
                            case DataTypes.TIME:
                                entry.Encode(_encIter, paylTime);
                                break;
                            case DataTypes.DATETIME:
                                entry.Encode(_encIter, paylDateTime);
                                break;
                            case DataTypes.ARRAY:
                                // encode two entries with the same array but different way of encoding
                                entry.Encode(_encIter, preencodedArray);

                                entry.Clear();
                                entry.FieldId = fids[j - 1];
                                entry.DataType = DataTypes.ARRAY;
                                entry.EncodeInit(_encIter, 0);
                                array.ItemLength = 0;
                                array.PrimitiveType = DataTypes.INT;
                                array.EncodeInit(_encIter);
                                for (int l = 0; l < 3; l++)
                                {
                                    ae.Encode(_encIter, data);
                                }
                                array.EncodeComplete(_encIter, true);
                                entry.EncodeComplete(_encIter, true);
                                break;
                        }
                    }
                    container.EncodeComplete(_encIter, true);

                    // decode encoded element list and compare
                    DecodeFieldList(i + 1, k);
                }
            }
        }

        private void DecodeFieldList()
        {
            int[] fids = { 1, 2, 3, 4, 5 };

            FieldList decFieldList = new FieldList();
            FieldEntry decEntry = new FieldEntry();

            Int decInt = new Int();
            UInt decUInt = new UInt();
            Real decReal = new Real();
            Date decDate = new Date();
            Time decTime = new Time();

            decFieldList.Decode(_decIter, null);
            Assert.Equal(FieldListFlags.HAS_STANDARD_DATA, decFieldList.Flags);
            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = decEntry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(fids[eCount], decEntry.FieldId);
                switch (decEntry.DataType)
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decInt.Decode(_decIter));
                        Assert.Equal(-2049, decInt.ToLong());
                        break;
                    case DataTypes.UINT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decUInt.Decode(_decIter));
                        Assert.Equal(2049L, decUInt.ToLong());
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, decReal.Decode(_decIter));
                        Assert.Equal(0xFFFFF, decReal.ToLong());
                        Assert.Equal(1, decReal.Hint);
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDate.Decode(_decIter));
                        Assert.Equal(3, decDate.Day());
                        Assert.Equal(8, decDate.Month());
                        Assert.Equal(1892, decDate.Year());
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decTime.Decode(_decIter));
                        Assert.Equal(23, decTime.Hour());
                        Assert.Equal(59, decTime.Minute());
                        Assert.Equal(59, decTime.Second());
                        Assert.Equal(999, decTime.Millisecond());
                        break;
                }
                decEntry.Clear();
                eCount++;
            }
            Assert.Equal(1, eCount);

        }

        private void EncodeFieldList()
        {
            FieldList container = new FieldList();
            FieldEntry entry = new FieldEntry();

            int[] fids = { 1, 2, 3, 4, 5 };

            int[] dataTypes=
                {
                DataTypes.INT,
                DataTypes.UINT,
                DataTypes.REAL,
                DataTypes.DATE,
                DataTypes.TIME,
            };

            Int paylInt = new Int();
            paylInt.Value(-2049);

            UInt paylUInt = new UInt();
            paylUInt.Value(2049);

            Real paylReal = new Real();
            paylReal.Value(0xFFFFF, 1);

            Date paylDate = new Date();
            paylDate.Day(3);
            paylDate.Month(8);
            paylDate.Year(1892);

            Time paylTime = new Time();
            paylTime.Hour(23);
            paylTime.Minute(59);
            paylTime.Second(59);
            paylTime.Millisecond(999);

            container.Flags = FieldListFlags.HAS_STANDARD_DATA;
            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeInit(_encIter, null, 0));
            for (int j = 0; j < 1; j++)
            {
                entry.Clear();
                entry.FieldId = fids[j];
                entry.DataType = dataTypes[j];

                switch (dataTypes[j])
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylInt));
                        break;
                    case DataTypes.UINT:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylUInt));
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylReal));
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylDate));
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylTime));
                        break;
                }
            }
            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeComplete(_encIter, true));
        }

        private void DecodeMap(int ai, int fi, bool hasPermData)
        {
            Map container = new Map();
            MapEntry entry = new MapEntry();
            Buffer deckey = new Buffer();
            deckey.Data(new ByteBuffer(10));

            MapFlags[] flags = {MapFlags.HAS_SUMMARY_DATA,
                MapFlags.HAS_PER_ENTRY_PERM_DATA,
                MapFlags.HAS_TOTAL_COUNT_HINT,
                MapFlags.HAS_KEY_FIELD_ID
        };

            MapEntryActions[] entryActions = { MapEntryActions.ADD, MapEntryActions.DELETE, MapEntryActions.UPDATE };
            Buffer keyData = new Buffer();
            keyData.Data("keyData");
            Buffer permissionData = new Buffer();
            permissionData.Data("permission");
            Buffer summaryData = new Buffer();
            summaryData.Data("$");
            Buffer delName = new Buffer();
            delName.Data("Del");

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            container.Decode(_decIter);
            if ((fi == (int)MapFlags.HAS_SUMMARY_DATA) || (fi == (int)MapFlags.HAS_TOTAL_COUNT_HINT) ||
                    (hasPermData) || (fi == (int)MapFlags.HAS_KEY_FIELD_ID))
                Assert.Equal(flags[fi], container.Flags);
            Assert.Equal(DataTypes.FIELD_LIST, container.ContainerType);
            Assert.Equal(DataTypes.ASCII_STRING, container.KeyPrimitiveType);
            if ((flags[fi] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
            {
                Assert.Equal(5, container.TotalCountHint);
            }

            if ((flags[fi] & MapFlags.HAS_SUMMARY_DATA) > 0)
            {
                Assert.True(summaryData.Equals(container.EncodedSummaryData));
            }

            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = entry.Decode(_decIter, deckey)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                if (hasPermData)
                {
                    Assert.Equal(MapEntryFlags.HAS_PERM_DATA, entry.Flags);
                    Assert.True(permissionData.Equals(entry.PermData));
                }
                else
                {
                    Assert.Equal(MapEntryFlags.NONE, entry.Flags);
                }
                Assert.Equal(entryActions[ai], entry.Action);

                if (entry.Action == MapEntryActions.DELETE)
                {
                    Assert.True(delName.Equals(entry.EncodedKey));
                }
                else
                {
                    Assert.True(keyData.Equals(deckey));
                    //Assert.Equal(keyData, entry.encodedKey());
                    DecodeFieldList();
                }
                entry.Clear();
                eCount++;
            }
            Assert.Equal(fi * ai + 1, eCount);
        }

        [Fact]
        public void MapUnitTests()
        {
            Map container = new Map();
            MapEntry entry = new MapEntry();

            MapFlags[] flags = {MapFlags.HAS_SUMMARY_DATA,
                MapFlags.HAS_PER_ENTRY_PERM_DATA,
                MapFlags.HAS_TOTAL_COUNT_HINT,
                MapFlags.HAS_KEY_FIELD_ID
        };

            MapEntryActions[] entryActions = { MapEntryActions.ADD, MapEntryActions.DELETE, MapEntryActions.UPDATE };
            Buffer keyData = new Buffer();
            keyData.Data("keyData");
            Buffer permissionData = new Buffer();
            permissionData.Data("permission");
            Buffer summaryData = new Buffer();
            summaryData.Data("$");
            Buffer delName = new Buffer();
            delName.Data("Del");


            for (int i = 0; i < entryActions.Length; i++)
            {
                for (int k = 0; k < flags.Length; k++)
                {
                    // for each entryAction/mapFlag test with entry with and with no permission data
                    _encIter.Clear();
                    _buffer.Data(new ByteBuffer(500));
                    _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    container.Clear();
                    container.Flags = flags[k];
                    if ((flags[k] & MapFlags.HAS_SUMMARY_DATA) > 0)
                    {
                        container.EncodedSummaryData = summaryData;
                    }
                    if ((flags[k] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
                    {
                        container.TotalCountHint = 5;
                    }
                    container.ContainerType = DataTypes.FIELD_LIST;
                    container.KeyPrimitiveType = DataTypes.ASCII_STRING;

                    container.EncodeInit(_encIter, 0, 0);

                    // encode entries
                    for (int j = 0; j <= k * i; j++)
                    {
                        entry.Clear();
                        entry.Flags = MapEntryFlags.NONE;
                        entry.Action = entryActions[i];
                        if (entry.Action == MapEntryActions.DELETE)
                        {
                            entry.Encode(_encIter, delName);
                        }
                        else
                        {
                            entry.EncodeInit(_encIter, keyData, 0);
                            EncodeFieldList();
                            entry.EncodeComplete(_encIter, true);
                        }
                    }
                    container.EncodeComplete(_encIter, true);

                    // decode encoded element list and compare
                    DecodeMap(i, k, false);
                    container.Clear();

                    if ((flags[k] & MapFlags.HAS_PER_ENTRY_PERM_DATA) > 0)
                    {
                        // for each entryAction/mapFlag test with entry with and with no permission data
                        _encIter.Clear();
                        _buffer.Data(new ByteBuffer(500));
                        _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        container.Flags = flags[k];
                        if ((flags[k] & MapFlags.HAS_SUMMARY_DATA) > 0)
                        {
                            container.EncodedSummaryData = summaryData;
                        }
                        if ((flags[k] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
                        {
                            container.TotalCountHint = 5;
                        }
                        container.ContainerType = DataTypes.FIELD_LIST;
                        container.KeyPrimitiveType = DataTypes.ASCII_STRING;

                        container.EncodeInit(_encIter, 0, 0);

                        // encode entries
                        for (int j = 0; j <= k * i; j++)
                        {
                            entry.Clear();
                            entry.Flags = MapEntryFlags.HAS_PERM_DATA;
                            entry.PermData = permissionData;
                            entry.Action = entryActions[i];
                            if (entry.Action == MapEntryActions.DELETE)
                            {
                                entry.Encode(_encIter, delName);
                            }
                            else
                            {
                                entry.EncodeInit(_encIter, keyData, 0);
                                EncodeFieldList();
                                entry.EncodeComplete(_encIter, true);
                            }
                        }
                        container.EncodeComplete(_encIter, true);

                        // decode encoded element list and compare
                        DecodeMap(i, k, true);
                        container.Clear();
                    }
                }
            }
        }


        private void DecodeFilterList(int ai, int fi, int ef, bool hasPermData)
        {
            FilterList container = new FilterList();
            FilterEntry entry = new FilterEntry();

            FilterListFlags[] flags = {FilterListFlags.HAS_PER_ENTRY_PERM_DATA,
                FilterListFlags.HAS_TOTAL_COUNT_HINT
        };

            FilterEntryActions[] entryActions = { FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE };
            FilterEntryFlags[] entryFlags = { FilterEntryFlags.NONE, FilterEntryFlags.HAS_CONTAINER_TYPE };
            Buffer permissionData = new Buffer();
            permissionData.Data("permission");

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            container.Decode(_decIter);
            if (( (FilterListFlags)fi == FilterListFlags.HAS_TOTAL_COUNT_HINT) || (hasPermData))
                Assert.Equal(flags[fi], container.Flags);
            Assert.Equal(DataTypes.FIELD_LIST, container.ContainerType);
            if ((flags[fi] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
            {
                Assert.Equal(5, container.TotalCountHint);
            }

            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = entry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                if (hasPermData)
                {
                    FilterEntryFlags f = FilterEntryFlags.HAS_PERM_DATA;
                    if (ef == 1)
                        f |= FilterEntryFlags.HAS_CONTAINER_TYPE;
                    Assert.Equal(f, entry.Flags);
                    Assert.True(permissionData.Equals(entry.PermData));
                }
                else
                {
                    Assert.Equal(entryFlags[ef], entry.Flags);
                }
                Assert.Equal(entryActions[ai], entry.Action);

                if (entry.Action != FilterEntryActions.CLEAR)
                {
                    DecodeFieldList();
                }
                entry.Clear();
                eCount++;
            }
            Assert.Equal((fi + 1) * (ai + 1) * (ef + 1), eCount);
        }

        [Fact]
        public void FilterListUnitTests()
        {
            FilterList container = new FilterList();
            FilterEntry entry = new FilterEntry();

            FilterListFlags[] flags = {FilterListFlags.HAS_PER_ENTRY_PERM_DATA,
                FilterListFlags.HAS_TOTAL_COUNT_HINT
        };

            FilterEntryActions[] entryActions = { FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE };
            FilterEntryFlags[] entryFlags = { FilterEntryFlags.NONE, FilterEntryFlags.HAS_CONTAINER_TYPE };
            Buffer permissionData = new Buffer();
            permissionData.Data("permission");

            for (int i = 0; i < entryActions.Length; i++)
            {
                for (int k = 0; k < flags.Length; k++)
                {
                    for (int l = 0; l < entryFlags.Length; l++)
                    {
                        // for each entryAction/flag test with entry with no permission data
                        _encIter.Clear();
                        _buffer.Data(new ByteBuffer(200));
                        _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        container.Clear();
                        container.Flags = flags[k];
                        container.ContainerType = DataTypes.FIELD_LIST;
                        if ((flags[k] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
                        {
                            container.TotalCountHint = 5;
                        }
                        container.ContainerType = DataTypes.FIELD_LIST;
                        Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeInit(_encIter));

                        // encode entries
                        for (int j = 0; j < (k + 1) * (i + 1) * (l + 1); j++)
                        {
                            entry.Clear();
                            entry.Flags = entryFlags[l];
                            entry.Action = entryActions[i];
                            entry.Id = k * i + 1;
                            if ((entryFlags[l] & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
                            {
                                entry.ContainerType = DataTypes.FIELD_LIST;
                            }
                            Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(_encIter, 0));
                            if (entry.Action != FilterEntryActions.CLEAR)
                            {
                                EncodeFieldList();
                            }
                            Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeComplete(_encIter, true));
                        }
                        Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeComplete(_encIter, true));

                        // decode encoded element list and compare
                        DecodeFilterList(i, k, l, false);
                        container.Clear();

                        // for each entryAction/flag test with entry with permission data
                        if (k == 0)
                        {
                            _encIter.Clear();
                            _buffer.Data(new ByteBuffer(200));
                            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                            container.Clear();
                            container.Flags = flags[k];
                            if ((flags[k] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
                            {
                                container.TotalCountHint = 5;
                            }
                            container.ContainerType = DataTypes.FIELD_LIST;
                            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeInit(_encIter));

                            // encode entries
                            for (int j = 0; j < (k + 1) * (i + 1) * (l + 1); j++)
                            {
                                entry.Clear();
                                entry.Flags = entryFlags[l] | FilterEntryFlags.HAS_PERM_DATA;
                                entry.PermData = permissionData;
                                entry.Action = entryActions[i];
                                entry.Id = k * i + 1;
                                if ((entryFlags[l] & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
                                {
                                    entry.ContainerType = DataTypes.FIELD_LIST;
                                }
                                Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(_encIter, 0));
                                if (entry.Action != FilterEntryActions.CLEAR)
                                {
                                    EncodeFieldList();
                                }
                                Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeComplete(_encIter, true));
                            }
                            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeComplete(_encIter, true));

                            // decode encoded element list and compare
                            DecodeFilterList(i, k, l, true);
                            container.Clear();
                        }
                    }
                }
            }
        }

        private void EncodeElementList()
        {
            ElementList container = new ElementList();
            ElementEntry entry = new ElementEntry();

            Buffer[] names = {new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer()
                };
            names[0].Data("INT");
            names[1].Data("UINT");
            names[2].Data("REAL");
            names[3].Data("DATE");
            names[4].Data("TIME");

            int[] dataTypes =
                {
                DataTypes.INT,
                DataTypes.UINT,
                DataTypes.REAL,
                DataTypes.DATE,
                DataTypes.TIME,
            };

            Int paylInt = new Int();
            paylInt.Value(-2049);

            UInt paylUInt = new UInt();
            paylUInt.Value(2049);

            Real paylReal = new Real();
            paylReal.Value(0xFFFFF, 1);

            Date paylDate = new Date();
            paylDate.Day(3);
            paylDate.Month(8);
            paylDate.Year(1892);

            Time paylTime = new Time();
            paylTime.Hour(23);
            paylTime.Minute(59);
            paylTime.Second(59);
            paylTime.Millisecond(999);

            container.Flags = ElementListFlags.HAS_STANDARD_DATA;
            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeInit(_encIter, null, 0));
            for (int j = 0; j < 1; j++)
            {
                entry.Clear();
                entry.Name = names[j];
                entry.DataType = dataTypes[j];

                switch (dataTypes[j])
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylInt));
                        break;
                    case DataTypes.UINT:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylUInt));
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylReal));
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylDate));
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylTime));
                        break;
                }
            }
            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeComplete(_encIter, true));
        }

        private void DecodeElementList()
        {
            ElementList decElementList = new ElementList();
            ElementEntry decEntry = new ElementEntry();

            Buffer[] names = {new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer(),
                new Buffer()
                };
            names[0].Data("INT");
            names[1].Data("UINT");
            names[2].Data("REAL");
            names[3].Data("DATE");
            names[4].Data("TIME");

            int[] dataTypes =
                {
                DataTypes.INT,
                DataTypes.UINT,
                DataTypes.REAL,
                DataTypes.DATE,
                DataTypes.TIME,
            };

            Int decInt = new Int();
            UInt decUInt = new UInt();
            Real decReal = new Real();
            Date decDate = new Date();
            Time decTime = new Time();

            Assert.Equal(CodecReturnCode.SUCCESS, decElementList.Decode(_decIter, null));

            Assert.Equal(ElementListFlags.HAS_STANDARD_DATA, decElementList.Flags);
            int eCount = 0;
            CodecReturnCode ret;

            while ((ret = decEntry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(names[eCount], decEntry.Name);
                Assert.Equal(decEntry.DataType, dataTypes[eCount]);

                switch (dataTypes[eCount])
                {
                    case DataTypes.INT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decInt.Decode(_decIter));
                        Assert.Equal(-2049, decInt.ToLong());
                        break;
                    case DataTypes.UINT:
                        Assert.Equal(CodecReturnCode.SUCCESS, decUInt.Decode(_decIter));
                        Assert.Equal(2049L, decUInt.ToLong());
                        break;
                    case DataTypes.REAL:
                        Assert.Equal(CodecReturnCode.SUCCESS, decReal.Decode(_decIter));
                        Assert.Equal(0xFFFFF, decReal.ToLong());
                        Assert.Equal(1, decReal.Hint);
                        break;
                    case DataTypes.DATE:
                        Assert.Equal(CodecReturnCode.SUCCESS, decDate.Decode(_decIter));
                        Assert.Equal(3, decDate.Day());
                        Assert.Equal(8, decDate.Month());
                        Assert.Equal(1892, decDate.Year());
                        break;
                    case DataTypes.TIME:
                        Assert.Equal(CodecReturnCode.SUCCESS, decTime.Decode(_decIter));
                        Assert.Equal(23, decTime.Hour());
                        Assert.Equal(59, decTime.Minute());
                        Assert.Equal(59, decTime.Second());
                        Assert.Equal(999, decTime.Millisecond());
                        break;
                }

                decEntry.Clear();
                eCount++;
            }
        }

        private void DecodeVector(int ai, int fi, bool hasPermData)
        {
            Vector container = new Vector();
            VectorEntry entry = new VectorEntry();

            VectorFlags[] flags = {VectorFlags.HAS_SUMMARY_DATA,
                VectorFlags.HAS_PER_ENTRY_PERM_DATA,
                VectorFlags.HAS_TOTAL_COUNT_HINT,
                VectorFlags.SUPPORTS_SORTING
            };

            VectorEntryActions[] entryActions = { VectorEntryActions.SET, VectorEntryActions.DELETE, VectorEntryActions.UPDATE, VectorEntryActions.CLEAR, VectorEntryActions.INSERT};
            Buffer permissionData = new Buffer();
            permissionData.Data("permission");
            Buffer summaryData = new Buffer();
            summaryData.Data("$");

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            container.Decode(_decIter);
            if ((fi == (int)VectorFlags.HAS_SUMMARY_DATA) || (fi == (int)VectorFlags.HAS_TOTAL_COUNT_HINT) || (fi == (int)VectorFlags.HAS_PER_ENTRY_PERM_DATA) ||
                    (hasPermData) )
                Assert.Equal(flags[fi], container.Flags);
            Assert.Equal(DataTypes.ELEMENT_LIST, container.ContainerType);
            if ((flags[fi] & VectorFlags.HAS_TOTAL_COUNT_HINT) > 0)
            {
                Assert.Equal(5, container.TotalCountHint);
            }

            if ((flags[fi] & VectorFlags.HAS_SUMMARY_DATA) > 0)
            {
                Assert.True(summaryData.Equals(container.EncodedSummaryData));
            }

            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = entry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                if (hasPermData)
                {
                    Assert.Equal(VectorEntryFlags.HAS_PERM_DATA, entry.Flags);
                    Assert.True(permissionData.Equals(entry.PermData));
                }
                else
                {
                    Assert.Equal(VectorEntryFlags.NONE, entry.Flags);
                }
                Assert.Equal(entryActions[ai], entry.Action);

                uint expectedIndex = (uint)(fi * ai);
                Assert.Equal<uint>(expectedIndex, entry.Index);
                if ( (entry.Action == VectorEntryActions.DELETE) ||
                    (entry.Action == VectorEntryActions.CLEAR) )
                {
                    // No data to decode
                }
                else
                {
                    DecodeElementList();
                }
                entry.Clear();
                eCount++;
            }
            Assert.Equal(fi * ai + 1, eCount);
        }

        private void DecodeSeries(int fi)
        {
            Series container = new Series();
            SeriesEntry entry = new SeriesEntry();

            SeriesFlags[] flags = {SeriesFlags.HAS_SUMMARY_DATA,
                SeriesFlags.HAS_TOTAL_COUNT_HINT
            };

            Buffer summaryData = new Buffer();
            summaryData.Data("$");

            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            container.Decode(_decIter);
            if ((fi == (int)SeriesFlags.HAS_SUMMARY_DATA) || (fi == (int)SeriesFlags.HAS_TOTAL_COUNT_HINT) )
                Assert.Equal(flags[fi], container.Flags);
            Assert.Equal(DataTypes.ELEMENT_LIST, container.ContainerType);
            if ((flags[fi] & SeriesFlags.HAS_TOTAL_COUNT_HINT) > 0)
            {
                Assert.Equal(5, container.TotalCountHint);
            }

            if ((flags[fi] & SeriesFlags.HAS_SUMMARY_DATA) > 0)
            {
                Assert.True(summaryData.Equals(container.EncodedSummaryData));
            }

            int eCount = 0;
            CodecReturnCode ret;
            while ((ret = entry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                DecodeElementList();

                entry.Clear();
                eCount++;
            }
            Assert.Equal(fi + 1, eCount);
        }

        [Fact]
        public void VectorUnitTests()
        {
            Vector container = new Vector();
            VectorEntry entry = new VectorEntry();

            VectorFlags[] flags = {VectorFlags.HAS_SUMMARY_DATA,
                VectorFlags.HAS_PER_ENTRY_PERM_DATA,
                VectorFlags.HAS_TOTAL_COUNT_HINT
            };

            VectorEntryActions[] entryActions = { VectorEntryActions.SET, VectorEntryActions.DELETE, VectorEntryActions.UPDATE, VectorEntryActions.CLEAR, VectorEntryActions.INSERT };
            Buffer keyData = new Buffer();
            keyData.Data("keyData");
            Buffer permissionData = new Buffer();
            permissionData.Data("permission");
            Buffer summaryData = new Buffer();
            summaryData.Data("$");

            for (int i = 0; i < entryActions.Length; i++)
            {
                for (int k = 0; k < flags.Length; k++)
                {
                    // for each entryAction/mapFlag test with entry with and with no permission data
                    _encIter.Clear();
                    _buffer.Data(new ByteBuffer(500));
                    _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    container.Clear();
                    container.Flags = flags[k];
                    if ((flags[k] & VectorFlags.HAS_SUMMARY_DATA) > 0)
                    {
                        container.EncodedSummaryData = summaryData;
                    }
                    if ((flags[k] & VectorFlags.HAS_TOTAL_COUNT_HINT) > 0)
                    {
                        container.TotalCountHint = 5;
                    }
                    container.ContainerType = DataTypes.ELEMENT_LIST;

                    container.EncodeInit(_encIter, 0, 0);

                    // encode entries
                    for (int j = 0; j <= (k * i); j++)
                    {
                        entry.Clear();
                        entry.Index = (uint)(i*k);
                        entry.Flags = VectorEntryFlags.NONE;
                        entry.Action = entryActions[i];
                        if (entry.Action == VectorEntryActions.DELETE || entry.Action == VectorEntryActions.CLEAR)
                        {
                            entry.Encode(_encIter);
                        }
                        else
                        {
                            entry.EncodeInit(_encIter, 0);
                            EncodeElementList();
                            entry.EncodeComplete(_encIter, true);
                        }
                    }
                    container.EncodeComplete(_encIter, true);

                    // decode encoded element list and compare
                    DecodeVector(i, k, false);
                    container.Clear();

                    if ((flags[k] & VectorFlags.HAS_PER_ENTRY_PERM_DATA) > 0)
                    {
                        // for each entryAction/mapFlag test with entry with and with no permission data
                        _encIter.Clear();
                        _buffer.Data(new ByteBuffer(500));
                        _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                        container.Flags = flags[k];
                        if ((flags[k] & VectorFlags.HAS_SUMMARY_DATA) > 0)
                        {
                            container.EncodedSummaryData = summaryData;
                        }
                        if ((flags[k] & VectorFlags.HAS_TOTAL_COUNT_HINT) > 0)
                        {
                            container.TotalCountHint = 5;
                        }
                        container.ContainerType = DataTypes.ELEMENT_LIST;

                        container.EncodeInit(_encIter, 0, 0);

                        // encode entries
                        for (int j = 0; j <= k * i; j++)
                        {
                            entry.Clear();
                            entry.Index = (uint)(i * k);
                            entry.Flags = VectorEntryFlags.HAS_PERM_DATA;
                            entry.PermData = permissionData;
                            entry.Action = entryActions[i];
                            if (entry.Action == VectorEntryActions.DELETE || entry.Action == VectorEntryActions.CLEAR)
                            {
                                entry.Encode(_encIter);
                            }
                            else
                            {
                                entry.EncodeInit(_encIter, 0);
                                EncodeElementList();
                                entry.EncodeComplete(_encIter, true);
                            }
                        }
                        container.EncodeComplete(_encIter, true);

                        // decode encoded element list and compare
                        DecodeVector(i, k, true);
                        container.Clear();
                    }
                }
            }
        }

        [Fact]
        public void SeriesUnitTests()
        {
            Series container = new Series();
            SeriesEntry entry = new SeriesEntry();

            SeriesFlags[] flags = {SeriesFlags.HAS_SUMMARY_DATA,
                SeriesFlags.HAS_TOTAL_COUNT_HINT
            };

            Buffer summaryData = new Buffer();
            summaryData.Data("$");

            for (int k = 0; k < flags.Length; k++)
            {
                // for each entryAction/mapFlag test with entry with and with no permission data
                _encIter.Clear();
                _buffer.Data(new ByteBuffer(500));
                _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                container.Clear();
                container.Flags = flags[k];
                if ((flags[k] & SeriesFlags.HAS_SUMMARY_DATA) > 0)
                {
                    container.EncodedSummaryData = summaryData;
                }
                if ((flags[k] & SeriesFlags.HAS_TOTAL_COUNT_HINT) > 0)
                {
                    container.TotalCountHint = 5;
                }
                container.ContainerType = DataTypes.ELEMENT_LIST;

                container.EncodeInit(_encIter, 0, 0);

                // encode entries
                for (int j = 0; j <= k; j++)
                {
                    entry.Clear();
                    entry.EncodeInit(_encIter, 0);
                    EncodeElementList();
                    entry.EncodeComplete(_encIter, true);
                }
                container.EncodeComplete(_encIter, true);

                // decode encoded element list and compare
                DecodeSeries(k);
                container.Clear();
            }
        }

        [Fact]
        public void BasicFieldListNestingTest()
        {
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(200));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            FieldList container = new FieldList();
            FieldList container1 = new FieldList();
            FieldEntry entry = new FieldEntry();
            FieldEntry entry1 = new FieldEntry();

            Real paylReal = new Real();
            paylReal.Value(97, 2);

            Date paylDate = new Date();
            paylDate.Day(21);
            paylDate.Month(10);
            paylDate.Year(1978);

            container.ApplyHasStandardData();
            container.SetId = 0;
            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeInit(_encIter, null, 0));

            entry.Clear();
            entry.DataType = DataTypes.DATE;
            entry.FieldId = 1;
            Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylDate));

            entry.Clear();
            entry.FieldId = 2;
            Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(_encIter, 100));

            container1.ApplyHasStandardData();
            Assert.Equal(CodecReturnCode.SUCCESS, container1.EncodeInit(_encIter, null, 0));
            entry1.Clear();
            entry1.FieldId = 3;
            entry1.DataType = DataTypes.REAL;
            Assert.Equal(CodecReturnCode.SUCCESS, entry1.Encode(_encIter, paylReal));
            Assert.Equal(CodecReturnCode.SUCCESS, container1.EncodeComplete(_encIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeComplete(_encIter, true));

            entry.Clear();
            entry.FieldId = 4;
            entry.DataType = DataTypes.DATE;
            Assert.Equal(CodecReturnCode.SUCCESS, entry.Encode(_encIter, paylDate));

            Assert.Equal(CodecReturnCode.SUCCESS, container.EncodeComplete(_encIter, true));

            // now decode
            _decIter.Clear();
            _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            // reuse the containers
            container.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, container.Decode(_decIter, null));
            Assert.Equal(0, container.SetId);
            Assert.Equal(FieldListFlags.HAS_STANDARD_DATA, container.Flags);

            CodecReturnCode ret;
            entry.Clear();
            entry1.Clear();
            while ((ret = entry.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if ((entry.FieldId == 1) || (entry.FieldId == 4))
                {
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    paylDate.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, paylDate.Decode(_decIter));
                    Assert.Equal(21, paylDate.Day());
                    Assert.Equal(10, paylDate.Month());
                    Assert.Equal(1978, paylDate.Year());
                }
                else if (entry.FieldId == 2)
                {
                    container1.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, container1.Decode(_decIter, null));

                    entry1.Clear();
                    while ((ret = entry1.Decode(_decIter)) != CodecReturnCode.END_OF_CONTAINER)
                    {
                        Assert.Equal(CodecReturnCode.SUCCESS, ret);
                        if (entry1.FieldId == 3)
                        {
                            paylReal.Clear();
                            Assert.Equal(CodecReturnCode.SUCCESS, paylReal.Decode(_decIter));
                            Assert.Equal(97, paylReal.ToLong());
                            Assert.Equal(2, paylReal.Hint);
                        }
                    }
                }
                entry.Clear();
            }
        }

        private void EncodeNestedStructure(int type, int nested)
        {
            FieldList fieldList = new FieldList();
            FieldEntry fieldEntry = new FieldEntry();
            Map map = new Map();
            MapEntry mapEntry = new MapEntry();
            FilterList filterList = new FilterList();
            FilterEntry filterEntry = new FilterEntry();
            UInt key = new UInt();
            key.Value(nested);

            if (nested == 0)
            {
                EncodeFieldList();
            }
            else
            {
                switch (type)
                {
                    case DataTypes.FIELD_LIST:
                        fieldList.ApplyHasStandardData();
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(_encIter, null, 0));
                        fieldEntry.Clear();
                        fieldEntry.FieldId = 2;
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeInit(_encIter, 100));
                        EncodeNestedStructure(DataTypes.FIELD_LIST, nested - 1);
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeComplete(_encIter, true));
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(_encIter, true));
                        break;
                    case DataTypes.MAP:
                        map.ContainerType = DataTypes.MAP;
                        if (nested == 1)
                            map.ContainerType = DataTypes.FIELD_LIST;
                        map.KeyPrimitiveType = DataTypes.UINT;
                        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(_encIter, 0, 0));
                        mapEntry.Clear();
                        mapEntry.Action = MapEntryActions.ADD;
                        key.Value(nested * 2);
                        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(_encIter, key, 100));
                        EncodeNestedStructure(DataTypes.MAP, nested - 1);
                        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeComplete(_encIter, true));
                        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(_encIter, true));
                        break;
                    case DataTypes.FILTER_LIST:
                        filterList.ContainerType = DataTypes.MAP;
                        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(_encIter));
                        filterEntry.Clear();
                        filterEntry.ApplyHasContainerType();
                        if (nested == 1)
                            filterEntry.ContainerType = DataTypes.FIELD_LIST;
                        else
                            filterEntry.ContainerType = DataTypes.FILTER_LIST;
                        filterEntry.Action = FilterEntryActions.SET;
                        filterEntry.Id = nested + 1;
                        Assert.Equal(CodecReturnCode.SUCCESS, filterEntry.EncodeInit(_encIter, 100));
                        EncodeNestedStructure(DataTypes.FILTER_LIST, nested - 1);
                        Assert.Equal(CodecReturnCode.SUCCESS, filterEntry.EncodeComplete(_encIter, true));
                        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(_encIter, true));
                        break;
                    default:
                        throw new System.NotImplementedException("Not implement data tpe: " + type);
                }
            }

        }

        private void DecodeNestedStructure(int type, int nested)
        {
            FieldList fieldList = new FieldList();
            FieldEntry fieldEntry = new FieldEntry();
            Map map = new Map();
            MapEntry mapEntry = new MapEntry();
            FilterList filterList = new FilterList();
            FilterEntry filterEntry = new FilterEntry();
            UInt key = new UInt();
            key.Value((uint)nested);

            if (nested == 0)
            {
                DecodeFieldList();
            }
            else
            {
                switch (type)
                {
                    case DataTypes.FIELD_LIST:
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.Decode(_decIter, null));
                        Assert.True(fieldList.CheckHasStandardData());
                        fieldEntry.Clear();
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(_decIter));
                        Assert.Equal(2, fieldEntry.FieldId);
                        DecodeNestedStructure(DataTypes.FIELD_LIST, nested - 1);
                        break;
                    case DataTypes.MAP:
                        Assert.Equal(CodecReturnCode.SUCCESS, map.Decode(_decIter));
                        if (nested == 1)
                            Assert.Equal(DataTypes.FIELD_LIST, map.ContainerType);
                        else
                            Assert.Equal(DataTypes.MAP, map.ContainerType);
                        Assert.Equal(DataTypes.UINT, map.KeyPrimitiveType);
                        mapEntry.Clear();
                        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Decode(_decIter, key));
                        Assert.Equal(nested * 2, key.ToLong());
                        DecodeNestedStructure(DataTypes.MAP, nested - 1);
                        break;
                    case DataTypes.FILTER_LIST:
                        Assert.Equal(CodecReturnCode.SUCCESS, filterList.Decode(_decIter));
                        Assert.Equal(FilterListFlags.NONE, filterList.Flags);
                        filterEntry.Clear();
                        Assert.Equal(DataTypes.MAP, filterList.ContainerType);
                        Assert.Equal(CodecReturnCode.SUCCESS, filterEntry.Decode(_decIter));
                        if (nested == 1)
                            Assert.Equal(DataTypes.FIELD_LIST, filterEntry.ContainerType);
                        else
                            Assert.Equal(DataTypes.FILTER_LIST, filterEntry.ContainerType);
                        Assert.Equal(nested + 1, filterEntry.Id);
                        DecodeNestedStructure(DataTypes.FILTER_LIST, nested - 1);
                        break;
                    default:
                        throw new System.NotImplementedException("Not implemented data type : " + type);
                }
            }

        }

        [Fact]
        public void NestedEncDecTest()
        {
            int nested = 5;
            _encIter.Clear();
            _buffer.Data(new ByteBuffer(200));
            _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            for (int i = 0; i < nested; i++)
            {
                _encIter.Clear();
                _buffer.Data(new ByteBuffer(200));
                _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                EncodeNestedStructure(DataTypes.FIELD_LIST, nested);
                // now decode
                _decIter.Clear();
                _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                DecodeNestedStructure(DataTypes.FIELD_LIST, nested);

                _encIter.Clear();
                _buffer.Data(new ByteBuffer(200));
                _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                EncodeNestedStructure(DataTypes.MAP, nested);
                // now decode
                _decIter.Clear();
                _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                DecodeNestedStructure(DataTypes.MAP, nested);

                _encIter.Clear();
                _buffer.Data(new ByteBuffer(200));
                _encIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                EncodeNestedStructure(DataTypes.FILTER_LIST, nested);
                // now decode
                _decIter.Clear();
                _decIter.SetBufferAndRWFVersion(_buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                DecodeNestedStructure(DataTypes.FILTER_LIST, nested);

            }
        }

        [Fact]
        [Category("Unit")]
        public void EntrySkippingTest()
        {
            ByteBuffer bufBig = new ByteBuffer(16000000);
            Buffer tbufBig = new Buffer();
            Map map = new Map(); MapEntry mapEntry = new MapEntry();
            FieldList fieldList = new FieldList(); FieldEntry fieldEntry = new FieldEntry();
            int iMapEntries, iFields, iSkipField;

            /* Some values to use - total 4 mapEntries with 4 fields in each entry. */
            const int totalMapEntries = 4;
            const int totalFields = 4;
            Buffer[] mapKeyStrings = new Buffer[4];
            mapKeyStrings[0] = new Buffer();
            mapKeyStrings[0].Data("GROUCHO");
            mapKeyStrings[1] = new Buffer();
            mapKeyStrings[1].Data("ZEPPO");
            mapKeyStrings[2] = new Buffer();
            mapKeyStrings[2].Data("HARPO");
            mapKeyStrings[3] = new Buffer();
            mapKeyStrings[3].Data("GUMMO");
            int[] fids = { 22, 26, 30, 31 };
            Real[] reals = new Real[4];
            reals[0] = new Real();
            reals[0].Value(3194, RealHints.EXPONENT_2);
            reals[1] = new Real();
            reals[1].Value(3196, RealHints.EXPONENT_2);
            reals[2] = new Real();
            reals[2].Value(4, RealHints.EXPONENT_2);
            reals[3] = new Real();
            reals[3].Value(5, RealHints.EXPONENT_2);

            tbufBig.Data(bufBig);

            _encIter.Clear();
            _encIter.SetBufferAndRWFVersion(tbufBig, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            /* Encode a map with a field list in each entry */

            /* Map */
            map.Clear();
            map.Flags = MapFlags.NONE;
            map.KeyPrimitiveType = DataTypes.ASCII_STRING;
            map.ContainerType = DataTypes.FIELD_LIST;
            Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(_encIter, 0, 0));

            /* MapEntry */
            for (iMapEntries = 0; iMapEntries < totalMapEntries; ++iMapEntries)
            {
                mapEntry.Clear();
                mapEntry.Flags = MapEntryFlags.NONE;
                mapEntry.Action = MapEntryActions.ADD;
                Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(_encIter, mapKeyStrings[iMapEntries], 0));

                /* FieldList */
                fieldList.Clear();
                fieldList.Flags = FieldListFlags.HAS_STANDARD_DATA;
                Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(_encIter, null, 0));
                for (iFields = 0; iFields < totalFields; ++iFields)
                {
                    fieldEntry.Clear();
                    fieldEntry.FieldId = fids[iFields]; fieldEntry.DataType = DataTypes.REAL;
                    Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(_encIter, reals[iFields]));
                }
                Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(_encIter, true));

                Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeComplete(_encIter, true));
            }

            Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(_encIter, true));

            Buffer decodedBuffer = new Buffer();

            /* Decode all map entries, skipping at various points */
            for (iSkipField = 0; iSkipField <= totalFields /* Includes decoding all fields before 'skipping' */; ++iSkipField)
            {
                decodedBuffer.Clear();
                decodedBuffer.Data(new ByteBuffer(16000000));
                tbufBig.Copy(decodedBuffer);
                Real real = new Real();
                Buffer mapKey = new Buffer(); ;
                _decIter.Clear();
                _decIter.SetBufferAndRWFVersion(decodedBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                /* Decode map */
                map.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, map.Decode(_decIter));
                Assert.Equal(MapFlags.NONE, map.Flags);
                Assert.Equal(DataTypes.ASCII_STRING, map.KeyPrimitiveType);

                for (iMapEntries = 0; iMapEntries < totalMapEntries; ++iMapEntries)
                {
                    /* Decode mapEntry */
                    mapEntry.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Decode(_decIter, mapKey));
                    Assert.True(mapKey.Equals(mapKeyStrings[iMapEntries]));

                    fieldList.Clear();
                    Assert.Equal(CodecReturnCode.SUCCESS, fieldList.Decode(_decIter, null));

                    for (iFields = 0; iFields < iSkipField; ++iFields)
                    {
                        /* Decode fieldEntry and check value. */
                        fieldEntry.Clear();
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(_decIter));
                        Assert.Equal(fids[iFields], fieldEntry.FieldId);
                        Assert.Equal(CodecReturnCode.SUCCESS, real.Decode(_decIter));

                        Assert.Equal(reals[iFields].Hint, real.Hint);
                        Assert.Equal(reals[iFields].IsBlank, real.IsBlank);
                        Assert.Equal(reals[iFields].ToLong(), real.ToLong());
                    }
                    Assert.Equal(CodecReturnCode.END_OF_CONTAINER, _decIter.FinishDecodeEntries());
                }
            }
        }

        LSEG.Eta.Codec.Buffer primitiveEntryOverrunTest_encodeBuffer = new();
        ByteBuffer primitiveEntryOverrunTest_byteBuffer = new(1024);

        /* Realign to a bytebuffer that has the specified space. */
        private void PrimitiveEntryOverrunTest_realignBuffer(EncodeIterator eIter, int newSize)
        {
            primitiveEntryOverrunTest_byteBuffer = new ByteBuffer(newSize);
            primitiveEntryOverrunTest_encodeBuffer.Data(primitiveEntryOverrunTest_byteBuffer);

            Assert.Equal(CodecReturnCode.SUCCESS, eIter.RealignBuffer(primitiveEntryOverrunTest_encodeBuffer));
        }

        [Fact]
        [Category("Unit")]
        public void SetDefinedAndStandardDataOverrunTest()
        {
            // Test overrun & rollback of encoding a FieldList & ElementList with the
            // different possible primitives, both as set-defined and standard data.
            //
            // Wrap the FieldList or ElementList in a series that also encodes the
            // relevant Field/Element SetDef, to test overrun & rollback of encoding them.
            //
            // When encoding fails, increase buffer size by 1, realign and try again.
            //
            // See ETA-2340. */

            // step I.1

            /* FieldList */

            /* Setup field set def db, with entries. */
            LocalFieldSetDefDb fsetDb = new();
            FieldSetDefEntry[] entries = new FieldSetDefEntry[34];

            entries[0] = new FieldSetDefEntry();
            entries[0].FieldId = 0;
            entries[0].DataType = DataTypes.INT_1;

            entries[1] = new FieldSetDefEntry();
            entries[1].FieldId = 1;
            entries[1].DataType = DataTypes.INT_2;

            entries[2] = new FieldSetDefEntry();
            entries[2].FieldId = 2;
            entries[2].DataType = DataTypes.INT_4;

            entries[3] = new FieldSetDefEntry();
            entries[3].FieldId = 3;
            entries[3].DataType = DataTypes.INT_8;

            entries[4] = new FieldSetDefEntry();
            entries[4].FieldId = 4;
            entries[4].DataType = DataTypes.UINT_1;

            entries[5] = new FieldSetDefEntry();
            entries[5].FieldId = 5;
            entries[5].DataType = DataTypes.UINT_2;

            entries[6] = new FieldSetDefEntry();
            entries[6].FieldId = 6;
            entries[6].DataType = DataTypes.UINT_4;

            entries[7] = new FieldSetDefEntry();
            entries[7].FieldId = 7;
            entries[7].DataType = DataTypes.UINT_8;

            entries[8] = new FieldSetDefEntry();
            entries[8].FieldId = 8;
            entries[8].DataType = DataTypes.FLOAT_4;

            entries[9] = new FieldSetDefEntry();
            entries[9].FieldId = 9;
            entries[9].DataType = DataTypes.DOUBLE_8;

            entries[10] = new FieldSetDefEntry();
            entries[10].FieldId = 10;
            entries[10].DataType = DataTypes.REAL_4RB;

            entries[11] = new FieldSetDefEntry();
            entries[11].FieldId = 11;
            entries[11].DataType = DataTypes.REAL_8RB;

            entries[12] = new FieldSetDefEntry();
            entries[12].FieldId = 12;
            entries[12].DataType = DataTypes.DATE_4;

            entries[13] = new FieldSetDefEntry();
            entries[13].FieldId = 13;
            entries[13].DataType = DataTypes.TIME_3;

            entries[14] = new FieldSetDefEntry();
            entries[14].FieldId = 14;
            entries[14].DataType = DataTypes.TIME_5;

            entries[15] = new FieldSetDefEntry();
            entries[15].FieldId = 15;
            entries[15].DataType = DataTypes.DATETIME_7;

            entries[16] = new FieldSetDefEntry();
            entries[16].FieldId = 16;
            entries[16].DataType = DataTypes.DATETIME_11;

            entries[17] = new FieldSetDefEntry();
            entries[17].FieldId = 17;
            entries[17].DataType = DataTypes.DATETIME_12;

            entries[18] = new FieldSetDefEntry();
            entries[18].FieldId = 18;
            entries[18].DataType = DataTypes.TIME_7;

            entries[19] = new FieldSetDefEntry();
            entries[19].FieldId = 19;
            entries[19].DataType = DataTypes.TIME_8;

            entries[20] = new FieldSetDefEntry();
            entries[20].FieldId = 20;
            entries[20].DataType = DataTypes.INT;

            entries[21] = new FieldSetDefEntry();
            entries[21].FieldId = 21;
            entries[21].DataType = DataTypes.UINT;

            entries[22] = new FieldSetDefEntry();
            entries[22].FieldId = 22;
            entries[22].DataType = DataTypes.FLOAT;

            entries[23] = new FieldSetDefEntry();
            entries[23].FieldId = 23;
            entries[23].DataType = DataTypes.DOUBLE;

            entries[24] = new FieldSetDefEntry();
            entries[24].FieldId = 24;
            entries[24].DataType = DataTypes.REAL;

            entries[25] = new FieldSetDefEntry();
            entries[25].FieldId = 25;
            entries[25].DataType = DataTypes.DATE;

            entries[26] = new FieldSetDefEntry();
            entries[26].FieldId = 26;
            entries[26].DataType = DataTypes.TIME;

            entries[27] = new FieldSetDefEntry();
            entries[27].FieldId = 27;
            entries[27].DataType = DataTypes.DATETIME;

            entries[28] = new FieldSetDefEntry();
            entries[28].FieldId = 28;
            entries[28].DataType = DataTypes.QOS;

            entries[29] = new FieldSetDefEntry();
            entries[29].FieldId = 29;
            entries[29].DataType = DataTypes.ENUM;

            entries[30] = new FieldSetDefEntry();
            entries[30].FieldId = 30;
            entries[30].DataType = DataTypes.BUFFER;

            entries[31] = new FieldSetDefEntry();
            entries[31].FieldId = 31;
            entries[31].DataType = DataTypes.ASCII_STRING;

            entries[32] = new FieldSetDefEntry();
            entries[32].FieldId = 32;
            entries[32].DataType = DataTypes.UTF8_STRING;

            entries[33] = new FieldSetDefEntry();
            entries[33].FieldId = 33;
            entries[33].DataType = DataTypes.RMTES_STRING;

            fsetDb.Definitions[0].SetId = 0;
            fsetDb.Definitions[0].Count = 34;
            fsetDb.Definitions[0].Entries = entries;

            EncodeIterator eIter = new();
            Series series = new();
            SeriesEntry seriesEntry = new();
            FieldList fieldList = new();
            FieldEntry fieldEntry = new();
            Int intVal = new();
            UInt uintVal = new();
            Float floatVal = new();
            Double doubleVal = new();
            Real realVal = new();
            Date dateVal = new();
            Time timeVal = new();
            DateTime dateTimeVal = new();
            Qos qosVal = new();
            Enum enumVal = new();
            Buffer bufferVal = new();

            /* Buffer size needed to encode (starting at series header) */
            int seriesHeaderEncodedSize = 1 /* Flags */ + 1 /* Container type */ + 2 /* SetDef size bytes */ + 2 /* Entry count */;
            int bufSizeNeeded = seriesHeaderEncodedSize;
            int encodeBufSize = bufSizeNeeded;

            primitiveEntryOverrunTest_byteBuffer = new ByteBuffer(encodeBufSize);
            primitiveEntryOverrunTest_encodeBuffer.Data(primitiveEntryOverrunTest_byteBuffer);
            CodecReturnCode codecReturnCode = eIter.SetBufferAndRWFVersion(primitiveEntryOverrunTest_encodeBuffer,
                Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            /* Encode Series. */
            series.Clear();
            series.ContainerType = DataTypes.FIELD_LIST;
            series.ApplyHasSetDefs();
            Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(eIter, 0, 0));

            /* Encode set definition. */
            bufSizeNeeded += 1 /* Flags */ + 1 /* SetDef Count */ + 1 /* Set ID */ + 1 /* Entry count */
                + 34 * 3 /* entries (fieldId, dataType) */;
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fsetDb.Encode(eIter));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(0, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fsetDb.Encode(eIter));
            Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeSetDefsComplete(eIter, true));

            /* Encode SeriesEntry. */
            bufSizeNeeded += 3 /* Size bytes */;
            encodeBufSize = bufSizeNeeded;
            PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            seriesEntry.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(eIter, 0));

            /* Encode field list. */
            bufSizeNeeded += 1 /* FieldList flags */ + 2 /* Set data length */ + 2 /* Standard data length */;
            encodeBufSize = bufSizeNeeded;
            PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);

            fieldList.Clear();
            fieldList.ApplyHasSetData();
            fieldList.ApplyHasStandardData();
            Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(eIter, fsetDb, 0));

            /* INT_1 needs 1 byte. */
            bufSizeNeeded += 1;
            fieldEntry.Clear();
            fieldEntry.FieldId = 0;
            fieldEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, intVal));

            /* INT_2 needs 2 bytes. */
            bufSizeNeeded += 2;
            fieldEntry.Clear();
            fieldEntry.FieldId = 1;
            fieldEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, intVal));

            /* INT_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            fieldEntry.Clear();
            fieldEntry.FieldId = 2;
            fieldEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, intVal));

            /* INT_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 3;
            fieldEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, intVal));

            /* UINT_1 needs 1 byte. */
            bufSizeNeeded += 1;
            fieldEntry.Clear();
            fieldEntry.FieldId = 4;
            fieldEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, uintVal));

            /* UINT_2 needs 2 bytes. */
            bufSizeNeeded += 2;
            fieldEntry.Clear();
            fieldEntry.FieldId = 5;
            fieldEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, uintVal));

            /* UINT_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            fieldEntry.Clear();
            fieldEntry.FieldId = 6;
            fieldEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, uintVal));

            /* UINT_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 7;
            fieldEntry.DataType = DataTypes.UINT;
            uintVal.Value(ulong.MaxValue);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, uintVal));

            /* FLOAT_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            fieldEntry.Clear();
            fieldEntry.FieldId = 8;
            fieldEntry.DataType = DataTypes.FLOAT;
            floatVal.Value(1.0f);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, floatVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, floatVal));

            /* DOUBLE_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 9;
            fieldEntry.DataType = DataTypes.DOUBLE;
            doubleVal.Value(1.0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, doubleVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, doubleVal));

            /* REAL_4RB needs 5 bytes for this value. */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 10;
            fieldEntry.DataType = DataTypes.REAL;
            realVal.Value(2147483647, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, realVal));

            /* REAL_8RB needs 9 bytes for this value. */
            bufSizeNeeded += 9;
            fieldEntry.Clear();
            fieldEntry.FieldId = 11;
            fieldEntry.DataType = DataTypes.REAL;
            realVal.Value(9223372036854775807L, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, realVal));

            /* DATE_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            fieldEntry.Clear();
            fieldEntry.FieldId = 12;
            fieldEntry.DataType = DataTypes.DATE;
            dateVal.Day(12);
            dateVal.Month(11);
            dateVal.Year(1955);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateVal));

            /* TIME_3 needs 3 bytes. */
            bufSizeNeeded += 3;
            fieldEntry.Clear();
            fieldEntry.FieldId = 13;
            fieldEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, timeVal));

            /* TIME_5 needs 5 bytes. */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 14;
            fieldEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, timeVal));

            /* DATETIME_7 needs 7 bytes. */
            bufSizeNeeded += 7;
            fieldEntry.Clear();
            fieldEntry.FieldId = 15;
            fieldEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateTimeVal));

            /* DATETIME_11 needs 11 bytes. */
            bufSizeNeeded += 11;
            fieldEntry.Clear();
            fieldEntry.FieldId = 16;
            fieldEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateTimeVal));

            /* DATETIME_12 needs 12 bytes. */
            bufSizeNeeded += 12;
            fieldEntry.Clear();
            fieldEntry.FieldId = 17;
            fieldEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            dateTimeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateTimeVal));

            /* TIME_7 needs 7 bytes. */
            bufSizeNeeded += 7;
            fieldEntry.Clear();
            fieldEntry.FieldId = 18;
            fieldEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, timeVal));

            /* TIME_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 19;
            fieldEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            timeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, timeVal));

            /* This INT needs 3 bytes. */
            bufSizeNeeded += 3;
            fieldEntry.Clear();
            fieldEntry.FieldId = 20;
            fieldEntry.DataType = DataTypes.INT;
            intVal.Value(255);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, intVal));

            /* This UINT needs 3 bytes. */
            bufSizeNeeded += 3;
            fieldEntry.Clear();
            fieldEntry.FieldId = 21;
            fieldEntry.DataType = DataTypes.UINT;
            uintVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, uintVal));

            /* This FLOAT needs 5 bytes. */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 22;
            fieldEntry.DataType = DataTypes.FLOAT;
            floatVal.Value(1.0f);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, floatVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, floatVal));

            /* This DOUBLE needs 9 bytes. */
            bufSizeNeeded += 9;
            fieldEntry.Clear();
            fieldEntry.FieldId = 23;
            fieldEntry.DataType = DataTypes.DOUBLE;
            doubleVal.Value(1.0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, doubleVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, doubleVal));

            /* This REAL needs 6 bytes. */
            bufSizeNeeded += 6;
            fieldEntry.Clear();
            fieldEntry.FieldId = 24;
            fieldEntry.DataType = DataTypes.REAL;
            realVal.Value(2147483647, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, realVal));

            /* This DATE needs 5 bytes. */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 25;
            fieldEntry.DataType = DataTypes.DATE;
            dateVal.Day(12);
            dateVal.Month(11);
            dateVal.Year(1955);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }

            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateVal));

            /* This TIME needs 9 bytes. */
            bufSizeNeeded += 9;
            fieldEntry.Clear();
            fieldEntry.FieldId = 26;
            fieldEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            timeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, timeVal));

            /* This DATETIME needs 13 bytes. */
            bufSizeNeeded += 13;
            fieldEntry.Clear();
            fieldEntry.FieldId = 27;
            fieldEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            dateTimeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateTimeVal));

            /* This QOS needs 6 bytes. */
            bufSizeNeeded += 6;
            fieldEntry.Clear();
            fieldEntry.FieldId = 28;
            fieldEntry.DataType = DataTypes.QOS;
            qosVal.Timeliness(QosTimeliness.DELAYED);
            qosVal.TimeInfo(3000);
            qosVal.Rate(QosRates.TIME_CONFLATED);
            qosVal.RateInfo(5000);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, qosVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, qosVal));

            /* This ENUM needs 3 bytes. */
            bufSizeNeeded += 3;
            fieldEntry.Clear();
            fieldEntry.FieldId = 29;
            fieldEntry.DataType = DataTypes.ENUM;
            enumVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, enumVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, enumVal));

            /* This BUFFER needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 30;
            fieldEntry.DataType = DataTypes.BUFFER;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This ASCII_STRING needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 31;
            fieldEntry.DataType = DataTypes.ASCII_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This UTF8_STRING needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 32;
            fieldEntry.DataType = DataTypes.UTF8_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This RMTES_STRING needs 8 bytes. */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 33;
            fieldEntry.DataType = DataTypes.RMTES_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SET_COMPLETE, fieldEntry.Encode(eIter, bufferVal));

            /* Standard data */

            /* This INT needs 5 bytes (2 for fieldId, 3 for value). */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 20;
            fieldEntry.DataType = DataTypes.INT;
            intVal.Value(255);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, intVal));

            /* This UINT needs 5 bytes (2 for fieldId, 3 for value). */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 21;
            fieldEntry.DataType = DataTypes.UINT;
            uintVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, uintVal));

            /* This UINT needs 7 bytes (2 for fieldId, 5 for value). */
            bufSizeNeeded += 7;
            fieldEntry.Clear();
            fieldEntry.FieldId = 22;
            fieldEntry.DataType = DataTypes.FLOAT;
            floatVal.Value(1.0f);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, floatVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, floatVal));

            /* This DOUBLE needs 11 bytes (2 for fieldId, 9 for value). */
            bufSizeNeeded += 11;
            fieldEntry.Clear();
            fieldEntry.FieldId = 23;
            fieldEntry.DataType = DataTypes.DOUBLE;
            doubleVal.Value(1.0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, doubleVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, doubleVal));

            /* This REAL needs 8 bytes (2 for fieldId, 6 for value). */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 24;
            fieldEntry.DataType = DataTypes.REAL;
            realVal.Value(2147483647, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, realVal));

            /* This DATE needs 7 bytes (2 for fieldId, 5 for value). */
            bufSizeNeeded += 7;
            fieldEntry.Clear();
            fieldEntry.FieldId = 25;
            fieldEntry.DataType = DataTypes.DATE;
            dateVal.Day(12);
            dateVal.Month(11);
            dateVal.Year(1955);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }

            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateVal));

            /* This TIME needs 11 bytes (2 for fieldId, 9 for value). */
            bufSizeNeeded += 11;
            fieldEntry.Clear();
            fieldEntry.FieldId = 26;
            fieldEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            timeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, timeVal));

            /* This DATETIME needs 13 bytes (2 for fieldId, 13 for value). */
            bufSizeNeeded += 15;
            fieldEntry.Clear();
            fieldEntry.FieldId = 27;
            fieldEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            dateTimeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, dateTimeVal));

            /* This QOS needs 8 bytes (2 for fieldId, 6 for value). */
            bufSizeNeeded += 8;
            fieldEntry.Clear();
            fieldEntry.FieldId = 28;
            fieldEntry.DataType = DataTypes.QOS;
            qosVal.Timeliness(QosTimeliness.DELAYED);
            qosVal.TimeInfo(3000);
            qosVal.Rate(QosRates.TIME_CONFLATED);
            qosVal.RateInfo(5000);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, qosVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, qosVal));

            /* This ENUM needs 5 bytes (2 for fieldId, 3 for value). */
            bufSizeNeeded += 5;
            fieldEntry.Clear();
            fieldEntry.FieldId = 29;
            fieldEntry.DataType = DataTypes.ENUM;
            enumVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, enumVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, enumVal));

            /* This BUFFER needs 10 bytes (2 for fieldId, 8 for value). */
            bufSizeNeeded += 10;
            fieldEntry.Clear();
            fieldEntry.FieldId = 30;
            fieldEntry.DataType = DataTypes.BUFFER;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This ASCII_STRING needs 10 bytes (2 for fieldId, 8 for value). */
            bufSizeNeeded += 10;
            fieldEntry.Clear();
            fieldEntry.FieldId = 31;
            fieldEntry.DataType = DataTypes.ASCII_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This UTF8_STRING needs 10 bytes (2 for fieldId, 8 for value). */
            bufSizeNeeded += 10;
            fieldEntry.Clear();
            fieldEntry.FieldId = 32;
            fieldEntry.DataType = DataTypes.UTF8_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This RMTES_STRING needs 10 bytes (2 for fieldId, 8 for value). */
            bufSizeNeeded += 10;
            fieldEntry.Clear();
            fieldEntry.FieldId = 33;
            fieldEntry.DataType = DataTypes.RMTES_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter, bufferVal));

            /* This RMTES_STRING needs 10 bytes (2 for fieldId, 8 for value). */
            /* Encode this one via encodedData. */
            bufSizeNeeded += 10;
            fieldEntry.Clear();
            fieldEntry.FieldId = 34;
            fieldEntry.DataType = DataTypes.RMTES_STRING;
            fieldEntry.EncodedData.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter));

            /* This RMTES_STRING will be blank and needs 3 bytes (2 for fieldId, 1 for value). */
            bufSizeNeeded += 3;
            fieldEntry.Clear();
            fieldEntry.FieldId = 35;
            fieldEntry.DataType = DataTypes.RMTES_STRING;

            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(eIter));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(eIter));

            Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(eIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeComplete(eIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeComplete(eIter, true));

            // step I.2 finished encoding, now start decoding

            /* Decode the series, set definition, and fields, to validate data. */
            primitiveEntryOverrunTest_byteBuffer.Flip();
            DecodeIterator dIter = new();
            Buffer decodeBuffer = new();
            decodeBuffer.Data(primitiveEntryOverrunTest_byteBuffer);
            dIter.SetBufferAndRWFVersion(decodeBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, series.Decode(dIter));
            Assert.Equal(DataTypes.FIELD_LIST, series.ContainerType);
            Assert.True(series.CheckHasSetDefs());

            fsetDb.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, fsetDb.Decode(dIter));
            Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.Decode(dIter));
            Assert.Equal(CodecReturnCode.SUCCESS, fieldList.Decode(dIter, fsetDb));

            /* INT_1 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(0, fieldEntry.FieldId);
            Assert.Equal(DataTypes.INT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* INT_2 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(1, fieldEntry.FieldId);
            Assert.Equal(DataTypes.INT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* INT_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(2, fieldEntry.FieldId);
            Assert.Equal(DataTypes.INT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* INT_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(3, fieldEntry.FieldId);
            Assert.Equal(DataTypes.INT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* UINT_1 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(4, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UINT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* UINT_2 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(5, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UINT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* UINT_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(6, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UINT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* UINT_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(7, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UINT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(ulong.MaxValue, uintVal.ToULong());

            /* FLOAT_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(8, fieldEntry.FieldId);
            Assert.Equal(DataTypes.FLOAT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, floatVal.Decode(dIter));
            Assert.Equal(1.0f, floatVal.ToFloat(), 0);

            /* DOUBLE_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(9, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DOUBLE, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, doubleVal.Decode(dIter));
            Assert.Equal(1.0, doubleVal.ToDouble(), 0);

            /* REAL_4RB */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(10, fieldEntry.FieldId);
            Assert.Equal(DataTypes.REAL, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(2147483647L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* REAL_8RB */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(11, fieldEntry.FieldId);
            Assert.Equal(DataTypes.REAL, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(9223372036854775807L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* DATE_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(12, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DATE, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateVal.Decode(dIter));
            Assert.Equal(12, dateVal.Day());
            Assert.Equal(11, dateVal.Month());
            Assert.Equal(1955, dateVal.Year());

            /* TIME_3 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(13, fieldEntry.FieldId);
            Assert.Equal(DataTypes.TIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(0, timeVal.Millisecond());
            Assert.Equal(0, timeVal.Microsecond());
            Assert.Equal(0, timeVal.Nanosecond());

            /* TIME_5 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(14, fieldEntry.FieldId);
            Assert.Equal(DataTypes.TIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(0, timeVal.Microsecond());
            Assert.Equal(0, timeVal.Nanosecond());

            /* DATETIME_7 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(15, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DATETIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(0, dateTimeVal.Millisecond());
            Assert.Equal(0, dateTimeVal.Microsecond());
            Assert.Equal(0, dateTimeVal.Nanosecond());

            /* DATETIME_11 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(16, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DATETIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(0, dateTimeVal.Nanosecond());

            /* DATETIME_12 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(17, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DATETIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(3, dateTimeVal.Nanosecond());

            /* TIME_7 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(18, fieldEntry.FieldId);
            Assert.Equal(DataTypes.TIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(0, timeVal.Nanosecond());

            /* TIME_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(19, fieldEntry.FieldId);
            Assert.Equal(DataTypes.TIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(3, timeVal.Nanosecond());

            /* INT */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(20, fieldEntry.FieldId);
            Assert.Equal(DataTypes.INT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(255, intVal.ToLong());

            /* UINT */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(21, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UINT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(256, uintVal.ToLong());

            /* FLOAT */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(22, fieldEntry.FieldId);
            Assert.Equal(DataTypes.FLOAT, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, floatVal.Decode(dIter));
            Assert.Equal(1.0f, floatVal.ToFloat(), 0);

            /* DOUBLE */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(23, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DOUBLE, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, doubleVal.Decode(dIter));
            Assert.Equal(1.0, doubleVal.ToDouble(), 0);

            /* REAL */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(24, fieldEntry.FieldId);
            Assert.Equal(DataTypes.REAL, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(2147483647L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* DATE */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(25, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DATE, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateVal.Decode(dIter));
            Assert.Equal(12, dateVal.Day());
            Assert.Equal(11, dateVal.Month());
            Assert.Equal(1955, dateVal.Year());

            /* TIME */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(26, fieldEntry.FieldId);
            Assert.Equal(DataTypes.TIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(3, timeVal.Nanosecond());

            /* DATETIME */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(27, fieldEntry.FieldId);
            Assert.Equal(DataTypes.DATETIME, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(3, dateTimeVal.Nanosecond());

            /* QOS */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(28, fieldEntry.FieldId);
            Assert.Equal(DataTypes.QOS, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, qosVal.Decode(dIter));
            Assert.Equal(QosTimeliness.DELAYED, qosVal.Timeliness());
            Assert.Equal(3000, qosVal.TimeInfo());
            Assert.Equal(QosRates.TIME_CONFLATED, qosVal.Rate());
            Assert.Equal(5000, qosVal.RateInfo());

            /* ENUM */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(29, fieldEntry.FieldId);
            Assert.Equal(DataTypes.ENUM, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, enumVal.Decode(dIter));
            Assert.Equal(256, enumVal.ToInt());

            /* BUFFER */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(30, fieldEntry.FieldId);
            Assert.Equal(DataTypes.BUFFER, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* ASCII_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(31, fieldEntry.FieldId);
            Assert.Equal(DataTypes.ASCII_STRING, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* UTF8_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(32, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UTF8_STRING, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(33, fieldEntry.FieldId);
            Assert.Equal(DataTypes.RMTES_STRING, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* Standard data */

            /* INT */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(20, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(255, intVal.ToLong());

            /* UINT */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(21, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(256, uintVal.ToLong());

            /* FLOAT */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(22, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, floatVal.Decode(dIter));
            Assert.Equal(1.0f, floatVal.ToFloat(), 0);

            /* DOUBLE */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(23, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, doubleVal.Decode(dIter));
            Assert.Equal(1.0, doubleVal.ToDouble(), 0);

            /* REAL */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(24, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(2147483647L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* DATE */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(25, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateVal.Decode(dIter));
            Assert.Equal(12, dateVal.Day());
            Assert.Equal(11, dateVal.Month());
            Assert.Equal(1955, dateVal.Year());

            /* TIME */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(26, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(3, timeVal.Nanosecond());

            /* DATETIME */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(27, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(3, dateTimeVal.Nanosecond());

            /* QOS */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(28, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, qosVal.Decode(dIter));
            Assert.Equal(QosTimeliness.DELAYED, qosVal.Timeliness());
            Assert.Equal(3000, qosVal.TimeInfo());
            Assert.Equal(QosRates.TIME_CONFLATED, qosVal.Rate());
            Assert.Equal(5000, qosVal.RateInfo());

            /* ENUM */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(29, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, enumVal.Decode(dIter));
            Assert.Equal(256, enumVal.ToInt());

            /* BUFFER */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(30, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* ASCII_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(31, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* UTF8_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(32, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(33, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING (encoded via encodedData) */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(34, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING (encoded as blank) */
            Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(dIter));
            Assert.Equal(35, fieldEntry.FieldId);
            Assert.Equal(DataTypes.UNKNOWN, fieldEntry.DataType);
            Assert.Equal(CodecReturnCode.BLANK_DATA, bufferVal.Decode(dIter));

            Assert.Equal(CodecReturnCode.END_OF_CONTAINER, fieldEntry.Decode(dIter));
            Assert.Equal(CodecReturnCode.END_OF_CONTAINER, seriesEntry.Decode(dIter));

            /* ElementList */

            /* Setup element set def db, with entries. */
            LocalElementSetDefDb esetDb = new LocalElementSetDefDb();
            ElementSetDefEntry[] elementSetEntries = new ElementSetDefEntry[34];

            elementSetEntries[0] = new ElementSetDefEntry();
            elementSetEntries[0].Name.Data("0");
            elementSetEntries[0].DataType = DataTypes.INT_1;

            elementSetEntries[1] = new ElementSetDefEntry();
            elementSetEntries[1].Name.Data("1");
            elementSetEntries[1].DataType = DataTypes.INT_2;

            elementSetEntries[2] = new ElementSetDefEntry();
            elementSetEntries[2].Name.Data("2");
            elementSetEntries[2].DataType = DataTypes.INT_4;

            elementSetEntries[3] = new ElementSetDefEntry();
            elementSetEntries[3].Name.Data("3");
            elementSetEntries[3].DataType = DataTypes.INT_8;

            elementSetEntries[4] = new ElementSetDefEntry();
            elementSetEntries[4].Name.Data("4");
            elementSetEntries[4].DataType = DataTypes.UINT_1;

            elementSetEntries[5] = new ElementSetDefEntry();
            elementSetEntries[5].Name.Data("5");
            elementSetEntries[5].DataType = DataTypes.UINT_2;

            elementSetEntries[6] = new ElementSetDefEntry();
            elementSetEntries[6].Name.Data("6");
            elementSetEntries[6].DataType = DataTypes.UINT_4;

            elementSetEntries[7] = new ElementSetDefEntry();
            elementSetEntries[7].Name.Data("7");
            elementSetEntries[7].DataType = DataTypes.UINT_8;

            elementSetEntries[8] = new ElementSetDefEntry();
            elementSetEntries[8].Name.Data("8");
            elementSetEntries[8].DataType = DataTypes.FLOAT_4;

            elementSetEntries[9] = new ElementSetDefEntry();
            elementSetEntries[9].Name.Data("9");
            elementSetEntries[9].DataType = DataTypes.DOUBLE_8;

            elementSetEntries[10] = new ElementSetDefEntry();
            elementSetEntries[10].Name.Data("10");
            elementSetEntries[10].DataType = DataTypes.REAL_4RB;

            elementSetEntries[11] = new ElementSetDefEntry();
            elementSetEntries[11].Name.Data("11");
            elementSetEntries[11].DataType = DataTypes.REAL_8RB;

            elementSetEntries[12] = new ElementSetDefEntry();
            elementSetEntries[12].Name.Data("12");
            elementSetEntries[12].DataType = DataTypes.DATE_4;

            elementSetEntries[13] = new ElementSetDefEntry();
            elementSetEntries[13].Name.Data("13");
            elementSetEntries[13].DataType = DataTypes.TIME_3;

            elementSetEntries[14] = new ElementSetDefEntry();
            elementSetEntries[14].Name.Data("14");
            elementSetEntries[14].DataType = DataTypes.TIME_5;

            elementSetEntries[15] = new ElementSetDefEntry();
            elementSetEntries[15].Name.Data("15");
            elementSetEntries[15].DataType = DataTypes.DATETIME_7;

            elementSetEntries[16] = new ElementSetDefEntry();
            elementSetEntries[16].Name.Data("16");
            elementSetEntries[16].DataType = DataTypes.DATETIME_11;

            elementSetEntries[17] = new ElementSetDefEntry();
            elementSetEntries[17].Name.Data("17");
            elementSetEntries[17].DataType = DataTypes.DATETIME_12;

            elementSetEntries[18] = new ElementSetDefEntry();
            elementSetEntries[18].Name.Data("18");
            elementSetEntries[18].DataType = DataTypes.TIME_7;

            elementSetEntries[19] = new ElementSetDefEntry();
            elementSetEntries[19].Name.Data("19");
            elementSetEntries[19].DataType = DataTypes.TIME_8;

            elementSetEntries[20] = new ElementSetDefEntry();
            elementSetEntries[20].Name.Data("20");
            elementSetEntries[20].DataType = DataTypes.INT;

            elementSetEntries[21] = new ElementSetDefEntry();
            elementSetEntries[21].Name.Data("21");
            elementSetEntries[21].DataType = DataTypes.UINT;

            elementSetEntries[22] = new ElementSetDefEntry();
            elementSetEntries[22].Name.Data("22");
            elementSetEntries[22].DataType = DataTypes.FLOAT;

            elementSetEntries[23] = new ElementSetDefEntry();
            elementSetEntries[23].Name.Data("23");
            elementSetEntries[23].DataType = DataTypes.DOUBLE;

            elementSetEntries[24] = new ElementSetDefEntry();
            elementSetEntries[24].Name.Data("24");
            elementSetEntries[24].DataType = DataTypes.REAL;

            elementSetEntries[25] = new ElementSetDefEntry();
            elementSetEntries[25].Name.Data("25");
            elementSetEntries[25].DataType = DataTypes.DATE;

            elementSetEntries[26] = new ElementSetDefEntry();
            elementSetEntries[26].Name.Data("26");
            elementSetEntries[26].DataType = DataTypes.TIME;

            elementSetEntries[27] = new ElementSetDefEntry();
            elementSetEntries[27].Name.Data("27");
            elementSetEntries[27].DataType = DataTypes.DATETIME;

            elementSetEntries[28] = new ElementSetDefEntry();
            elementSetEntries[28].Name.Data("28");
            elementSetEntries[28].DataType = DataTypes.QOS;

            elementSetEntries[29] = new ElementSetDefEntry();
            elementSetEntries[29].Name.Data("29");
            elementSetEntries[29].DataType = DataTypes.ENUM;

            elementSetEntries[30] = new ElementSetDefEntry();
            elementSetEntries[30].Name.Data("30");
            elementSetEntries[30].DataType = DataTypes.BUFFER;

            elementSetEntries[31] = new ElementSetDefEntry();
            elementSetEntries[31].Name.Data("31");
            elementSetEntries[31].DataType = DataTypes.ASCII_STRING;

            elementSetEntries[32] = new ElementSetDefEntry();
            elementSetEntries[32].Name.Data("32");
            elementSetEntries[32].DataType = DataTypes.UTF8_STRING;

            elementSetEntries[33] = new ElementSetDefEntry();
            elementSetEntries[33].Name.Data("33");
            elementSetEntries[33].DataType = DataTypes.RMTES_STRING;

            esetDb.Definitions[0].SetId = 0;
            esetDb.Definitions[0].Count = 34;
            esetDb.Definitions[0].Entries = elementSetEntries;

            /* Buffer size needed to encode (starting at series header) */
            bufSizeNeeded = seriesHeaderEncodedSize;
            encodeBufSize = bufSizeNeeded;

            ElementList elementList = new();
            ElementEntry elementEntry = new();

            // step II.1 finished decoding, start encoding

            primitiveEntryOverrunTest_byteBuffer = new ByteBuffer(encodeBufSize);
            primitiveEntryOverrunTest_encodeBuffer.Data(primitiveEntryOverrunTest_byteBuffer);
            eIter.Clear();
            eIter.SetBufferAndRWFVersion(primitiveEntryOverrunTest_encodeBuffer,
                Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            /* Encode Series. */
            series.Clear();
            series.ContainerType = DataTypes.ELEMENT_LIST;
            series.ApplyHasSetDefs();
            Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(eIter, 0, 0));

            /* Encode set definition. */
            bufSizeNeeded += 1 /* Flags */ + 1 /* SetDef Count */ + 1 /* Set ID */ + 1 /* Entry count */
                + 10 * 3 /* The first ten entries which have one-character names, "0" to "9" (name-length, name, dataType) */
                + 24 * 4 /* The remaining entries, have two-character names, "10" to "33" (name-length, name, dataType) */
                + 1 /* The check for the length of each entry's name-length assumes the worst-case (2 bytes), even
                 * though the entries only actually use one. So we will need one more byte available to completely encode it. */
                ;
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, esetDb.Encode(eIter));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(0, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, esetDb.Encode(eIter));
            Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeSetDefsComplete(eIter, true));

            bufSizeNeeded -= 1 /* Back off the one byte the encoder didn't use for the set definition. */;

            /* Encode SeriesEntry. */
            bufSizeNeeded += 3 /* Size bytes */;
            encodeBufSize = bufSizeNeeded;
            PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            seriesEntry.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(eIter, 0));

            bufSizeNeeded += 1 /* ElementList flags */ + 2 /* Set data length */ + 2 /* Standard data length */;
            encodeBufSize = bufSizeNeeded;
            PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);

            /* Encode element list. */
            elementList.Clear();
            elementList.ApplyHasSetData();
            elementList.ApplyHasStandardData();
            Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(eIter, esetDb, 0));

            /* INT_1 needs 1 byte. */
            bufSizeNeeded += 1;
            elementEntry.Clear();
            elementEntry.Name.Data("0");
            elementEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, intVal));

            /* INT_2 needs 2 bytes. */
            bufSizeNeeded += 2;
            elementEntry.Clear();
            elementEntry.Name.Data("1");
            elementEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, intVal));

            /* INT_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            elementEntry.Clear();
            elementEntry.Name.Data("2");
            elementEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, intVal));

            /* INT_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("3");
            elementEntry.DataType = DataTypes.INT;
            intVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, intVal));

            /* UINT_1 needs 1 byte. */
            bufSizeNeeded += 1;
            elementEntry.Clear();
            elementEntry.Name.Data("4");
            elementEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, uintVal));

            /* UINT_2 needs 2 bytes. */
            bufSizeNeeded += 2;
            elementEntry.Clear();
            elementEntry.Name.Data("5");
            elementEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, uintVal));

            /* UINT_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            elementEntry.Clear();
            elementEntry.Name.Data("6");
            elementEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, uintVal));

            /* UINT_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("7");
            elementEntry.DataType = DataTypes.UINT;
            uintVal.Value(64);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, uintVal));

            /* FLOAT_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            elementEntry.Clear();
            elementEntry.Name.Data("8");
            elementEntry.DataType = DataTypes.FLOAT;
            floatVal.Value(1.0f);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, floatVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, floatVal));

            /* DOUBLE_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("9");
            elementEntry.DataType = DataTypes.DOUBLE;
            doubleVal.Value(1.0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, doubleVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, doubleVal));

            /* REAL_4RB needs 5 bytes for this value. */
            bufSizeNeeded += 5;
            elementEntry.Clear();
            elementEntry.Name.Data("10");
            elementEntry.DataType = DataTypes.REAL;
            realVal.Value(2147483647, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, realVal));

            /* REAL_8RB needs 9 bytes for this value. */
            bufSizeNeeded += 9;
            elementEntry.Clear();
            elementEntry.Name.Data("11");
            elementEntry.DataType = DataTypes.REAL;
            realVal.Value(9223372036854775807L, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, realVal));

            /* DATE_4 needs 4 bytes. */
            bufSizeNeeded += 4;
            elementEntry.Clear();
            elementEntry.Name.Data("12");
            elementEntry.DataType = DataTypes.DATE;
            dateVal.Day(12);
            dateVal.Month(11);
            dateVal.Year(1955);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateVal));

            /* TIME_3 needs 3 bytes. */
            bufSizeNeeded += 3;
            elementEntry.Clear();
            elementEntry.Name.Data("13");
            elementEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, timeVal));

            /* TIME_5 needs 5 bytes. */
            bufSizeNeeded += 5;
            elementEntry.Clear();
            elementEntry.Name.Data("14");
            elementEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, timeVal));

            /* DATETIME_7 needs 7 bytes. */
            bufSizeNeeded += 7;
            elementEntry.Clear();
            elementEntry.Name.Data("15");
            elementEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateTimeVal));

            /* DATETIME_11 needs 11 bytes. */
            bufSizeNeeded += 11;
            elementEntry.Clear();
            elementEntry.Name.Data("16");
            elementEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateTimeVal));

            /* DATETIME_12 needs 12 bytes. */
            bufSizeNeeded += 12;
            elementEntry.Clear();
            elementEntry.Name.Data("17");
            elementEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            dateTimeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateTimeVal));

            /* TIME_7 needs 7 bytes. */
            bufSizeNeeded += 7;
            elementEntry.Clear();
            elementEntry.Name.Data("18");
            elementEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, timeVal));

            /* TIME_8 needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("19");
            elementEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            timeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, timeVal));

            /* This INT needs 3 bytes. */
            bufSizeNeeded += 3;
            elementEntry.Clear();
            elementEntry.Name.Data("20");
            elementEntry.DataType = DataTypes.INT;
            intVal.Value(255);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, intVal));

            /* This UINT needs 3 bytes. */
            bufSizeNeeded += 3;
            elementEntry.Clear();
            elementEntry.Name.Data("21");
            elementEntry.DataType = DataTypes.UINT;
            uintVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, uintVal));

            /* This FLOAT needs 5 bytes. */
            bufSizeNeeded += 5;
            elementEntry.Clear();
            elementEntry.Name.Data("22");
            elementEntry.DataType = DataTypes.FLOAT;
            floatVal.Value(1.0f);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, floatVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, floatVal));

            /* This DOUBLE needs 9 bytes. */
            bufSizeNeeded += 9;
            elementEntry.Clear();
            elementEntry.Name.Data("23");
            elementEntry.DataType = DataTypes.DOUBLE;
            doubleVal.Value(1.0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, doubleVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, doubleVal));

            /* This REAL needs 6 bytes. */
            bufSizeNeeded += 6;
            elementEntry.Clear();
            elementEntry.Name.Data("24");
            elementEntry.DataType = DataTypes.REAL;
            realVal.Value(2147483647, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, realVal));

            /* This DATE needs 5 bytes. */
            bufSizeNeeded += 5;
            elementEntry.Clear();
            elementEntry.Name.Data("25");
            elementEntry.DataType = DataTypes.DATE;
            dateVal.Day(12);
            dateVal.Month(11);
            dateVal.Year(1955);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }

            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateVal));

            /* This TIME needs 9 bytes. */
            bufSizeNeeded += 9;
            elementEntry.Clear();
            elementEntry.Name.Data("26");
            elementEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            timeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, timeVal));

            /* This DATETIME needs 13 bytes. */
            bufSizeNeeded += 13;
            elementEntry.Clear();
            elementEntry.Name.Data("27");
            elementEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            dateTimeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateTimeVal));

            /* This QOS needs 6 bytes. */
            bufSizeNeeded += 6;
            elementEntry.Clear();
            elementEntry.Name.Data("28");
            elementEntry.DataType = DataTypes.QOS;
            qosVal.Timeliness(QosTimeliness.DELAYED);
            qosVal.TimeInfo(3000);
            qosVal.Rate(QosRates.TIME_CONFLATED);
            qosVal.RateInfo(5000);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, qosVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, qosVal));

            /* This ENUM needs 3 bytes. */
            bufSizeNeeded += 3;
            elementEntry.Clear();
            elementEntry.Name.Data("29");
            elementEntry.DataType = DataTypes.ENUM;
            enumVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, enumVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, enumVal));

            /* This BUFFER needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("30");
            elementEntry.DataType = DataTypes.BUFFER;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This ASCII_STRING needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("31");
            elementEntry.DataType = DataTypes.ASCII_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This UTF8_STRING needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("32");
            elementEntry.DataType = DataTypes.UTF8_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This RMTES_STRING needs 8 bytes. */
            bufSizeNeeded += 8;
            elementEntry.Clear();
            elementEntry.Name.Data("33");
            elementEntry.DataType = DataTypes.RMTES_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SET_COMPLETE, elementEntry.Encode(eIter, bufferVal));

            /* Standard data */

            /* This INT needs 7 bytes (1 for dataType, 3 for name, 3 for value) */
            bufSizeNeeded += 7;
            elementEntry.Clear();
            elementEntry.Name.Data("20");
            elementEntry.DataType = DataTypes.INT;
            intVal.Value(255);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, intVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, intVal));

            /* This UINT needs 7 bytes (1 for dataType, 3 for name, 3 for value) */
            bufSizeNeeded += 7;
            elementEntry.Clear();
            elementEntry.Name.Data("21");
            elementEntry.DataType = DataTypes.UINT;
            uintVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, uintVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, uintVal));

            /* This FLOAT needs 7 bytes (1 for dataType, 3 for name, 5 for value) */
            bufSizeNeeded += 9;
            elementEntry.Clear();
            elementEntry.Name.Data("22");
            elementEntry.DataType = DataTypes.FLOAT;
            floatVal.Value(1.0f);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, floatVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, floatVal));

            /* This DOUBLE needs 13 bytes (1 for dataType, 3 for name, 9 for value) */
            bufSizeNeeded += 13;
            elementEntry.Clear();
            elementEntry.Name.Data("23");
            elementEntry.DataType = DataTypes.DOUBLE;
            doubleVal.Value(1.0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, doubleVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, doubleVal));

            /* This REAL needs 10 bytes (1 for dataType, 3 for name, 6 for value) */
            bufSizeNeeded += 10;
            elementEntry.Clear();
            elementEntry.Name.Data("24");
            elementEntry.DataType = DataTypes.REAL;
            realVal.Value(2147483647, RealHints.EXPONENT0);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, realVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, realVal));

            /* This DATE needs 9 bytes (1 for dataType, 3 for name, 5 for value) */
            bufSizeNeeded += 9;
            elementEntry.Clear();
            elementEntry.Name.Data("25");
            elementEntry.DataType = DataTypes.DATE;
            dateVal.Day(12);
            dateVal.Month(11);
            dateVal.Year(1955);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }

            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateVal));

            /* This TIME needs 13 bytes (1 for dataType, 3 for name, 9 for value) */
            bufSizeNeeded += 13;
            elementEntry.Clear();
            elementEntry.Name.Data("26");
            elementEntry.DataType = DataTypes.TIME;
            timeVal.Hour(22);
            timeVal.Minute(4);
            timeVal.Second(0);
            timeVal.Millisecond(1);
            timeVal.Microsecond(2);
            timeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, timeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, timeVal));

            /* This DATETIME needs 17 bytes (1 for dataType, 3 for name, 13 for value) */
            bufSizeNeeded += 17;
            elementEntry.Clear();
            elementEntry.Name.Data("27");
            elementEntry.DataType = DataTypes.DATETIME;
            dateTimeVal.Day(12);
            dateTimeVal.Month(11);
            dateTimeVal.Year(1955);
            dateTimeVal.Hour(22);
            dateTimeVal.Minute(4);
            dateTimeVal.Second(0);
            dateTimeVal.Millisecond(1);
            dateTimeVal.Microsecond(2);
            dateTimeVal.Nanosecond(3);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, dateTimeVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, dateTimeVal));

            /* This QOS needs 10 bytes (1 for dataType, 3 for name, 6 for value) */
            bufSizeNeeded += 10;
            elementEntry.Clear();
            elementEntry.Name.Data("28");
            elementEntry.DataType = DataTypes.QOS;
            qosVal.Timeliness(QosTimeliness.DELAYED);
            qosVal.TimeInfo(3000);
            qosVal.Rate(QosRates.TIME_CONFLATED);
            qosVal.RateInfo(5000);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, qosVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, qosVal));

            /* This ENUM needs 7 bytes (1 for dataType, 3 for name, 3 for value) */
            bufSizeNeeded += 7;
            elementEntry.Clear();
            elementEntry.Name.Data("29");
            elementEntry.DataType = DataTypes.ENUM;
            enumVal.Value(256);
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, enumVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, enumVal));

            /* This BUFFER needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
            bufSizeNeeded += 12;
            elementEntry.Clear();
            elementEntry.Name.Data("30");
            elementEntry.DataType = DataTypes.BUFFER;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This ASCII_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
            bufSizeNeeded += 12;
            elementEntry.Clear();
            elementEntry.Name.Data("31");
            elementEntry.DataType = DataTypes.ASCII_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This UTF8_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
            bufSizeNeeded += 12;
            elementEntry.Clear();
            elementEntry.Name.Data("32");
            elementEntry.DataType = DataTypes.UTF8_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This RMTES_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
            bufSizeNeeded += 12;
            elementEntry.Clear();
            elementEntry.Name.Data("33");
            elementEntry.DataType = DataTypes.RMTES_STRING;
            bufferVal.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter, bufferVal));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter, bufferVal));

            /* This RMTES_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
            /* Encode this one via encodedData. */
            bufSizeNeeded += 12;
            elementEntry.Clear();
            elementEntry.Name.Data("34");
            elementEntry.DataType = DataTypes.RMTES_STRING;
            elementEntry.EncodedData.Data("OVERRUN");
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter));

            /* This RMTES_STRING will be blank and needs 5 bytes (1 for dataType, 3 for name, 1 for value) */
            bufSizeNeeded += 5;
            elementEntry.Clear();
            elementEntry.Name.Data("35");
            elementEntry.DataType = DataTypes.RMTES_STRING;
            while (encodeBufSize < bufSizeNeeded)
            {
                int startPos = eIter._curBufPos;
                Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(eIter));
                Assert.Equal(startPos, eIter._curBufPos);
                Assert.Equal(startPos, eIter._writer.Position());
                Assert.Equal(1, eIter._encodingLevel);
                ++encodeBufSize;
                PrimitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
            }
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(eIter));

            Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(eIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeComplete(eIter, true));
            Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeComplete(eIter, true));

            // step II.2 finished encoding, start decoding

            /* Decode the series, set definition, and elements, to validate data. */
            primitiveEntryOverrunTest_byteBuffer.Flip();
            decodeBuffer.Data(primitiveEntryOverrunTest_byteBuffer);
            dIter.SetBufferAndRWFVersion(decodeBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, series.Decode(dIter));
            Assert.Equal(DataTypes.ELEMENT_LIST, series.ContainerType);
            Assert.True(series.CheckHasSetDefs());

            esetDb.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, esetDb.Decode(dIter));
            Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.Decode(dIter));
            Assert.Equal(CodecReturnCode.SUCCESS, elementList.Decode(dIter, esetDb));

            /* INT_1 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("0", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.INT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* INT_2 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("1", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.INT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* INT_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("2", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.INT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* INT_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("3", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.INT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(64, intVal.ToLong());

            /* UINT_1 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("4", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UINT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* UINT_2 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("5", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UINT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* UINT_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("6", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UINT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* UINT_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("7", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UINT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(64, uintVal.ToLong());

            /* FLOAT_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("8", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.FLOAT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, floatVal.Decode(dIter));
            Assert.Equal(1.0f, floatVal.ToFloat(), 0);

            /* DOUBLE_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("9", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DOUBLE, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, doubleVal.Decode(dIter));
            Assert.Equal(1.0, doubleVal.ToDouble(), 0);

            /* REAL_4RB */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("10", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.REAL, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(2147483647, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* REAL_8RB */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("11", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.REAL, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(9223372036854775807L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* DATE_4 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("12", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATE, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateVal.Decode(dIter));
            Assert.Equal(12, dateVal.Day());
            Assert.Equal(11, dateVal.Month());
            Assert.Equal(1955, dateVal.Year());

            /* TIME_3 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("13", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.TIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(0, timeVal.Millisecond());
            Assert.Equal(0, timeVal.Microsecond());
            Assert.Equal(0, timeVal.Nanosecond());

            /* TIME_5 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("14", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.TIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(0, timeVal.Microsecond());
            Assert.Equal(0, timeVal.Nanosecond());

            /* DATETIME_7 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("15", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATETIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(0, dateTimeVal.Millisecond());
            Assert.Equal(0, dateTimeVal.Microsecond());
            Assert.Equal(0, dateTimeVal.Nanosecond());

            /* DATETIME_11 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("16", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATETIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(0, dateTimeVal.Nanosecond());

            /* DATETIME_12 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("17", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATETIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(3, dateTimeVal.Nanosecond());

            /* TIME_7 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("18", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.TIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(0, timeVal.Nanosecond());

            /* TIME_8 */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("19", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.TIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(3, timeVal.Nanosecond());

            /* INT */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("20", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.INT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(255, intVal.ToLong());

            /* UINT */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("21", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UINT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(256, uintVal.ToLong());

            /* FLOAT */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("22", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.FLOAT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, floatVal.Decode(dIter));
            Assert.Equal(1.0f, floatVal.ToFloat(), 0);

            /* DOUBLE */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("23", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DOUBLE, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, doubleVal.Decode(dIter));
            Assert.Equal(1.0, doubleVal.ToDouble(), 0);

            /* REAL */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("24", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.REAL, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(2147483647L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* DATE */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("25", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATE, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateVal.Decode(dIter));
            Assert.Equal(12, dateVal.Day());
            Assert.Equal(11, dateVal.Month());
            Assert.Equal(1955, dateVal.Year());

            /* TIME */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("26", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.TIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(3, timeVal.Nanosecond());

            /* DATETIME */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("27", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATETIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(3, dateTimeVal.Nanosecond());

            /* QOS */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("28", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.QOS, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, qosVal.Decode(dIter));
            Assert.Equal(QosTimeliness.DELAYED, qosVal.Timeliness());
            Assert.Equal(3000, qosVal.TimeInfo());
            Assert.Equal(QosRates.TIME_CONFLATED, qosVal.Rate());
            Assert.Equal(5000, qosVal.RateInfo());

            /* ENUM */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("29", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.ENUM, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, enumVal.Decode(dIter));
            Assert.Equal(256, enumVal.ToInt());

            /* BUFFER */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("30", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.BUFFER, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* ASCII_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("31", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.ASCII_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* UTF8_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("32", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UTF8_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("33", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.RMTES_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* Standard data */

            /* INT */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("20", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.INT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, intVal.Decode(dIter));
            Assert.Equal(255, intVal.ToLong());

            /* UINT */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("21", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UINT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, uintVal.Decode(dIter));
            Assert.Equal(256, uintVal.ToLong());

            /* FLOAT */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("22", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.FLOAT, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, floatVal.Decode(dIter));
            Assert.Equal(1.0f, floatVal.ToFloat(), 0);

            /* DOUBLE */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("23", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DOUBLE, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, doubleVal.Decode(dIter));
            Assert.Equal(1.0, doubleVal.ToDouble(), 0);

            /* REAL */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("24", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.REAL, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, realVal.Decode(dIter));
            Assert.Equal(2147483647L, realVal.ToLong());
            Assert.Equal(RealHints.EXPONENT0, realVal.Hint);

            /* DATE */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("25", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATE, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateVal.Decode(dIter));
            Assert.Equal(12, dateVal.Day());
            Assert.Equal(11, dateVal.Month());
            Assert.Equal(1955, dateVal.Year());

            /* TIME */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("26", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.TIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, timeVal.Decode(dIter));
            Assert.Equal(22, timeVal.Hour());
            Assert.Equal(4, timeVal.Minute());
            Assert.Equal(0, timeVal.Second());
            Assert.Equal(1, timeVal.Millisecond());
            Assert.Equal(2, timeVal.Microsecond());
            Assert.Equal(3, timeVal.Nanosecond());

            /* DATETIME */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("27", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.DATETIME, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, dateTimeVal.Decode(dIter));
            Assert.Equal(12, dateTimeVal.Day());
            Assert.Equal(11, dateTimeVal.Month());
            Assert.Equal(1955, dateTimeVal.Year());
            Assert.Equal(22, dateTimeVal.Hour());
            Assert.Equal(4, dateTimeVal.Minute());
            Assert.Equal(0, dateTimeVal.Second());
            Assert.Equal(1, dateTimeVal.Millisecond());
            Assert.Equal(2, dateTimeVal.Microsecond());
            Assert.Equal(3, dateTimeVal.Nanosecond());

            /* QOS */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("28", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.QOS, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, qosVal.Decode(dIter));
            Assert.Equal(QosTimeliness.DELAYED, qosVal.Timeliness());
            Assert.Equal(3000, qosVal.TimeInfo());
            Assert.Equal(QosRates.TIME_CONFLATED, qosVal.Rate());
            Assert.Equal(5000, qosVal.RateInfo());

            /* ENUM */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("29", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.ENUM, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, enumVal.Decode(dIter));
            Assert.Equal(256, enumVal.ToInt());

            /* BUFFER */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("30", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.BUFFER, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* ASCII_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("31", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.ASCII_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* UTF8_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("32", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.UTF8_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("33", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.RMTES_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING (encoded via encodedData) */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("34", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.RMTES_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.SUCCESS, bufferVal.Decode(dIter));
            Assert.Equal("OVERRUN", bufferVal.ToString());

            /* RMTES_STRING (encoded as blank) */
            Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Decode(dIter));
            Assert.Equal("35", elementEntry.Name.ToString());
            Assert.Equal(DataTypes.RMTES_STRING, elementEntry.DataType);
            Assert.Equal(CodecReturnCode.BLANK_DATA, bufferVal.Decode(dIter));

            Assert.Equal(CodecReturnCode.END_OF_CONTAINER, elementEntry.Decode(dIter));
            Assert.Equal(CodecReturnCode.END_OF_CONTAINER, seriesEntry.Decode(dIter));
        }
    }
}
