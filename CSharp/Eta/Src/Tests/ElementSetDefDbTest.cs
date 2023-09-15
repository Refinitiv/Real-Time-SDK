/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using Xunit;
using Xunit.Categories;

namespace LSEG.Eta.Transports.Tests
{
    public class ElementSetDefDbTest
    {

        [Fact]
        [Category("Unit")]
        [Category("ByteBuffer")]
        public void elementSetDefEncDecTest()
        {
            int i, j;

            Int currentSetDef = new Int();

            Int tempInt = new Int();

            Buffer versionBuffer = new Buffer();

            ElementSetDefEntry[] setEntryArray;
            ElementSetDef[] setArray;


            GlobalElementSetDefDb encDb = new GlobalElementSetDefDb(), decDb = new GlobalElementSetDefDb();

            Buffer buf = new Buffer();
            buf.Data(new ByteBuffer(2000));
            Buffer nameBuffer = new Buffer();

            EncodeIterator encIter = new EncodeIterator();
            DecodeIterator decIter = new DecodeIterator();

            CodecError error;

            setEntryArray = new ElementSetDefEntry[8];


            setEntryArray[0] = new ElementSetDefEntry();
            nameBuffer.Data("INT");
            setEntryArray[0].Name = nameBuffer;
            setEntryArray[0].DataType = DataTypes.INT;


            setEntryArray[1] = new ElementSetDefEntry();
            nameBuffer.Data("DOUBLE");
            setEntryArray[1].Name = nameBuffer;
            setEntryArray[1].DataType = DataTypes.DOUBLE;


            setEntryArray[2] = new ElementSetDefEntry();
            nameBuffer.Data("REAL");
            setEntryArray[2].Name = nameBuffer;
            setEntryArray[2].DataType = DataTypes.REAL;


            setEntryArray[3] = new ElementSetDefEntry();
            nameBuffer.Data("DATE");
            setEntryArray[3].Name = nameBuffer;
            setEntryArray[3].DataType = DataTypes.DATE;

            setEntryArray[4] = new ElementSetDefEntry();
            nameBuffer.Data("TIME");
            setEntryArray[4].Name = nameBuffer;
            setEntryArray[4].DataType = DataTypes.TIME;

            setEntryArray[5] = new ElementSetDefEntry();
            nameBuffer.Data("DATETIME");
            setEntryArray[5].Name = nameBuffer;
            setEntryArray[5].DataType = DataTypes.DATETIME;

            setEntryArray[6] = new ElementSetDefEntry();
            nameBuffer.Data("ARRAY");
            setEntryArray[6].Name = nameBuffer;
            setEntryArray[6].DataType = DataTypes.ARRAY;

            setEntryArray[7] = new ElementSetDefEntry();
            nameBuffer.Data("UINT");
            setEntryArray[7].Name = nameBuffer;
            setEntryArray[7].DataType = DataTypes.UINT;

            setArray = new ElementSetDef[8];

            setArray[0] = new ElementSetDef();
            setArray[0].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[0].Count = 1;
            setArray[0].SetId = 16;

            setArray[1] = new ElementSetDef();
            setArray[1].Entries = setEntryArray;
            tempInt.Value(2);
            setArray[1].Count = 2;
            setArray[1].SetId = 17;

            setArray[2] = new ElementSetDef();
            setArray[2].Entries = setEntryArray;
            tempInt.Value(3);
            setArray[2].Count = 3;
            setArray[2].SetId = 18;

            setArray[3] = new ElementSetDef();
            setArray[3].Entries = setEntryArray;
            tempInt.Value(4);
            setArray[3].Count = 4;
            setArray[3].SetId = 19;

            setArray[4] = new ElementSetDef();
            setArray[4].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[4].Count = 5;
            setArray[4].SetId = 20;

            setArray[5] = new ElementSetDef();
            setArray[5].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[5].Count = 6;
            setArray[5].SetId = 21;

            setArray[6] = new ElementSetDef();
            setArray[6].Entries = setEntryArray;
            tempInt.Value(1);
            setArray[6].Count = 7;
            setArray[6].SetId = 22;

            setArray[7] = new ElementSetDef();
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

            buf.Data().Rewind();
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
                    Assert.True(decDb.Definitions[(int)setArray[i].SetId].Entries[j].Name.Equals(setArray[i].Entries[j].Name));
                    Assert.True(decDb.Definitions[(int)setArray[i].SetId].Entries[j].DataType == setArray[i].Entries[j].DataType);
                }
            }
        }

    }
}
