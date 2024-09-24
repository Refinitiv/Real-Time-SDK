/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using System.Collections.Generic;
using System.Text;
using Xunit;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Tests
{
    public class RmtesTests
    {

        public class RmtesTestCase
        {
            byte[] _testCase;
            byte[] _endResult;
            String _name;

            public RmtesTestCase(String name, byte[] testCase, byte[] endResult)
            {
                _name = name;
                _testCase = testCase;
                _endResult = endResult;
            }

            public byte[] testCase()
            {
                return _testCase;
            }

            public byte[] endResult()
            {
                return _endResult;
            }

            public String name()
            {
                return _name;
            }
        }

        // ----------------------------------- TEST CASES ----------------------------------

        static RmtesTestCase STS23003a = new RmtesTestCase("STS23003a", new byte[] { 0x1b, 0x6f, 0x28, 0x41, 0x28, 0x7e, 0x74, 0x27 }, new byte[] { 0xFF, 0xFD, 0xFF, 0xFD, 0xFF, 0xFD });
        static RmtesTestCase STS23003b = new RmtesTestCase("STS23003b", new byte[] { 0x1b, 0x6f, 0x29, 0x21, 0x2f, 0x7e, 0x75, 0x21 }, new byte[] { 0xFF, 0xFD, 0xFF, 0xFD, 0xFF, 0xFD });
        static RmtesTestCase STS23003c = new RmtesTestCase("STS23003c", new byte[] { 0x0e, 0x20, 0x7f }, new byte[] { 0x00, 0x20, 0xFF, 0xFD });
        static RmtesTestCase STS23003d = new RmtesTestCase("STS23003d", new byte[] { 0x1b, 0x7c, 0xa8, 0xc1, 0xf4, 0xa7 }, new byte[] { 0xFF, 0xFD, 0xFF, 0xFD });
        static RmtesTestCase STS23003e = new RmtesTestCase("STS23003e", new byte[] { 0x1b, 0x7c, 0xa9, 0xa1, 0xaf, 0xfe, 0xfe, 0xa1 }, new byte[] { 0xFF, 0xFD, 0xFF, 0xFD, 0xFF, 0xFD });

        static RmtesTestCase STS23004a = new RmtesTestCase("STS23004a", new byte[] { 0x1b, 0x5b, 0x30, 0x60, 0x61, 0x1b, 0x5b, 0x32, 0x62, 0x1b, 0x5b, 0x33, 0x60, 0x61, 0x1b, 0x5b, 0x33, 0x62, 0x1b, 0x5b, 0x38, 0x60, 0x61, 0x1b, 0x5b, 0x34, 0x62 }, new byte[] { 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61 });
        static RmtesTestCase STS23004b = new RmtesTestCase("STS23004b", new byte[] { 0x1b, 0x5b, 0x35, 0x60, 0x62, 0x1b, 0x5b, 0x33, 0x62 }, new byte[] { 0x00, 0x62, 0x00, 0x62, 0x00, 0x62, 0x00, 0x62 });
        static RmtesTestCase STS23004c = new RmtesTestCase("STS23004c", new byte[] { 0x1b, 0x5b, 0x34, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x1b, 0x5b, 0x35, 0x62 }, new byte[] { 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47 });
        static RmtesTestCase STS23004d = new RmtesTestCase("STS23004d", new byte[] { 0x1B, 0x5B, 0x31, 0x30, 0x60, 0x58, 0x59, 0x5A, 0x1B, 0x5B, 0x31, 0x30, 0x60, 0x58, 0x59, 0x5A, 0x1B, 0x5B, 0x33, 0x62, 0x1B, 0x5B, 0x33, 0x62 }, new byte[] { 0x00, 0x58, 0x00, 0x59, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a });
        static RmtesTestCase STS23004e = new RmtesTestCase("STS23004e", new byte[] { 0x1B, 0x5B, 0x35, 0x35, 0x60, 0x58, 0x59, 0x5A }, new byte[] { 0x00, 0x58, 0x00, 0x59, 0x00, 0x5a });

        static RmtesTestCase STS23005a = new RmtesTestCase("STS23005a", new byte[] { 0x1b, 0x5b, 0x30, 0x60, 0x41, 0x42, 0x43, 0x44, 0x1b, 0x5b, 0x38, 0x60, 0x45, 0x46, 0x47, 0x48 }, new byte[] { 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48 });
        static RmtesTestCase STS23005b = new RmtesTestCase("STS23005b", new byte[] { 0x1b, 0x5b, 0x33, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45 }, new byte[] { 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45 });
        static RmtesTestCase STS23005c = new RmtesTestCase("STS23005c", new byte[] { 0x1b, 0x5b, 0x34, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 }, new byte[] { 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47 });
        static RmtesTestCase STS23005d = new RmtesTestCase("STS23005d", new byte[] { 0x1B, 0x5B, 0x38, 0x60, 0x58, 0x58, 0x1B, 0x5B, 0x38, 0x38, 0x60, 0x59, 0x59, 0x1B, 0x5B, 0x38, 0x38, 0x38, 0x60, 0x5A, 0x5A }, new byte[] { 0x00, 0x58, 0x00, 0x58, 0x00, 0x59, 0x00, 0x59, 0x00, 0x5a, 0x00, 0x5a });
        static RmtesTestCase STS23005e = new RmtesTestCase("STS23005e", new byte[] { 0x1B, 0x5B, 0x35, 0x60, 0x58, 0x58, 0x1B, 0x5B, 0x35, 0x60, 0x59, 0x59, 0x1B, 0x5B, 0x35, 0x60, 0x5A, 0x5A }, new byte[] { 0x00, 0x5a, 0x00, 0x5a });

        static RmtesTestCase STS23089 = new RmtesTestCase("STS23089", new byte[] { 0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1b, 0x5B, 0x36, 0x34, 0x60, 0x7a }, new byte[] { 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x30, 0x00, 0x7a });
        static RmtesTestCase STS23090 = new RmtesTestCase("STS23090", new byte[] { 0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1b, 0x5B, 0x61, 0x35, 0x60, 0x41 }, new byte[] { }); // Invalid usage test, returns nothing
        static RmtesTestCase STS23091 = new RmtesTestCase("STS23091", new byte[] { 0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1B, 0x5B, 0x60, 0x41 }, new byte[] { 0x00, 0x41, 0x00, 0x25, 0x00, 0x30, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x30 });
        static RmtesTestCase STS23092 = new RmtesTestCase("STS23092", new byte[] { 0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1B, 0x5B, 0x30, 0x60 }, new byte[] { 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x30 }); // Invalid data usage case. Curser moves, but no data

        static RmtesTestCase ASY1001 = new RmtesTestCase("ASY1001 - Japanese Kanji", new byte[] { 0x1B, 0x24, 0x2B, 0x34, 0x1B, 0x7C, 0xEA, 0xDE, 0xEA, 0xDF }, new byte[] { 0x88, 0xB0, 0x88, 0xBF }); // Japanese Kanji character test
        static RmtesTestCase ASY1002 = new RmtesTestCase("ASY1002 - Chinese 1 & 2", new byte[] { 0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, 0x1B, 0x24, 0x2B, 0x36, 0x1B, 0x7C, 0x46, 0x47, 0x62, 0x3D, 0xCE, 0xE4, 0xB2, 0xAC }, new byte[] { 0x5C, 0x3C, 0x8D, 0x8A, 0x92, 0x94, 0x80, 0x39 }); // Chinese 1 and 2 character set test
        static RmtesTestCase ASY1003 = new RmtesTestCase("ASY1003 - Japenese Katakana and Japanese Latin", new byte[] { 0x1B, 0x2A, 0x32, 0x1B, 0x2B, 0x33, 0x1B, 0x6E, 0x1B, 0x7C, 0x46, 0x47, 0xAF, 0xDE }, new byte[] { 0xFF, 0x86, 0xFF, 0x87, 0x00, 0x2F, 0x00, 0x5E }); // Japanese Katakana and Japanese Latin test
        static RmtesTestCase ASY1004 = new RmtesTestCase("ASY1004 - UTF8 Encoded", new byte[] { 0x1B, 0x25, 0x30, 0x34, 0xCF, 0xEF, 0x2F }, new byte[] { 0x00, 0x34, 0x03, 0xEF, 0x00, 0x2F }); // Testing UTF8 sequence
        static RmtesTestCase ASY1005 = new RmtesTestCase("ASY1005 - Partial repeat on Chinese that fails", new byte[] { 0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, 0x46, 0x47, 0x62, 0x3D, 0x1B, 0x5B, 0x33, 0x62 }, new byte[] { }); // Testing partial update repeat of a chinese character. Puts 0x3D at end 3 times, which do not decode correctly because that would ask for a character outside of scope. Fails RMTES.
        static RmtesTestCase ASY1006 = new RmtesTestCase("ASY1006 - Partial repeat on Chinese that works", new byte[] { 0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, 0x46, 0x47, 0x62, 0x3D, 0x1B, 0x5B, 0x34, 0x62 }, new byte[] { 0x5C, 0x3C, 0x8D, 0x8A, 0xFF, 0xFD, 0xFF, 0xFD }); // Testing partial update repeat of a chinese character. Puts 0x3D at end 4 times, which can be found in scope but is just an FFFD character, twice
        static RmtesTestCase ASY1007 = new RmtesTestCase("ASY1007 - Multiple language/character sets", new byte[] { 0x1B, 0x2A, 0x32, 0x1B, 0x2B, 0x33, 0x1B, 0x6E, 0x1B, 0x7C, 0x46, 0x47, 0xAF, 0xDE, 0x1B, 0x24, 0x2B, 0x34, 0x1B, 0x7C, 0xEA, 0xDE, 0xEA, 0xDF, 0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, 0x1B, 0x24, 0x2B, 0x36, 0x1B, 0x7C, 0x46, 0x47, 0x62, 0x3D, 0xCE, 0xE4, 0xB2, 0xAC }, new byte[] { }); // Testing changing out character sets in mid-sequence

        static RmtesTestCase ASY1008 = new RmtesTestCase("ASY1008 - InitialContextWithShiftFn Test 1", new byte[] { 0x1b, 0x6f, 0x21, 0x21, 0x28, 0x40, 0x26, 0x7e, 0x27, 0x21 }, new byte[] { 0x30, 0x00, 0x25, 0x42, 0xff, 0xfd, 0x04, 0x10 });
        static RmtesTestCase ASY1009 = new RmtesTestCase("ASY1009 - BUA2 Test 2", new byte[] { 0x1b, 0x24, 0x2b, 0x34, 0x1b, 0x6f, 0x30, 0x21, 0x32, 0x2b, 0x3f, 0x37 }, new byte[] { 0x4e, 0x9c, 0x9e, 0xc4, 0x65, 0xb0 });
        static RmtesTestCase ASY1010 = new RmtesTestCase("ASY1010 - LockingAndSingleShift Test 6", new byte[] { 0x1b, 0x24, 0x2a, 0x35, 0x1b, 0x7d, 0xa1, 0xa1, 0xa4, 0xa1, 0xa4, 0xa2, 0xa4, 0xfe, 0xa5, 0xa1, 0xa7, 0xa1 }, new byte[] { });
        static RmtesTestCase ASY1011 = new RmtesTestCase("ASY1011 - PartialNo62 missing", new byte[] { 0x1b, 0x5b, 0x30, 0x60, 0x50, 0x1b, 0x5b, 0x32 }, new byte[] { });
        static RmtesTestCase ASY1012 = new RmtesTestCase("ASY1012 - PartialWithoutFullField Test 2", new byte[] { 0x1b, 0x5b, 0x30, 0x60, 0x61 }, new byte[] { 0x00, 0x61 });
        static RmtesTestCase ASY1013 = new RmtesTestCase("ASY1013 - PartialWithoutFullField Test 3", new byte[] { 0x1b, 0x5b, 0x30, 0x60, 0x62 }, new byte[] { 0x00, 0x62 });
        static RmtesTestCase ASY1014 = new RmtesTestCase("ASY1014 - MultiLang AR test", new byte[] { 0x1b, 0x25, 0x30, 0xd8, 0xa1, 0xd8, 0xa2, 0xd9, 0x81, 0xda, 0xa4, 0xdb, 0xbc }, new byte[] { 0x06, 0x21, 0x06, 0x22, 0x06, 0x41, 0x06, 0xa4, 0x06, 0xfc });
        static RmtesTestCase ASY1015 = new RmtesTestCase("ASY1015 - MultiLang GC test", new byte[] { 0x1b, 0x25, 0x30, 0xcd, 0xb4, 0xce, 0x86, 0xce, 0xa9, 0xcf, 0x94, 0xcf, 0xb6 }, new byte[] { 0x03, 0x74, 0x03, 0x86, 0x03, 0xa9, 0x03, 0xd4, 0x03, 0xf6 });
        static RmtesTestCase ASY1016 = new RmtesTestCase("ASY1016 - MultiLang CS test", new byte[] { 0x1b, 0x25, 0x30, 0xe2, 0x82, 0xa0, 0xe2, 0x82, 0xa1, 0xe2, 0x82, 0xa2, 0xe2, 0x82, 0xa3, 0xe2, 0x82, 0xa4, 0xe2, 0x82, 0xa5, 0xe2, 0x82, 0xa6, 0xe2, 0x82, 0xa7, 0xe2, 0x82, 0xa8, 0xe2, 0x82, 0xa9, 0xe2, 0x82, 0xaa, 0xe2, 0x82, 0xab, 0xe2, 0x82, 0xac, 0xe2, 0x82, 0xad, 0xe2, 0x82, 0xae, 0xe2, 0x82, 0xaf, 0xe2, 0x82, 0xb0, 0xe2, 0x82, 0xb1 }, new byte[] { 0x20, 0xa0, 0x20, 0xa1, 0x20, 0xa2, 0x20, 0xa3, 0x20, 0xa4, 0x20, 0xa5, 0x20, 0xa6, 0x20, 0xa7, 0x20, 0xa8, 0x20, 0xa9, 0x20, 0xaa, 0x20, 0xab, 0x20, 0xac, 0x20, 0xad, 0x20, 0xae, 0x20, 0xaf, 0x20, 0xb0, 0x20, 0xb1 });
        static RmtesTestCase ASY1017 = new RmtesTestCase("ASY1017 - DataFollowedByPartial", new byte[] { 0x1b, 0x25, 0x30, 0xe1, 0x9c, 0x80, 0xe1, 0x9c, 0x88, 0x1b, 0x5b, 0x30, 0x60, 0x1b, 0x25, 0x30, 0x24, 0x35 }, new byte[] { 0x00, 0x24, 0x00, 0x35, 0x00, 0x00, 0x17, 0x08 });

        static RmtesTestCase ASY1018 = new RmtesTestCase("ASY1018 - Changing GL/GR mid-message, expecting zeroed out GL/GR", new byte[] { 0x1B, 0x6F, 0x21, 0x7D, 0x25, 0x6D, 0x25, 0x73, 0x25, 0x49, 0x25, 0x73, 0x47, 0x72, 0x45, 0x7C, 0x40, 0x68, 0x4A, 0x2A, 0x41, 0x6A, 0x3E, 0x6C, 0x0F, 0x28, 0x35, 0x1B, 0x6F, 0x39, 0x66, 0x4C, 0x73, 0x44, 0x6A, 0x21, 0x22, 0x0F, 0x24, 0x2F, 0x1B, 0x7D, 0xC4, 0xDD, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, 0xDB, 0xB2, 0xC0, 0xB0, 0x45, 0x53, 0x1B, 0x6F, 0x3B, 0x7E, 0x3B, 0x76, 0x0F, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x46, 0x52, 0x39, 0x39, 0x20, 0x30, 0x36, 0x3A, 0x30, 0x39 }, new byte[] { 0x25, 0xce, 0x30, 0xed, 0x30, 0xf3, 0x30, 0xc9, 0x30, 0xf3, 0x76, 0x7d, 0x7c, 0xd6, 0x51, 0x48, 0x72, 0x69, 0x76, 0xf8, 0x58, 0x34, 0x00, 0x28, 0x00, 0x35, 0x53, 0xf7, 0x7d, 0x04, 0x5b, 0x9a, 0x30, 0x01, 0x00, 0x24, 0x00, 0x2f, 0xff, 0x84, 0xff, 0x9d, 0x00, 0x29, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x28, 0xff, 0x9b, 0xff, 0x72, 0xff, 0x80, 0xff, 0x70, 0x00, 0x45, 0x00, 0x53, 0x66, 0x42, 0x4e, 0x8b, 0x00, 0x29, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x46, 0x00, 0x52, 0x00, 0x39, 0x00, 0x39, 0x00, 0x20, 0x00, 0x30, 0x00, 0x36, 0x00, 0x3a, 0x00, 0x30, 0x00, 0x39 });

        static List<RmtesTestCase> allTestCases = new List<RmtesTestCase> { STS23003a, STS23003b, STS23003c, STS23003d, STS23003e, STS23004a, STS23004b, STS23004c, STS23004d, STS23004e,
                STS23005a, STS23005b, STS23005c, STS23005d, STS23005e, STS23089, STS23090, STS23091, STS23092, ASY1001, ASY1002, ASY1003, ASY1004, ASY1005, ASY1006,
                ASY1007, ASY1008, ASY1009, ASY1010, ASY1011, ASY1012, ASY1013, ASY1014, ASY1015, ASY1016, ASY1017, ASY1018 };

        /**
         * 		------ The following is a list of character sequence possibilities when switching and building character set tables as well as partial updates ------
         *
         * This is used for creating new tests, or understanding the tests used in RMTES.
         *
         * The codes can be followed by tab breaks.
         *
         RMTES Codes

         1B = Escape
         21 = ESC_21
         40 = LSEG Ctrl 1 to CL
         22 = ESC_22
         30 = LSEG Ctrl 2 to CR
         24 = ESC_24
         28 = ESC_24_28
         47 = G0 = Chinese1
         48 = G0 = Chinese2
         29 = ESC_24_29
         47 = G1 = Chinese1
         48 = G1 = Chinese2
         2A = ESC_24_2A
         47 = G2 = Chinese1
         35 = G2 = Chinese1
         48 = G2 = Chinese2
         2B = ESC_24_2B
         47 = G3 = Chinese1
         48 = G3 = Chinese2
         36 = G3 = Chinese2
         34 = G3 = JapaneseKanji
         25 = ESC_25
         30 = UTF_ENC
         26 = ESC_26
         40 = ESC_26_40
         1B = ESC_26_40_ESC
         24 = ESC_26_40_ESC_24
         42 = G0 = JapaneseKanji
         29 = ESC_26_40_ESC_24_29
         42 = G1 = JapaneseKanji
         2A = ESC_26_40_ESC_24_2A
         42 = G2 = JapaneseKanji
         2B = ESC_26_40_ESC_24_2B
         42 = G3 = JapaneseKanji
         28 = ESC_28
         42 = G0 = LSEG Basic 1
         49 = G0 = JapaneseKatakana
         4A = G0 = JapaneseLatin
         29 = ESC_29
         31 = G1 = LSEG Basic 2
         42 = G1 = LSEG Basic 1
         49 = G1 = JapaneseKatakana
         4A = G1 = JapaneseLatin
         2A = ESC_2A
         32 = G2 = JapaneseKatakana
         2B = ESC_2B
         33 = G3 = JapaneseLatin
         5B = LBRKT
         60 = Partial update move cursor
         62 = Partial update repeat char at cursor
         6E = GL = G2
         6F = GL = G3
         7E = GR = G1
         7D = GR = G2
         7C = GR = G3

         */

        [Fact]
        public void RMTESToUCS2Test1()
        {
            Buffer inBuffer = new Buffer();
            inBuffer.Data(new ByteBuffer(1000));
            RmtesDecoder decoder = new RmtesDecoder();
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(1000);
            RmtesBuffer rmtesBuffer = new RmtesBuffer(1000);

            RmtesTestCase testCase;
            CodecReturnCode retVal;

            for (int x = 0; x < allTestCases.Count; ++x)
            {
                testCase = allTestCases[x];

                Console.Write("---Test: " + testCase.name() + " ---");

                byte[] byteArray = testCase.testCase();
                Console.Write("Input byteArray Byte for Byte data: ");
                for (int i = 0; i < byteArray.Length; ++i)
                {
                    Console.Write(String.Format("%02x", byteArray[i]));
                    if (i % 2 != 0)
                        Console.Write(" ");
                }
                Console.Write("");
                if (testCase.name().Contains("ASY1012"))
                {
                    // Used for debugging location
                    byteArray = testCase.testCase();
                }

                ByteBuffer byteBuffer = new ByteBuffer(byteArray.Length);
                byteBuffer.Put(byteArray);
                byteBuffer.Flip();
                inBuffer.Data(byteBuffer, byteBuffer.Position, byteArray.Length);

                // Fully clear out buffer because these Junit tests do not account for previous partial update existences
                cacheBuffer.Clear();
                for (int i = 0; i < cacheBuffer.AllocatedLength; ++i)
                {
                    cacheBuffer.Data.Write((byte)0);
                }
                cacheBuffer.Clear();
                retVal = decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
                if (retVal < CodecReturnCode.SUCCESS)
                {
                    Console.Write("Error in RMTESApplyToCache: " + retVal.GetAsString() + "\n");
                }

                // Fully clear out buffer because the tests do not account for previous partial update existences
                rmtesBuffer.Clear();
                for (int i = 0; i < rmtesBuffer.AllocatedLength; ++i)
                {
                    rmtesBuffer.Data.Write((byte)0);
                }
                rmtesBuffer.Clear();
                retVal = decoder.RMTESToUCS2(rmtesBuffer, cacheBuffer);
                if (retVal < CodecReturnCode.SUCCESS)
                {
                    Console.Write("Error in RMTESToUCS2: " + retVal.GetAsString());
                }
                Console.Write("Input cacheBuffer Byte for Byte data: ");
                for (int i = 0; i < cacheBuffer.Length; ++i)
                {
                    Console.Write(String.Format("%02x", cacheBuffer.Data.Contents[i]));
                    if (i % 2 != 0)
                        Console.Write(" ");
                }
                Console.Write("");
                Console.Write("Output RmtesBuffer string:" + System.Text.Encoding.UTF8.GetString(rmtesBuffer.Data.Contents, 0, rmtesBuffer.Length));
                Console.Write("RmtesBuffer Byte for Byte data:     ");
                for (int i = 0; i < rmtesBuffer.Length; ++i)
                {
                    Console.Write(String.Format("%02x", rmtesBuffer.Data.Contents[i]));
                    if (i % 2 != 0)
                        Console.Write(" ");
                }
                Console.Write("\n");

                // Validation
                byte[] endResult = testCase.endResult();

                Console.Write("Expected result Byte for Byte data: ");
                for (int i = 0; i < endResult.Length; ++i)
                {
                    Console.Write(String.Format("%02x", endResult[i]));
                    if (i % 2 != 0)
                        Console.Write(" ");
                }
                Console.Write("\n");
                for (int i = 0; i < endResult.Length; ++i)
                {
                    if (endResult[i] != rmtesBuffer.Data.Contents[i])
                    {
                        Console.Write("Assert error on above test\n");
                    }
                    Assert.Equal(endResult[i], rmtesBuffer.Data.Contents[i]);
                }
            }
        }

        void CompareASCIIToUCS2(byte[] ascii, byte[] ucs2)
        {
            for (int i = 0; i < ascii.Length; i++)
            {
                Assert.Equal(0, ucs2[2 * i]);
                Assert.Equal(ascii[i], ucs2[2 * i + 1]);
            }
        }

        [Fact]
        public void RMTESToUCS2Test2()
        {
            Buffer inBuffer = new Buffer();
            RmtesDecoder decoder = new RmtesDecoder();
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(1000);
            RmtesBuffer rmtesBuffer = new RmtesBuffer(1000);

            String stringText1 = "This is a test";

            inBuffer.Data(stringText1);

            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            decoder.RMTESToUCS2(rmtesBuffer, cacheBuffer);

            CompareASCIIToUCS2(System.Text.Encoding.ASCII.GetBytes(stringText1), rmtesBuffer.Data.Contents);

            String stringText2 = "The quick brown fox jumps over the lazy dog1234567890,./;l'[]";

            inBuffer.Data(stringText2);

            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            decoder.RMTESToUCS2(rmtesBuffer, cacheBuffer);

            CompareASCIIToUCS2(System.Text.Encoding.ASCII.GetBytes(stringText2), rmtesBuffer.Data.Contents);
        }

        [Fact]
        public void RMTESToUTF8Test()
        {
            Buffer inBuffer = new Buffer();
            RmtesDecoder decoder = new RmtesDecoder();
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(1000);
            RmtesBuffer rmtesBuffer = new RmtesBuffer(1000);

            String stringText1 = "This is a test";

            inBuffer.Data(stringText1);

            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer);

            String str1 = Encoding.UTF8.GetString(rmtesBuffer.Data.Contents, 0, rmtesBuffer.Length);
            Assert.Equal(0, String.CompareOrdinal(stringText1, 0, str1, 0, stringText1.Length));

            String stringText2 = "The quick brown fox jumps over the lazy dog1234567890,./;l'[]";

            inBuffer.Data(stringText2);

            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer);

            String str2 = Encoding.UTF8.GetString(rmtesBuffer.Data.Contents, 0, rmtesBuffer.Length);
            Assert.Equal(0, String.CompareOrdinal(stringText2, 0, str2, 0, stringText2.Length));
        }

        [Fact]
        public void PartialUpdateTest()
        {
            Buffer inBuffer = new Buffer();
            RmtesDecoder decoder = new RmtesDecoder();
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(1000);
            char[] charBuf1 = new char[100];
            String charBuf1String = new String(charBuf1);
            char[] charBuf2 = new char[100];
            String charBuf2String = new String(charBuf2);
            char[] outBuf;
            char[] inBuf1 = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', (char)0x1B, (char)0x5B, '3', (char)0x60, 'j', 'k', 'l' };
            String inBuf1String = new String(inBuf1);

            inBuffer.Data(charBuf1String);
            cacheBuffer.Data = cacheBuffer.Data.Put(Encoding.UTF8.GetBytes(charBuf2String));
            cacheBuffer.Length = 0;
            cacheBuffer.AllocatedLength = 100;

            Assert.False(decoder.HasPartialRMTESUpdate(inBuffer));
            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);            
            inBuffer.Data(inBuf1String);

            Assert.True(decoder.HasPartialRMTESUpdate(inBuffer));

            char[] inBuf2 = { 'a', 'b', 'c', 'd', (char)0x1B, (char)0x5B, '3', (char)0x62, 'e', 'f', 'g' };
            String inBuf2String = new String(inBuf2);
            inBuffer.Data(inBuf2String);
            Assert.True(decoder.HasPartialRMTESUpdate(inBuffer));

            char[] inBuf3 = { (char)0x1B, (char)0x5B, '1', '0', (char)0x60, 'm', 'n', 'o' };
            String inBuf3String = new String(inBuf3);
            inBuffer.Data(inBuf3String);

            cacheBuffer.Clear();
            cacheBuffer.Data = cacheBuffer.Data.Put(Encoding.UTF8.GetBytes("abcdefghijklm"));
            cacheBuffer.Length = cacheBuffer.Data.Position;
            cacheBuffer.AllocatedLength = 100;
            outBuf = "abcdefghijmno".ToCharArray();

            Assert.True(decoder.HasPartialRMTESUpdate(inBuffer));
            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            inBuffer.Data(cacheBuffer.Data.ToString());
            cacheBuffer.Data.Flip();
            for (int i = 0; i < cacheBuffer.Length; i++)
            {
                Assert.Equal(outBuf[i], (char)cacheBuffer.Data.Contents[i]);
            }

            inBuffer.Data("abcdefgh");

            cacheBuffer.Clear();
            cacheBuffer.Data = cacheBuffer.Data.Put(System.Text.Encoding.UTF8.GetBytes("abcdefghijkl"));
            cacheBuffer.Length = 12;
            cacheBuffer.AllocatedLength = 100;
            outBuf = "abcdefgh".ToCharArray();

            Assert.False(decoder.HasPartialRMTESUpdate(inBuffer));   // No update present

            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            inBuffer.Data(new String(cacheBuffer.Data.ToString()));
            Assert.Equal(8, cacheBuffer.Length);
            cacheBuffer.Data.Flip();
            for (int i = 0; i < cacheBuffer.Length; i++)
            {
                Assert.Equal(outBuf[i], (char)cacheBuffer.Data.Contents[i]);
            }

            char[] inBuf4 = { 'a', 'b', 'c', (char)0x1B, (char)0x5B, '2', (char)0x60, 'd', 'e', (char)0x1B, (char)0x5B, '3', (char)0x62 };
            String inBuf4String = new String(inBuf4);
            inBuffer.Data(inBuf4String);

            cacheBuffer.Clear();
            cacheBuffer.Data = cacheBuffer.Data.Put(Encoding.UTF8.GetBytes(charBuf2String));
            cacheBuffer.Length = 0;
            cacheBuffer.AllocatedLength = 100;
            outBuf = "abdeeee".ToCharArray();

            Assert.True(decoder.HasPartialRMTESUpdate(inBuffer));

            decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            inBuffer.Data(cacheBuffer.Data.ToString());
            Assert.Equal(7, cacheBuffer.Length);
            cacheBuffer.Data.Flip();
            for (int i = 0; i < cacheBuffer.Length; i++)
            {
                Assert.Equal(outBuf[i], (char)cacheBuffer.Data.Contents[i]);
            }

        }

        public enum LRCode
        {
            NOLR = 0,
            G0ToGL = 1,
            G1ToGL = 2,
            G2ToGL = 3,
            G3ToGL = 4,
            G1ToGR = 5,
            G2ToGR = 6,
            G3ToGR = 7
        }

        public class ControlTest
        {
            internal byte[] testString;
            internal int strLen;
            internal ESCReturnCode expectedRet;
            internal RmtesWorkingSet endSet;
            internal LRCode endLR;
            internal int returnValue;

            internal ControlTest(byte[] _testString, int _strLen, ESCReturnCode _expectedRet, RmtesWorkingSet _endSet, RmtesTests.LRCode _endLR, int _returnValue)
            {
                testString = _testString;
                strLen = _strLen;
                expectedRet = _expectedRet;
                endSet = _endSet;
                endLR = _endLR;
                returnValue = _returnValue;
            }
        }

        static RmtesWorkingSet StandardSet = new RmtesWorkingSet();

        static RmtesWorkingSet C1G0 = new RmtesWorkingSet();
        static RmtesWorkingSet C2G0 = new RmtesWorkingSet();
        static RmtesWorkingSet C1G1 = new RmtesWorkingSet();
        static RmtesWorkingSet C2G1 = new RmtesWorkingSet();
        static RmtesWorkingSet C1G2 = new RmtesWorkingSet();
        static RmtesWorkingSet C2G2 = new RmtesWorkingSet();
        static RmtesWorkingSet C1G3 = new RmtesWorkingSet();
        static RmtesWorkingSet C2G3 = new RmtesWorkingSet();
        static RmtesWorkingSet JLG3 = new RmtesWorkingSet();
        static RmtesWorkingSet JKG2 = new RmtesWorkingSet();
        static RmtesWorkingSet R2G1 = new RmtesWorkingSet();
        static RmtesWorkingSet R1G1 = new RmtesWorkingSet();
        static RmtesWorkingSet JKG1 = new RmtesWorkingSet();
        static RmtesWorkingSet JLG1 = new RmtesWorkingSet();
        static RmtesWorkingSet R1G0 = new RmtesWorkingSet();
        static RmtesWorkingSet JKG0 = new RmtesWorkingSet();
        static RmtesWorkingSet JLG0 = new RmtesWorkingSet();
        static RmtesWorkingSet K1G0 = new RmtesWorkingSet();
        static RmtesWorkingSet K1G1 = new RmtesWorkingSet();
        static RmtesWorkingSet K1G2 = new RmtesWorkingSet();
        static RmtesWorkingSet K1G3 = new RmtesWorkingSet();

        static byte[] nullControlChar = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C1G0ControlChar = { 0x1B, 0x24, 0x28, 0x47, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C2G0ControlChar = { 0x1B, 0x24, 0x28, 0x48, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C1G1ControlChar = { 0x1B, 0x24, 0x29, 0x47, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C2G1ControlChar = { 0x1B, 0x24, 0x29, 0x48, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C1G247ControlChar = { 0x1B, 0x24, 0x2A, 0x47, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C1G235ControlChar = { 0x1B, 0x24, 0x2A, 0x35, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C2G2ControlChar = { 0x1B, 0x24, 0x2A, 0x48, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C1G3ControlChar = { 0x1B, 0x24, 0x2B, 0x47, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C2G348ControlChar = { 0x1B, 0x24, 0x2B, 0x48, 0x00, 0x00, 0x00, 0x00 };
        static byte[] C2G336ControlChar = { 0x1B, 0x24, 0x2B, 0x36, 0x00, 0x00, 0x00, 0x00 };
        static byte[] K1G3ControlChar = { 0x1B, 0x24, 0x2B, 0x34, 0x00, 0x00, 0x00, 0x00 };
        static byte[] JLG3ControlChar = { 0x1B, 0x2B, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] JKG2ControlChar = { 0x1B, 0x2A, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] R2G1ControlChar = { 0x1B, 0x29, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] R1G1ControlChar = { 0x1B, 0x29, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] JKG1ControlChar = { 0x1B, 0x29, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] JLG1ControlChar = { 0x1B, 0x29, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] UTFControlChar = { 0x1B, 0x25, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] K1G0LongControlChar = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x42, 0x00, 0x00 };
        static byte[] K1G1LongControlChar = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x29, 0x42, 0x00 };
        static byte[] K1G2LongControlChar = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x2A, 0x42, 0x00 };
        static byte[] K1G3LongControlChar = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x2B, 0x42, 0x00 };
        static byte[] G1GRControlChar = { 0x1B, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] G2GLControlChar = { 0x1B, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] G2GRControlChar = { 0x1B, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] G3GLControlChar = { 0x1B, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] G3GRControlChar = { 0x1B, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] G0GLControlChar = { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] G1GLControlChar = { 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        static byte[] EndCharControlChar = { 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        /* Fail cases */
        static byte[] FailureControlChar = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        ControlTest[] controlTestArray = {
            new ControlTest(nullControlChar, 1, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.NOLR, 1),
            new ControlTest(C1G0ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G0, LRCode.NOLR, 4),
            new ControlTest(C2G0ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G0, LRCode.NOLR, 4),
            new ControlTest(C1G1ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G1, LRCode.NOLR, 4),
            new ControlTest(C2G1ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G1, LRCode.NOLR, 4),
            new ControlTest(C1G247ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G2, LRCode.NOLR, 4),
            new ControlTest(C1G235ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G2, LRCode.NOLR, 4),
            new ControlTest(C2G2ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G2, LRCode.NOLR, 4),
            new ControlTest(C1G3ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G3, LRCode.NOLR, 4),
            new ControlTest(C2G348ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G3, LRCode.NOLR, 4),
            new ControlTest(C2G336ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G3, LRCode.NOLR, 4),
            new ControlTest(K1G3ControlChar, 4, ESCReturnCode.ESC_SUCCESS, K1G3, LRCode.NOLR, 4),
            new ControlTest(JLG3ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JLG3, LRCode.NOLR, 3),
            new ControlTest(JKG2ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JKG2, LRCode.NOLR, 3),
            new ControlTest(R2G1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, R2G1, LRCode.NOLR, 3),
            new ControlTest(R1G1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, R1G1, LRCode.NOLR, 3),
            new ControlTest(JKG1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JKG1, LRCode.NOLR, 3),
            new ControlTest(JLG1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JLG1, LRCode.NOLR, 3),
            new ControlTest(UTFControlChar, 3, ESCReturnCode.UTF_ENC, StandardSet, LRCode.NOLR, 3),
            new ControlTest(K1G0LongControlChar, 6, ESCReturnCode.ESC_SUCCESS, K1G0, LRCode.NOLR, 6),
            new ControlTest(K1G1LongControlChar, 7, ESCReturnCode.ESC_SUCCESS, K1G1, LRCode.NOLR, 7),
            new ControlTest(K1G2LongControlChar, 7, ESCReturnCode.ESC_SUCCESS, K1G2, LRCode.NOLR, 7),
            new ControlTest(K1G3LongControlChar, 7, ESCReturnCode.ESC_SUCCESS, K1G3, LRCode.NOLR, 7),
            new ControlTest(G1GRControlChar, 2, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G1ToGR, 2),
            new ControlTest(G2GLControlChar, 2, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G2ToGL, 2),
            new ControlTest(G2GRControlChar, 2, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G2ToGR, 2),
            new ControlTest(G3GLControlChar, 2, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G3ToGL, 2),
            new ControlTest(G3GRControlChar, 2, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G3ToGR, 2),
            new ControlTest(G0GLControlChar, 1, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G0ToGL, 1),
            new ControlTest(G1GLControlChar, 1, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.G1ToGL, 1), // 30
            new ControlTest(EndCharControlChar, 1, ESCReturnCode.END_CHAR, StandardSet,
                    LRCode.NOLR, 0),
            new ControlTest(FailureControlChar, 1, ESCReturnCode.ESC_SUCCESS, StandardSet,
                    LRCode.NOLR, 0), };

        void WorkingSetSetup()
        {
            CharSet characterSet = new CharSet();
            characterSet.initWorkingSet(StandardSet);

            characterSet.initWorkingSet(C1G0);
            C1G0.G0 = characterSet._rsslChinese1;

            characterSet.initWorkingSet(C2G0);
            C2G0.G0 = characterSet._rsslChinese2;

            characterSet.initWorkingSet(C1G1);
            C1G1.G1 = characterSet._rsslChinese1;

            characterSet.initWorkingSet(C2G1);
            C2G1.G1 = characterSet._rsslChinese2;

            characterSet.initWorkingSet(C1G2);
            C1G2.G2 = characterSet._rsslChinese1;

            characterSet.initWorkingSet(C2G2);
            C2G2.G2 = characterSet._rsslChinese2;

            characterSet.initWorkingSet(C1G3);
            C1G3.G3 = characterSet._rsslChinese1;

            characterSet.initWorkingSet(C2G3);
            C2G3.G3 = characterSet._rsslChinese2;

            characterSet.initWorkingSet(JLG3);
            JLG3.G3 = characterSet._rsslJapaneseLatin;

            characterSet.initWorkingSet(JKG2);
            JKG2.G2 = characterSet._rsslJapaneseKatakana;

            characterSet.initWorkingSet(R2G1);
            R2G1.G1 = characterSet._rsslReuterBasic2;

            characterSet.initWorkingSet(R1G1);
            R1G1.G1 = characterSet._rsslReuterBasic1;

            characterSet.initWorkingSet(JKG1);
            JKG1.G1 = characterSet._rsslJapaneseKatakana;

            characterSet.initWorkingSet(JLG1);
            JLG1.G1 = characterSet._rsslJapaneseLatin;

            characterSet.initWorkingSet(R1G0);
            R1G0.G0 = characterSet._rsslReuterBasic1;

            characterSet.initWorkingSet(JLG0);
            JLG0.G0 = characterSet._rsslJapaneseLatin;

            characterSet.initWorkingSet(K1G0);
            K1G0.G0 = characterSet._rsslJapaneseKanji;

            characterSet.initWorkingSet(K1G1);
            K1G1.G1 = characterSet._rsslJapaneseKanji;

            characterSet.initWorkingSet(K1G2);
            K1G2.G2 = characterSet._rsslJapaneseKanji;

            characterSet.initWorkingSet(K1G3);
            K1G3.G3 = characterSet._rsslJapaneseKanji;

        }


        [Fact]
        public void ControlParseTest()
        {
            RmtesWorkingSet testSet = new RmtesWorkingSet();
            ControlTest curTest;
            int testCount;
            ESCReturnCode retCode;
            int ret;
            ByteBuffer start = new ByteBuffer(1000);
            CharSet characterSet = new CharSet();
            RmtesDecoder decoder = new RmtesDecoder();

            RmtesInfo tempInfo;

            Console.Write("Control Parse Tests:\n\n");

            WorkingSetSetup();

            for (testCount = 0; testCount < controlTestArray.Length; testCount++)
            {
                Console.Write("Testcount = " + testCount);
                characterSet.initWorkingSet(testSet);
                curTest = controlTestArray[testCount];
                start.Clear();
                start.Put(curTest.testString);

                tempInfo = decoder.ControlParse(start, 0, curTest.strLen, testSet);
                ret = tempInfo.Value;
                testSet = tempInfo.Set;
                retCode = tempInfo.ESCRetCode;

                Assert.Equal(curTest.expectedRet, retCode);
                Console.Write("retCode = " + retCode + "\n");
                Assert.Equal(curTest.returnValue, ret);
                Console.Write("ret = " + ret + "\n");

                Assert.True(curTest.endSet.G0.Equals(testSet.G0));
                Console.Write("testSet.G0 = " + testSet.G0 + ", curTest.endSet.G0 = "
                                   + curTest.endSet.G0);
                Assert.True(curTest.endSet.G1.Equals(testSet.G1));
                Console.Write("testSet.G1 = " + testSet.G1 + ", curTest.endSet.G1 = "
                                   + curTest.endSet.G1 + "\n");
                Assert.True(curTest.endSet.G2.Equals(testSet.G2));
                Assert.True(curTest.endSet.G3.Equals(testSet.G3));

                switch (curTest.endLR)
                {
                    case LRCode.NOLR:
                        break;
                    case LRCode.G0ToGL:
                        Assert.True(testSet.GL.Equals(testSet.G0));
                        break;
                    case LRCode.G1ToGL:
                        Assert.True(testSet.GL.Equals(testSet.G1));
                        break;
                    case LRCode.G2ToGL:
                        Assert.True(testSet.GL.Equals(testSet.G2));
                        break;
                    case LRCode.G3ToGL:
                        Assert.True(testSet.GL.Equals(testSet.G3));
                        break;
                    case LRCode.G1ToGR:
                        Assert.True(testSet.GR.Equals(testSet.G1));
                        break;
                    case LRCode.G2ToGR:
                        Assert.True(testSet.GR.Equals(testSet.G2));
                        break;
                    case LRCode.G3ToGR:
                        Assert.True(testSet.GR.Equals(testSet.G3));
                        break;
                    default:
                        break;
                }
            }
        }

        [Fact]
        public void ConstructorTest()
        {
            RmtesBuffer rmtesBuffer;
            RmtesCacheBuffer rmtesCacheBuffer;
            ByteBuffer byteBuffer = new ByteBuffer(100);
            byte[] byteArray = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            byteBuffer.Put(byteArray);
            rmtesBuffer = new RmtesBuffer(10, byteBuffer, 100);
            rmtesCacheBuffer = new RmtesCacheBuffer(10, byteBuffer, 100);
            Assert.Equal(10, rmtesBuffer.Length);
            Assert.Equal(100, rmtesBuffer.AllocatedLength);
            Assert.Equal(byteBuffer, rmtesBuffer.Data);

            Assert.Equal(10, rmtesCacheBuffer.Length);
            Assert.Equal(100, rmtesCacheBuffer.AllocatedLength);
            Assert.Equal(byteBuffer, rmtesCacheBuffer.Data);

            rmtesBuffer = new RmtesBuffer(100);
            rmtesCacheBuffer = new RmtesCacheBuffer(100);
            rmtesBuffer.Data = byteBuffer;
            rmtesBuffer.Length = byteBuffer.Position;
            rmtesCacheBuffer.Data = byteBuffer;
            rmtesCacheBuffer.Length = byteBuffer.Position;
            Assert.Equal(10, rmtesBuffer.Length);
            Assert.Equal(100, rmtesBuffer.AllocatedLength);
            Assert.Equal(byteBuffer, rmtesBuffer.Data);

            Assert.Equal(10, rmtesCacheBuffer.Length);
            Assert.Equal(100, rmtesCacheBuffer.AllocatedLength);
            Assert.Equal(byteBuffer, rmtesCacheBuffer.Data);
        }

        [Fact]
        public void OutOffBufferEscSeqTest()
        {
            byte[] invalidRmtestByteArray = new byte[] { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x1B, 0x25, 0x55 };
            Buffer buffer = new Buffer();

            buffer.Data(new ByteBuffer(invalidRmtestByteArray), 6, 2); /* Set the position and length of RMTES data portion in the buffer */

            RmtesDecoder decoder = new RmtesDecoder();
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(5);

            Assert.Equal(CodecReturnCode.FAILURE, decoder.RMTESApplyToCache(buffer, cacheBuffer));
        }

        [Fact]
        public void OutOffBufferEscSeqTest2()
        {
            Buffer inBuffer = new Buffer();
            ByteBuffer byteBuffer = new ByteBuffer(new byte[] { 27, 37 });
            byteBuffer.Flip();
            inBuffer.Data(byteBuffer);

            RmtesDecoder decoder = new RmtesDecoder();
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(5);

            Assert.Equal(CodecReturnCode.FAILURE, decoder.RMTESApplyToCache(inBuffer, cacheBuffer));
        }

        [Fact]
        public void PartialUpdateConversionTest()
        {
            RmtesBuffer rmtesBuffer = new RmtesBuffer(100);
            RmtesDecoder decoder = new RmtesDecoder();
            

            byte[] buf1 = { (byte)'a', (byte)'b', (byte)'c', (byte)'d', (byte)'e', (byte)'f', (byte)'g', (byte)'h', (byte)'i', (byte)'j', (byte)'k', (byte)'l' };
            String stringText1 = Encoding.UTF8.GetString(buf1, 0, buf1.Length);
            RmtesCacheBuffer cacheBuffer = new RmtesCacheBuffer(buf1.Length, new ByteBuffer(buf1), 100);
            
      
            Assert.Equal(CodecReturnCode.SUCCESS, decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer));
            Assert.Equal(12, rmtesBuffer.Length);

            String str1 = Encoding.UTF8.GetString(rmtesBuffer.Data.Contents, 0, rmtesBuffer.Length);
            Assert.Equal(0, String.CompareOrdinal(stringText1, 0, str1, 0, stringText1.Length));

            byte[] buf2 = { (byte)'a', (byte)'b', (byte)'c', (byte)'d', (byte)'e', (byte)'f', (byte)'g', (byte)'h', (byte)'i', 0x1B, 0x5B, (byte)'3', 0x60, (byte)'j', (byte)'k', (byte)'l' };
            String stringText2 = Encoding.UTF8.GetString(buf2, 0, buf2.Length);
            rmtesBuffer.Clear();
            cacheBuffer = new RmtesCacheBuffer(buf2.Length, new ByteBuffer(buf2), 100);

            Assert.Equal(CodecReturnCode.SUCCESS, decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer));
            Assert.Equal(16, rmtesBuffer.Length);

            String str2 = Encoding.UTF8.GetString(rmtesBuffer.Data.Contents, 0, rmtesBuffer.Length);
            Assert.Equal(0, String.CompareOrdinal(stringText2, 0, str2, 0, stringText2.Length));

            byte[] buf3 = { (byte)'a', (byte)'b', (byte)'c', (byte)'d', 0x1B, 0x5B, (byte)'3', 0x62, (byte)'e', (byte)'f', (byte)'g' };
            String stringText3 = Encoding.UTF8.GetString(buf3, 0, buf3.Length);
            rmtesBuffer.Clear();
            cacheBuffer = new RmtesCacheBuffer(buf3.Length, new ByteBuffer(buf3), 100);
            
            Assert.Equal(CodecReturnCode.SUCCESS, decoder.RMTESToUTF8(rmtesBuffer, cacheBuffer));
            Assert.Equal(11, rmtesBuffer.Length);

            String str3 = Encoding.UTF8.GetString(rmtesBuffer.Data.Contents, 0, rmtesBuffer.Length);
            Assert.Equal(0, String.CompareOrdinal(stringText3, 0, str3, 0, stringText3.Length));

        }

    }
    
}
