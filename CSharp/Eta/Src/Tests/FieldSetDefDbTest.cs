/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;

using Xunit;
using Xunit.Categories;

namespace Refinitiv.Eta.Transports.Tests
{
    [Category("ByteBuffer")]
    public class FieldSetDefDbTest
    {
        [Fact]
        [Category("Unit")]
        public void FieldSetDefEncDecTest()
        {
            int i, j;

            Int fid = new Int();
            Int type = new Int();
            Int currentSetDef = new Int();

            Int tempInt = new Int();

            Buffer versionBuffer = new Buffer();

            FieldSetDefEntry[] setEntryArray;
            FieldSetDef[] setArray;


            GlobalFieldSetDefDb encDb = new GlobalFieldSetDefDb(), decDb = new GlobalFieldSetDefDb();

            Buffer buf = new Buffer();
            buf.Data(new ByteBuffer(2000));

            EncodeIterator encIter = new EncodeIterator();
            DecodeIterator decIter = new DecodeIterator();

            CodecError error;

            setEntryArray = new FieldSetDefEntry[8];


            setEntryArray[0] = new FieldSetDefEntry();
            setEntryArray[0].FieldId = 1;
            setEntryArray[0].DataType = DataTypes.INT;


            setEntryArray[1] = new FieldSetDefEntry();
            setEntryArray[1].FieldId = 2;
            setEntryArray[1].DataType = DataTypes.DOUBLE;


            setEntryArray[2] = new FieldSetDefEntry();
            setEntryArray[2].FieldId = 3;
            setEntryArray[2].DataType = DataTypes.REAL;


            setEntryArray[3] = new FieldSetDefEntry();
            setEntryArray[3].FieldId = 4;
            setEntryArray[3].DataType = DataTypes.DATE;

            fid.Value(5);
            type.Value(DataTypes.TIME);
            setEntryArray[4] = new FieldSetDefEntry();
            setEntryArray[4].FieldId = 5;
            setEntryArray[4].DataType = DataTypes.TIME;

            fid.Value(6);
            type.Value(DataTypes.DATETIME);
            setEntryArray[5] = new FieldSetDefEntry();
            setEntryArray[5].FieldId = 6;
            setEntryArray[5].DataType = DataTypes.DATETIME;

            setEntryArray[6] = new FieldSetDefEntry();
            setEntryArray[6].FieldId = 7;
            setEntryArray[6].DataType = DataTypes.ARRAY;

            setEntryArray[7] = new FieldSetDefEntry();
            setEntryArray[7].FieldId = 8;
            setEntryArray[7].DataType = DataTypes.UINT;

            setArray = new FieldSetDef[8];

            setArray[0] = new FieldSetDef();
            setArray[0].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[0].Count = 1;
            setArray[0].SetId = 16;

            setArray[1] = new FieldSetDef();
            setArray[1].Entries = setEntryArray;
            tempInt.Value(2);
            setArray[1].Count = 2;
            setArray[1].SetId = 17;

            setArray[2] = new FieldSetDef();
            setArray[2].Entries = setEntryArray;
            tempInt.Value(3);
            setArray[2].Count = 3;
            setArray[2].SetId = 18;

            setArray[3] = new FieldSetDef();
            setArray[3].Entries = setEntryArray;
            tempInt.Value(4);
            setArray[3].Count = 4;
            setArray[3].SetId = 19;

            setArray[4] = new FieldSetDef();
            setArray[4].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[4].Count = 5;
            setArray[4].SetId = 20;

            setArray[5] = new FieldSetDef();
            setArray[5].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[5].Count = 6;
            setArray[5].SetId = 21;

            setArray[6] = new FieldSetDef();
            setArray[6].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[6].Count = 7;
            setArray[6].SetId = 22;

            setArray[7] = new FieldSetDef();
            setArray[7].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[7].Count = 8;
            setArray[7].SetId = 23;

            versionBuffer.Data("1.0.1");

            for (i = 0; i < 8; i++)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, encDb.AddSetDef(setArray[i], out error));
            }

            encDb.Info_DictionaryID = 10;
            encDb.Info_version = versionBuffer;

            buf.Data().WritePosition = 0;
            encIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            currentSetDef.Value(0);
            Assert.Equal(CodecReturnCode.SUCCESS, encDb.Encode(encIter, currentSetDef, Dictionary.VerbosityValues.VERBOSE, out error));
            Assert.Equal(24, (int)currentSetDef.ToLong());

            decIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            Assert.Equal(CodecReturnCode.SUCCESS, decDb.Decode(decIter, Dictionary.VerbosityValues.VERBOSE, out error));
            Assert.True(versionBuffer.Equals(decDb.Info_version));
            Assert.Equal(10, decDb.Info_DictionaryID);

            for (i = 0; i < 8; i++)
            {
                Assert.True(decDb.Definitions[(int)setArray[i].SetId].SetId == setArray[i].SetId);
                Assert.True(decDb.Definitions[(int)setArray[i].SetId].Count == setArray[i].Count);
                for (j = 0; j < decDb.Definitions[(int)setArray[i].SetId].Count; j++)
                {
                    Assert.True(decDb.Definitions[(int)setArray[i].SetId].Entries[j].FieldId == setArray[i].Entries[j].FieldId);
                    Assert.True(decDb.Definitions[(int)setArray[i].SetId].Entries[j].DataType == setArray[i].Entries[j].DataType);
                }
            }
        }

        [Fact]
        [Category("Unit")]
        public void FieldSetEncDecTest()
        {
            FieldList encList = new FieldList(), decList = new FieldList();
            FieldEntry encEntry = new FieldEntry(), decEntry = new FieldEntry();

            Int encInt = new Int(), decInt = new Int();
            UInt encUInt = new UInt(), decUInt = new UInt();
            Real encReal = new Real(), decReal = new Real();
            Buffer encBuf = new Buffer();
            Date encDate = new Date(), decDate = new Date();
            Time encTime = new Time(), decTime = new Time();
            DateTime encDateTime = new DateTime(), decDateTime = new DateTime();
            Double encDouble = new Double(), decDouble = new Double();


            encInt.Value(-2000);
            encUInt.Value(2500);
            encReal.Value(15000, RealHints.EXPONENT_3);
            encBuf.Data("test");
            encDate.Value("25 JAN 2015");
            encTime.Value("12:10:5");
            encDateTime.Value("25 JAN 2015 12:10:5");
            encDouble.Value(1.23);

            int i;

            Int tempInt = new Int();

            Buffer versionBuffer = new Buffer();

            FieldSetDefEntry[] setEntryArray;
            FieldSetDef[] setArray;


            GlobalFieldSetDefDb encDb = new GlobalFieldSetDefDb();

            Buffer buf = new Buffer();
            buf.Data(new ByteBuffer(2000));

            EncodeIterator encIter = new EncodeIterator();
            DecodeIterator decIter = new DecodeIterator();

            CodecError error;

            setEntryArray = new FieldSetDefEntry[7];

            setEntryArray[0] = new FieldSetDefEntry
            {
                FieldId = 1,
                DataType = DataTypes.INT
            };

            setEntryArray[1] = new FieldSetDefEntry
            {
                FieldId = 2,
                DataType = DataTypes.DOUBLE
            };


            setEntryArray[2] = new FieldSetDefEntry
            {
                FieldId = 3,
                DataType = DataTypes.REAL
            };


            setEntryArray[3] = new FieldSetDefEntry
            {
                FieldId = 4,
                DataType = DataTypes.DATE
            };

            setEntryArray[4] = new FieldSetDefEntry
            {
                FieldId = 5,
                DataType = DataTypes.TIME
            };

            setEntryArray[5] = new FieldSetDefEntry
            {
                FieldId = 6,
                DataType = DataTypes.DATETIME
            };

            setEntryArray[6] = new FieldSetDefEntry
            {
                FieldId = 7,
                DataType = DataTypes.UINT
            };

            setArray = new FieldSetDef[8];

            setArray[0] = new FieldSetDef();
            setArray[0].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[0].Count = 1;
            setArray[0].SetId = 16;

            setArray[1] = new FieldSetDef();
            setArray[1].Entries = setEntryArray;
            tempInt.Value(2);
            setArray[1].Count = 2;
            setArray[1].SetId = 17;

            setArray[2] = new FieldSetDef();
            setArray[2].Entries = setEntryArray;
            tempInt.Value(3);
            setArray[2].Count = 3;
            setArray[2].SetId = 18;

            setArray[3] = new FieldSetDef();
            setArray[3].Entries = setEntryArray;
            tempInt.Value(4);
            setArray[3].Count = 4;
            setArray[3].SetId = 19;

            setArray[4] = new FieldSetDef();
            setArray[4].Entries = setEntryArray;
            tempInt.Value(5);
            setArray[4].Count = 5;
            setArray[4].SetId = 20;

            setArray[5] = new FieldSetDef();
            setArray[5].Entries = setEntryArray;
            tempInt.Value(6);
            setArray[5].Count = 6;
            setArray[5].SetId = 21;

            setArray[6] = new FieldSetDef();
            setArray[6].Entries = setEntryArray;
            tempInt.Value(7);
            setArray[6].Count = 7;
            setArray[6].SetId = 22;

            versionBuffer.Data("1.0.1");

            for (i = 0; i < 7; i++)
            {
                Assert.Equal(CodecReturnCode.SUCCESS, encDb.AddSetDef(setArray[i], out error));
            }

            encDb.Info_DictionaryID = 10;
            encDb.Info_version = versionBuffer;

            for (i = 0; i <= 6; i++)
            {
                buf.Data().Rewind();

                encIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                encIter.SetGlobalFieldSetDefDb(encDb);

                encList.Clear();
                encList.ApplyHasSetData();
                encList.ApplyHasSetId();

                encList.SetId = i + 16;

                if (i < 6)
                    encList.ApplyHasStandardData();

                Assert.Equal(CodecReturnCode.SUCCESS, encList.EncodeInit(encIter, null, 0));

                encEntry.Clear();
                encEntry.DataType = DataTypes.INT;
                encEntry.FieldId = 1;

                Assert.Equal((i == 0 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encInt));

                encEntry.Clear();
                encEntry.DataType = DataTypes.DOUBLE;
                encEntry.FieldId = 2;

                Assert.Equal((i == 1 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encDouble));

                encEntry.Clear();
                encEntry.DataType = DataTypes.REAL;
                encEntry.FieldId = 3;

                Assert.Equal((i == 2 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encReal));

                encEntry.Clear();
                encEntry.DataType = DataTypes.DATE;
                encEntry.FieldId = 4;

                Assert.Equal((i == 3 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encDate));

                encEntry.Clear();
                encEntry.DataType = DataTypes.TIME;
                encEntry.FieldId = 5;

                Assert.Equal((i == 4 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encTime));

                encEntry.Clear();
                encEntry.DataType = DataTypes.DATETIME;
                encEntry.FieldId = 6;

                Assert.Equal((i == 5 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encDateTime));

                encEntry.Clear();
                encEntry.DataType = DataTypes.UINT;
                encEntry.FieldId = 7;

                Assert.Equal((i == 6 ? CodecReturnCode.SET_COMPLETE : CodecReturnCode.SUCCESS), encEntry.Encode(encIter, encUInt));

                Assert.Equal(CodecReturnCode.SUCCESS, encList.EncodeComplete(encIter, true));

                decIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                decIter.SetGlobalFieldSetDefDb(encDb);

                decList.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decList.Decode(decIter, null));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[0].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decInt.Decode(decIter));
                Assert.True(decInt.Equals(encInt));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[1].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decDouble.Decode(decIter));
                Assert.True(decDouble.Equals(encDouble));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[2].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decReal.Decode(decIter));
                Assert.True(decReal.Equals(encReal));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[3].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decDate.Decode(decIter));
                Assert.True(decDate.Equals(encDate));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[4].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decTime.Decode(decIter));
                Assert.True(decTime.Equals(encTime));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[5].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decDateTime.Decode(decIter));
                Assert.True(decDateTime.Equals(encDateTime));

                decEntry.Clear();
                Assert.Equal(CodecReturnCode.SUCCESS, decEntry.Decode(decIter));
                Assert.Equal(setEntryArray[6].FieldId, decEntry.FieldId);

                Assert.Equal(CodecReturnCode.SUCCESS, decUInt.Decode(decIter));
                Assert.True(decUInt.Equals(encUInt));

            }
        }
    }
}
