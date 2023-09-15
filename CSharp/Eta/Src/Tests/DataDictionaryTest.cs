/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Xunit;
using Xunit.Categories;

namespace LSEG.Eta.Transports.Tests
{
    public class DataDictionaryTest
    {
        /**
        * Load a field dictionary with boundary values and verify contents.
        */
        [Fact]
        [Category("Unit")]
        public void LoadFieldDictionaryTest()
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();

            // load field dictionary
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.FAILURE, dictionary.LoadFieldDictionary(null, out error));
            Assert.Equal(CodecReturnCode.FAILURE, dictionary.LoadFieldDictionary("xyz", out error));
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../../Src/Tests/RDMFieldDictionary", out error));
            Assert.Equal(15047, dictionary.NumberOfEntries);
            Assert.Equal(32767, dictionary.MaxFid);
            Assert.Equal(-32768, dictionary.MinFid);

            // verify tags
            Assert.Equal("RWF.DAT", dictionary.InfoFieldFilename.ToString());
            Assert.Equal("RDF-D RWF field set", dictionary.InfoFieldDesc.ToString());
            Assert.Equal("4.20.29", dictionary.InfoFieldVersion.ToString());
            Assert.Equal("002", dictionary.InfoFieldBuild.ToString());
            Assert.Equal("25-May-2017", dictionary.InfoFieldDate.ToString());

            // verify contents
            // FID 1
            IDictionaryEntry entry = dictionary.Entry(1);
            Assert.NotNull(entry);
            Assert.Equal("PROD_PERM", entry.GetAcronym().ToString());
            Assert.Equal("PERMISSION", entry.GetDdeAcronym().ToString());
            Assert.Equal(1, entry.GetFid());
            Assert.Equal(0, entry.GetRippleToField());
            Assert.Equal(MfFieldTypes.INTEGER, entry.GetFieldType());
            Assert.Equal(5, entry.GetLength());
            Assert.Equal(0, entry.GetEnumLength());
            Assert.Equal(DataTypes.UINT, entry.GetRwfType());
            Assert.Equal(2, entry.GetRwfLength());

            // FID 32767
            entry = dictionary.Entry(32767);
            Assert.NotNull(entry);
            Assert.Equal("MAX_FID", entry.GetAcronym().ToString());
            Assert.Equal("MAX_FID_DDE", entry.GetDdeAcronym().ToString());
            Assert.Equal(32767, entry.GetFid());
            Assert.Equal(0, entry.GetRippleToField());
            Assert.Equal(MfFieldTypes.ENUMERATED, entry.GetFieldType());
            Assert.Equal(3, entry.GetLength());
            Assert.Equal(3, entry.GetEnumLength());
            Assert.Equal(DataTypes.ENUM, entry.GetRwfType());
            Assert.Equal(1, entry.GetRwfLength());

            // FID -32768
            entry = dictionary.Entry(-32768);
            Assert.NotNull(entry);
            Assert.Equal("MIN_FID", entry.GetAcronym().ToString());
            Assert.Equal("MIN_FID_DDE", entry.GetDdeAcronym().ToString());
            Assert.Equal(-32768, entry.GetFid());
            Assert.Equal(0, entry.GetRippleToField());
            Assert.Equal(MfFieldTypes.ENUMERATED, entry.GetFieldType());
            Assert.Equal(3, entry.GetLength());
            Assert.Equal(3, entry.GetEnumLength());
            Assert.Equal(DataTypes.ENUM, entry.GetRwfType());
            Assert.Equal(1, entry.GetRwfLength());

            // FID 6
            entry = dictionary.Entry(6);
            Assert.NotNull(entry);
            Assert.Equal("TRDPRC_1", entry.GetAcronym().ToString());
            Assert.Equal("LAST", entry.GetDdeAcronym().ToString());
            Assert.Equal(6, entry.GetFid());
            Assert.Equal(7, entry.GetRippleToField());
            Assert.Equal(MfFieldTypes.PRICE, entry.GetFieldType());
            Assert.Equal(17, entry.GetLength());
            Assert.Equal(0, entry.GetEnumLength());
            Assert.Equal(DataTypes.REAL, entry.GetRwfType());
            Assert.Equal(7, entry.GetRwfLength());
        }

        /**
    * Load a enum type dictionary with boundary values and verify contents.
    */
        [Fact]
        [Category("Unit")]
        public void LoadEnumTypeDictionaryTest()
        {
            CodecError error;
            DataDictionary dictionary = new DataDictionary();

            // load enumType dictionary
            dictionary.Clear();
            Assert.Equal(CodecReturnCode.FAILURE, dictionary.LoadEnumTypeDictionary(null, out error));
            Assert.Equal(CodecReturnCode.FAILURE, dictionary.LoadEnumTypeDictionary("xyz", out error));
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadEnumTypeDictionary("../../../../Src/Tests/enumtype.def", out error));

            // verify tags
            Assert.Equal("ENUMTYPE.001", dictionary.InfoEnumFilename.ToString());
            Assert.Equal("IDN Marketstream enumerated tables", dictionary.InfoEnumDesc.ToString());
            Assert.Equal("4.20.29", dictionary.InfoEnumRTVersion.ToString());
            Assert.Equal("17.81", dictionary.InfoEnumDTVersion.ToString());
            Assert.Equal("10-Oct-2017", dictionary.InfoEnumDate.ToString());

            // verify contents
            List<IEnumTypeTable> enumTypeTable = dictionary.EnumTables;
            Assert.NotNull(enumTypeTable[0]);
            Assert.NotNull( enumTypeTable[1]);
            IEnumTypeTable enumTypeTableEntry = enumTypeTable[0];
            Assert.Equal(2, enumTypeTableEntry.MaxValue);
            List<Int16> fidRefs = enumTypeTableEntry.FidReferences;
            Assert.Equal(32767, fidRefs[0]);
            Assert.Equal(-32768, fidRefs[1]);
            List<IEnumType> enumTypes = enumTypeTableEntry.EnumTypes;
            Assert.Equal(3, enumTypes.Count);
            Assert.Equal(0, enumTypes[0].Value);
            Assert.Equal("   ", enumTypes[0].Display.ToString());
            Assert.Equal(1, enumTypes[1].Value);
            Assert.Equal("MAX", enumTypes[1].Display.ToString());
            Assert.Equal(2, enumTypes[2].Value);
            Assert.Equal("MIN", enumTypes[2].Display.ToString());
        }
    }
}
