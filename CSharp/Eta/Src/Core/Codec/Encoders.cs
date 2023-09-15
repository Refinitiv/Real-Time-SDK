/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System;

namespace LSEG.Eta.Codec
{
	internal class Encoders
	{
		static readonly int __RSZFLT = 4;
		static readonly int __RSZDBL = 8;

		static readonly int RWF_MAX_16 = 0xFFFF;
		static readonly int RWF_MAX_U15 = 0x7FFF;
        static int HAS_MSG_KEY = 1;
        static int HAS_EXT_HEADER = 2;

        public class PrimitiveEncoder
		{
            private Encoders _encoders;
            private static readonly HashSet<int> _primitveDataTypesSet = new(40);

            static PrimitiveEncoder()
            {
                _primitveDataTypesSet.Add(3);
                _primitveDataTypesSet.Add(4);
                _primitveDataTypesSet.Add(5);
                _primitveDataTypesSet.Add(6);
                _primitveDataTypesSet.Add(8);
                _primitveDataTypesSet.Add(9);
                _primitveDataTypesSet.Add(10);
                _primitveDataTypesSet.Add(11);
                _primitveDataTypesSet.Add(12);
                _primitveDataTypesSet.Add(13);
                _primitveDataTypesSet.Add(14);
                _primitveDataTypesSet.Add(16);
                _primitveDataTypesSet.Add(17);
                _primitveDataTypesSet.Add(18);
                _primitveDataTypesSet.Add(19);
                _primitveDataTypesSet.Add(64);
                _primitveDataTypesSet.Add(65);
                _primitveDataTypesSet.Add(66);
                _primitveDataTypesSet.Add(67);
                _primitveDataTypesSet.Add(68);
                _primitveDataTypesSet.Add(69);
                _primitveDataTypesSet.Add(70);
                _primitveDataTypesSet.Add(71);
                _primitveDataTypesSet.Add(72);
                _primitveDataTypesSet.Add(73);
                _primitveDataTypesSet.Add(74);
                _primitveDataTypesSet.Add(75);
                _primitveDataTypesSet.Add(76);
                _primitveDataTypesSet.Add(77);
                _primitveDataTypesSet.Add(78);
                _primitveDataTypesSet.Add(79);
                _primitveDataTypesSet.Add(80);
                _primitveDataTypesSet.Add(81);
                _primitveDataTypesSet.Add(82);
                _primitveDataTypesSet.Add(83);
                _primitveDataTypesSet.Add(84);

                _setEncodeActions[DataTypes.INT] = (iter, data) => EncodeIntWithLength(iter, (Int)data);
                _setEncodeActions[DataTypes.UINT] = (iter, data) => EncodeUIntWithLength(iter, (UInt)data);
                _setEncodeActions[DataTypes.FLOAT] = (iter, data) => EncodeFloatWithLength(iter, (Float)data);
                _setEncodeActions[DataTypes.DOUBLE] = (iter, data) => EncodeDoubleWithLength(iter, (Double)data);
                _setEncodeActions[DataTypes.REAL] = (iter, data) => EncodeRealWithLength(iter, (Real)data);
                _setEncodeActions[DataTypes.DATE] = (iter, data) => EncodeDateWithLength(iter, (Date)data);
                _setEncodeActions[DataTypes.TIME] = (iter, data) => EncodeTimeWithLength(iter, (Time)data);
                _setEncodeActions[DataTypes.DATETIME] = (iter, data) => EncodeDateTimeWithLength(iter, (DateTime)data);
                _setEncodeActions[DataTypes.QOS] = (iter, data) => EncodeQosWithLength(iter, (Qos)data);
                _setEncodeActions[DataTypes.STATE] = (iter, data) => EncodeStateWithLength(iter, (State)data);
                _setEncodeActions[DataTypes.ENUM] = (iter, data) => EncodeEnumWithLength(iter, (Enum)data);
                _setEncodeActions[DataTypes.BUFFER] = (iter, data) => EncodeBufferWithLength(iter, (Buffer)data);
                _setEncodeActions[DataTypes.ASCII_STRING] = (iter, data) => EncodeBufferWithLength(iter, (Buffer)data);
                _setEncodeActions[DataTypes.UTF8_STRING] = (iter, data) => EncodeBufferWithLength(iter, (Buffer)data);
                _setEncodeActions[DataTypes.RMTES_STRING] = (iter, data) => EncodeBufferWithLength(iter, (Buffer)data);
                _setEncodeActions[DataTypes.INT_1] = (iter, data) => EncodeInt1(iter, (Int)data);
                _setEncodeActions[DataTypes.UINT_1] = (iter, data) => EncodeUInt1(iter, (UInt)data);
                _setEncodeActions[DataTypes.INT_2] = (iter, data) => EncodeInt2(iter, (Int)data);
                _setEncodeActions[DataTypes.UINT_2] = (iter, data) => EncodeUInt2(iter, (UInt)data);
                _setEncodeActions[DataTypes.INT_4] = (iter, data) => EncodeInt4(iter, (Int)data);
                _setEncodeActions[DataTypes.UINT_4] = (iter, data) => EncodeUInt4(iter, (UInt)data);
                _setEncodeActions[DataTypes.INT_8] = (iter, data) => EncodeInt8(iter, (Int)data);
                _setEncodeActions[DataTypes.UINT_8] = (iter, data) => EncodeUInt8(iter, (UInt)data);
                _setEncodeActions[DataTypes.FLOAT_4] = (iter, data) => EncodeFloat4(iter, (Float)data);
                _setEncodeActions[DataTypes.DOUBLE_8] = (iter, data) => EncodeDouble8(iter, (Double)data);
                _setEncodeActions[DataTypes.REAL_4RB] = (iter, data) => EncodeReal4(iter, (Real)data);
                _setEncodeActions[DataTypes.REAL_8RB] = (iter, data) => EncodeReal8(iter, (Real)data);
                _setEncodeActions[DataTypes.DATE_4] = (iter, data) => EncodeDate4(iter, (Date)data);
                _setEncodeActions[DataTypes.TIME_3] = (iter, data) => EncodeTime3(iter, (Time)data);
                _setEncodeActions[DataTypes.TIME_5] = (iter, data) => EncodeTime5(iter, (Time)data);
                _setEncodeActions[DataTypes.DATETIME_7] = (iter, data) => EncodeDateTime7(iter, (DateTime)data);
                _setEncodeActions[DataTypes.DATETIME_9] = (iter, data) => EncodeDateTime9(iter, (DateTime)data);
                _setEncodeActions[DataTypes.DATETIME_11] = (iter, data) => EncodeDateTime11(iter, (DateTime)data);
                _setEncodeActions[DataTypes.DATETIME_12] = (iter, data) => EncodeDateTime12(iter, (DateTime)data);
                _setEncodeActions[DataTypes.TIME_7] = (iter, data) => EncodeTime7(iter, (Time)data);
                _setEncodeActions[DataTypes.TIME_8] = (iter, data) => EncodeTime8(iter, (Time)data);
            }

            private static Func<EncodeIterator, object, CodecReturnCode>[] _setEncodeActions = new Func<EncodeIterator, object, CodecReturnCode>[85];

            public PrimitiveEncoder(Encoders encoders)
            {
                _encoders = encoders;
            }

            public static bool ValidEncodeActions(int type)
            {
                return _primitveDataTypesSet.Contains(type);
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode SetEncodeActions(int type, EncodeIterator iter, object data)
            {
                try
                {
                    return _setEncodeActions[type](iter, data);
                }
                catch (Exception)
                {
                    return CodecReturnCode.FAILURE;
                }
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static bool ValidArrayEncodeActions(int type, int length)
            {
                switch (type)
                {
                    case 3:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 1: 
                                case 2: 
                                case 4: 
                                case 8:
                                    return true;
                            }
                        }
                        break;
                    case 4:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 1: 
                                case 2: 
                                case 4: 
                                case 8:
                                    return true;
                            }
                        }
                        break;
                    case 5:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 4:
                                    return true;
                            }
                        }
                        break;
                    case 6:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 8:
                                    return true;
                            }
                        }
                        break;
                    case 8:
                        {
                            switch (length)
                            {
                                case 0: return true;
                            }
                        }
                        break;
                    case 9:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 4:
                                    return true;
                            }
                        }
                        break;
                    case 10:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 3: 
                                case 5:
                                    return true;
                            }
                        }
                        break;
                    case 11:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 7: 
                                case 9:
                                    return true;
                            }
                        }
                        break;
                    case 12:
                        {
                            switch (length)
                            {
                                case 0: return true;
                            }
                        }
                        break;
                    case 13:
                        {
                            switch (length)
                            {
                                case 0: return true;
                            }
                        }
                        break;
                    case 14:
                        {
                            switch (length)
                            {
                                case 0: 
                                case 1: 
                                case 2:
                                    return true;
                            }
                        }
                        break;
                    default:
                        break;
                }
                return false;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode SetArrayEncodeActions(int type, int length, EncodeIterator iter, object data)
            {
                switch (type)
                {
                    case 3:
                        {
                            switch (length)
                            {
                                case 0: return EncodeIntWithLength(iter, (Int)data);
                                case 1: return EncodeInt1(iter, (Int)data);
                                case 2: return EncodeInt2(iter, (Int)data);
                                case 4: return EncodeInt4(iter, (Int)data);
                                case 8: return EncodeInt8(iter, (Int)data);
                            }
                        }
                        break;
                    case 4:
                        {
                            switch (length)
                            {
                                case 0: return EncodeUIntWithLength(iter, (UInt)data);
                                case 1: return EncodeUInt1(iter, (UInt)data);
                                case 2: return EncodeUInt2(iter, (UInt)data);
                                case 4: return EncodeUInt4(iter, (UInt)data);
                                case 8: return EncodeUInt8(iter, (UInt)data);
                            }
                        }
                        break;
                    case 5:
                        {
                            switch (length)
                            {
                                case 0: return EncodeFloatWithLength(iter, (Float)data);
                                case 4: return EncodeFloat4(iter, (Float)data);
                            }
                        }
                        break;
                    case 6:
                        {
                            switch (length)
                            {
                                case 0: return EncodeDoubleWithLength(iter, (Double)data);
                                case 8: return EncodeDouble8(iter, (Double)data);
                            }
                        }
                        break;
                    case 8:
                        {
                            switch (length)
                            {
                                case 0: return EncodeRealWithLength(iter, (Real)data);
                            }
                        }
                        break;
                    case 9:
                        {
                            switch (length)
                            {
                                case 0: return EncodeDateWithLength(iter, (Date)data);
                                case 4: return EncodeDate4(iter, (Date)data);
                            }
                        }
                        break;
                    case 10:
                        {
                            switch (length)
                            {
                                case 0: return EncodeTimeWithLength(iter, (Time)data);
                                case 3: return EncodeTime3(iter, (Time)data);
                                case 5: return EncodeTime5(iter, (Time)data);
                            }
                        }
                        break;
                    case 11:
                        {
                            switch (length)
                            {
                                case 0: return EncodeDateTimeWithLength(iter, (DateTime)data);
                                case 7: return EncodeDateTime7(iter, (DateTime)data);
                                case 9: return EncodeDateTime9(iter, (DateTime)data);
                            }
                        }
                        break;
                    case 12:
                        {
                            switch (length)
                            {
                                case 0: return EncodeQosWithLength(iter, (Qos)data);
                            }
                        }
                        break;
                    case 13:
                        {
                            switch (length)
                            {
                                case 0: return EncodeStateWithLength(iter, (State)data);
                            }
                        }
                        break;
                    case 14:
                        {
                            switch (length)
                            {
                                case 0: return EncodeEnumWithLength(iter, (Enum)data);
                                case 1: return EncodeEnum1(iter, (Enum)data);
                                case 2: return EncodeEnum2(iter, (Enum)data);
                            }
                        }
                        break;
                    default:
                        break;
                }
                return CodecReturnCode.FAILURE;
            }

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeArrayData(EncodeIterator iter, object data, int type, int length)
			{
                return SetArrayEncodeActions(type, length, iter, data);
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeSetData(EncodeIterator iter, object data, int type)
			{
				return SetEncodeActions(type, iter, data);
			}

            // the encode methods
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeBuffer(EncodeIterator iter, Buffer data)
			{
				if (iter.IsIteratorOverrun(data.Length))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.Write(data);
				iter._curBufPos = iter._writer.Position();
				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeBufferWithLength(EncodeIterator iter, Buffer data)
			{
				EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

				/* Check to see if what type of length encoding. */
				if (_levelInfo._encodingState == EncodeIteratorStates.PRIMITIVE_U15)
				{
					int len = data.GetLength();
					if (len > RWF_MAX_U15)
					{
						return CodecReturnCode.ENCODING_UNAVAILABLE;
					}

					// len is written on wire as uShort15rb
					// If the value is smaller than 0x80, it is written on the wire as one byte,
					// otherwise, it is written as two bytes by calling writeUShort15rbLong method.
					// The code below checks if the buffer is sufficient for each case.
					if (len < 0x80)
					{
						if (iter.IsIteratorOverrun(1 + len)) // 1 byte len + len
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer._buffer.Write((byte)len);
						iter._writer.Write(data);
					}
					else
					{
						if (iter.IsIteratorOverrun(2 + len)) // 2 bytes len + len
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer.WriteUShort15rbLong((short)len);
						iter._writer.Write(data);
					}
					iter._curBufPos = iter._writer.Position();
				}
				else
				{
					int len = data.GetLength();

					// len value is written on wire as uShort16ob
					// If the value is smaller than 0xFE, it is written on the wire as one byte,
					// otherwise, it is written as three bytes by calling writeUShort16obLong method.
					// The code below checks if the buffer is sufficient for each case.
					if (len < 0xFE)
					{
						if (iter.IsIteratorOverrun(1 + len))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}

						iter._writer._buffer.Write((byte)len);
						iter._writer.Write(data);
					}
					else if (len <= 0xFFFF)
					{
						if (iter.IsIteratorOverrun(3 + len))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}

						iter._writer.WriteUShort16obLong(len);
						iter._writer.Write(data);

					}
					else
					{
						return CodecReturnCode.INVALID_DATA;
					}

					iter._curBufPos = iter._writer.Position();
				}
				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeInt(EncodeIterator iter, Int data)
			{
				long val = data.ToLong();

                CodecReturnCode ret = iter._writer.WriteLong64ls(val);
				if (ret != CodecReturnCode.SUCCESS) // checks for buffer too small
				{
					return ret;
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeIntWithLength(EncodeIterator iter, Int data)
			{
				long val = data.ToLong();

                CodecReturnCode ret = iter._writer.WriteLong64lsWithLength(val);
				if (ret != CodecReturnCode.SUCCESS) // checks for buffer too small
				{
					return ret;
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeInt1(EncodeIterator iter, Int data)
			{
				long val = data.ToLong();

				if ((val < -0x80) || (val >= 0x80))
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeInt2(EncodeIterator iter, Int data)
			{
				long val = data.ToLong();

				if ((val < -0x8000) || (val >= 0x8000))
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(2))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteShort((int)val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeInt4(EncodeIterator iter, Int data)
			{
				long val = data.ToLong();

				if ((val < -0x80000000L) || (val >= 0x80000000L))
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(4))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteInt((int)val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeInt8(EncodeIterator iter, Int data)
			{
				long val = data.ToLong();

				if (iter.IsIteratorOverrun(8))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteLong(val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeUInt(EncodeIterator iter, UInt data)
			{
				long val = data.ToLong();

                CodecReturnCode ret = iter._writer.WriteULong64ls(val);
				if (ret != CodecReturnCode.SUCCESS) // checks for buffer too small
				{
					return ret;
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeUIntWithLength(EncodeIterator iter, UInt data)
			{
				long val = data.ToLong();

                CodecReturnCode ret = iter._writer.WriteULong64lsWithLength(val);
				if (ret != CodecReturnCode.SUCCESS) // checks for buffer too small
				{
					return ret;
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeUInt1(EncodeIterator iter, UInt data)
			{
				long val = data.ToLong();

				if ((val < 0) || (val > 0xFFL))
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeUInt2(EncodeIterator iter, UInt data)
			{
				long val = data.ToLong();

				if ((val < 0) || (val > 0xFFFFL))
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(2))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteShort((int)val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeUInt4(EncodeIterator iter, UInt data)
			{
				long val = data.ToLong();

				if ((val < 0) || (val > 0xFFFFFFFFL))
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(4))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteInt((int)val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeUInt8(EncodeIterator iter, UInt data)
			{
				long val = data.ToLong();

				if (iter.IsIteratorOverrun(8))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteLong(val);
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeFloat4(EncodeIterator iter, Float data)
			{
				Debug.Assert(iter._levelInfo[iter._encodingLevel]._encodingState == EncodeIteratorStates.SET_DATA,
                    "Unexpected encoding attempted");

                return EncodeFloat(iter, data);
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeFloat(EncodeIterator iter, Float data)
			{
				if (iter.IsIteratorOverrun(__RSZFLT))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteFloat(data.ToFloat());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDouble8(EncodeIterator iter, Double data)
			{
				Debug.Assert(iter._levelInfo[iter._encodingLevel]._encodingState == EncodeIteratorStates.SET_DATA,
                    "Unexpected encoding attempted");

                return EncodeDouble(iter, data);
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeDouble(EncodeIterator iter, Double data)
			{
				if (iter.IsIteratorOverrun(__RSZDBL))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteDouble(data.ToDouble());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeFloatWithLength(EncodeIterator iter, Float data)
			{
				if (iter.IsIteratorOverrun(5))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)4);
				iter._writer.WriteFloat(data.ToFloat());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDoubleWithLength(EncodeIterator iter, Double data)
			{
				if (iter.IsIteratorOverrun(9))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)8);
				iter._writer.WriteDouble(data.ToDouble());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeReal(EncodeIterator iter, Real data)
			{
				if (iter.IsIteratorOverrun(1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				if (data.IsBlank)
				{
					PutLenSpecBlank(iter);
					return CodecReturnCode.SUCCESS;
				}

				if (data.Hint > RealHints.NOT_A_NUMBER || data.Hint == 31 || data.Hint == 32)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				iter._writer.WriteUByte(data.Hint);

				switch (data.Hint)
				{ 	
					case RealHints.INFINITY:
					case RealHints.NEG_INFINITY:
					case RealHints.NOT_A_NUMBER:
						// do nothing, just skip out
						break;
					default: // all other hints
					{
                            CodecReturnCode ret = iter._writer.WriteLong64ls(data.ToLong()); // checks if buffer too small
						if (ret != CodecReturnCode.SUCCESS)
						{
							return ret;
						}
					}
				break;
				}

				iter._curBufPos = iter._writer.Position();
				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeRealWithLength(EncodeIterator iter, Real data)
			{
				if (iter.IsIteratorOverrun(2)) // check if byte for length and byte for hint would fit
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				if (data.IsBlank)
				{
					PutLenSpecBlank(iter);
					return CodecReturnCode.SUCCESS;
				}

				if (data.Hint > RealHints.NOT_A_NUMBER || data.Hint == 31 || data.Hint == 32)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				switch (data.Hint)
				{ 
					case RealHints.INFINITY:
					case RealHints.NEG_INFINITY:
					case RealHints.NOT_A_NUMBER:
						iter._writer.WriteUByte(1);
						iter._writer.WriteUByte(data.Hint);
						break;
					default: // all other hints
					{
						int lenPos = iter._writer.Position();
						iter._writer.SkipBytes(2); // byte for length and byte for hint
						CodecReturnCode ret;
						if ((ret = iter._writer.WriteLong64ls(data.ToLong())) != CodecReturnCode.SUCCESS)
						{
							return ret;
						}
						int endPosition = iter._writer.Position();
						iter._writer.Position(lenPos); // length including hint
						iter._writer.WriteUByte(endPosition - lenPos - 1);
						iter._writer.WriteUByte(data.Hint);
						iter._writer.Position(endPosition);
					}
				break;
				}

				iter._curBufPos = iter._writer.Position();
				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeReal4(EncodeIterator iter, Real data)
			{
				if (data.ToLong() <= int.MaxValue)
				{
					if (iter.IsIteratorOverrun(5))
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}

					byte rwfHint;
					if (data.IsBlank)
					{
						rwfHint = 0x20;
					}
					else
					{
						if (data.Hint > RealHints.MAX_DIVISOR) // we don't want to write +/-Infinity, NaN yet
						{
							return CodecReturnCode.INVALID_DATA;
						}

						int value = (int)data.ToLong();
						byte length = 0;
						while (value != 0)
						{
							value = value >> 8;
							length++;
						}
						rwfHint = (byte)(((length - 1) << 6) | (byte)data.Hint);
					}
                    iter._writer._buffer.Write(rwfHint);
					iter._writer.WriteLong64ls(data.ToLong());

					iter._curBufPos = iter._writer.Position();

					return CodecReturnCode.SUCCESS;
				}
				else
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeReal8(EncodeIterator iter, Real data)
			{
				if (iter.IsIteratorOverrun(9))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				byte rwfHint;
				if (data.IsBlank)
				{
					rwfHint = 0x20;
				}
				else
				{
					if (data.Hint > RealHints.MAX_DIVISOR) // we don't want to write +/-Infinity, NaN yet
					{
						return CodecReturnCode.INVALID_DATA;
					}

					long value = data.ToLong();
					byte length = 0;
					while (value != 0)
					{
						value = value >> 8;
						length++;
					}
					rwfHint = (byte)((((length - 1) / 2) << 6) | (byte)data.Hint);
				}
                iter._writer._buffer.Write(rwfHint);
				iter._writer.WriteLong64ls(data.ToLong());

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeDate(EncodeIterator iter, Date data)
			{
				if (iter.IsIteratorOverrun(4))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDateWithLength(EncodeIterator iter, Date data)
			{
				if (iter.IsIteratorOverrun(5))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
                iter._writer._buffer.Write((byte)4);

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDate4(EncodeIterator iter, Date data)
			{
				return EncodeDate(iter, data);
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeTime(EncodeIterator iter, Time data)
			{
				int len;

				if (data.Nanosecond() != 0)
				{
					len = 8;
				}
				else if (data.Microsecond() != 0)
				{
					len = 7;
				}
				else if (data.Millisecond() != 0)
				{
					len = 5;
				}
				else if (data.Second() != 0)
				{
					len = 3;
				}
				else
				{
					len = 2;
				}

				if (iter.IsIteratorOverrun(len))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());

				switch (len)
				{
					case 2: // already done
						break;
					case 3:
                        iter._writer._buffer.Write((byte)data.Second());
						break;
					case 5:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						break;
					case 7:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(data.Microsecond());
						break;
					case 8:
					{
						byte tempNano = (byte)data.Nanosecond();
						short tempMicro = (short)(((data.Nanosecond() & 0xFF00) << 3) | data.Microsecond());
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(tempMicro);
                        iter._writer._buffer.Write(tempNano);
						break;
					}
					default:
						return CodecReturnCode.INVALID_DATA;
				}

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeTimeWithLength(EncodeIterator iter, Time data)
			{
				int len;

				if (data.Nanosecond() != 0)
				{
					len = 8;
				}
				else if (data.Microsecond() != 0)
				{
					len = 7;
				}
				else if (data.Millisecond() != 0)
				{
					len = 5;
				}
				else if (data.Second() != 0)
				{
					len = 3;
				}
				else
				{
					len = 2;
				}

				if (iter.IsIteratorOverrun(len + 1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
                iter._writer._buffer.Write((byte)len);

                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());

				switch (len)
				{
					case 2: // already done
						break;
					case 3:
                        iter._writer._buffer.Write((byte)data.Second());
						break;
					case 5:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						break;
					case 7:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(data.Microsecond());
						break;
					case 8:
					{
						byte tempNano = (byte)data.Nanosecond();
						short tempMicro = (short)(((data.Nanosecond() & 0xFF00) << 3) | data.Microsecond());
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(tempMicro);
                        iter._writer._buffer.Write(tempNano);
						break;
					}
					default:
						return CodecReturnCode.INVALID_DATA;
				}

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeTime3(EncodeIterator iter, Time data)
			{
				if (iter.IsIteratorOverrun(3))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeTime5(EncodeIterator iter, Time data)
			{
				if (iter.IsIteratorOverrun(5))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());
				iter._writer.WriteShort(data.Millisecond());

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeTime7(EncodeIterator iter, Time data)
			{
				if (iter.IsIteratorOverrun(7))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());
				iter._writer.WriteShort(data.Millisecond());
				iter._writer.WriteShort(data.Microsecond());

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeTime8(EncodeIterator iter, Time data)
			{
				if (iter.IsIteratorOverrun(8))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				byte tempNano = (byte)data.Nanosecond();
				short tempMicro = (short)(((data.Nanosecond() & 0xFF00) << 3) | data.Microsecond());
                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());
				iter._writer.WriteShort(data.Millisecond());
				iter._writer.WriteShort(tempMicro);
                iter._writer._buffer.Write(tempNano);

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeDateTime(EncodeIterator iter, DateTime data)
			{
				int len;

				if (data.Nanosecond() != 0)
				{
					len = 12;
				}
				else if (data.Microsecond() != 0)
				{
					len = 11;
				}
				else if (data.Millisecond() != 0)
				{
					len = 9;
				}
				else if (data.Second() != 0)
				{
					len = 7;
				}
				else
				{
					len = 6;
				}

				if (iter.IsIteratorOverrun(len))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());

				switch (len)
				{
					case 6: // already done
						break;
					case 7:
                        iter._writer._buffer.Write((byte)data.Second());
						break;
					case 9:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						break;
					case 11:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(data.Microsecond());
						break;
					case 12:
					{
						byte tempNano = (byte)data.Nanosecond();
						short tempMicro = (short)(((data.Nanosecond() & 0xFF00) << 3) | data.Microsecond());
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(tempMicro);
                        iter._writer._buffer.Write(tempNano);

					}
						break;
					default:
						return CodecReturnCode.INVALID_DATA;
				}

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDateTimeWithLength(EncodeIterator iter, DateTime data)
			{
				int len;

				if (data.Nanosecond() != 0)
				{
					len = 12;
				}
				else if (data.Microsecond() != 0)
				{
					len = 11;
				}
				else if (data.Millisecond() != 0)
				{
					len = 9;
				}
				else if (data.Second() != 0)
				{
					len = 7;
				}
				else
				{
					len = 6;
				}

				if (iter.IsIteratorOverrun(len + 1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
                iter._writer._buffer.Write((byte)len);

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());

				switch (len)
				{
					case 6: // already done
						break;
					case 7:
                        iter._writer._buffer.Write((byte)data.Second());
						break;
					case 9:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						break;
					case 11:
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(data.Microsecond());
						break;
					case 12:
					{
						byte tempNano = (byte)data.Nanosecond();
						short tempMicro = (short)(((data.Nanosecond() & 0xFF00) << 3) | data.Microsecond());
                        iter._writer._buffer.Write((byte)data.Second());
						iter._writer.WriteShort(data.Millisecond());
						iter._writer.WriteShort(tempMicro);
                        iter._writer._buffer.Write(tempNano);
						break;
					}
					default:
						return CodecReturnCode.INVALID_DATA;
				}

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDateTime7(EncodeIterator iter, DateTime data)
			{
                if (iter.IsIteratorOverrun(7))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDateTime9(EncodeIterator iter, DateTime data)
			{
				if (iter.IsIteratorOverrun(9))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());
				iter._writer.WriteShort(data.Millisecond());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDateTime11(EncodeIterator iter, DateTime data)
			{
				if (iter.IsIteratorOverrun(11))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

                iter._writer._buffer.Write((byte)data.Day());
                iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
                iter._writer._buffer.Write((byte)data.Hour());
                iter._writer._buffer.Write((byte)data.Minute());
                iter._writer._buffer.Write((byte)data.Second());
				iter._writer.WriteShort(data.Millisecond());
				iter._writer.WriteShort(data.Microsecond());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeDateTime12(EncodeIterator iter, DateTime data)
			{
				byte tempNano = (byte)data.Nanosecond();
				short tempMicro = (short)(((data.Nanosecond() & 0xFF00) << 3) | data.Microsecond());

				if (iter.IsIteratorOverrun(12))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer._buffer.Write((byte)data.Day());
				iter._writer._buffer.Write((byte)data.Month());
				iter._writer.WriteShort(data.Year());
				iter._writer._buffer.Write((byte)data.Hour());
				iter._writer._buffer.Write((byte)data.Minute());
				iter._writer._buffer.Write((byte)data.Second());
				iter._writer.WriteShort(data.Millisecond());
				iter._writer.WriteShort(tempMicro);
				iter._writer._buffer.Write((byte)tempNano);

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeQos(EncodeIterator iter, Qos data)
			{
				int dataLength = 1;
				int Qos;

				if (data.Timeliness() == QosTimeliness.UNSPECIFIED || data.Rate() == QosRates.UNSPECIFIED)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				dataLength += (data.Timeliness() > QosTimeliness.DELAYED_UNKNOWN) ? 2 : 0;
				dataLength += (data.Rate() > QosRates.JIT_CONFLATED) ? 2 : 0;

				if (iter.IsIteratorOverrun(dataLength))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				Qos = (data.Timeliness() << 5);
				Qos |= (data.Rate() << 1);
				Qos |= (data.IsDynamic ? 1 : 0);

				iter._writer._buffer.Write((byte)Qos);
				if (data.Timeliness() > QosTimeliness.DELAYED_UNKNOWN)
				{
					iter._writer.WriteShort(data.TimeInfo());
				}
				if (data.Rate() > QosRates.JIT_CONFLATED)
				{
					iter._writer.WriteShort(data.RateInfo());
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeQosWithLength(EncodeIterator iter, Qos data)
			{
				int dataLength = 1;
				int Qos;

				if (data.Timeliness() == QosTimeliness.UNSPECIFIED || data.Rate() == QosRates.UNSPECIFIED)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				dataLength += (data.Timeliness() > QosTimeliness.DELAYED_UNKNOWN) ? 2 : 0;
				dataLength += (data.Rate() > QosRates.JIT_CONFLATED) ? 2 : 0;

				if (iter.IsIteratorOverrun(dataLength + 1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}
				iter._writer._buffer.Write((byte)dataLength);

				Qos = (data.Timeliness() << 5);
				Qos |= (data.Rate() << 1);
				Qos |= (data.IsDynamic ? 1 : 0);

				iter._writer._buffer.Write((byte)Qos);
				if (data.Timeliness() > QosTimeliness.DELAYED_UNKNOWN)
				{
					iter._writer.WriteShort(data.TimeInfo());
				}
				if (data.Rate() > QosRates.JIT_CONFLATED)
				{
					iter._writer.WriteShort(data.RateInfo());
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeState(EncodeIterator iter, State data)
			{
				if (data.StreamState() == StreamStates.UNSPECIFIED)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				int State = (data.StreamState() << 3);
				State |= data.DataState();

				int len = data.Text().GetLength();

				// len value is written on wire as uShort15rb
				// If the value is smaller than 0x80, it is written on the wire as one byte,
				// otherwise, it is written as two bytes by calling writeUShort15rbLong method.
				// The code below checks if the buffer is sufficient for each case.
				if (len < 0x80)
				{
					if (iter.IsIteratorOverrun(3 + len)) // 1 byte state + 1 byte code + 1 byte len + len
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}
					iter._writer._buffer.Write((byte)State);
					iter._writer._buffer.Write((byte)data.Code());
					iter._writer._buffer.Write((byte)len);
					iter._writer.Write(data.Text());
				}
				else
				{
					if (iter.IsIteratorOverrun(4 + len)) // 2 bytes state + 1 byte code + 1 byte len + len
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}
					iter._writer._buffer.Write((byte)State);
					iter._writer._buffer.Write((byte)data.Code());
					iter._writer.WriteUShort15rbLong((short)len);
					iter._writer.Write(data.Text());
				}

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeStateWithLength(EncodeIterator iter, State data)
			{
				if (data.StreamState() == StreamStates.UNSPECIFIED)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				int State = (data.StreamState() << 3);
				State |= data.DataState();

				int len = data.Text().GetLength();

				// len value is written on wire as uShort15rb
				// If the value is smaller than 0x80, it is written on the wire as one byte,
				// otherwise, it is written as two bytes by calling writeUShort15rbLong method.
				// The code below checks if the buffer is sufficient for each case.
				if (len < 0x80)
				{
					if ((len + 1) < 0x80)
					{
						if (iter.IsIteratorOverrun(4 + len)) // 1 byte state len + 1 byte state + 1 byte code + 1 byte len + len
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer._buffer.Write((byte)(len + 3));
					}
					else
					{
						if (iter.IsIteratorOverrun(5 + len)) // 2 bytes state len + 1 byte state + 1 byte code + 1 byte len + len
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer.WriteUShort15rbLong((short)(len + 3));
					}
					iter._writer._buffer.Write((byte)State);
					iter._writer._buffer.Write((byte)data.Code());
					iter._writer._buffer.Write((byte)len);
					iter._writer.Write(data.Text());
				}
				else
				{
					if (iter.IsIteratorOverrun(6 + len)) // 2 byte state len + 1 byte state + 1 byte code + 2 bytes len + len
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}
					iter._writer.WriteUShort15rbLong((short)(len + 4));
					iter._writer._buffer.Write((byte)State);
					iter._writer._buffer.Write((byte)data.Code());
					iter._writer.WriteUShort15rbLong((short)len);
					iter._writer.Write(data.Text());
				}

				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public static CodecReturnCode EncodeEnum(EncodeIterator iter, Enum data)
			{
                CodecReturnCode ret = iter._writer.WriteUInt16ls(data.ToInt());
				if (ret != CodecReturnCode.SUCCESS)
				{
					return ret;
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeEnumWithLength(EncodeIterator iter, Enum data)
			{
                CodecReturnCode ret = iter._writer.WriteUInt16lsWithLength(data.ToInt());
				if (ret != CodecReturnCode.SUCCESS)
				{
					return ret;
				}
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeEnum1(EncodeIterator iter, Enum data)
			{
				if (data.ToInt() > 0xFF)
				{
					return CodecReturnCode.VALUE_OUT_OF_RANGE;
				}

				if (iter.IsIteratorOverrun(1))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer._buffer.Write((byte)data.ToInt());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}

            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            internal static CodecReturnCode EncodeEnum2(EncodeIterator iter, Enum data)
			{
				if (iter.IsIteratorOverrun(2))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.WriteShort(data.ToInt());
				iter._curBufPos = iter._writer.Position();

				return CodecReturnCode.SUCCESS;
			}
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeMsg(EncodeIterator iter, IMsg msg)
        {
            CodecReturnCode ret;
            int headerSize;
            EncodingLevel _levelInfo;

            Debug.Assert(null != msg, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");

            _levelInfo = iter._levelInfo[++iter._encodingLevel];
            if (iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }

            /* _initElemStartPos and encoding state should be the only two members used at a msg encoding level. */
            _levelInfo._initElemStartPos = iter._curBufPos;

            _levelInfo._listType = msg;

            /* header length */
            _levelInfo._countWritePos = iter._curBufPos;

            /* reset key and extended header reserved flags */
            ((Msg)msg)._extendedHeaderReserved = false;
            ((Msg)msg)._keyReserved = false;

            if ((ret = EncodeMsgInternal(iter, msg)) < 0)
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                return ret;
            }

            /* Encode end of the header */
            if ((ret = FinishMsgHeader(iter, msg)) < 0)
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                return ret;
            }

            /* Keys and or extendedHeaders have to be encoded */
            if (((Msg)msg)._keyReserved || ((Msg)msg)._extendedHeaderReserved)
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            /* Make sure NoData is handled properly */
            if ((msg.ContainerType == DataTypes.NO_DATA) && (msg.EncodedDataBody.GetLength() > 0))
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.UNEXPECTED_ENCODER_CALL;
            }

            /* write header size */
            headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
            iter._writer.Position(_levelInfo._countWritePos);
            iter._writer.WriteShort(headerSize);
            iter._writer.Position(iter._curBufPos);

            if (msg.EncodedDataBody.Length > 0)
            {
                if (iter.IsIteratorOverrun(msg.EncodedDataBody.Length))
                {
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                EncodeCopyData(iter, msg.EncodedDataBody);
            }

            iter._curBufPos = iter._writer.Position();

            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeMsgInit(EncodeIterator iter, IMsg msg, int dataMaxSize)
        {
            CodecReturnCode retVal;
            int headerSize;
            EncodingLevel _levelInfo;

            Debug.Assert(null != msg, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");

            _levelInfo = iter._levelInfo[++iter._encodingLevel];
            if (iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }

            _levelInfo._internalMark._sizePos = 0;
            _levelInfo._internalMark._sizeBytes = 0;
            _levelInfo._internalMark2._sizePos = 0;
            _levelInfo._internalMark2._sizeBytes = 0;

            /* _initElemStartPos and encoding state should be the only two members used at a msg encoding level. */
            _levelInfo._initElemStartPos = iter._curBufPos;

            /* Set the message onto the current _levelInfo */
            _levelInfo._listType = (object)msg;
            _levelInfo._containerType = DataTypes.MSG;

            /* header length */
            _levelInfo._countWritePos = iter._curBufPos;

            if ((retVal = EncodeMsgInternal(iter, msg)) < 0)
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return retVal;
            }

            if (((Msg)msg)._keyReserved && ((Msg)msg)._extendedHeaderReserved)
            {
                _levelInfo._encodingState = EncodeIteratorStates.OPAQUE_AND_EXTENDED_HEADER;
                return CodecReturnCode.ENCODE_MSG_KEY_ATTRIB;
            }
            else if (((Msg)msg)._keyReserved)
            {
                _levelInfo._encodingState = EncodeIteratorStates.OPAQUE;
                return CodecReturnCode.ENCODE_MSG_KEY_ATTRIB;
            }
            else if (((Msg)msg)._extendedHeaderReserved)
            {
                _levelInfo._encodingState = EncodeIteratorStates.EXTENDED_HEADER;
                return CodecReturnCode.ENCODE_EXTENDED_HEADER;
            }
            else
            {
                /* Header is finished, encode the end of the header */
                if ((retVal = FinishMsgHeader(iter, (IMsg)_levelInfo._listType)) != CodecReturnCode.SUCCESS)
                {
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return retVal;
                }

                /* write header size */
                headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteShort(headerSize);
                _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
                iter._writer.Position(iter._curBufPos);

                return CodecReturnCode.ENCODE_CONTAINER;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeMsgComplete(EncodeIterator iter, bool success)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (!success)
            {
                /* _initElemStartPos is at the start of the msg at this level.
				 * Typically this should be the same as iter.buffer.data. */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeMsgInternal(EncodeIterator iter, IMsg msg)
        {
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;

            /* ensure container type is valid */
            if (!(ValidAggregateDataType(msg.ContainerType)))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            /* make sure required elements can be encoded */
            /* msgClass (1) domainType(1) stream id (4) */
            if (iter.IsIteratorOverrun(8))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            iter._curBufPos += 2;
            iter._writer.SkipBytes(2);

            /* Store msgClass as UInt8 */
            iter._writer.WriteUByte(msg.MsgClass);
            iter._curBufPos = iter._writer.Position();

            /* Store domainType as UInt8 */
            iter._writer.WriteUByte(msg.DomainType);
            iter._curBufPos = iter._writer.Position();

            /* Store streamId as Int32 */
            iter._writer.WriteInt(msg.StreamId);
            iter._curBufPos = iter._writer.Position();

            /* IMPORTANT: When new message classes are added, CopyMsg and ValidateMsg have to modified as well */

            switch (msg.MsgClass)
            {
                case MsgClasses.UPDATE:
                    IUpdateMsg updateMsg = (IUpdateMsg)msg;
                    retVal = EncodeUpdateMsg(iter, updateMsg);
                    break;
                case MsgClasses.REFRESH:
                    IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                    retVal = EncodeRefreshMsg(iter, refreshMsg);
                    break;
                case MsgClasses.POST:
                    IPostMsg postMsg = (IPostMsg)msg;
                    retVal = EncodePostMsg(iter, postMsg);
                    break;
                case MsgClasses.REQUEST:
                    IRequestMsg requestMsg = (IRequestMsg)msg;
                    retVal = EncodeRequestMsg(iter, requestMsg);
                    break;
                case MsgClasses.CLOSE:
                    ICloseMsg closeMsg = (ICloseMsg)msg;
                    retVal = EncodeCloseMsg(iter, closeMsg);
                    break;
                case MsgClasses.STATUS:
                    IStatusMsg statusMsg = (IStatusMsg)msg;
                    retVal = EncodeStatusMsg(iter, statusMsg);
                    break;
                case MsgClasses.GENERIC:
                    IGenericMsg genericMsg = (IGenericMsg)msg;
                    retVal = EncodeGenericMsg(iter, genericMsg);
                    break;
                case MsgClasses.ACK:
                    IAckMsg ackMsg = (IAckMsg)msg;
                    retVal = EncodeAckMsg(iter, ackMsg);
                    break;
                default:
                    retVal = CodecReturnCode.INVALID_ARGUMENT;
                    break;
            }

            return retVal;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeUpdateMsg(EncodeIterator iter, IUpdateMsg msg)
        {

            /* Store update flags as UInt15 */
            int flags = msg.Flags;

            if (msg.CheckHasPermData() && msg.PermData.Length == 0)
            {
                flags &= ~UpdateMsgFlags.HAS_PERM_DATA;
            }
            if (msg.CheckHasConfInfo() && (msg.ConflationCount == 0 && msg.ConflationTime == 0))
            {
                flags &= ~UpdateMsgFlags.HAS_CONF_INFO;
            }

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(3)) // 1 byte flags + 1 byte container type + 1 byte update type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(4)) // 2 bytes flags + 1 byte container type + 1 byte update type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)flags);
            }

            iter._curBufPos = iter._writer.Position();

            /* Store containerType as UInt8 */
            /* container type needs to be scaled before encoding */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            /* Store update type as UInt8 */
            iter._writer._buffer.Write((byte)msg.UpdateType);
            iter._curBufPos = iter._writer.Position();

            if ((flags & UpdateMsgFlags.HAS_SEQ_NUM) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._writer.WriteUInt(msg.SeqNum);
                iter._curBufPos = iter._writer.Position();
            }

            if ((flags & UpdateMsgFlags.HAS_CONF_INFO) > 0)
            {
                if (msg.ConflationCount < 0x80)
                {
                    if (iter.IsIteratorOverrun(3))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)msg.ConflationCount);
                }
                else
                {
                    if (iter.IsIteratorOverrun(4))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)msg.ConflationCount);
                }
                iter._writer.WriteShort(msg.ConflationTime);
                iter._curBufPos = iter._writer.Position();
            }

            /* Store Perm info */
            if ((flags & UpdateMsgFlags.HAS_PERM_DATA) > 0)
            {
                int len = msg.PermData.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(msg.PermData);
                }
                else
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(msg.PermData);
                }
                iter._curBufPos = iter._writer.Position();
            }

            int keyAndExtHeader = 0;
            if ((flags & UpdateMsgFlags.HAS_MSG_KEY) > 0)
            {
                keyAndExtHeader = HAS_MSG_KEY;
            }
            if ((flags & UpdateMsgFlags.HAS_EXTENDED_HEADER) > 0)
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeRefreshMsg(EncodeIterator iter, IRefreshMsg msg)
        {
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;
            /* Store refresh flags as UInt16, cleaning perm flag if necessary */
            int flags = msg.Flags;

            if (msg.CheckHasPermData() && msg.PermData.Length == 0)
            {
                flags &= ~RefreshMsgFlags.HAS_PERM_DATA;
            }

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)flags);
            }

            /* Store containerType as UInt8 */
            /* container type needs to be scaled */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._writer.WriteUInt(msg.SeqNum);
                iter._curBufPos = iter._writer.Position();
            }

            if ((retVal = PrimitiveEncoder.EncodeState(iter, msg.State)) < 0)
            {
                return retVal;
            }

            /* Store groupId as small buffer */
            if (msg.GroupId.Length > 0)
            {
                if (iter.IsIteratorOverrun(msg.GroupId.Length + 1))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                EncodeBuffer8(iter, msg.GroupId);
                iter._curBufPos = iter._writer.Position();
            }
            else
            {
                if (iter.IsIteratorOverrun(3))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                /* No group Id - store 0 in the format used by group mechanism */
                /* length is 2 */
                /* write length */
                iter._writer._buffer.Write((byte)2);
                /* now write value */
                int groupId = 0;
                iter._writer.WriteShort(groupId);
                iter._curBufPos = iter._writer.Position();
            }

            if ((flags & RefreshMsgFlags.HAS_PERM_DATA) > 0)
            {
                int len = msg.PermData.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(msg.PermData);
                }
                else
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(msg.PermData);
                }

                iter._curBufPos = iter._writer.Position();
            }

            /* Store QoS */
            if ((flags & RefreshMsgFlags.HAS_QOS) > 0)
            {
                if ((retVal = PrimitiveEncoder.EncodeQos(iter, msg.Qos)) < 0)
                {
                    return retVal;
                }
            }

            int keyAndExtHeader = 0;
            if ((flags & RefreshMsgFlags.HAS_MSG_KEY) > 0)
            {
                keyAndExtHeader = HAS_MSG_KEY;
            }
            if ((flags & RefreshMsgFlags.HAS_EXTENDED_HEADER) > 0)
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodePostMsg(EncodeIterator iter, IPostMsg msg)
        {
            int flags = msg.Flags;

            if (msg.CheckHasPermData() && msg.PermData.Length == 0)
            {
                flags &= ~PostMsgFlags.HAS_PERM_DATA;
            }

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(10)) // 1 byte flags + 1 byte container type + 4 bytes user address + 4 bytes user Id
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(11)) // 2 bytes flags + 1 byte container type + 4 bytes user address + 4 bytes user Id
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)flags);
            }

            /* Store containerType as UInt8 */
            /* container type needs to be scaled */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            /* Put User Address */
            iter._writer.WriteUInt(msg.PostUserInfo.UserAddr);
            iter._curBufPos = iter._writer.Position();

            /* Put User ID */
            iter._writer.WriteUInt(msg.PostUserInfo.UserId);
            iter._curBufPos = iter._writer.Position();

            if ((flags & PostMsgFlags.HAS_SEQ_NUM) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._writer.WriteUInt(msg.SeqNum);
                iter._curBufPos = iter._writer.Position();
            }

            if ((flags & PostMsgFlags.HAS_POST_ID) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._writer.WriteUInt(msg.PostId);
                iter._curBufPos = iter._writer.Position();
            }

            if ((flags & PostMsgFlags.HAS_PERM_DATA) > 0)
            {
                int len = msg.PermData.GetLength();

                // len value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(msg.PermData);
                }
                else
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(msg.PermData);
                }

                iter._curBufPos = iter._writer.Position();
            }

            int keyAndExtHeader = 0;
            if ((flags & PostMsgFlags.HAS_MSG_KEY) > 0)
            {
                keyAndExtHeader = HAS_MSG_KEY;
            }
            if ((flags & PostMsgFlags.HAS_EXTENDED_HEADER) > 0)
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeRequestMsg(EncodeIterator iter, IRequestMsg msg)
        {
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;

            if (msg.Flags < 0x80)
            {
                if (iter.IsIteratorOverrun(2))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)msg.Flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(3))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)msg.Flags);
            }

            /* Store containerType as UInt8 */
            /* container type needs to be scaled */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            if (msg.CheckHasPriority())
            {

                // count value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (msg.Priority.Count < 0xFE)
                {
                    if (iter.IsIteratorOverrun(2)) // 1 byte priority class + 1 byte count
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUByte(msg.Priority.PriorityClass);
                    iter._writer._buffer.Write((byte)msg.Priority.Count);
                }
                else
                {
                    if (iter.IsIteratorOverrun(4)) // 1 byte priority class + 3 bytes count
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUByte(msg.Priority.PriorityClass);
                    iter._writer.WriteUShort16obLong(msg.Priority.Count);

                }
                iter._curBufPos = iter._writer.Position();
            }

            /* Store Qos */
            if (msg.CheckHasQos())
            {
                if ((retVal = PrimitiveEncoder.EncodeQos(iter, msg.Qos)) < 0)
                {
                    return retVal;
                }
            }

            /* Store WorstQos */
            if (msg.CheckHasWorstQos())
            {
                if ((retVal = PrimitiveEncoder.EncodeQos(iter, msg.WorstQos)) < 0)
                {
                    return retVal;
                }
            }

            int keyAndExtHeader = HAS_MSG_KEY;
            if (msg.CheckHasExtendedHdr())
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeCloseMsg(EncodeIterator iter, ICloseMsg msg)
        {
            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (msg.Flags < 0x80)
            {
                if (iter.IsIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)msg.Flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)msg.Flags);
            }
            iter._curBufPos = iter._writer.Position();

            /* container type needs to be scaled */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            if (msg.CheckHasExtendedHdr())
            {
                if (msg.ExtendedHeader.Length > 0)
                {
                    /* now put data header there */
                    if (iter.IsIteratorOverrun(msg.ExtendedHeader.Length + 1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    EncodeBuffer8(iter, msg.ExtendedHeader);
                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    if (iter.IsIteratorOverrun(1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    ((Msg)msg)._extendedHeaderReserved = true;
                    /* must reserve space now */
                    iter._levelInfo[iter._encodingLevel]._internalMark._sizePos = iter._curBufPos;
                    iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 1;
                    iter._curBufPos++; // move pointer */
                    iter._writer.Position(iter._curBufPos);
                }
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeStatusMsg(EncodeIterator iter, IStatusMsg msg)
        {
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;
            /* Store flags as UInt16, cleaning perm flag if necessary */
            int flags = msg.Flags;

            if (msg.CheckHasPermData() && msg.PermData.Length == 0)
            {
                flags &= ~StatusMsgFlags.HAS_PERM_DATA;
            }

            if (msg.CheckHasGroupId() && (msg.GroupId.Length == 0))
            {
                flags &= ~StatusMsgFlags.HAS_GROUP_ID;
            }

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)flags);
            }

            /* Store containerType as UInt8 */
            /* container type needs to be scaled */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            /* Store state */
            if ((flags & StatusMsgFlags.HAS_STATE) > 0)
            {
                if ((retVal = PrimitiveEncoder.EncodeState(iter, msg.State)) < 0)
                {
                    return retVal;
                }
            }

            /* Store groupId as small buffer */
            if ((flags & StatusMsgFlags.HAS_GROUP_ID) > 0)
            {
                if (iter.IsIteratorOverrun(msg.GroupId.Length + 1))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                EncodeBuffer8(iter, msg.GroupId);
                iter._curBufPos = iter._writer.Position();
            }

            /* Store Permission info */
            if ((flags & StatusMsgFlags.HAS_PERM_DATA) > 0)
            {
                int len = msg.PermData.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(msg.PermData);
                }
                else
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(msg.PermData);
                }

                iter._curBufPos = iter._writer.Position();
            }

            int keyAndExtHeader = 0;
            if ((flags & StatusMsgFlags.HAS_MSG_KEY) > 0)
            {
                keyAndExtHeader = HAS_MSG_KEY;
            }
            if ((flags & StatusMsgFlags.HAS_EXTENDED_HEADER) > 0)
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeGenericMsg(EncodeIterator iter, IGenericMsg msg)
        {
            /* Store update flags as UInt15 */
            int flags = msg.Flags;

            if (msg.CheckHasPermData() && msg.PermData.Length == 0)
            {
                flags &= ~GenericMsgFlags.HAS_PERM_DATA;
            }

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(2)) // 1 byte flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(3)) // 2 bytes flags + 1 byte container type
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)flags);
            }

            /* Store containerType as UInt8 */
            /* container type needs to be scaled before encoding */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            if ((flags & GenericMsgFlags.HAS_SEQ_NUM) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUInt(msg.SeqNum);
                iter._curBufPos = iter._writer.Position();
            }

            if ((flags & GenericMsgFlags.HAS_SECONDARY_SEQ_NUM) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUInt(msg.SecondarySeqNum);
                iter._curBufPos = iter._writer.Position();
            }

            /* Store Perm info */
            if ((flags & GenericMsgFlags.HAS_PERM_DATA) > 0)
            {
                int len = msg.PermData.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(msg.PermData);
                }
                else
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(msg.PermData);
                }

                iter._curBufPos = iter._writer.Position();
            }

            int keyAndExtHeader = 0;
            if ((flags & GenericMsgFlags.HAS_MSG_KEY) > 0)
            {
                keyAndExtHeader = HAS_MSG_KEY;
            }
            if ((flags & GenericMsgFlags.HAS_EXTENDED_HEADER) > 0)
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeAckMsg(EncodeIterator iter, IAckMsg msg)
        {
            int flags = msg.Flags;

            if (msg.CheckHasNakCode() && msg.NakCode == NakCodes.NONE)
            {
                flags &= ~AckMsgFlags.HAS_NAK_CODE;
            }

            if (msg.CheckHasText() && msg.Text.Length == 0)
            {
                flags &= ~AckMsgFlags.HAS_TEXT;
            }

            /* Store flags flags as UInt15 */
            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(6)) // 1 byte flags + 1 byte container type + 4 bytes ack id
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(7)) // 2 bytes flags + 1 byte container type + 4 bytes ack id
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)flags);
            }

            /* Store containerType as UInt8 */
            /* container type needs to be scaled before encoding */
            iter._writer.WriteUByte(msg.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            /* Store ackId as UInt32 */
            iter._writer.WriteUInt(msg.AckId);

            if ((flags & AckMsgFlags.HAS_NAK_CODE) > 0)
            {
                if (iter.IsIteratorOverrun(1))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                /* Store nakCode as UInt8 */
                iter._writer.WriteUByte(msg.NakCode);
            }

            if ((flags & AckMsgFlags.HAS_TEXT) > 0)
            {
                /* Store text as a string */
                int len = msg.Text.GetLength();

                // len value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0xFE)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(msg.Text);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUShort16obLong(len);
                    iter._writer.Write(msg.Text);
                }
                else
                {
                    return CodecReturnCode.INVALID_DATA;
                }
            }

            if ((flags & AckMsgFlags.HAS_SEQ_NUM) > 0)
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._writer.WriteInt((int)msg.SeqNum);
            }
            iter._curBufPos = iter._writer.Position();

            int keyAndExtHeader = 0;
            if ((flags & AckMsgFlags.HAS_MSG_KEY) > 0)
            {
                keyAndExtHeader = HAS_MSG_KEY;
            }
            if ((flags & AckMsgFlags.HAS_EXTENDED_HEADER) > 0)
            {
                keyAndExtHeader = keyAndExtHeader + HAS_EXT_HEADER;
            }

            return EncodeKeyAndExtHeader(iter, msg, keyAndExtHeader);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeKeyAndExtHeader(EncodeIterator iter, IMsg msg, int flags)
        {
            CodecReturnCode retVal = CodecReturnCode.SUCCESS;

            if ((flags & HAS_MSG_KEY) > 0)
            {
                /* save position for storing key size */
                int lenPos = iter._curBufPos;

                if ((retVal = EncodeKeyInternal(iter, (MsgKey)msg.MsgKey)) < 0)
                {
                    return retVal;
                }

                /* Store opaque as SmallBuffer */
                if (msg.MsgKey.CheckHasAttrib())
                {
                    /* write opaque data format and save length position */
                    if (iter.IsIteratorOverrun(1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    if (!(ValidAggregateDataType(msg.MsgKey.AttribContainerType)))
                    {
                        return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
                    }

                    /* opaque container type needs to be scaled before encoding */
                    iter._writer.WriteUByte(msg.MsgKey.AttribContainerType - DataTypes.CONTAINER_TYPE_MIN);
                    iter._curBufPos = iter._writer.Position();

                    /* if we have a key opaque here, put it on the wire */
                    if (msg.MsgKey.EncodedAttrib.Length > 0)
                    {
                        int len = msg.MsgKey.EncodedAttrib.GetLength();

                        // len value is written on wire as uShort15rb
                        // If the value is smaller than 0x80, it is written on the wire as one byte,
                        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                        // The code below checks if the buffer is sufficient for each case.
                        if (len < 0x80)
                        {
                            if (iter.IsIteratorOverrun(1 + len))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer._buffer.Write((byte)len);
                            iter._writer.Write(msg.MsgKey.EncodedAttrib);
                        }
                        else
                        {
                            if (iter.IsIteratorOverrun(2 + len))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer.WriteUShort15rbLong((short)len);
                            iter._writer.Write(msg.MsgKey.EncodedAttrib);
                        }

                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        /* opaque needs to be encoded */
                        /* save U15 mark */
                        if (msg.MsgKey.AttribContainerType != DataTypes.NO_DATA)
                        {
                            if (iter.IsIteratorOverrun(2))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._curBufPos = SetupU15Mark(iter._levelInfo[iter._encodingLevel]._internalMark2, 0, iter._curBufPos);
                            iter._writer.Position(iter._curBufPos);
                        }
                        else
                        {
                            iter._levelInfo[iter._encodingLevel]._internalMark2._sizeBytes = 0;
                            iter._levelInfo[iter._encodingLevel]._internalMark2._sizePos = iter._curBufPos;
                        }
                        ((Msg)msg)._keyReserved = true;
                    }
                }

                /* user is done with key and there is no opaque they still need to write */
                if (!((Msg)msg)._keyReserved)
                {
                    /* write key length */
                    /* now store the size - have to trick it into being an RB15 */
                    /* only want the encoded size of the key */
                    int keySize = (iter._curBufPos - lenPos - 2);
                    /* now set the RB bit */
                    keySize |= 0x8000;
                    /* store it - don't need to increment iterator because its already at end of key */
                    iter._writer.Position(lenPos);
                    iter._writer.WriteShort(keySize);
                    iter._writer.Position(iter._curBufPos);
                }
                else
                {
                    /* user still has to encode key opaque */
                    /* store this in the internalMark to fill in later */
                    iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 2;
                    iter._levelInfo[iter._encodingLevel]._internalMark._sizePos = lenPos;
                }
            }

            if ((flags & HAS_EXT_HEADER) > 0)
            {
                /* user passed it in, lets encode it now */
                if (((Msg)msg)._keyReserved)
                {
                    /* User set flag to indicate there is an extended header but they didn't encode their opaque yet. */
                    /* Set us up to expect opaque. */
                    ((Msg)msg)._extendedHeaderReserved = true;
                }
                else
                {
                    /* no key opaque - if the extended header is here, put it on the wire,
					 * otherwise set it up so we expect extended header */
                    if (msg.ExtendedHeader.Length > 0)
                    {
                        if (iter.IsIteratorOverrun(msg.ExtendedHeader.Length + 1))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        EncodeBuffer8(iter, msg.ExtendedHeader);
                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(1))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        ((Msg)msg)._extendedHeaderReserved = true;
                        /* must reserve space now */
                        iter._levelInfo[iter._encodingLevel]._internalMark._sizePos = iter._curBufPos;
                        iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 1;
                        iter._curBufPos++; // move pointer */
                        iter._writer.Position(iter._curBufPos);
                    }
                }
            }
            return retVal;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeKeyInternal(EncodeIterator iter, MsgKey key)
        {
            int flags;

            if (iter.IsIteratorOverrun(2))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            /* Store flags as UInt15-rb */
            flags = ((MsgKey)key).Flags;

            if (key.CheckHasName() && key.Name.Length == 0)
            {
                flags &= ~MsgKeyFlags.HAS_NAME;
            }

            if (!key.CheckHasName() && key.CheckHasNameType())
            {
                flags &= ~MsgKeyFlags.HAS_NAME_TYPE;
            }

            // flags value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (flags < 0x80)
            {
                if (iter.IsIteratorOverrun(3)) // 2 bytes skip + 1 byte flags
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._curBufPos += 2;
                iter._writer.SkipBytes(2);
                iter._writer._buffer.Write((byte)flags);
            }
            else
            {
                if (iter.IsIteratorOverrun(4)) // 2 bytes skip + 2 bytes flags
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._curBufPos += 2;
                iter._writer.SkipBytes(2);
                iter._writer.WriteUShort15rbLong((short)flags);
            }
            iter._curBufPos = iter._writer.Position();

            /* Store SourceId as UINt16_ob */
            if (key.CheckHasServiceId())
            {
                // ID value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (key.ServiceId < 0xFE)
                {
                    if (iter.IsIteratorOverrun(1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)key.ServiceId);
                }
                else
                {
                    if (iter.IsIteratorOverrun(3))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUShort16obLong(key.ServiceId);

                }
                iter._curBufPos = iter._writer.Position();
            }

            /* Store name as CharPtr == SmallBuffer */
            if (key.CheckHasName())
            {
                /* verify name length is only 1 byte */
                if (key.Name.Length > 255)
                {
                    return CodecReturnCode.INVALID_ARGUMENT;
                }

                if (iter.IsIteratorOverrun(key.Name.Length + 1))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                EncodeBuffer8(iter, key.Name);
                iter._curBufPos = iter._writer.Position();

                /* should only do things with NameType if we have name */
                if (key.CheckHasNameType())
                {
                    if (iter.IsIteratorOverrun(1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)key.NameType);
                    iter._curBufPos = iter._writer.Position();
                }
            }

            /* Store Filter as UInt32 */
            if (key.CheckHasFilter())
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUInt(key.Filter);
                iter._curBufPos = iter._writer.Position();
            }

            /* Store Identifier as UInt32 */
            if (key.CheckHasIdentifier())
            {
                if (iter.IsIteratorOverrun(4))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteInt(key.Identifier);
                iter._curBufPos = iter._writer.Position();
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMsgKeyAttribComplete(EncodeIterator iter, bool success)
        {
            CodecReturnCode ret;
            int headerSize;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                if (_levelInfo._internalMark2._sizeBytes > 0)
                {
                    if ((ret = FinishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                    {
                        /* rollback */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        iter._curBufPos = _levelInfo._internalMark2._sizePos;
                        iter._writer.Position(iter._curBufPos);
                        return ret;
                    }
                }
                else
                {
                    /* no opaque was encoded - failure */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return CodecReturnCode.FAILURE;
                }

                /* write key length into buffer */
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    if ((ret = FinishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                    {
                        /* roll back */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        iter._curBufPos = _levelInfo._internalMark._sizePos;
                        iter._writer.Position(iter._curBufPos);
                        return ret;
                    }
                }
                else
                {
                    /* no key was attempted to be encoded - failure */
                    /* go to start of buffer */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return CodecReturnCode.FAILURE;
                }

                /* if they still need to encode extended header do that */
                if (_levelInfo._encodingState == EncodeIteratorStates.OPAQUE_AND_EXTENDED_HEADER)
                {
                    IMsg msg = (IMsg)_levelInfo._listType;
                    Buffer extHdr;
                    Debug.Assert(null != msg, "Invalid parameters or parameters passed in as NULL");
                    /* see if the extended header was pre-encoded */
                    /* if we can, write it, if not reserve space */

                    extHdr = msg.ExtendedHeader;

                    if (extHdr != null && extHdr.GetLength() > 0)
                    {
                        /* we can encode this here and now */
                        if (iter.IsIteratorOverrun(extHdr.Length + 1))
                        {
                            /* rollback */
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            iter._curBufPos = _levelInfo._initElemStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        EncodeBuffer8(iter, extHdr);
                        iter._curBufPos = iter._writer.Position();

                        /* Header is finished, encode the end of the header */
                        if ((ret = FinishMsgHeader(iter, msg)) != CodecReturnCode.SUCCESS)
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            iter._curBufPos = _levelInfo._initElemStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return ret;
                        }

                        /* write header size */
                        headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
                        iter._writer.Position(_levelInfo._countWritePos);
                        iter._writer.WriteShort(headerSize);
                        iter._writer.Position(iter._curBufPos);

                        _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

                        ret = CodecReturnCode.ENCODE_CONTAINER;
                    }
                    else
                    {
                        /* it will be encoded after this, reserve space for size */

                        if (iter.IsIteratorOverrun(1))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            iter._curBufPos = _levelInfo._initElemStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        /* must reserve space now */
                        _levelInfo._internalMark._sizePos = iter._curBufPos;
                        _levelInfo._internalMark._sizeBytes = 1;
                        iter._curBufPos++; // move pointer */
                        iter._writer.Position(iter._curBufPos);

                        _levelInfo._encodingState = EncodeIteratorStates.EXTENDED_HEADER;

                        ret = CodecReturnCode.ENCODE_EXTENDED_HEADER;
                    }
                }
                else
                {
                    /* Header is finished, encode the end of the header */
                    if ((ret = FinishMsgHeader(iter, (IMsg)(_levelInfo._listType))) != CodecReturnCode.SUCCESS)
                    {
                        /* rollback */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return ret;
                    }

                    /* write header size */
                    headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
                    iter._writer.Position(_levelInfo._countWritePos);
                    iter._writer.WriteShort(headerSize);
                    iter._writer.Position(iter._curBufPos);

                    _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

                    /* now store current location so we can check it to ensure user did not put data
                    * when they shouldnt */
                    /* count has been filled in already */
                    _levelInfo._countWritePos = iter._curBufPos;

                    if (((IMsg)(_levelInfo._listType)).ContainerType != DataTypes.NO_DATA)
                        ret = CodecReturnCode.ENCODE_CONTAINER;
                    else
                        ret = CodecReturnCode.SUCCESS;

                }
            }
            else
            {
                /* roll back and change state */
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);

                ret = CodecReturnCode.SUCCESS;
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeExtendedHeaderComplete(EncodeIterator iter, bool success)
        {
            CodecReturnCode ret;
            int size;
            int headerSize;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
            IMsg msg = (IMsg)_levelInfo._listType;

            /* Validations */
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.EXTENDED_HEADER, "Unexpected encoding attempted");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    /* write one byte length onto wire */
                    size = (iter._curBufPos - _levelInfo._internalMark._sizePos - _levelInfo._internalMark._sizeBytes);
                    iter._writer.Position(_levelInfo._internalMark._sizePos);
                    iter._writer.WriteUByte(size);
                    iter._writer.Position(iter._curBufPos);
                }
                else
                {
                    /* extended header shouldn't have been encoded */
                    /* roll back and change state */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                }

                /* Extended Header is finished, encode the end of the header */
                if ((ret = FinishMsgHeader(iter, msg)) != CodecReturnCode.SUCCESS)
                {
                    /* roll back and change state */
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return ret;
                }

                /* write header size */
                headerSize = iter._curBufPos - _levelInfo._countWritePos - 2;
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteShort(headerSize);
                iter._writer.Position(iter._curBufPos);

                /* should be ready to encode entries - don't save length for data */
                _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

                if (msg.ContainerType != DataTypes.NO_DATA)
                {
                    return CodecReturnCode.ENCODE_CONTAINER;
                }
                else
                {
                    return CodecReturnCode.SUCCESS;
                }
            }
            else
            {
                /* roll back and change state */
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void EncodeBuffer8(EncodeIterator iter, Buffer buffer)
		{
			int tlen = buffer.Length;
			iter._writer.WriteUByte(tlen);
			iter._writer.Write(buffer);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void EncodeBuffer15(EncodeIterator iter, Buffer buffer)
		{
			int tlen = buffer.Length;
			if (tlen < 0x80)
			{
				iter._writer._buffer.Write((byte)tlen);
			}
			else
			{
				iter._writer.WriteUShort15rbLong((short)tlen);
			}
			iter._writer.Write(buffer);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void EncodeBuffer32(EncodeIterator iter, Buffer buffer)
		{
			int tlen = buffer.Length;
			iter._writer.WriteUInt32ob(tlen);
			iter._writer.Write(buffer);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static int SetupU15Mark(EncodeSizeMark mark, int maxSize, int position)
		{
			Debug.Assert(0 != position, "Invalid encoding attempted");
			Debug.Assert(null != mark && mark._sizePos == 0, "Invalid encoding attempted");

			mark._sizePos = position;

			if ((maxSize == 0) || (maxSize >= 0x80))
			{
				mark._sizeBytes = 2;
				position += 2;
			}
			else
			{
				mark._sizeBytes = 1;
				position += 1;
			}

			return position;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static int SetupU16Mark(EncodeSizeMark mark, int maxSize, int position)
		{
			Debug.Assert(position != 0, "Invalid encoding attempted");
			Debug.Assert(null != mark && mark._sizePos == 0, "Invalid encoding attempted");

			mark._sizePos = position;

			if ((maxSize == 0) || (maxSize >= 0xFE))
			{
				mark._sizeBytes = 3;
				position += 3;
			}
			else
			{
				mark._sizeBytes = 1;
				position += 1;
			}

			return position;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode FinishMsgHeader(EncodeIterator iter, IMsg msg)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.UPDATE:
                    IUpdateMsg updateMsg = (IUpdateMsg)msg;
                    if (updateMsg.CheckHasPostUserInfo())
                    {
                        if (iter.IsIteratorOverrun(8))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.WriteUInt(updateMsg.PostUserInfo.UserAddr);
                        iter._writer.WriteUInt(updateMsg.PostUserInfo.UserId);
                        iter._curBufPos = iter._writer.Position();
                    }
                    break;
                case MsgClasses.REFRESH:
                    IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                    if (refreshMsg.CheckHasPostUserInfo())
                    {
                        if (iter.IsIteratorOverrun(8))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.WriteUInt(refreshMsg.PostUserInfo.UserAddr);
                        iter._writer.WriteUInt(refreshMsg.PostUserInfo.UserId);
                        iter._curBufPos = iter._writer.Position();
                    }
                    if (refreshMsg.CheckHasPartNum())
                    {
                        // flags value is written on wire as uShort15rb
                        // If the value is smaller than 0x80, it is written on the wire as one byte,
                        // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                        // The code below checks if the buffer is sufficient for each case.
                        if (refreshMsg.PartNum < 0x80)
                        {
                            if (iter.IsIteratorOverrun(1))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer._buffer.Write((byte)refreshMsg.PartNum);
                        }
                        else
                        {
                            if (iter.IsIteratorOverrun(2))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer.WriteUShort15rbLong((short)refreshMsg.PartNum);
                        }
                    }
                    iter._curBufPos = iter._writer.Position();
                    break;
                case MsgClasses.STATUS:
                    IStatusMsg statusMsg = (IStatusMsg)msg;
                    if (statusMsg.CheckHasPostUserInfo())
                    {
                        if (iter.IsIteratorOverrun(8))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUInt(statusMsg.PostUserInfo.UserAddr);
                        iter._writer.WriteUInt(statusMsg.PostUserInfo.UserId);
                        iter._curBufPos = iter._writer.Position();
                    }
                    break;
                case MsgClasses.GENERIC:
                    IGenericMsg genericMsg = (IGenericMsg)msg;
                    if (genericMsg.CheckHasPartNum())
                    {
                        if (genericMsg.PartNum < 0x80)
                        {
                            if (iter.IsIteratorOverrun(1))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer._buffer.Write((byte)genericMsg.PartNum);
                        }
                        else
                        {
                            if (iter.IsIteratorOverrun(2))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer.WriteUShort15rbLong((short)genericMsg.PartNum);
                        }
                    }
                    iter._curBufPos = iter._writer.Position();
                    break;
                case MsgClasses.POST:
                    IPostMsg postMsg = (IPostMsg)msg;
                    if (postMsg.CheckHasPartNum())
                    {
                        if (postMsg.PartNum < 0x80)
                        {
                            if (iter.IsIteratorOverrun(1))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer._buffer.Write((byte)postMsg.PartNum);
                        }
                        else
                        {
                            if (iter.IsIteratorOverrun(2))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer.WriteUShort15rbLong((short)postMsg.PartNum);
                        }
                    }
                    if (postMsg.CheckHasPostUserRights())
                    {
                        if (postMsg.PostUserRights < 0x80)
                        {
                            if (iter.IsIteratorOverrun(1))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer._buffer.Write((byte)postMsg.PostUserRights);
                        }
                        else
                        {
                            if (iter.IsIteratorOverrun(2))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer.WriteUShort15rbLong((short)postMsg.PostUserRights);
                        }
                    }
                    iter._curBufPos = iter._writer.Position();
                    break;
                case MsgClasses.REQUEST:
                case MsgClasses.CLOSE:
                case MsgClasses.ACK:
                    break;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeElementListInit(EncodeIterator iter, ElementList elementList, LocalElementSetDefDb setDb, int setEncodingMaxSize)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != elementList, "Invalid elementListInt in as NULL");

            EncodingLevel levelInfo;
            int setId;

            if (++iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }

            levelInfo = iter._levelInfo[iter._encodingLevel];
            levelInfo.Init(DataTypes.ELEMENT_LIST, EncodeIteratorStates.NONE, elementList, iter._curBufPos);

            /* make sure required elements can be encoded */
            if (iter.IsIteratorOverrun(1))
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            /* store flags as UInt8 */
            iter._writer.WriteUByte((int)elementList.Flags);
            iter._curBufPos = iter._writer.Position();

            /* check for List Info */
            if (elementList.CheckHasInfo())
            {
                int infoLen;
                int startPos;

                /* make sure that required elements can be encoded */
                if (iter.IsIteratorOverrun(3))
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                /* save info length position */
                startPos = iter._curBufPos;

                /* move past it */
                iter._curBufPos += 1;
                iter._writer.SkipBytes(1);

                /* put element list number into element list */
                iter._writer.WriteShort(elementList.ElementListNum);
                iter._curBufPos = iter._writer.Position();

                /* encode the length into the element list */
                infoLen = iter._curBufPos - startPos - 1;
                iter._writer.Position(startPos);
                iter._writer.WriteUByte(infoLen);
                iter._writer.Position(iter._curBufPos);
            }

            /* check for set data */
            if (elementList.CheckHasSetData())
            {
                /* we have element list set data */
                /* make sure set id and set length can be encoded */
                /* setID (2), setData (2) */
                if (iter.IsIteratorOverrun(4))
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                /* check for/encode optional element list set id */
                if (elementList.CheckHasSetId())
                {

                    // ID value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (elementList.SetId < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)elementList.SetId);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)elementList.SetId);
                    }

                    iter._curBufPos = iter._writer.Position();
                    setId = elementList.SetId;
                }
                else
                {
                    setId = 0;
                }

                if (setDb != null && setId <= LocalElementSetDefDb.MAX_LOCAL_ID && setDb.Definitions[setId].SetId != LocalElementSetDefDb.BLANK_ID)
                {
                    levelInfo._elemListSetDef = setDb.Definitions[setId];
                }
                else if (iter._elementSetDefDb != null && setId <= iter._elementSetDefDb.MaxSetId && iter._elementSetDefDb.Definitions[setId] != null)
                {
                    levelInfo._elemListSetDef = (ElementSetDef)iter._elementSetDefDb.Definitions[setId];
                }

                /* check for element list data after the set data */
                if (elementList.CheckHasStandardData())
                {
                    /* if have one set data and field list data, need length in front of the set */
                    if (elementList._encodedSetData.Length > 0)
                    {
                        /* the set data is already encoded */
                        /* make sure that the set data can be encoded */
                        int len = elementList._encodedSetData.GetLength();
                        if (len < 0x80)
                        {
                            if (iter.IsIteratorOverrun(3 + len))
                            {
                                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer._buffer.Write((byte)len);
                            iter._writer.Write(elementList._encodedSetData);
                        }
                        else
                        {
                            if (iter.IsIteratorOverrun(4 + len))
                            {
                                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._writer.WriteUShort15rbLong((short)len);
                            iter._writer.Write(elementList._encodedSetData);
                        }

                        iter._curBufPos = iter._writer.Position();

                        /* save bytes for field list data count */
                        levelInfo._countWritePos = iter._curBufPos;
                        levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
                        iter._curBufPos += 2;
                        iter._writer.SkipBytes(2);
                        return CodecReturnCode.SUCCESS;
                    }
                    else
                    {
                        const int reservedBytes = 2;

                        /* the set data needs to be encoded */
                        /* save state and return */
                        if (levelInfo._elemListSetDef != null)
                        {
                            if (iter.IsIteratorOverrun(((setEncodingMaxSize >= 0x80 || setEncodingMaxSize == 0) ? 2 : 1) + reservedBytes))
                            {
                                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }
                            iter._curBufPos = SetupU15Mark(levelInfo._internalMark, setEncodingMaxSize, iter._curBufPos);
                            iter._writer.Position(iter._curBufPos);

                            /* Back up endBufPos to account for reserved bytes. */
                            levelInfo._reservedBytes = reservedBytes;
                            iter._endBufPos -= reservedBytes;
                            iter._writer.ReserveBytes(reservedBytes);

                            /* If the set actually has no entries, just complete the set. */
                            if (levelInfo._elemListSetDef.Count > 0)
                            {
                                levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                            }
                            else
                            {
                                return CompleteElementSet(iter, levelInfo, elementList);
                            }
                        }
                        else
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.SET_DEF_NOT_PROVIDED;
                        }
                    }
                }
                else
                {
                    /* don't need length in front of set data */
                    /* encode set data if it exists */
                    if (elementList._encodedSetData.Length > 0)
                    {
                        /* make sure that set data can be encoded */
                        if (iter.IsIteratorOverrun(elementList._encodedSetData.Length))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        /* don't need a length in front of set data */
                        EncodeCopyData(iter, elementList._encodedSetData);
                        iter._curBufPos = iter._writer.Position();

                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.SUCCESS;
                    }
                    else
                    {
                        /* don't need length in front of set data */
                        /* save size pointer in case of rewind */

                        if (levelInfo._elemListSetDef != null)
                        {
                            levelInfo._internalMark._sizePos = iter._curBufPos;
                            levelInfo._internalMark._sizeBytes = 0;

                            /* If the set actually has no entries, turn around and complete the set. */
                            if (levelInfo._elemListSetDef.Count > 0)
                            {
                                levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                            }
                            else
                            {
                                return CompleteElementSet(iter, levelInfo, elementList);
                            }
                        }
                        else
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.SET_DEF_NOT_PROVIDED;
                        }
                    }
                }
            }
            else if (elementList.CheckHasStandardData())
            {
                /* we only have data */
                if (iter.IsIteratorOverrun(2))
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                levelInfo._countWritePos = iter._curBufPos;
                levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
                iter._curBufPos += 2;
                iter._writer.SkipBytes(2);
            }

            if (levelInfo._encodingState == EncodeIteratorStates.NONE)
            {
                levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeElementListComplete(EncodeIterator iter, bool success)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.ELEMENT_LIST, "Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                if (((ElementList)_levelInfo._listType).CheckHasStandardData())
                {
                    Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
                    iter._writer.Position(_levelInfo._countWritePos);
                    iter._writer.WriteShort(_levelInfo._currentCount);
                    iter._writer.Position(iter._curBufPos);
                }
            }
            else
            {
                iter._curBufPos = _levelInfo._containerStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            // _levelInfo._encodingState = EncodeIteratorStates.EIS_COMPLETE;
            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeElementEntry(EncodeIterator iter, ElementEntry element, object data)
        {
            CodecReturnCode ret = 0;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(null != element, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES || _levelInfo._encodingState == EncodeIteratorStates.SET_DATA, "Unexpected encoding attempted");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            _levelInfo._initElemStartPos = iter._curBufPos;

            if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
            {
                ElementList elementList = (ElementList)_levelInfo._listType;
                ElementSetDef def = _levelInfo._elemListSetDef;
                Debug.Assert(null != def, "Invalid parameters or parameters passed in as NULL");

                /* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
                ElementSetDefEntry encoding = def.Entries[_levelInfo._currentCount];

                /* Validate name (if present) */
                if (element._name != null && element._name.Equals(encoding.Name) == false)
                {
                    return CodecReturnCode.INVALID_DATA;
                }

                /* Validate type */
                if (element.DataType != Decoders.ConvertToPrimitiveType(encoding.DataType))
                {
                    return CodecReturnCode.INVALID_DATA;
                }

                /* Encode item according to set type */
                if (data != null)
                {
                    if ((ret = PrimitiveEncoder.EncodeSetData(iter, data, encoding.DataType)) < 0)
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return ret;
                    }
                }
                else if (element._encodedData.Length > 0) // Encoding pre-encoded data from the field entry
                {
                    /* if the dataType is primitive we need to make sure the data is length specified */
                    if ((element.DataType < DataTypes.SET_PRIMITIVE_MIN) || (element.DataType > DataTypes.CONTAINER_TYPE_MIN))
                    {
                        int len = element._encodedData.GetLength();

                        // len value is written on wire as uShort16ob
                        // If the value is smaller than 0xFE, it is written on the wire as one byte,
                        // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                        // The code below checks if the buffer is sufficient for each case.
                        if (len < 0xFE)
                        {
                            if (iter.IsIteratorOverrun(1 + len))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }

                            iter._writer._buffer.Write((byte)len);
                            iter._writer.Write(element._encodedData);
                        }
                        else if (len <= 0xFFFF)
                        {
                            if (iter.IsIteratorOverrun(3 + len))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }

                            iter._writer.WriteUShort16obLong(len);
                            iter._writer.Write(element._encodedData);
                        }
                        else
                        {
                            return CodecReturnCode.INVALID_DATA;
                        }

                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun((element._encodedData).Length))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.Write(element._encodedData);
                        iter._curBufPos = iter._writer.Position();
                    }
                }
                else
                /* blank */
                {
                    if ((ret = EncodeBlank(iter, encoding.DataType)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                if (++_levelInfo._currentCount < def.Count)
                {
                    return CodecReturnCode.SUCCESS;
                }

                /* Set is complete. */
                if ((ret = CompleteElementSet(iter, _levelInfo, elementList)) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                return CodecReturnCode.SET_COMPLETE;
            }

            /* Encoding standard entries. */
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");

            if (data != null)
            {
                Debug.Assert(null != element._name.Data(), "Missing element name");

                /* store element name as buffer 15 */
                int len = element._name.GetLength();

                // flags value is written on wire as uShort15rb
                // If the value is smaller than 0x80, it is written on the wire as one byte,
                // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(2 + len)) // 1 byte len + 1 byte datatype + len
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(element._name);
                }
                else
                {
                    if (iter.IsIteratorOverrun(3 + len)) // 2 bytes len + 1 byte datatype + len
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(element._name);
                }

                /* store data type */
                iter._writer.WriteUByte(element.DataType);
                iter._curBufPos = iter._writer.Position();

                if (element.DataType != DataTypes.NO_DATA)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
                    if ((element.DataType >= DataTypes.DATETIME_9) || !(PrimitiveEncoder.ValidEncodeActions(element.DataType)))
                    {
                        ret = CodecReturnCode.UNSUPPORTED_DATA_TYPE;
                    }
                    else
                    {
                        ret = PrimitiveEncoder.EncodeSetData(iter, data, element.DataType);
                    }
                }
                _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

                if (ret < 0)
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return ret;
                }
                _levelInfo._currentCount++;
                return CodecReturnCode.SUCCESS;
            }
            else if (element._encodedData.GetLength() > 0)
            {
                int len = element._name.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(element._name);
                }
                else
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(element._name);
                }

                /* store datatype */
                iter._writer.WriteUByte(element.DataType);
                iter._curBufPos = iter._writer.Position();

                /* copy encoded data */
                if (element.DataType != DataTypes.NO_DATA)
                {
                    len = element._encodedData.GetLength();
                    if (len < 0xFE)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._initElemStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(element._encodedData);
                    }
                    else if (len <= 0xFFFF)
                    {
                        if (iter.IsIteratorOverrun(3 + len))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._initElemStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.WriteUShort16obLong(len);
                        iter._writer.Write(element._encodedData);

                    }
                    else
                    {
                        return CodecReturnCode.INVALID_DATA;
                    }

                    iter._curBufPos = iter._writer.Position();
                }

                _levelInfo._currentCount++;
                return CodecReturnCode.SUCCESS;
            }
            else
            {
                int len = element._name.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(element._name);
                }
                else
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(element._name);
                }

                /* store datatype */
                iter._writer.WriteUByte(element.DataType);
                iter._curBufPos = iter._writer.Position();

                /* copy encoded data */
                if (element.DataType != DataTypes.NO_DATA)
                {
                    if (iter.IsIteratorOverrun(1))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    int zero = 0;
                    iter._writer.WriteUByte(zero);
                    iter._curBufPos = iter._writer.Position();
                }

                _levelInfo._currentCount++;
                return CodecReturnCode.SUCCESS;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeElementEntryInit(EncodeIterator iter, ElementEntry element, int encodingMaxSize)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.ELEMENT_LIST, "Invalid encoding attempted");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES || _levelInfo._encodingState == EncodeIteratorStates.SET_DATA, "Unexpected encoding attempted");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(null != element && null != element._name.Data(), "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, " Data exceeds iterators buffer length");

            _levelInfo._initElemStartPos = iter._curBufPos;

            /* Set data. */
            if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
            {
                ElementSetDef def = _levelInfo._elemListSetDef;

                /* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
                ElementSetDefEntry encoding = (ElementSetDefEntry)def.Entries[_levelInfo._currentCount];

                Debug.Assert(null != def, "Invalid parameters or parameters passed in as NULL");

                if (!ValidAggregateDataType(_levelInfo._containerType))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
                }

                /* Validate name (if present) */
                if (element._name != null && element._name.Equals(encoding.Name) == false)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.INVALID_DATA;
                }

                /* Validate type */
                if (element.DataType != Decoders.ConvertToPrimitiveType(encoding.DataType))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.INVALID_DATA;
                }

                if (iter.IsIteratorOverrun(((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                _levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_INIT;

                /* need to use mark 2 here because mark 1 is used to length specify all the set data */
                iter._curBufPos = SetupU16Mark(_levelInfo._internalMark2, encodingMaxSize, iter._curBufPos);
                iter._writer.Position(iter._curBufPos);

                return CodecReturnCode.SUCCESS;
            }

            /* Standard data. */
            int len = element._name.GetLength();

            // len value is written on wire as uShort15rb
            // If the value is smaller than 0x80, it is written on the wire as one byte,
            // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (len < 0x80)
            {
                if (iter.IsIteratorOverrun(2 + len)) // 1 byte len + len + 1 byte dataType
                {
                    _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer._buffer.Write((byte)len);
                iter._writer.Write(element._name);
            }
            else
            {
                if (iter.IsIteratorOverrun(3 + len)) // 2 bytes len + len + 1 byte dataType
                {
                    _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUShort15rbLong((short)len);
                iter._writer.Write(element._name);
            }

            iter._curBufPos = iter._writer.Position();
            /* store data type */
            iter._writer.WriteUByte(element.DataType);
            iter._curBufPos = iter._writer.Position();

            _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

            if (element.DataType != DataTypes.NO_DATA)
            {
                if (iter.IsIteratorOverrun(((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = SetupU16Mark(_levelInfo._internalMark, encodingMaxSize, iter._curBufPos);
                iter._writer.Position(iter._curBufPos);
            }
            else
            {
                _levelInfo._internalMark._sizeBytes = 0;
                _levelInfo._internalMark._sizePos = iter._curBufPos;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static bool ValidAggregateDataType(int dataType)
		{
			bool retVal = false;

			switch (dataType)
			{
				case DataTypes.NO_DATA:
				case DataTypes.OPAQUE:
				case DataTypes.XML:
				case DataTypes.FIELD_LIST:
				case DataTypes.ELEMENT_LIST:
				case DataTypes.ANSI_PAGE:
				case DataTypes.FILTER_LIST:
				case DataTypes.VECTOR:
				case DataTypes.MAP:
				case DataTypes.SERIES:
				case 139:
				case 140:
				case DataTypes.MSG:
				case DataTypes.JSON:
				case 223:
					retVal = true;
					break;
				default:
					if (dataType > DataTypes.MAX_RESERVED && dataType <= DataTypes.LAST)
					{
						retVal = true;
					}
					break;
			}

			return retVal;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static bool ValidPrimitiveDataType(int dataType)
		{
			bool retVal = false;

			switch (dataType)
			{
				case DataTypes.INT:
				case DataTypes.UINT:
				case DataTypes.FLOAT:
				case DataTypes.DOUBLE:
				case DataTypes.REAL:
				case DataTypes.DATE:
				case DataTypes.TIME:
				case DataTypes.DATETIME:
				case DataTypes.QOS:
				case DataTypes.STATE:
				case DataTypes.ENUM:
				case DataTypes.ARRAY:
				case DataTypes.BUFFER:
				case DataTypes.ASCII_STRING:
				case DataTypes.UTF8_STRING:
				case DataTypes.RMTES_STRING:
					retVal = true;
					break;
				default:
					break;
			}

			return retVal;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeElementEntryComplete(EncodeIterator iter, bool success)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.ELEMENT_LIST, "Invalid encoding attempted");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE || _levelInfo._encodingState == EncodeIteratorStates.ENTRY_WAIT_COMPLETE || _levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            /* Set data. */
            if (_levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE)
            {
                ElementList elementList = (ElementList)_levelInfo._listType;
                ElementSetDef def = _levelInfo._elemListSetDef;
                if (success)
                {
                    if (_levelInfo._internalMark2._sizeBytes > 0)
                    {
                        /* need to use mark 2 here because mark 1 is used to length specify all the set data */
                        if ((ret = FinishU16Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            _levelInfo._initElemStartPos = 0;
                            return ret;
                        }
                    }
                    _levelInfo._initElemStartPos = 0;

                    if (++_levelInfo._currentCount < def.Count)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                        return CodecReturnCode.SUCCESS;
                    }

                    /* Set is complete. */
                    if ((ret = CompleteElementSet(iter, _levelInfo, elementList)) < 0)
                    {
                        return ret;
                    }

                    return CodecReturnCode.SET_COMPLETE;
                }
                else
                {
                    /* reset the pointer */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    _levelInfo._initElemStartPos = 0;
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                    return CodecReturnCode.SUCCESS;
                }
            }

            /* Standard data. */
            if (success)
            {
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    if ((ret = FinishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return ret;
                    }
                }
                _levelInfo._currentCount++;
            }
            else
            {
                /* reset the pointer */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            _levelInfo._initElemStartPos = 0;
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode CompleteElementSet(EncodeIterator iter, EncodingLevel _levelInfo, ElementList elementList)
        {
            CodecReturnCode ret;

            /* Set definition completed. Write the length, and move on to standard data if any. */
            /* length may not be encoded in certain cases */
            if (_levelInfo._internalMark._sizeBytes != 0)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return ret;
                }

                if (((ElementList)_levelInfo._listType).CheckHasStandardData())
                {
                    /* Move endBufPos back to original position if bytes were reserved. */
                    iter._endBufPos += _levelInfo._reservedBytes;
                    iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
                    _levelInfo._reservedBytes = 0;

                    /* Reset entry count. Only Standard Entries actually count towards it. */
                    _levelInfo._currentCount = 0;
                    _levelInfo._countWritePos = iter._curBufPos;
                    iter._curBufPos += 2;
                    iter._writer.SkipBytes(2);
                    _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
                }
                else
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                }
            }
            else
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFieldListInit(EncodeIterator iter, FieldList fieldList, LocalFieldSetDefDb localFieldFidDb,int setEncodingMaxSize)
		{
            CodecReturnCode ret;
			EncodingLevel levelInfo;
			int setId;
            LocalFieldSetDefDb setDb = localFieldFidDb;

            /* Assertions */
            Debug.Assert(null != fieldList && null != iter, "Invalid parameters or parameters passed in as NULL");

			levelInfo = iter._levelInfo[++iter._encodingLevel];
			if (iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
			{
				return CodecReturnCode.ITERATOR_OVERRUN;
			}
			levelInfo.Init(DataTypes.FIELD_LIST, EncodeIteratorStates.NONE, fieldList, iter._curBufPos);

			/* Make sure that required elements can be encoded */
			if (iter.IsIteratorOverrun(1))
			{
				levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

			/* Store flags as UInt8 */
			iter._writer._buffer.Write((byte)fieldList.Flags);
			iter._curBufPos = iter._writer.Position();

			/* Check for field list info */
			if (fieldList.CheckHasInfo())
			{
				int infoLen;
				int startPos;

				// ID value is written on wire as uShort15rb
				// If the value is smaller than 0x80, it is written on the wire as one byte,
				// otherwise, it is written as two bytes by calling writeUShort15rbLong method.
				// The code below checks if the buffer is sufficient for each case.
				if (fieldList.DictionaryId < 0x80)
				{
					if (iter.IsIteratorOverrun(4)) // 1 byte skip + 1 byte id + 2 bytes fieldListNum
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}
					/* Save info length position */
					startPos = iter._curBufPos;

					/* Skip info length so it can be encoded later */
					iter._writer.SkipBytes(1);
					iter._curBufPos += 1;

					iter._writer._buffer.Write((byte)fieldList.DictionaryId);
				}
				else
				{
					if (iter.IsIteratorOverrun(5)) // 1 byte skip + 2 bytes id + 2 bytes fieldListNum
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}
					/* Save info length position */
					startPos = iter._curBufPos;

					/* Skip info length so it can be encoded later */
					iter._writer.SkipBytes(1);
					iter._curBufPos += 1;

					iter._writer.WriteUShort15rbLong((short)fieldList.DictionaryId);
				}

				/* Put field list number */
				iter._writer.WriteShort(fieldList.FieldListNum);
				iter._curBufPos = iter._writer.Position();

				/* encode the field list info length */
				infoLen = iter._curBufPos - startPos - 1;
				iter._writer.Position(startPos);
				iter._writer._buffer.Write((byte)infoLen);
				iter._writer.Position(iter._curBufPos);
			}

			/* Check for set data */
			if (fieldList.CheckHasSetData())
			{
				/* We have Field List Set Data */
				/* Check for encode optional Field List Set Id */
				if (fieldList.CheckHasSetId())
				{
					if (fieldList.SetId < 0x80)
					{
						if (iter.IsIteratorOverrun(1))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer._buffer.Write((byte)fieldList.SetId);
					}
					else
					{
						if (iter.IsIteratorOverrun(2))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer.WriteUShort15rbLong((short)fieldList.SetId);
					}
					iter._curBufPos = iter._writer.Position();
					setId = fieldList.SetId;
				}
				else
				{
					setId = 0;
				}

                if (setDb != null && setId <= LocalFieldSetDefDb.MAX_LOCAL_ID && setDb._definitions[setId].SetId != LocalFieldSetDefDb.BLANK_ID)
                {
                    levelInfo._fieldListSetDef = setDb._definitions[setId];
                }
                else if (iter._fieldSetDefDb != null && setId <= iter._fieldSetDefDb.MaxSetId && iter._fieldSetDefDb.Definitions[setId] != null)
                {
                    levelInfo._fieldListSetDef = (FieldSetDef)iter._fieldSetDefDb.Definitions[setId];
                }

                /* Check for Field List Data after the Set Data */
                if (fieldList.CheckHasStandardData())
				{
					/* If have set data and field list data, need length in front of the set */
					if (fieldList._encodedSetData.Length > 0)
					{
						/* The set data is already encoded. */
						/* Make sure that the set data can be encoded. The length was included in the previous check. */
						int len = fieldList._encodedSetData.GetLength();
						if (len < 0x80)
						{
							if (iter.IsIteratorOverrun(3 + len))
							{
								return CodecReturnCode.BUFFER_TOO_SMALL;
							}
							iter._writer._buffer.Write((byte)len);
							iter._writer.Write(fieldList._encodedSetData);
						}
						else
						{
							if (iter.IsIteratorOverrun(4 + len))
							{
								return CodecReturnCode.BUFFER_TOO_SMALL;
							}
							iter._writer.WriteUShort15rbLong((short)len);
							iter._writer.Write(fieldList._encodedSetData);
						}

						iter._curBufPos = iter._writer.Position();
						/* Save bytes for field list data count */
						levelInfo._countWritePos = iter._curBufPos;
						levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
						iter._writer.SkipBytes(2);
						iter._curBufPos += 2;
						return CodecReturnCode.SUCCESS;
					}
					else
					{
						const int reservedBytes = 2;

						/* The set data need to be encoded. */
						/* Save state and return */
						if (levelInfo._fieldListSetDef != null)
						{
							if (iter.IsIteratorOverrun(((setEncodingMaxSize >= 0x80 || setEncodingMaxSize == 0) ? 2 : 1) + reservedBytes))
							{
								levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
								return CodecReturnCode.BUFFER_TOO_SMALL;
							}
							iter._curBufPos = SetupU15Mark(levelInfo._internalMark, setEncodingMaxSize, iter._curBufPos);
							iter._writer.Position(iter._curBufPos);

							/* Back up endBufPos to account for reserved bytes. */
							levelInfo._reservedBytes = reservedBytes;
							iter._endBufPos -= reservedBytes;
							iter._writer.ReserveBytes(reservedBytes);

							/* If the set actually has no entries, just complete the set. */
							if (levelInfo._fieldListSetDef.Count > 0)
							{
								levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
								return CodecReturnCode.SUCCESS;
							}
							else
							{
								if ((ret = CompleteFieldSet(iter, levelInfo, fieldList)) != CodecReturnCode.SUCCESS)
								{
									levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
									return ret;
								}

								return CodecReturnCode.SUCCESS;
							}
						}
						else
						{
							levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
							return CodecReturnCode.SET_DEF_NOT_PROVIDED;
						}
					}
				}
				else
				{
					/* Don't need a length in front of set data. */
					/* Encode set data if it exists */
					if (fieldList._encodedSetData.Length > 0)
					{
						/* Make sure that the set data can be encoded. */
						if (iter.IsIteratorOverrun(fieldList._encodedSetData.Length))
						{
							levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}

						/* Don't need a length in front of set data. */
						EncodeCopyData(iter, fieldList._encodedSetData);
						iter._curBufPos = iter._writer.Position();

						levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
						return CodecReturnCode.SUCCESS;
					}
					else
					{
						/* Don't need a length in front of set data. */
						/* Save the size pointer in case we need to rewind */
						if (levelInfo._fieldListSetDef != null)
						{
							levelInfo._internalMark._sizePos = iter._curBufPos;
							levelInfo._internalMark._sizeBytes = 0;

							/* If the set actually has no entries, just complete the set. */
							if (levelInfo._fieldListSetDef.Count > 0)
							{
								levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
							}
							else
							{
								if ((ret = CompleteFieldSet(iter, levelInfo, fieldList)) != CodecReturnCode.SUCCESS)
								{
									levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
									return ret;
								}
							}
							return CodecReturnCode.SUCCESS;
						}
						else
						{
							levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
							return CodecReturnCode.SET_DEF_NOT_PROVIDED;
						}
					}
				}
			}
			else if (fieldList.CheckHasStandardData())
			{
				/* We only have field list data */
				if (iter.IsIteratorOverrun(2))
				{
					levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				levelInfo._countWritePos = iter._curBufPos;
				levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
				iter._writer.SkipBytes(2);
				iter._curBufPos += 2;
			}

			if (levelInfo._encodingState == EncodeIteratorStates.NONE)
			{
				levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
			}

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode CompleteFieldSet(EncodeIterator iter, EncodingLevel _levelInfo, FieldList fieldList)
		{
            CodecReturnCode ret;

			/* Set definition completed. Write the length, and move on to standard data if any. */
			/* length may not be encoded in certain cases */
			if (_levelInfo._internalMark._sizeBytes != 0)
			{
				if ((ret = FinishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
				{
					_levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
					return ret;
				}

				if (fieldList.CheckHasStandardData())
				{
					/* Move endBufPos back to original position if bytes were reserved. */
					iter._endBufPos += _levelInfo._reservedBytes;
					iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
					_levelInfo._reservedBytes = 0;

					/* Reset entry count. Only Standard Entries actually count towards it. */
					_levelInfo._currentCount = 0;
					_levelInfo._countWritePos = iter._curBufPos;
					iter._curBufPos += 2;
					iter._writer.SkipBytes(2);
					_levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
				}
				else
				{
					_levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
				}
			}
			else
			{
				_levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
			}

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static CodecReturnCode EncodeFieldListComplete(EncodeIterator iter, bool success)
		{
			EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

			/* Validations */
			Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(_levelInfo._containerType == DataTypes.FIELD_LIST, "Invalid encoding attempted - wrong type");
			Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
			Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

			if (success)
			{
				if (_levelInfo._encodingState == EncodeIteratorStates.ENTRIES)
				{
					Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
					iter._writer.Position(_levelInfo._countWritePos);
					iter._writer.WriteShort(_levelInfo._currentCount);
					iter._writer.Position(iter._curBufPos);
				}
			}
			else
			{
				iter._curBufPos = _levelInfo._containerStartPos;
				iter._writer.Position(iter._curBufPos);
			}
			--iter._encodingLevel;

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFieldEntry(EncodeIterator iter, FieldEntry field, object data)
		{
            CodecReturnCode ret;
			EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

			/* Validations */
			Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
			// assert field, Invalid parameters or parameters passed in as NULL";
			Debug.Assert(_levelInfo._containerType == DataTypes.FIELD_LIST, "Invalid encoding attempted - wrong container");
			Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES || _levelInfo._encodingState == EncodeIteratorStates.SET_DATA, "Unexpected encoding attempted");
			Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
			Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

			_levelInfo._initElemStartPos = iter._curBufPos;

			if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
			{
				FieldList fieldList = (FieldList)_levelInfo._listType;
				FieldSetDef def = _levelInfo._fieldListSetDef;

				Debug.Assert(null != def, "Invalid parameters or parameters passed in as NULL");
				/* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
				FieldSetDefEntry encoding = def.Entries[_levelInfo._currentCount];

				/* Validate fid */
				if (field.FieldId != encoding.FieldId)
				{
					return CodecReturnCode.INVALID_DATA;
				}

				/* Validate type */
				if (field.DataType != Decoders.ConvertToPrimitiveType((int)encoding.DataType))
				{
					return CodecReturnCode.INVALID_DATA;
				}

				/* Encode item according to set type */
				if (data != null)
				{
					if ((ret = PrimitiveEncoder.EncodeSetData(iter, data, (int)encoding.DataType)) < 0)
					{
						/* rollback */
						iter._curBufPos = _levelInfo._initElemStartPos;
						iter._writer.Position(iter._curBufPos);
						return ret;
					}
				}
				else if (field._encodedData.Length > 0) // Encoding pre-encoded data from the field entry
				{
					/* if the dataType is primitive we need to make sure the data is length specified */
					if ((field.DataType < DataTypes.SET_PRIMITIVE_MIN) || (field.DataType > DataTypes.CONTAINER_TYPE_MIN))
					{
						int len = field._encodedData.GetLength();

						// len value is written on wire as uShort16ob
						// If the value is smaller than 0xFE, it is written on the wire as one byte,
						// otherwise, it is written as three bytes by calling writeUShort16obLong method.
						// The code below checks if the buffer is sufficient for each case.
						if (len < 0xFE)
						{
							if (iter.IsIteratorOverrun(1 + len))
							{
								return CodecReturnCode.BUFFER_TOO_SMALL;
							}

							iter._writer._buffer.Write((byte)len);
							iter._writer.Write(field._encodedData);
						}
						else if (len <= 0xFFFF)
						{
							if (iter.IsIteratorOverrun(3 + len))
							{
								return CodecReturnCode.BUFFER_TOO_SMALL;
							}

							iter._writer.WriteUShort16obLong(len);
							iter._writer.Write(field._encodedData);

						}
						else
						{
							return CodecReturnCode.INVALID_DATA;
						}

						iter._curBufPos = iter._writer.Position();
					}
					else
					{
						if (iter.IsIteratorOverrun(field._encodedData.Length))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}

						iter._writer.Write(field._encodedData);
						iter._curBufPos = iter._writer.Position();
					}
				}
				else
				/* blank */
				{
					if ((ret = EncodeBlank(iter, (int)encoding.DataType)) != CodecReturnCode.SUCCESS)
					{
						return ret;
					}
				}

				if (++_levelInfo._currentCount < def.Count)
				{
					return CodecReturnCode.SUCCESS;
				}

				if ((ret = CompleteFieldSet(iter, _levelInfo, (FieldList)fieldList)) < 0)
				{
					return ret;
				}

				return CodecReturnCode.SET_COMPLETE;
			}

			if (data != null)
			{
				if (iter.IsIteratorOverrun(2))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				/* Store FieldId as Uint16 */
				iter._writer.WriteShort(field.FieldId);
				iter._curBufPos = iter._writer.Position();

				/* Encode the data type */
				_levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
				if ((field.DataType >= DataTypes.DATETIME_9) || !(PrimitiveEncoder.ValidEncodeActions(field.DataType)))
				{
					ret = CodecReturnCode.UNSUPPORTED_DATA_TYPE;
				}
				else
				{
					ret = PrimitiveEncoder.EncodeSetData(iter, data, field.DataType);
				}

				_levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

				if (ret < 0)
				{
					iter._curBufPos = _levelInfo._initElemStartPos;
					iter._writer.Position(iter._curBufPos);
					return ret;
				}

				_levelInfo._currentCount++;
				return CodecReturnCode.SUCCESS;
			}
			else if (field._encodedData.Length > 0) // Encoding pre-encoded data from the field entry
			{
				if (iter.IsIteratorOverrun(2))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				/* Store FieldId as Uint16 */
				iter._writer.WriteShort(field.FieldId);
				iter._curBufPos = iter._writer.Position();

				int len = field._encodedData.GetLength();
				if (len < 0xFE)
				{
					if (iter.IsIteratorOverrun(1 + len))
					{
						/* rollback */
						iter._curBufPos = _levelInfo._initElemStartPos;
						iter._writer.Position(iter._curBufPos);
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}

					iter._writer._buffer.Write((byte)len);
					iter._writer.Write(field._encodedData);
				}
				else if (len <= 0xFFFF)
				{
					if (iter.IsIteratorOverrun(3 + len))
					{
						/* rollback */
						iter._curBufPos = _levelInfo._initElemStartPos;
						iter._writer.Position(iter._curBufPos);
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}

					iter._writer.WriteUShort16obLong(len);
					iter._writer.Write(field._encodedData);
				}
				else
				{
					/* rollback */
					iter._curBufPos = _levelInfo._initElemStartPos;
					iter._writer.Position(iter._curBufPos);
					return CodecReturnCode.INVALID_DATA;
				}

				iter._curBufPos = iter._writer.Position();

				_levelInfo._currentCount++;
				return CodecReturnCode.SUCCESS;
			}
			else
			/* Encoding as blank */
			{
				int zero = 0;
				if (iter.IsIteratorOverrun(3))
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				/* Store FieldId as Uint16 */
				iter._writer.WriteShort(field.DataType);
				iter._curBufPos = iter._writer.Position();

				iter._writer._buffer.Write((byte)zero);
				iter._curBufPos = iter._writer.Position();

				_levelInfo._currentCount++;
				return CodecReturnCode.SUCCESS;
			}
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFieldEntryInit(EncodeIterator iter, FieldEntry field, int encodingMaxSize)
		{
			EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

			/* Validations */
			Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(null != field, "Invalid parameters or parameters passed in as NULL");

			Debug.Assert(_levelInfo._containerType == DataTypes.FIELD_LIST, "Invalid encoding attempted - wrong type");
			Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES || _levelInfo._encodingState == EncodeIteratorStates.SET_DATA, "Unexpected encoding attempted");
			Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
			Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

			_levelInfo._initElemStartPos = iter._curBufPos;

			/* Set data */
			if (_levelInfo._encodingState == EncodeIteratorStates.SET_DATA)
			{
				FieldSetDef def = _levelInfo._fieldListSetDef;
				Debug.Assert(null != def, "Invalid parameters or parameters passed in as NULL");
				/* Use currentCount as a temporary set iterator. It should be reset before doing standard entries. */
				FieldSetDefEntry encoding = def.Entries[_levelInfo._currentCount];

				if (!ValidAggregateDataType(_levelInfo._containerType))
				{
					_levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
					return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
				}

				/* Validate fid */
				if (field.FieldId != encoding.FieldId)
				{
					_levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
					return CodecReturnCode.INVALID_DATA;
				}

				/* Validate type */
				if (field.DataType != Decoders.ConvertToPrimitiveType((int)encoding.DataType))
				{
					_levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
					return CodecReturnCode.INVALID_DATA;
				}

				if (iter.IsIteratorOverrun(((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 3 : 1)))
				{
					_levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE;
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				_levelInfo._encodingState = EncodeIteratorStates.SET_ENTRY_INIT;

				/* need to use internal mark 2 here because mark 1 is used to length specify set data */
				iter._curBufPos = SetupU16Mark(_levelInfo._internalMark2, encodingMaxSize, iter._curBufPos);
				iter._writer.Position(iter._curBufPos);

				return CodecReturnCode.SUCCESS;

			}

			/* Standard data */
			if (iter.IsIteratorOverrun((3 + ((encodingMaxSize == 0 || encodingMaxSize >= 0xFE) ? 2 : 0))))
			{
				_levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

			/* Store FieldId as Uint16 */
			iter._writer.WriteShort((short)field.FieldId);
			iter._curBufPos = iter._writer.Position();
			_levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

			iter._curBufPos = SetupU16Mark(_levelInfo._internalMark, encodingMaxSize, iter._curBufPos);
			iter._writer.Position(iter._curBufPos);

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFieldEntryComplete(EncodeIterator iter, bool success)
		{
            CodecReturnCode ret;
			EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

			/* Validations */
			Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(_levelInfo._containerType == DataTypes.FIELD_LIST, "Invalid encoding attempted - wrong type");
			Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRY_INIT || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT || _levelInfo._encodingState == EncodeIteratorStates.ENTRY_WAIT_COMPLETE || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE || _levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
			Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
			Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

			/* Set data (user was encoding a container in the set) */
			if (_levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_INIT || _levelInfo._encodingState == EncodeIteratorStates.SET_ENTRY_WAIT_COMPLETE)
			{
				FieldList fieldList = (FieldList)_levelInfo._listType;
				FieldSetDef def = _levelInfo._fieldListSetDef;
				if (success)
				{
					/* need to use internal mark 2 here because mark 1 is used to length specify set data */
					if ((ret = FinishU16Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
					{
						_levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
						_levelInfo._initElemStartPos = 0;
						return ret;
					}
					_levelInfo._initElemStartPos = 0;

					if (++_levelInfo._currentCount < def.Count)
					{
						_levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
						return CodecReturnCode.SUCCESS;
					}

					/* Set is complete. */
					if ((ret = CompleteFieldSet(iter, _levelInfo, (FieldList)fieldList)) < 0)
					{
						return ret;
					}

					return CodecReturnCode.SET_COMPLETE;
				}
				else
				{
					/* Reset the pointer */
					iter._curBufPos = _levelInfo._initElemStartPos;
					iter._writer.Position(iter._curBufPos);
					_levelInfo._initElemStartPos = 0;
					_levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
					return CodecReturnCode.SUCCESS;
				}
			}

			/* Standard data. */
			if (success)
			{
				if ((ret = FinishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
				{
					_levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
					_levelInfo._initElemStartPos = 0;
					return ret;
				}
				_levelInfo._currentCount++;
			}
			else
			{
				/* Reset the pointer */
				iter._curBufPos = _levelInfo._initElemStartPos;
				iter._writer.Position(iter._curBufPos);
			}
			_levelInfo._initElemStartPos = 0;
			_levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode FinishU15Mark(EncodeIterator iter, EncodeSizeMark mark, int position)
		{
			int dataLength;

			Debug.Assert(null != mark, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(mark._sizePos != 0, "Invalid encoding attempted");
			Debug.Assert(mark._sizeBytes != 0, "Invalid encoding attempted");
			Debug.Assert(position != 0, "Invalid encoding attempted");

			dataLength = position - mark._sizePos - mark._sizeBytes;

			if (dataLength > RWF_MAX_U15)
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

			if (mark._sizeBytes == 1)
			{
				if (dataLength >= 0x80)
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.Position(mark._sizePos);
				iter._writer._buffer.Write((byte)dataLength);
				iter._writer.Position(iter._curBufPos);
			}
			else
			{
				Debug.Assert(mark._sizeBytes == 2, "Invalid encoding attempted");
				dataLength |= 0x8000;
				iter._writer.Position(mark._sizePos);
				iter._writer.WriteShort(dataLength);
				iter._writer.Position(iter._curBufPos);
			}
			mark._sizePos = 0;
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode FinishU16Mark(EncodeIterator iter, EncodeSizeMark mark, int position)
		{
			int dataLength;

			Debug.Assert(null != mark, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(mark._sizePos != 0, "Invalid encoding attempted");
			Debug.Assert(mark._sizeBytes != 0, "Invalid encoding attempted");
			Debug.Assert(position != 0, "Invalid encoding attempted");

			dataLength = position - mark._sizePos - mark._sizeBytes;

			if (mark._sizeBytes == 1)
			{
				if (dataLength >= 0xFE)
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.Position(mark._sizePos);
				iter._writer._buffer.Write((byte)dataLength);
				iter._writer.Position(iter._curBufPos);
			}
			else
			{
				int dl = dataLength;
				Debug.Assert(mark._sizeBytes == 3, "Invalid encoding attempted");

				if (dataLength > RWF_MAX_16)
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._writer.Position(mark._sizePos);
				iter._writer._buffer.Write((byte)0xFE);
				iter._writer.WriteShort(dl);
				iter._writer.Position(iter._curBufPos);
			}
			mark._sizePos = 0;
			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void EncodeCopyData(EncodeIterator iter, Buffer buf)
		{
			iter._writer.Write(buf);
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeArrayInit(EncodeIterator iter, Array array)
        {
            EncodingLevel levelInfo;

            if ((array._primitiveType >= DataTypes.SET_PRIMITIVE_MIN) || (array._primitiveType == DataTypes.ARRAY))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            levelInfo = iter._levelInfo[++iter._encodingLevel];
            if (iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            levelInfo.Init(DataTypes.ARRAY, EncodeIteratorStates.NONE, array, iter._curBufPos);

            // length value is written on wire as uShort16ob
            // If the value is smaller than 0xFE, it is written on the wire as one byte,
            // otherwise, it is written as three bytes by calling writeUShort16obLong method.
            // The code below checks if the buffer is sufficient for each case.
            if (array._itemLength < 0xFE)
            {
                if (iter.IsIteratorOverrun(4)) // 1 byte primitive type + 1 byte length + 2 bytes skip
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUByte(array._primitiveType);
                iter._writer._buffer.Write((byte)array._itemLength);
            }
            else
            {
                if (iter.IsIteratorOverrun(6)) // 1 byte primitive type + 3 bytes length + 2 bytes skip
                {
                    levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
                iter._writer.WriteUByte(array._primitiveType);
                iter._writer.WriteUShort16obLong(array._itemLength);
            }

            iter._curBufPos = iter._writer.Position();
            levelInfo._countWritePos = iter._curBufPos;
            iter._curBufPos += 2;
            iter._writer.SkipBytes(2);
            /* change encoding state */
            levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeArrayEntry(EncodeIterator iter, object data)
        {
            Array array;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations 
            assert(iterInt != null);
            assert(data != null);
            assert(_levelInfo._containerType == DataTypes.ARRAY);
            assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES);
            */
            array = (Array)_levelInfo._listType;

            // set start position for this entry - needed for rollback
            _levelInfo._initElemStartPos = iter._curBufPos;

            switch (array._primitiveType)
            {
                case DataTypes.BUFFER:
                case DataTypes.ASCII_STRING:
                case DataTypes.UTF8_STRING:
                case DataTypes.RMTES_STRING:
                    if (array._itemLength == 0)
                    {
                        int len = ((Buffer)data).GetLength();

                        // len value is written on wire as uShort16ob
                        // If the value is smaller than 0xFE, it is written on the wire as one byte,
                        // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                        // The code below checks if the buffer is sufficient for each case.
                        if (len < 0xFE)
                        {
                            if (iter.IsIteratorOverrun(1 + len))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }

                            iter._writer._buffer.Write((byte)len);
                            iter._writer.Write((Buffer)data);
                        }
                        else if (len <= 0xFFFF)
                        {
                            if (iter.IsIteratorOverrun(3 + len))
                            {
                                return CodecReturnCode.BUFFER_TOO_SMALL;
                            }

                            iter._writer.WriteUShort16obLong(len);
                            iter._writer.Write((Buffer)data);

                        }
                        else
                        {
                            return CodecReturnCode.INVALID_DATA;
                        }
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(array._itemLength))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        Buffer buffer = (Buffer)data;

                        // Buffer can be shorter than itemLength.
                        if (buffer.Length < array._itemLength)
                        {
                            for (int i = 0; i < buffer.Length; i++)
                            {
                                iter._writer._buffer.Write((byte)(buffer).DataByte(buffer.Position + i));
                            }
                            for (int i = buffer.GetLength(); i < array._itemLength; i++)
                            {
                                iter._writer._buffer.Write((byte)0x00);
                            }
                        }
                        else
                        {
                            // Make sure we don't copy more bytes than necessary, truncate extra bytes
                            for (int i = 0; i < array._itemLength; i++)
                            {
                                iter._writer._buffer.Write((byte)(buffer).DataByte(buffer.Position + i));
                            }
                        }
                    }
                    iter._curBufPos = iter._writer.Position();
                    ret = CodecReturnCode.SUCCESS;
                    break;

                default:
                    /* Use the appropriate primitive encode method, if one exists for this type and itemLength */
                    if ((array._primitiveType > DataTypes.ENUM) || (array._itemLength > 9))
                    {
                        return CodecReturnCode.INVALID_ARGUMENT;
                    }

                    if (!PrimitiveEncoder.ValidArrayEncodeActions(array._primitiveType, array._itemLength))
                    {
                        return CodecReturnCode.INVALID_ARGUMENT;
                    }

                    if (array._itemLength == 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
                    }
                    else
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                    }

                    ret = PrimitiveEncoder.EncodeArrayData(iter, data, array._primitiveType, array._itemLength);
                    break;
            }

            if (ret >= CodecReturnCode.SUCCESS)
            {
                _levelInfo._currentCount++;
            }
            else
            {
                /* failure - roll back */
                iter._curBufPos = _levelInfo._initElemStartPos;
                _levelInfo._initElemStartPos = 0;
                iter._writer.Position(iter._curBufPos);
            }

            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodePreencodedArrayEntry(EncodeIterator iter, Buffer encodedData)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Array array = (Array)_levelInfo._listType;

            // set start position for this entry - needed for rollback
            _levelInfo._initElemStartPos = iter._curBufPos;

            if (encodedData.Length > 0)
            {
                if (array._itemLength > 0)
                {
                    /* fixed length items */
                    /* check that the length given to us is what we expect */
                    Debug.Assert(array._itemLength == encodedData.Length, "Invalid encoded data length");

                    if (iter.IsIteratorOverrun(array._itemLength))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.Write(encodedData);
                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    int len = encodedData.GetLength();

                    // len value is written on wire as uShort16ob
                    // If the value is smaller than 0xFE, it is written on the wire as one byte,
                    // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0xFE)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(encodedData);
                    }
                    else if (len <= 0xFFFF)
                    {
                        if (iter.IsIteratorOverrun(3 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.WriteUShort16obLong(len);
                        iter._writer.Write(encodedData);

                    }
                    else
                    {
                        return CodecReturnCode.INVALID_DATA;
                    }

                    iter._curBufPos = iter._writer.Position();
                }
            }
            else
            {
                /* Blank */
                if (array._itemLength > 0)
                {
                    if (array._primitiveType <= DataTypes.RMTES_STRING && array._itemLength <= 9)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.SET_DATA;
                        if ((ret = EncodeBlank(iter, array._primitiveType)) != CodecReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        return CodecReturnCode.INVALID_ARGUMENT;
                    }
                }
                else
                {
                    _levelInfo._encodingState = EncodeIteratorStates.PRIMITIVE;
                    if ((ret = EncodeBlank(iter, array._primitiveType)) != CodecReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    iter._curBufPos = iter._writer.Position();
                }
            }

            _levelInfo._currentCount++;
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeArrayComplete(EncodeIterator iter, bool success)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.ARRAY, " Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteShort(_levelInfo._currentCount);
                iter._writer.Position(iter._curBufPos);
            }
            else
            {
                iter._curBufPos = _levelInfo._containerStartPos;
                iter._writer.Position(iter._curBufPos);
            }
            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeBlank(EncodeIterator iter, int type)
		{
			switch (type)
			{
				case DataTypes.REAL_4RB:
				{
					Real blankReal = new Real();
					blankReal.Blank();
					return PrimitiveEncoder.EncodeReal4(iter, blankReal);
				}
				case DataTypes.REAL_8RB:
				{
					Real blankReal = new Real();
					blankReal.Blank();
					return PrimitiveEncoder.EncodeReal8(iter, blankReal);
				}
				case DataTypes.DATE_4:
				{
					Date blankDate = new Date();
					blankDate.Blank();
					return PrimitiveEncoder.EncodeDate4(iter, blankDate);
				}
				case DataTypes.TIME_3:
				{
					Time blankTime = new Time();
					blankTime.Blank();
					return PrimitiveEncoder.EncodeTime3(iter, blankTime);
				}
				case DataTypes.TIME_5:
				{
					Time blankTime = new Time();
					blankTime.Blank();
					return PrimitiveEncoder.EncodeTime5(iter, blankTime);
				}
				case DataTypes.TIME_7:
				{
					Time blankTime = new Time();
					blankTime.Blank();
					return PrimitiveEncoder.EncodeTime7(iter, blankTime);
				}
				case DataTypes.TIME_8:
				{
					Time blankTime = new Time();
					blankTime.Blank();
					return PrimitiveEncoder.EncodeTime8(iter, blankTime);
				}
				case DataTypes.DATETIME_7:
				{
					DateTime blankDateTime = new DateTime();
					blankDateTime.Blank();
					return PrimitiveEncoder.EncodeDateTime7(iter, blankDateTime);
				}
				case DataTypes.DATETIME_9:
				{
					DateTime blankDateTime = new DateTime();
					blankDateTime.Blank();
					return PrimitiveEncoder.EncodeDateTime9(iter, blankDateTime);
				}
				case DataTypes.DATETIME_11:
				{
					DateTime blankDateTime = new DateTime();
					blankDateTime.Blank();
					return PrimitiveEncoder.EncodeDateTime11(iter, blankDateTime);
				}
				case DataTypes.DATETIME_12:
				{
					DateTime blankDateTime = new DateTime();
					blankDateTime.Blank();
					return PrimitiveEncoder.EncodeDateTime12(iter, blankDateTime);
				}
				default:
				{
					int zero = 0;

					/* Except for the above, only length-spec primitives can be blank. */
					if (type > DataTypes.BASE_PRIMITIVE_MAX)
					{
						return CodecReturnCode.INVALID_ARGUMENT;
					}

					if (iter.IsIteratorOverrun(1))
					{
						return CodecReturnCode.BUFFER_TOO_SMALL;
					}

					iter._writer._buffer.Write((byte)zero);
					iter._curBufPos = iter._writer.Position();
					return CodecReturnCode.SUCCESS;
				}
			}
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeNonRWFInit(EncodeIterator iter, Buffer buffer)
		{
			EncodingLevel levelInfo;

			Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(null != buffer, "Invalid parameters or parameters passed in as NULL");

			if (++iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
			{
				return CodecReturnCode.ITERATOR_OVERRUN;
			}
			levelInfo = iter._levelInfo[iter._encodingLevel];
			levelInfo.Init(DataTypes.OPAQUE, EncodeIteratorStates.NON_RWF_DATA, null, iter._curBufPos);

			(buffer).Data_internal(iter._writer.Buffer(), iter._curBufPos, (iter._endBufPos - iter._curBufPos));

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeNonRWFComplete(EncodeIterator iter, Buffer buffer, bool success)
		{
			EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

			/* Validations */
			Debug.Assert(null != buffer, "Invalid buffer");
			Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
			Debug.Assert(_levelInfo._containerType == DataTypes.OPAQUE, "Invalid container type");
			Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
			Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

			if (success)
			{
				/* verify no overrun */
				if (iter._buffer != buffer.Data())
				{
					return CodecReturnCode.INVALID_DATA;
				}

				if (iter.IsIteratorOverrun(buffer.Length))
				{
					_levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				iter._curBufPos += buffer.Data().BufferPosition() - buffer.Position;
				iter._writer.Position(iter._curBufPos);
			}
			else
			{
				iter._curBufPos = _levelInfo._containerStartPos;
				iter._writer.Position(iter._curBufPos);
			}

			--iter._encodingLevel;

			return CodecReturnCode.SUCCESS;
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static void PutLenSpecBlank(EncodeIterator iter)
		{
			iter._writer._buffer.Write((byte)0);
			iter._curBufPos = iter._writer.Position();
		}

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFilterListInit(EncodeIterator iter, FilterList filterList)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != filterList, "Invalid filterListInt in as NULL");

            EncodingLevel _levelInfo;
            int flags;

            if (!ValidAggregateDataType(filterList.ContainerType))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            if (++iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }

            _levelInfo = iter._levelInfo[iter._encodingLevel];
            _levelInfo.Init(DataTypes.FILTER_LIST, EncodeIteratorStates.NONE, filterList, iter._curBufPos);
            _levelInfo._flags = EncodeIteratorFlags.NONE;

            /* Make sure that required elements can be encoded */
            /* Flags (1), Opt Data Format (1), Total Count Hint (1), Count (1) */
            if (iter.IsIteratorOverrun(4))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            flags = (int)(filterList.Flags & ~FilterListFlags.HAS_PER_ENTRY_PERM_DATA);
            iter._writer.WriteUByte(flags);
            /* container type needs to be scaled before we can send it */
            iter._writer.WriteUByte(filterList.ContainerType - DataTypes.CONTAINER_TYPE_MIN);

            if (filterList.CheckHasTotalCountHint())
            {
                iter._writer.WriteUByte(filterList.TotalCountHint);
            }

            iter._curBufPos = iter._writer.Position();

            _levelInfo._countWritePos = iter._curBufPos;
            iter._curBufPos += 1;
            iter._writer.SkipBytes(1);

            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFilterListComplete(EncodeIterator iter, bool success, FilterList filterList)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
            int count = _levelInfo._currentCount;
            int flags;

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES || (_levelInfo._encodingState == EncodeIteratorStates.WAIT_COMPLETE), "Unexpected encoding attempted");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, " Unexpected encoding attempted");
                Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteUByte(count);
                iter._writer.Position(iter._curBufPos);

                if ((_levelInfo._flags & EncodeIteratorFlags.HAS_PER_ENTRY_PERM) > 0)
                {
                    /* write per_entry_perm bit */
                    /* flags are first byte of container */
                    flags = (int)(filterList.Flags | FilterListFlags.HAS_PER_ENTRY_PERM_DATA);
                    iter._writer.Position(_levelInfo._containerStartPos);
                    iter._writer.WriteUByte(flags);
                    iter._writer.Position(iter._curBufPos);
                }
            }
            else
            {
                iter._curBufPos = _levelInfo._containerStartPos;
                iter._writer.Position(iter._curBufPos);
            }
            // levelInfo._encodingState = EncodeIteratorStates.COMPLETE;
            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFilterEntryInit(EncodeIterator iter, FilterEntry entry, int size)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, " Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.FILTER_LIST, "Invalid encoding attempted");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            FilterList filterList = (FilterList)_levelInfo._listType;
            _levelInfo._initElemStartPos = iter._curBufPos;

            if ((ret = EncodeFilterEntryInternal(iter, filterList, entry)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

            if (((filterList.ContainerType != DataTypes.NO_DATA) && !((entry.Flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0) 
                || (entry.ContainerType != DataTypes.NO_DATA) && ((entry.Flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)) 
                && (entry.Action != FilterEntryActions.CLEAR))
            {
                if (iter.IsIteratorOverrun(((size == 0 || size >= 0xFE) ? 3 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = SetupU16Mark(_levelInfo._internalMark, size, iter._curBufPos);
                iter._writer.Position(iter._curBufPos);
            }
            else
            {
                _levelInfo._internalMark._sizeBytes = 0;
                _levelInfo._internalMark._sizePos = iter._curBufPos;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFilterEntryComplete(EncodeIterator iter, bool success)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != _levelInfo._listType, "Invalid _levelInfo._listType in as NULL");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");

            if (success)
            {
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    if ((ret = FinishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return ret;
                    }
                }
                else
                {
                    if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.ENTRY_WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return CodecReturnCode.INVALID_DATA;
                    }
                    _levelInfo._internalMark._sizePos = 0;
                }
                _levelInfo._currentCount++;
            }
            else
            {
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            _levelInfo._initElemStartPos = 0;
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFilterEntry(EncodeIterator iter, FilterEntry filterEntry)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != filterEntry, "Invalid filterEntryInt in as NULL");

            CodecReturnCode ret;
            FilterList filterList;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");

            filterList = (FilterList)_levelInfo._listType;

            _levelInfo._initElemStartPos = iter._curBufPos;

            if ((ret = EncodeFilterEntryInternal(iter, filterList, filterEntry)) < 0)
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                return ret;
            }

            if (((filterList.ContainerType != DataTypes.NO_DATA) && !((filterEntry.Flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0) 
                || (filterEntry.ContainerType != DataTypes.NO_DATA) && ((filterEntry.Flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)) 
                && (filterEntry.Action != FilterEntryActions.CLEAR))
            {

                int len = filterEntry._encodedData.GetLength();

                // len value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0xFE)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(filterEntry._encodedData);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUShort16obLong(len);
                    iter._writer.Write(filterEntry._encodedData);

                }
                else
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return CodecReturnCode.INVALID_DATA;
                }

                iter._curBufPos = iter._writer.Position();
            }

            _levelInfo._currentCount++;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeFilterEntryInternal(EncodeIterator iter, FilterList filterList, FilterEntry filterEntry)
        {
            int flags = 0;

            if (iter.IsIteratorOverrun(2))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            flags = (int)filterEntry.Flags;
            flags <<= 4;
            flags += (int)filterEntry.Action;

            /* store flags */
            iter._writer.WriteUByte(flags);

            /* Store id as UInt8 */
            iter._writer.WriteUByte(filterEntry.Id);

            /* Store _containerType as UInt8 */
            if (filterEntry.CheckHasContainerType())
            {
                if (iter.IsIteratorOverrun(1))
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                if (!(ValidAggregateDataType(filterEntry.ContainerType)))
                {
                    return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
                }

                /* container type needs to be scaled before its encoded */
                iter._writer.WriteUByte(filterEntry.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            }

            iter._curBufPos = iter._writer.Position();

            /* Store perm lock */
            if (filterEntry.CheckHasPermData())
            {
                iter._levelInfo[iter._encodingLevel]._flags |= EncodeIteratorFlags.HAS_PER_ENTRY_PERM;
                /* Encode the permission expression */
                if (filterEntry._permData.Length == 0)
                {
                    /* just encode 0 bytes since none exists */
                    int zero = 0;
                    iter._writer.WriteUByte(zero);
                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    int len = filterEntry._permData.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(filterEntry.PermData);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(filterEntry.PermData);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapInit(EncodeIterator iter, Map map, int summaryMaxSize, int setMaxSize)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != map, "Invalid mapInt in as NULL");

            EncodingLevel _levelInfo;
            int flags;
            CodecReturnCode ret;

            if (!ValidPrimitiveDataType(map.KeyPrimitiveType))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            if (!ValidAggregateDataType(map.ContainerType))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            if (++iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            _levelInfo = iter._levelInfo[iter._encodingLevel];
            _levelInfo.Init(DataTypes.MAP, EncodeIteratorStates.NONE, map, iter._curBufPos);
            _levelInfo._flags = EncodeIteratorFlags.NONE;

            /* Make sure that required elements can be encoded */
            /* Flags (1), keyPrimitiveType (1), Data Format (1), Count (2) */
            if (iter.IsIteratorOverrun(5))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            /* If summary data and/or set definitions are to be encoded (but not pre-encoded),
			 * reserve space for the count & totalCountHint that will be encoded afterwards. */
            int reservedBytes = 2 + (map.CheckHasTotalCountHint() ? 4 : 0);

            flags = (int)(map.Flags & ~MapFlags.HAS_PER_ENTRY_PERM_DATA);

            iter._writer.WriteUByte(flags);
            iter._writer.WriteUByte(map.KeyPrimitiveType);
            /* container type needs to be scaled before its encoded */
            iter._writer.WriteUByte(map.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            if (map.CheckHasKeyFieldId())
            {
                if (iter.IsIteratorOverrun(2))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._writer.WriteShort(map.KeyFieldId);
                iter._curBufPos = iter._writer.Position();
            }

            /* Check for List Set Definitions */
            if ((ret = EncodeMapSetDefsInit(iter, map, summaryMaxSize, setMaxSize, _levelInfo)) <= CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            /* Check for Summary Data */
            if ((ret = EncodeMapSummaryDataInit(iter, map, summaryMaxSize, _levelInfo)) <= CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            if (iter.IsIteratorOverrun(reservedBytes))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            ret = FinishMapInit(map, iter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeMapSetDefsInit(EncodeIterator iter, Map map, int summaryMaxSize, int setMaxSize, EncodingLevel levelInfo)
        {
            if (map.CheckHasSetDefs())
            {
                /* We have list set definitions */
                if (map._encodedSetDefs.Length > 0)
                {
                    /* The set data is already encoded. */
                    /* Make sure the data can be encoded */
                    int len = map._encodedSetDefs.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(map._encodedSetDefs);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(map._encodedSetDefs);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    int reservedBytes = 2 + (map.CheckHasTotalCountHint() ? 4 : 0);

                    if (map.CheckHasSummaryData())
                    {
                        if (map.EncodedSummaryData.Data() != null)
                        {
                            /* Reserve space to encode the summaryData and its length */
                            int summaryLength = map.EncodedSummaryData.GetLength();
                            reservedBytes += summaryLength + ((summaryLength >= 0x80) ? 2 : 1);
                        }
                        else
                        {
                            /* store # of bytes for summary data so user does not pass it in again */
                            if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
                            {
                                levelInfo._internalMark2._sizeBytes = 2;
                            }
                            else
                            {
                                levelInfo._internalMark2._sizeBytes = 1;
                            }

                            /* Reserve space for the summaryData size mark. */
                            reservedBytes += levelInfo._internalMark2._sizeBytes;
                        }
                    }

                    if (iter.IsIteratorOverrun(reservedBytes + ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._curBufPos = SetupU15Mark(levelInfo._internalMark, setMaxSize, iter._curBufPos);
                    iter._writer.Position(iter._curBufPos);

                    /* Back up endBufPos to account for reserved bytes. */
                    levelInfo._reservedBytes = reservedBytes;
                    iter._endBufPos -= reservedBytes;
                    iter._writer.ReserveBytes(reservedBytes);

                    /* Save state and return */
                    levelInfo._encodingState = EncodeIteratorStates.SET_DEFINITIONS;
                    return CodecReturnCode.SUCCESS;
                }
            }

            return (CodecReturnCode)1;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeMapSummaryDataInit(EncodeIterator iter, Map map, int summaryMaxSize, EncodingLevel levelInfo)
        {
            if (map.CheckHasSummaryData())
            {
                /* We have summary data */
                if (map._encodedSummaryData.Length > 0)
                {
                    /* The summary data is already encoded. */
                    /* Make sure the data can be encoded */
                    int len = map._encodedSummaryData.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(map._encodedSummaryData);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(map._encodedSummaryData);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    int reservedBytes = 2 + (map.CheckHasTotalCountHint() ? 4 : 0);

                    if (iter.IsIteratorOverrun(reservedBytes + ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
                    {
                        levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._curBufPos = SetupU15Mark(levelInfo._internalMark2, summaryMaxSize, iter._curBufPos);
                    iter._writer.Position(iter._curBufPos);

                    /* Back up endBufPos to account for the reserved bytes. */
                    levelInfo._reservedBytes = reservedBytes;
                    iter._endBufPos -= reservedBytes;
                    iter._writer.ReserveBytes(reservedBytes);

                    /* Save state and return */
                    levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                    return (int)CodecReturnCode.SUCCESS;
                }
            }

            return (CodecReturnCode)1;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode FinishMapInit(Map map, EncodeIterator iter)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Move endBufPos back to original position if bytes were reserved. */
            iter._endBufPos += _levelInfo._reservedBytes;
            iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
            _levelInfo._reservedBytes = 0;

            /* Store Total count hint */
            if (map.CheckHasTotalCountHint())
            {
                if ((ret = iter._writer.WriteUInt30rb(map.TotalCountHint)) != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                iter._curBufPos = iter._writer.Position();
            }

            _levelInfo._countWritePos = iter._curBufPos;
            iter._curBufPos += 2;
            iter._writer.SkipBytes(2);
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapComplete(EncodeIterator iter, Map map, bool success)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
            int flags;

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.MAP, "Invalid encoding attempted - wrong container");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
                Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteShort(_levelInfo._currentCount);
                iter._writer.Position(iter._curBufPos);

                if ((_levelInfo._flags & EncodeIteratorFlags.HAS_PER_ENTRY_PERM) > 0)
                {
                    /* write per_entry_perm bit */
                    /* flags are first byte of container */
                    flags = (int)(map.Flags | MapFlags.HAS_PER_ENTRY_PERM_DATA);
                    iter._writer.Position(_levelInfo._containerStartPos);
                    iter._writer.WriteUByte(flags);
                    iter._writer.Position(iter._curBufPos);
                }
            }
            else
            {
                iter._curBufPos = _levelInfo._containerStartPos;
                iter._writer.Position(iter._curBufPos);

                /* Move endBufPos back to original position if bytes were reserved. */
                iter._endBufPos += _levelInfo._reservedBytes;
                iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
                _levelInfo._reservedBytes = 0;
            }
            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapSummaryDataComplete(EncodeIterator iter, Map mapInt, bool success)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.MAP, "Invalid encoding attempted - wrong container");
            Debug.Assert(iter._curBufPos != 0, "Invalid iterator use - check buffer");

            if (success)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return ret;
                }

                if ((ret = FinishMapInit((Map)_levelInfo._listType, iter)) != CodecReturnCode.SUCCESS)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return ret;
                }
            }
            else
            {
                Debug.Assert(_levelInfo._internalMark2._sizePos != 0, "Invalid encoding attempted");
                iter._curBufPos = _levelInfo._internalMark2._sizePos + _levelInfo._internalMark2._sizeBytes;
                iter._writer.Position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapEntryInit(EncodeIterator iter, MapEntry mapEntry, object keyData, int maxEncodingSize)
        {
            CodecReturnCode ret;
            Map map;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != mapEntry, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.MAP, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(iter._curBufPos != 0, "Invalid iterator use - check buffer");

            map = (Map)_levelInfo._listType;

            Debug.Assert(map.KeyPrimitiveType != DataTypes.NO_DATA, "Invalid key type specified");

            _levelInfo._initElemStartPos = iter._curBufPos;

            if ((ret = EncodeMapEntryInternal(iter, map, mapEntry, keyData)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

            if ((mapEntry.Action != MapEntryActions.DELETE) && (map.ContainerType != DataTypes.NO_DATA))
            {
                if (iter.IsIteratorOverrun(((maxEncodingSize == 0 || maxEncodingSize >= 0xFE) ? 3 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = SetupU16Mark(_levelInfo._internalMark, maxEncodingSize, iter._curBufPos);
                iter._writer.Position(iter._curBufPos);
            }
            else
            {
                /* set up mark so we know not to close it */
                _levelInfo._internalMark._sizeBytes = 0;
                _levelInfo._internalMark._sizePos = iter._curBufPos;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeMapEntryInternal(EncodeIterator iter, Map map, MapEntry mapEntry, object key)
        {
            int flags = 0;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Flags byte made up of flags and action */
            flags = (int)mapEntry.Flags;
            flags <<= 4;
            flags += (int)mapEntry.Action;

            if (iter.IsIteratorOverrun(1))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            /* Encode the flags */
            iter._writer.WriteUByte(flags);
            iter._curBufPos = iter._writer.Position();

            /* Check for optional permissions expression per entry */
            if (mapEntry.CheckHasPermData())
            {
                /* indicate that we want to set per-entry perm since the user encoded perm data */
                _levelInfo._flags |= EncodeIteratorFlags.HAS_PER_ENTRY_PERM;

                /* Encode the permission expression */
                if (mapEntry._permData.Length == 0)
                {
                    if (iter.IsIteratorOverrun(1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    /* Just encode 0 bytes since none exists */
                    int zero = 0;
                    iter._writer.WriteUByte(zero);
                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    int len = mapEntry.PermData.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(mapEntry._permData);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(mapEntry._permData);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
            }

            /* not pre-encoded - encode it */
            if (key != null)
            {
                CodecReturnCode ret = 0;

                if ((map.KeyPrimitiveType >= DataTypes.DATETIME_9) || !(PrimitiveEncoder.ValidEncodeActions(map.KeyPrimitiveType)))
                {
                    ret = CodecReturnCode.UNSUPPORTED_DATA_TYPE;
                }
                else
                {
                    ret = Encoders.PrimitiveEncoder.EncodeSetData(iter, key, map.KeyPrimitiveType);
                }

                if (ret < 0)
                {
                    return ret;
                }
            }
            else if (mapEntry._encodedKey.Length > 0)
            {
                /* Check for pre-encoded key */
                /* We probably don't need to check data or length as the ASSERTS should prevent it,
				 * however those are currently only on debug mode */
                Debug.Assert(0 != mapEntry._encodedKey.Length, "Blank key not allowed");

                /* Key is pre-encoded. */
                /* For buffers, etc; size need to be encoded, hence encoding it as small buffer */
                int len = mapEntry._encodedKey.GetLength();
                if (len < 0x80)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(mapEntry._encodedKey);
                }
                else
                {
                    if (iter.IsIteratorOverrun(2 + len))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    iter._writer.WriteUShort15rbLong((short)len);
                    iter._writer.Write(mapEntry._encodedKey);
                }

                iter._curBufPos = iter._writer.Position();
            }
            else
            {
                return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapEntryComplete(EncodeIterator iter, MapEntry mapEntryInt, bool success)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.MAP, "Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");

            if (success)
            {
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    if ((ret = FinishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return ret;
                    }
                }
                else
                {
                    /* size bytes is 0 - this means that action was clear or df was no data */
                    if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                    {
                        /* something was written when it shouldnt have been */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return CodecReturnCode.INVALID_DATA;
                    }
                    _levelInfo._internalMark._sizePos = 0;
                }
                _levelInfo._currentCount++;
            }
            else
            {
                /* Reset the pointer */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            _levelInfo._initElemStartPos = 0;
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapEntry(EncodeIterator iter, MapEntry mapEntry, object keyData)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != mapEntry, "Invalid mapEntryInt in as NULL");

            CodecReturnCode ret;
            Map map;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._containerType == DataTypes.MAP, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");

            map = (Map)_levelInfo._listType;
            _levelInfo._initElemStartPos = iter._curBufPos;

            if ((ret = EncodeMapEntryInternal(iter, map, mapEntry, keyData)) < 0)
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                return ret;
            }

            if ((mapEntry.Action != MapEntryActions.DELETE) && (map.ContainerType != DataTypes.NO_DATA))
            {
                int len = mapEntry._encodedData.GetLength();
                if (len < 0xFE)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(mapEntry._encodedData);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUShort16obLong(len);
                    iter._writer.Write(mapEntry._encodedData);

                }
                else
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return CodecReturnCode.INVALID_DATA;
                }

                iter._curBufPos = iter._writer.Position();
            }

            _levelInfo._currentCount++;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesInit(EncodeIterator iter, Series series, int summaryMaxSize, int setMaxSize)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != series, "Invalid seriesInt in as NULL");

            EncodingLevel _levelInfo;

            if (!(ValidAggregateDataType(series.ContainerType)))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            if (++iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            _levelInfo = iter._levelInfo[iter._encodingLevel];
            _levelInfo.Init(DataTypes.SERIES, EncodeIteratorStates.NONE, series, iter._curBufPos);

            /* Make sure required elements can be encoded */
            /* Flags (1), _containerType (1), Count (2) */
            if (iter.IsIteratorOverrun(2))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            iter._writer.WriteUByte((int)series.Flags);
            /* container type needs to be scaled before its encoded */
            iter._writer.WriteUByte(series.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            /* If summary data and/or set definitions are to be encoded (but not pre-encoded),
			 * reserve space for the count & totalCountHint that will be encoded afterwards. */
            int reservedBytes = 2 + (series.CheckHasTotalCountHint() ? 4 : 0);

            /* check if we have list set definitions */
            if (series.CheckHasSetDefs())
            {
                /* we have definitions */
                if (series.EncodedSetDefs.Data() != null)
                {
                    /* the set data is already encoded */
                    /* make sure it fits */
                    int len = series.EncodedSetDefs.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(series.EncodedSetDefs);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(series.EncodedSetDefs);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    if (series.CheckHasSummaryData())
                    {
                        if (series.EncodedSummaryData.Data() != null)
                        {
                            /* Reserve space to encode the summaryData and its length */
                            int summaryLength = series.EncodedSummaryData.GetLength();
                            reservedBytes += summaryLength + ((summaryLength >= 0x80) ? 2 : 1);
                        }
                        else
                        {
                            /* store # of bytes for summary data so user does not pass it in again */
                            if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
                            {
                                _levelInfo._internalMark2._sizeBytes = 2;
                            }
                            else
                            {
                                _levelInfo._internalMark2._sizeBytes = 1;
                            }

                            /* Reserve space for the summaryData size mark. */
                            reservedBytes += _levelInfo._internalMark2._sizeBytes;
                        }
                    }

                    if (iter.IsIteratorOverrun(reservedBytes + ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._curBufPos = SetupU15Mark(_levelInfo._internalMark, setMaxSize, iter._curBufPos);
                    iter._writer.Position(iter._curBufPos);

                    /* Back up endBufPos to account for reserved bytes. */
                    _levelInfo._reservedBytes = reservedBytes;
                    iter._endBufPos -= reservedBytes;
                    iter._writer.ReserveBytes(reservedBytes);

                    /* save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DEFINITIONS;
                    return CodecReturnCode.SUCCESS;
                }
            }

            /* check for summary data */
            if (series.CheckHasSummaryData())
            {
                /* we have summary data */
                if (series.EncodedSummaryData.Data() != null)
                {
                    /* this is already encoded */
                    /* make sure it fits */
                    int len = series.EncodedSummaryData.GetLength();
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(series.EncodedSummaryData);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(series.EncodedSummaryData);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    if (iter.IsIteratorOverrun(reservedBytes + ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._curBufPos = SetupU15Mark(_levelInfo._internalMark2, summaryMaxSize, iter._curBufPos);
                    iter._writer.Position(iter._curBufPos);

                    /* Back up endBufPos to account for the reserved bytes. */
                    _levelInfo._reservedBytes = reservedBytes;
                    iter._endBufPos -= reservedBytes;
                    iter._writer.ReserveBytes(reservedBytes);

                    /* save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                    return CodecReturnCode.SUCCESS;
                }
            }

            if (iter.IsIteratorOverrun(reservedBytes))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            CodecReturnCode ret = FinishSeriesInit(series, iter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode FinishSeriesInit(Series series, EncodeIterator iter)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Move endBufPos back to original position if bytes were reserved. */
            iter._endBufPos += _levelInfo._reservedBytes;
            iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
            _levelInfo._reservedBytes = 0;

            /* store count hint */
            if (series.CheckHasTotalCountHint())
            {
                ret = iter._writer.WriteUInt30rb(series.TotalCountHint);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                iter._curBufPos = iter._writer.Position();
            }

            /* store the count position */
            _levelInfo._countWritePos = iter._curBufPos;
            iter._curBufPos += 2;
            iter._writer.SkipBytes(2);
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesComplete(EncodeIterator iter, bool success)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.SERIES, "Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
                Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteShort(_levelInfo._currentCount);
                iter._writer.Position(iter._curBufPos);

            }
            else
            {
                iter._curBufPos = _levelInfo._containerStartPos;
                iter._writer.Position(iter._curBufPos);

                /* Move endBufPos back to original position if bytes were reserved. */
                iter._endBufPos += _levelInfo._reservedBytes;
                iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
                _levelInfo._reservedBytes = 0;
            }
            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesEntryInit(EncodeIterator iter, SeriesEntry seriesEntry, int maxEncodingSize)
        {
            Series series;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != seriesEntry, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.SERIES, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(iter._curBufPos != 0, "Invalid encoding attempted - check buffer");

            series = (Series)_levelInfo._listType;

            _levelInfo._initElemStartPos = iter._curBufPos;

            _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

            if (series.ContainerType != DataTypes.NO_DATA)
            {
                if (iter.IsIteratorOverrun(((maxEncodingSize == 0 || maxEncodingSize >= 0xFE) ? 3 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = SetupU16Mark(_levelInfo._internalMark, maxEncodingSize, iter._curBufPos);
                iter._writer.Position(iter._curBufPos);
            }
            else
            {
                _levelInfo._internalMark._sizeBytes = 0;
                _levelInfo._internalMark._sizePos = iter._curBufPos;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesEntryComplete(EncodeIterator iter, bool success)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.SERIES, "Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");

            if (success)
            {
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    if ((ret = FinishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return ret;
                    }
                }
                else
                {
                    /* size bytes is 0 - this means that action was clear or df was no data */
                    if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                    {
                        /* something was written when it shouldnt have been */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return CodecReturnCode.INVALID_DATA;
                    }
                    _levelInfo._internalMark._sizePos = 0;
                }

                _levelInfo._currentCount++;
            }
            else
            {
                /* reset the pointer */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            _levelInfo._initElemStartPos = 0;
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesEntry(EncodeIterator iter, SeriesEntry seriesEntry)
        {
            Series series;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != seriesEntry, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.SERIES, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(iter._curBufPos != 0, "Invalid iterator use - check buffer");

            series = (Series)_levelInfo._listType;

            if (series.ContainerType != DataTypes.NO_DATA)
            {
                int len = seriesEntry._encodedData.GetLength();

                // len value is written on wire as uShort16ob
                // If the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // The code below checks if the buffer is sufficient for each case.
                if (len < 0xFE)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        /* no need for rollback because iterator was never moved */
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(seriesEntry._encodedData);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        /* no need for rollback because iterator was never moved */
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUShort16obLong(len);
                    iter._writer.Write(seriesEntry._encodedData);
                }
                else
                {
                    return CodecReturnCode.INVALID_DATA;
                }

                iter._curBufPos = iter._writer.Position();
            }

            _levelInfo._currentCount++;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorInit(EncodeIterator iter, Vector vector, int summaryMaxSize, int setMaxSize)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != vector, "Invalid vectorInt in as NULL");

            EncodingLevel _levelInfo;
            int flags;

            if (!(ValidAggregateDataType(vector.ContainerType)))
            {
                return CodecReturnCode.UNSUPPORTED_DATA_TYPE;
            }

            if (++iter._encodingLevel >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            _levelInfo = iter._levelInfo[iter._encodingLevel];
            _levelInfo.Init(DataTypes.VECTOR, EncodeIteratorStates.NONE, vector, iter._curBufPos);
            _levelInfo._flags = EncodeIteratorFlags.NONE;

            /* Make sure required elements can be encoded */
            /* Flags (1), _containerType (1), Count (2) */
            if (iter.IsIteratorOverrun(2))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            flags = (int)(vector.Flags & ~VectorFlags.HAS_PER_ENTRY_PERM_DATA);
            iter._writer.WriteUByte(flags);
            /* container type needs to be scaled before its encoded */
            iter._writer.WriteUByte(vector.ContainerType - DataTypes.CONTAINER_TYPE_MIN);
            iter._curBufPos = iter._writer.Position();

            /* If summary data and/or set definitions are to be encoded (but not pre-encoded),
			 * reserve space for the count & totalCountHint that will be encoded afterwards. */
            int reservedBytes = 2 + (vector.CheckHasTotalCountHint() ? 4 : 0);

            /* check for list set definitions */
            if (vector.CheckHasSetDefs())
            {
                /* We have list set definitions */
                if (vector.EncodedSetDefs.Data() != null)
                {
                    /* set data is already encoded */
                    /* make sure it can be put in buffer */
                    int len = vector.EncodedSetDefs.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(vector.EncodedSetDefs);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(vector.EncodedSetDefs);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    if (vector.CheckHasSummaryData())
                    {
                        if (vector.EncodedSummaryData.Data() != null)
                        {
                            /* Reserve space to encode the summaryData and its length */
                            int summaryLength = vector.EncodedSummaryData.GetLength();
                            reservedBytes += summaryLength + ((summaryLength >= 0x80) ? 2 : 1);
                        }
                        else
                        {
                            /* store # of bytes for summary data so user does not pass it in again */
                            if (summaryMaxSize == 0 || summaryMaxSize >= 0x80)
                            {
                                _levelInfo._internalMark2._sizeBytes = 2;
                            }
                            else
                            {
                                _levelInfo._internalMark2._sizeBytes = 1;
                            }

                            /* Reserve space for the summaryData size mark. */
                            reservedBytes += _levelInfo._internalMark2._sizeBytes;
                        }
                    }

                    if (iter.IsIteratorOverrun(reservedBytes + ((setMaxSize >= 0x80 || setMaxSize == 0) ? 2 : 1)))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._curBufPos = SetupU15Mark(_levelInfo._internalMark, setMaxSize, iter._curBufPos);
                    iter._writer.Position(iter._curBufPos);

                    /* Back up endBufPos to account for reserved bytes. */
                    _levelInfo._reservedBytes = reservedBytes;
                    iter._endBufPos -= reservedBytes;
                    iter._writer.ReserveBytes(reservedBytes);

                    /* Save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SET_DEFINITIONS;

                    return CodecReturnCode.SUCCESS;
                }
            }

            /* Check for summary data */
            if (vector.CheckHasSummaryData())
            {
                /* we have summary data */
                if (vector.EncodedSummaryData.Data() != null)
                {
                    /* the summary data is already encoded */
                    /* Make sure it can be put in buffer */
                    int len = vector.EncodedSummaryData.GetLength();
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(vector.EncodedSummaryData);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(vector.EncodedSummaryData);
                    }

                    iter._curBufPos = iter._writer.Position();
                }
                else
                {
                    if (iter.IsIteratorOverrun(reservedBytes + ((summaryMaxSize >= 0x80 || summaryMaxSize == 0) ? 2 : 1)))
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._curBufPos = SetupU15Mark(_levelInfo._internalMark2, summaryMaxSize, iter._curBufPos);
                    iter._writer.Position(iter._curBufPos);

                    /* Back up endBufPos to account for the reserved bytes. */
                    _levelInfo._reservedBytes = reservedBytes;
                    iter._endBufPos -= reservedBytes;
                    iter._writer.ReserveBytes(reservedBytes);

                    /* save state and return */
                    _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                    return CodecReturnCode.SUCCESS;
                }
            }

            if (iter.IsIteratorOverrun(reservedBytes))
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            CodecReturnCode ret;
            if ((ret = FinishVectorInit(vector, iter)) != CodecReturnCode.SUCCESS)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode FinishVectorInit(Vector vector, EncodeIterator iter)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Move endBufPos back to original position if bytes were reserved. */
            iter._endBufPos += _levelInfo._reservedBytes;
            iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
            _levelInfo._reservedBytes = 0;

            /* store count hint */
            if (vector.CheckHasTotalCountHint())
            {
                ret = iter._writer.WriteUInt30rb(vector._totalCountHint);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
                iter._curBufPos = iter._writer.Position();
            }

            /* store the count position */
            _levelInfo._countWritePos = iter._curBufPos;
            iter._curBufPos += 2;
            iter._writer.SkipBytes(2);
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorComplete(EncodeIterator iter, bool success, Vector vector)
        {
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];
            int flags;

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.VECTOR, "Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");
            Debug.Assert(iter._curBufPos <= iter._endBufPos, "Data exceeds iterators buffer length");

            if (success)
            {
                Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
                Debug.Assert(_levelInfo._countWritePos != 0, "Invalid encoding attempted");
                iter._writer.Position(_levelInfo._countWritePos);
                iter._writer.WriteUShort(_levelInfo._currentCount);
                iter._writer.Position(iter._curBufPos);

                if ((_levelInfo._flags & EncodeIteratorFlags.HAS_PER_ENTRY_PERM) > 0)
                {
                    /* write per_entry_perm bit */
                    /* flags are first byte of container */
                    flags = (int)(vector.Flags | VectorFlags.HAS_PER_ENTRY_PERM_DATA);
                    iter._writer.Position(_levelInfo._containerStartPos);
                    iter._writer.WriteUByte(flags);
                    iter._writer.Position(iter._curBufPos);
                }
            }
            else
            {
                iter._curBufPos = _levelInfo._containerStartPos;
                iter._writer.Position(iter._curBufPos);

                /* Move endBufPos back to original position if bytes were reserved. */
                iter._endBufPos += _levelInfo._reservedBytes;
                iter._writer.UnreserveBytes(_levelInfo._reservedBytes);
                _levelInfo._reservedBytes = 0;

            }
            --iter._encodingLevel;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorSummaryDataComplete(EncodeIterator iter, Vector vector, bool success)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.VECTOR, "Invalid encoding attempted - wrong type");
            Debug.Assert(iter._curBufPos != 0, "Invalid encoding attempted");

            if (success)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return ret;
                }

                if ((ret = FinishVectorInit(vector, iter)) != CodecReturnCode.SUCCESS)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return ret;
                }
            }
            else
            {
                Debug.Assert(_levelInfo._internalMark2._sizePos != 0, "Invalid encoding attempted");
                iter._curBufPos = _levelInfo._internalMark2._sizePos + _levelInfo._internalMark2._sizeBytes;
                iter._writer.Position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        // Part 8
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorEntryInit(EncodeIterator iter, VectorEntry vectorEntry, int maxEncodingSize)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != vectorEntry, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.VECTOR, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(iter._curBufPos != 0, "Invalid encoding attempted");

            Vector vector = (Vector)_levelInfo._listType;

            _levelInfo._initElemStartPos = iter._curBufPos;

            if ((ret = EncodeVectorEntryInternal(iter, vector, vectorEntry)) < 0)
            {
                _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                return ret;
            }

            _levelInfo._encodingState = EncodeIteratorStates.ENTRY_INIT;

            if ((vectorEntry.Action != VectorEntryActions.CLEAR) && (vectorEntry.Action != VectorEntryActions.DELETE) && (vector.ContainerType != DataTypes.NO_DATA))
            {
                if (iter.IsIteratorOverrun(((maxEncodingSize == 0 || maxEncodingSize >= 0xFE) ? 3 : 1)))
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                iter._curBufPos = SetupU16Mark(_levelInfo._internalMark, maxEncodingSize, iter._curBufPos);
                iter._writer.Position(iter._curBufPos);
            }
            else
            {
                _levelInfo._internalMark._sizeBytes = 0;
                _levelInfo._internalMark._sizePos = iter._curBufPos;
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorEntry(EncodeIterator iter, VectorEntry vectorEntry)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != vectorEntry, "Invalid vectorEntryInt in as NULL");

            CodecReturnCode ret;
            Vector vector;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.ENTRIES, "Unexpected encoding attempted");
            Debug.Assert(_levelInfo._containerType == DataTypes.VECTOR, "Invalid encoding attempted - wrong type");

            vector = (Vector)_levelInfo._listType;

            _levelInfo._initElemStartPos = iter._curBufPos;

            if ((ret = EncodeVectorEntryInternal(iter, vector, vectorEntry)) < 0)
            {
                /* rollback */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
                return ret;
            }

            /* encode data as small buffer */
            /* we should only have data if we are not deleting/clearing a position */
            if ((vectorEntry.Action != VectorEntryActions.CLEAR) && (vectorEntry.Action != VectorEntryActions.DELETE) && (vector.ContainerType != DataTypes.NO_DATA))
            {
                int len = vectorEntry._encodedData.GetLength();

                // len value is written on wire as uShort16ob
                // if the value is smaller than 0xFE, it is written on the wire as one byte,
                // otherwise, it is written as three bytes by calling writeUShort16obLong method.
                // the code below checks if the buffer is sufficient for each case.
                if (len < 0xFE)
                {
                    if (iter.IsIteratorOverrun(1 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer._buffer.Write((byte)len);
                    iter._writer.Write(vectorEntry._encodedData);
                }
                else if (len <= 0xFFFF)
                {
                    if (iter.IsIteratorOverrun(3 + len))
                    {
                        /* rollback */
                        iter._curBufPos = _levelInfo._initElemStartPos;
                        iter._writer.Position(iter._curBufPos);
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    iter._writer.WriteUShort16obLong(len);
                    iter._writer.Write(vectorEntry._encodedData);
                }
                else
                {
                    /* rollback */
                    iter._curBufPos = _levelInfo._initElemStartPos;
                    iter._writer.Position(iter._curBufPos);
                    return CodecReturnCode.INVALID_DATA;
                }

                iter._curBufPos = iter._writer.Position();
            }

            _levelInfo._currentCount++;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private static CodecReturnCode EncodeVectorEntryInternal(EncodeIterator iter, Vector vector, VectorEntry vectorEntry)
        {
            CodecReturnCode ret;
            int flags = 0;

            if (iter.IsIteratorOverrun(1))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if (vectorEntry.Index > vectorEntry.MAX_INDEX)
            {
                return CodecReturnCode.INVALID_ARGUMENT;
            }

            /* Flags byte made up of flags and action */
            /* set action and flags in same 8 bit value */
            flags = (int)vectorEntry.Flags;
            /* shifts flags by 4 bits */
            flags <<= 4;
            /* sets action bits */
            flags += (int)vectorEntry.Action;

            /* put flags/action into packet */
            iter._writer.WriteUByte(flags);

            /* Store index as UInt30_rb */
            ret = iter._writer.WriteUInt30rb((int)vectorEntry.Index);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            if (vectorEntry.CheckHasPermData())
            {
                iter._levelInfo[iter._encodingLevel]._flags |= EncodeIteratorFlags.HAS_PER_ENTRY_PERM;
                /* encode perm exp as small buffer */
                if (vectorEntry.PermData.Data() == null)
                {
                    if (iter.IsIteratorOverrun(1))
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }
                    /* just encode 0 bytes since its empty */
                    int zero = 0;
                    iter._writer.WriteUByte(zero);
                }
                else
                {
                    int len = vectorEntry.PermData.GetLength();

                    // len value is written on wire as uShort15rb
                    // If the value is smaller than 0x80, it is written on the wire as one byte,
                    // otherwise, it is written as two bytes by calling writeUShort15rbLong method.
                    // The code below checks if the buffer is sufficient for each case.
                    if (len < 0x80)
                    {
                        if (iter.IsIteratorOverrun(1 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)len);
                        iter._writer.Write(vectorEntry.PermData);
                    }
                    else
                    {
                        if (iter.IsIteratorOverrun(2 + len))
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)len);
                        iter._writer.Write(vectorEntry.PermData);
                    }
                }
            }

            iter._curBufPos = iter._writer.Position();

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorEntryComplete(EncodeIterator iter, bool success)
        {
            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            /* Validations */
            Debug.Assert(null != iter && null != _levelInfo._listType, "Invalid parameters or parameters passed in as NULL");
            Debug.Assert(_levelInfo._containerType == DataTypes.VECTOR, "Invalid encoding attempted - wrong type");
            Debug.Assert(null != iter._buffer, "Invalid iterator use - check buffer");

            if (success)
            {
                if (_levelInfo._internalMark._sizeBytes > 0)
                {
                    if ((ret = FinishU16Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                    {
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return ret;
                    }
                }
                else
                {
                    /* size bytes is 0 - this means that action was clear or df was no data */
                    if (_levelInfo._internalMark._sizePos != iter._curBufPos)
                    {
                        /* something was written when it shouldnt have been */
                        _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                        _levelInfo._initElemStartPos = 0;
                        return CodecReturnCode.INVALID_DATA;
                    }
                    _levelInfo._internalMark._sizePos = 0;
                }
                _levelInfo._currentCount++;
            }
            else
            {
                /* reset the pointer */
                iter._curBufPos = _levelInfo._initElemStartPos;
                iter._writer.Position(iter._curBufPos);
            }

            _levelInfo._initElemStartPos = 0;
            _levelInfo._encodingState = EncodeIteratorStates.ENTRIES;

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeLocalElementSetDefDb(EncodeIterator iter, LocalElementSetDefDb setDb)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != setDb, "Invalid setDb in as NULL");

            int flags = 0;
            int defCount;
            int defs;
            EncodingLevel _levelInfo;

            if (iter._encodingLevel + 1 >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            _levelInfo = iter._levelInfo[iter._encodingLevel + 1];
            _levelInfo.Init(DataTypes.ELEMENT_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

            /* make sure that required elements can be encoded */
            if (iter.IsIteratorOverrun(2))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            /* store flags as uint8 */
            iter._writer._buffer.Write((byte)flags);
            iter._curBufPos = iter._writer.Position();

            _levelInfo._countWritePos = iter._curBufPos; // store count position
            iter._curBufPos++; // skip count byte
            iter._writer.Position(iter._curBufPos);
            /* go through defs and encode them */
            defCount = 0;
            for (defs = 0; defs <= LocalElementSetDefDb.MAX_LOCAL_ID; defs++)
            {
                if (setDb.Definitions[defs].SetId != LocalElementSetDefDb.BLANK_ID)
                {
                    int i;
                    ElementSetDef setDef = setDb.Definitions[defs];

                    Debug.Assert(setDef.SetId == defs, "Invalid set definition)");

                    if (setDef.SetId < 0x80)
                    {
                        /* make sure required elements fit */
                        if (iter.IsIteratorOverrun(2))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._containerStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)setDef.SetId);
                    }
                    else
                    {
                        /* make sure required elements fit */
                        if (iter.IsIteratorOverrun(3))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._containerStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)setDef.SetId);
                    }
                    iter._writer._buffer.Write((byte)setDef.Count);
                    iter._curBufPos = iter._writer.Position();

                    for (i = 0; i < setDef.Count; i++)
                    {
                        ElementSetDefEntry elementEnc = setDef.Entries[i];
                        Debug.Assert(null != elementEnc, "Invalid parameters or parameters passed in as NULL");

                        if (iter.IsIteratorOverrun(3 + elementEnc.Name.Length))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._containerStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        EncodeBuffer15(iter, elementEnc.Name);
                        iter._writer._buffer.Write((byte)elementEnc.DataType);
                        iter._curBufPos = iter._writer.Position();
                    }
                    ++defCount;
                }

            }

            iter._writer.Position(_levelInfo._countWritePos);
            iter._writer._buffer.Write((byte)defCount);
            iter._writer.Position(iter._curBufPos);

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesSetDefsComplete(EncodeIterator iter, bool success)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");

            CodecReturnCode ret;
            Series series;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._containerType == DataTypes.SERIES, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS, "Unexpected encoding attempted)");

            series = (Series)_levelInfo._listType;

            if (success)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    return (ret);
                }

                /* check for summary data */
                if (series.CheckHasSummaryData())
                {
                    if (series.EncodedSummaryData.Data() != null)
                    {
                        /* Buffer space was reserved to encode the entire summaryData.
						 * summaryData will be encoded now, so move endBufPos forward again. */
                        int summaryLength = series.EncodedSummaryData.GetLength();
                        int reservedBytes = summaryLength + ((summaryLength >= 0x80) ? 2 : 1);

                        iter._endBufPos += reservedBytes;
                        iter._writer.UnreserveBytes(reservedBytes);
                        _levelInfo._reservedBytes -= reservedBytes;

                        EncodeBuffer15(iter, series.EncodedSummaryData);
                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        /* we already stored length in the INIT call */
                        Debug.Assert(_levelInfo._internalMark2._sizePos == 0, "Invalid encoding attempted");

                        /* Buffer space was reserved for the summaryData size bytes.
						 * User will start encoding summaryData next, so move endBufPos forward again. */
                        int reservedBytes = _levelInfo._internalMark2._sizeBytes;
                        iter._endBufPos += reservedBytes;
                        iter._writer.UnreserveBytes(reservedBytes);
                        _levelInfo._reservedBytes -= reservedBytes;

                        /* Reserve size bytes for summary data. */
                        _levelInfo._internalMark2._sizePos = iter._curBufPos;
                        iter._curBufPos += _levelInfo._internalMark2._sizeBytes;
                        iter._writer.Position(iter._curBufPos);

                        /* save state and return */
                        _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                        return CodecReturnCode.SUCCESS;
                    }
                }

                if ((ret = FinishSeriesInit(series, iter)) != CodecReturnCode.SUCCESS)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
            }
            else
            {
                Debug.Assert(_levelInfo._internalMark._sizePos != 0, "Invalid encoding attempted");
                iter._curBufPos = _levelInfo._internalMark._sizePos + _levelInfo._internalMark._sizeBytes;
                iter._writer.Position(iter._curBufPos);
                /* Leave state as SET_DEFINITIONS */
            }

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeSeriesSummaryDataComplete(EncodeIterator iter, bool success)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");

            CodecReturnCode ret;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._containerType == DataTypes.SERIES, "Invalid encoding attempted - wrong type");

            if (success)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark2, iter._curBufPos)) < 0)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return (ret);
                }

                if ((ret = FinishSeriesInit((Series)_levelInfo._listType, iter)) != CodecReturnCode.SUCCESS)
                {
                    _levelInfo._encodingState = EncodeIteratorStates.WAIT_COMPLETE;
                    return ret;
                }
            }
            else
            {
                Debug.Assert(_levelInfo._internalMark2._sizePos != 0, "Invalid encoding attempted");
                iter._curBufPos = _levelInfo._internalMark2._sizePos + _levelInfo._internalMark2._sizeBytes;
                iter._writer.Position(iter._curBufPos);
                _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeMapSetDefsComplete(EncodeIterator iter, bool success)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");

            CodecReturnCode ret;
            Map map;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._containerType == DataTypes.MAP, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS, "Unexpected encoding attempted");

            map = (Map)_levelInfo._listType;

            if (success)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    return (ret);
                }

                /* check for summary data */
                if (map.CheckHasSummaryData())
                {
                    if (map.EncodedSummaryData.Data() != null)
                    {
                        /* Buffer space was reserved to encode the entire summaryData.
						 * summaryData will be encoded now, so move endBufPos forward again. */
                        int summaryLength = map.EncodedSummaryData.GetLength();
                        int reservedBytes = summaryLength + ((summaryLength >= 0x80) ? 2 : 1);

                        iter._endBufPos += reservedBytes;
                        iter._writer.UnreserveBytes(reservedBytes);
                        _levelInfo._reservedBytes -= reservedBytes;

                        Debug.Assert(_levelInfo._reservedBytes >= 0);

                        EncodeBuffer15(iter, map.EncodedSummaryData);
                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        /* we already stored length in the INIT call */
                        Debug.Assert(_levelInfo._internalMark2._sizePos == 0, "Invalid encoding attempted");

                        /* Buffer space was reserved for the summaryData size bytes.
						 * User will start encoding summaryData next, so move endBufPos forward again. */
                        int reservedBytes = _levelInfo._internalMark2._sizeBytes;
                        iter._endBufPos += reservedBytes;
                        iter._writer.UnreserveBytes(reservedBytes);
                        _levelInfo._reservedBytes -= reservedBytes;

                        _levelInfo._internalMark2._sizePos = iter._curBufPos;
                        iter._curBufPos += _levelInfo._internalMark2._sizeBytes;
                        iter._writer.Position(iter._curBufPos);

                        /* save state and return */
                        _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                        return CodecReturnCode.SUCCESS;
                    }
                }

                FinishMapInit(map, iter);
            }
            else
            {
                Debug.Assert(_levelInfo._internalMark._sizePos != 0, "Invalid encoding attempted");
                iter._curBufPos = _levelInfo._internalMark._sizePos + _levelInfo._internalMark._sizeBytes;
                iter._writer.Position(iter._curBufPos);
                /* Leave state as SET_DEFINITIONS */
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeVectorSetDefsComplete(EncodeIterator iter, bool success)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");

            CodecReturnCode ret;
            Vector vector;
            EncodingLevel _levelInfo = iter._levelInfo[iter._encodingLevel];

            Debug.Assert(_levelInfo._containerType == DataTypes.VECTOR, "Invalid encoding attempted - wrong type");
            Debug.Assert(_levelInfo._encodingState == EncodeIteratorStates.SET_DEFINITIONS, "Unexpected encoding attempted");

            vector = (Vector)_levelInfo._listType;

            if (success)
            {
                if ((ret = FinishU15Mark(iter, _levelInfo._internalMark, iter._curBufPos)) < 0)
                {
                    return (ret);
                }

                /* check for summary data */
                if (vector.CheckHasSummaryData())
                {
                    if (vector.EncodedSummaryData.Data() != null)
                    {
                        /* Buffer space was reserved to encode the entire summaryData.
						 * summaryData will be encoded now, so move endBufPos forward again. */
                        int summaryLength = vector.EncodedSummaryData.GetLength();
                        int reservedBytes = summaryLength + ((summaryLength >= 0x80) ? 2 : 1);

                        iter._endBufPos += reservedBytes;
                        iter._writer.UnreserveBytes(reservedBytes);
                        _levelInfo._reservedBytes -= reservedBytes;

                        EncodeBuffer15(iter, vector.EncodedSummaryData);
                        iter._curBufPos = iter._writer.Position();
                    }
                    else
                    {
                        /* we already stored length in the INIT call */
                        Debug.Assert(_levelInfo._internalMark2._sizePos == 0, "Invalid encoding attempted");

                        /* Buffer space was reserved for the summaryData size bytes. */
                        /* User will start encoding summaryData next, so move endBufPos forward again. */
                        int reservedBytes = _levelInfo._internalMark2._sizeBytes;
                        iter._endBufPos += reservedBytes;
                        iter._writer.UnreserveBytes(reservedBytes);
                        _levelInfo._reservedBytes -= reservedBytes;

                        /* Reserve size bytes for summary data. */
                        _levelInfo._internalMark2._sizePos = iter._curBufPos;
                        iter._curBufPos += _levelInfo._internalMark2._sizeBytes;
                        iter._writer.Position(iter._curBufPos);

                        /* save state and return */
                        _levelInfo._encodingState = EncodeIteratorStates.SUMMARY_DATA;
                        return CodecReturnCode.SUCCESS;
                    }
                }

                return FinishVectorInit(vector, iter);
            }
            else
            {
                Debug.Assert(_levelInfo._internalMark._sizePos != 0, "Invalid encoding attempted");
                iter._curBufPos = _levelInfo._internalMark._sizePos + _levelInfo._internalMark._sizeBytes;
                iter._writer.Position(iter._curBufPos);
                /* Leave state as SET_DEFINITIONS */
            }
            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeLocalFieldSetDefDb(EncodeIterator iter, LocalFieldSetDefDb setDb)
        {
            Debug.Assert(null != iter, "Invalid IterInt in as NULL");
            Debug.Assert(null != setDb, "Invalid setDb in as NULL");

            int flags = 0;
            int defCount;
            EncodingLevel _levelInfo;

            if (iter._encodingLevel + 1 >= EncodeIterator.ENC_ITER_MAX_LEVELS)
            {
                return CodecReturnCode.ITERATOR_OVERRUN;
            }
            _levelInfo = iter._levelInfo[iter._encodingLevel + 1];
            _levelInfo.Init(DataTypes.FIELD_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

            /* make sure that required elements can be encoded */
            if (iter.IsIteratorOverrun(2))
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            /* store flags as uint8 */
            iter._writer._buffer.Write((byte)flags);
            iter._curBufPos = iter._writer.Position();

            _levelInfo._countWritePos = iter._curBufPos; // store count position
            iter._curBufPos++; // skip count byte
            iter._writer.Position(iter._curBufPos);
            /* go through defs and encode them */
            defCount = 0;
            for (int defs = 0; defs <= LocalFieldSetDefDb.MAX_LOCAL_ID; defs++)
            {
                if (setDb.Definitions[defs].SetId != LocalFieldSetDefDb.BLANK_ID)
                {
                    int i;
                    FieldSetDef setDef = setDb.Definitions[defs];

                    Debug.Assert(setDef.SetId == defs, "Invalid set definition");

                    if (setDef.SetId < 0x80)
                    {
                        /* make sure required elements fit */
                        if (iter.IsIteratorOverrun(2))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._containerStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer._buffer.Write((byte)setDef.SetId);
                    }
                    else
                    {
                        /* make sure required elements fit */
                        if (iter.IsIteratorOverrun(3))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._containerStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }
                        iter._writer.WriteUShort15rbLong((short)setDef.SetId);
                    }
                    iter._writer._buffer.Write((byte)setDef.Count);
                    iter._curBufPos = iter._writer.Position();

                    for (i = 0; i < setDef.Count; i++)
                    {
                        FieldSetDefEntry fieldEnc = setDef.Entries[i];
                        Debug.Assert(null != fieldEnc, "Invalid parameters or parameters passed in as NULL");

                        if (iter.IsIteratorOverrun(3))
                        {
                            /* rollback */
                            iter._curBufPos = _levelInfo._containerStartPos;
                            iter._writer.Position(iter._curBufPos);
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        iter._writer.WriteShort((int)fieldEnc.FieldId);
                        iter._writer._buffer.Write((byte)fieldEnc.DataType);
                        iter._curBufPos = iter._writer.Position();
                    }
                    ++defCount;
                }
            }

            iter._writer.Position(_levelInfo._countWritePos);
            iter._writer._buffer.Write((byte)defCount);
            iter._writer.Position(iter._curBufPos);

            return CodecReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static CodecReturnCode EncodeFieldSetDefDb(EncodeIterator iter, FieldSetDefDb setDb)
		{
			Debug.Assert(null != iter, "Invalid IterInt in as NULL");
			Debug.Assert(null != setDb, "Invalid setDb in as NULL");

			int flags = 0;
			int defCount;
			EncodingLevel _levelInfo;

			if (iter._encodingLevel + 1 >= EncodeIterator.ENC_ITER_MAX_LEVELS)
			{
				return CodecReturnCode.ITERATOR_OVERRUN;
			}
			_levelInfo = iter._levelInfo[iter._encodingLevel + 1];
			_levelInfo.Init(DataTypes.FIELD_LIST, EncodeIteratorStates.SET_DEFINITIONS, setDb, iter._curBufPos);

			/* make sure that required elements can be encoded */
			if (iter.IsIteratorOverrun(2))
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

			/* store flags as uint8 */
			iter._writer._buffer.Write((byte)flags);
			iter._curBufPos = iter._writer.Position();

			_levelInfo._countWritePos = iter._curBufPos; // store count position
			iter._curBufPos++; // skip count byte
			iter._writer.Position(iter._curBufPos);
			/* go through defs and encode them */
			defCount = 0;
			for (int defs = 0; defs <= setDb.maxLocalId; defs++)
			{
				if (setDb.Definitions[defs].SetId != FieldSetDefDb.BLANK_ID)
				{
					int i;
					FieldSetDef setDef = setDb.Definitions[defs];

					Debug.Assert(setDef.SetId == defs, "Invalid set definition");

					if (setDef.SetId < 0x80)
					{
						/* make sure required elements fit */
						if (iter.IsIteratorOverrun(2))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer._buffer.Write((byte)setDef.SetId);
					}
					else
					{
						/* make sure required elements fit */
						if (iter.IsIteratorOverrun(3))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}
						iter._writer.WriteUShort15rbLong((short)setDef.SetId);
					}
					iter._writer._buffer.Write((byte)setDef.Count);
					iter._curBufPos = iter._writer.Position();

					for (i = 0; i < setDef.Count; i++)
					{
						FieldSetDefEntry fieldEnc = setDef.Entries[i];
						Debug.Assert(null != fieldEnc, "Invalid parameters or parameters passed in as NULL");

						if (iter.IsIteratorOverrun(3))
						{
							return CodecReturnCode.BUFFER_TOO_SMALL;
						}

						iter._writer.WriteShort((int)fieldEnc.FieldId);
						iter._writer._buffer.Write((byte)fieldEnc.DataType);
						iter._curBufPos = iter._writer.Position();
					}
					++defCount;
				}
			}

			iter._writer.Position(_levelInfo._countWritePos);
			iter._writer._buffer.Write((byte)defCount);
			iter._writer.Position(iter._curBufPos);

			return CodecReturnCode.SUCCESS;
		}
	}
}