/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Xunit;
using Xunit.Categories;

namespace Refinitiv.Eta.Transports.Tests
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
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadFieldDictionary("../../../RDMFieldDictionary", out error));
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
            Assert.Equal("PROD_PERM", entry.Acronym.ToString());
            Assert.Equal("PERMISSION", entry.DdeAcronym.ToString());
            Assert.Equal(1, entry.Fid);
            Assert.Equal(0, entry.RippleToField);
            Assert.Equal(MfFieldTypes.INTEGER, entry.FieldType);
            Assert.Equal(5, entry.Length);
            Assert.Equal(0, entry.EnumLength);
            Assert.Equal(DataTypes.UINT, entry.RwfType);
            Assert.Equal(2, entry.RwfLength);

            // FID 32767
            entry = dictionary.Entry(32767);
            Assert.NotNull(entry);
            Assert.Equal("MAX_FID", entry.Acronym.ToString());
            Assert.Equal("MAX_FID_DDE", entry.DdeAcronym.ToString());
            Assert.Equal(32767, entry.Fid);
            Assert.Equal(0, entry.RippleToField);
            Assert.Equal(MfFieldTypes.ENUMERATED, entry.FieldType);
            Assert.Equal(3, entry.Length);
            Assert.Equal(3, entry.EnumLength);
            Assert.Equal(DataTypes.ENUM, entry.RwfType);
            Assert.Equal(1, entry.RwfLength);

            // FID -32768
            entry = dictionary.Entry(-32768);
            Assert.NotNull(entry);
            Assert.Equal("MIN_FID", entry.Acronym.ToString());
            Assert.Equal("MIN_FID_DDE", entry.DdeAcronym.ToString());
            Assert.Equal(-32768, entry.Fid);
            Assert.Equal(0, entry.RippleToField);
            Assert.Equal(MfFieldTypes.ENUMERATED, entry.FieldType);
            Assert.Equal(3, entry.Length);
            Assert.Equal(3, entry.EnumLength);
            Assert.Equal(DataTypes.ENUM, entry.RwfType);
            Assert.Equal(1, entry.RwfLength);

            // FID 6
            entry = dictionary.Entry(6);
            Assert.NotNull(entry);
            Assert.Equal("TRDPRC_1", entry.Acronym.ToString());
            Assert.Equal("LAST", entry.DdeAcronym.ToString());
            Assert.Equal(6, entry.Fid);
            Assert.Equal(7, entry.RippleToField);
            Assert.Equal(MfFieldTypes.PRICE, entry.FieldType);
            Assert.Equal(17, entry.Length);
            Assert.Equal(0, entry.EnumLength);
            Assert.Equal(DataTypes.REAL, entry.RwfType);
            Assert.Equal(7, entry.RwfLength);
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
            Assert.Equal(CodecReturnCode.SUCCESS, dictionary.LoadEnumTypeDictionary("../../../enumtype.def", out error));

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
