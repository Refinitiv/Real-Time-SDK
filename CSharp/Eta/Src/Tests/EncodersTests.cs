/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;

using Xunit;

namespace LSEG.Eta.Tests;


public class EncodersTests
{

    public static readonly byte[] DATA_001_elementList_one_entry =
    {
        0x09, 0x02, 0x74, 0x23, 0x00, 0x01, 0x04, 0x75, 0x69, 0x6e, 0x74, 0x04, 0x02, 0x04, 0xd2        //   ...#...unit....
    };

    public static readonly byte[] DATA_001_requestMsg =
    {
        0x00, 0x7E, 0x01, 0x06, 0x00, 0x00, 0x00, 0x64, 0x8C, 0xDF, 0x05, 0x03, 0x04, 0x22, 0x66, 0xFF, //   .~.....d ....."f.
        0xFC, 0xFF, 0xFD, 0x80, 0x5B, 0x26, 0x0D, 0x42, 0x61, 0x74, 0x63, 0x68, 0x5F, 0x52, 0x65, 0x71, //   ....[&.B atch_Req
        0x75, 0x65, 0x73, 0x74, 0x01, 0x05, 0x80, 0x48, 0x08, 0x00, 0x03, 0x0D, 0x41, 0x70, 0x70, 0x6C, //   uest...H ....Appl
        0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, 0x64, 0x11, 0x03, 0x32, 0x35, 0x36, 0x0F, 0x41, //   icationI d..256.A
        0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x0C, //   pplicati onName..
        0x72, 0x73, 0x73, 0x6C, 0x43, 0x6F, 0x6E, 0x73, 0x75, 0x6D, 0x65, 0x72, 0x08, 0x50, 0x6F, 0x73, //   rsslCons umer.Pos
        0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x09, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73, 0x74, //   ition..l ocalhost
        0x0F, 0x45, 0x58, 0x54, 0x45, 0x4E, 0x44, 0x45, 0x44, 0x20, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52, //   .EXTENDE D HEADER
        0x08, 0x00, 0x03, 0x09, 0x3A, 0x49, 0x74, 0x65, 0x6D, 0x4C, 0x69, 0x73, 0x74, 0x0F, 0xFE, 0x00, //   ....:Ite mList...
        0x17, 0x11, 0x00, 0x00, 0x03, 0x05, 0x54, 0x52, 0x49, 0x2E, 0x4E, 0x05, 0x49, 0x42, 0x4D, 0x2E, //   ......TR I.N.IBM.
        0x4E, 0x06, 0x43, 0x53, 0x43, 0x4F, 0x2E, 0x4F, 0x09, 0x3A, 0x56, 0x69, 0x65, 0x77, 0x54, 0x79, //   N.CSCO.O .:ViewTy
        0x70, 0x65, 0x04, 0x01, 0x01, 0x09, 0x3A, 0x56, 0x69, 0x65, 0x77, 0x44, 0x61, 0x74, 0x61, 0x0F, //   pe....:V iewData.
        0xFE, 0x00, 0x08, 0x04, 0x02, 0x00, 0x02, 0x00, 0x16, 0x00, 0x19,               //   ........ ...
    };

    public static readonly byte[] DATA_001_requestMsgWithIdAndEncAttrib =
    {
        0x00, 0x26, 0x01, 0x06, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x80, 0x1C, 0x36, 0x05, 0x54, 0x52, //   .&.....d....6.TR
        0x49, 0x2E, 0x4E, 0x01, 0x00, 0x00, 0x7F, 0xFF, 0x02, 0x0E, 0x45, 0x4E, 0x43, 0x4F, 0x44, 0x45, //   I.N.......ENCODE
        0x44, 0x20, 0x41, 0x54, 0x54, 0x52, 0x49, 0x42,                         //   D ATTRIB
    };

    public static readonly byte[] DATA_002_elementList_oneEntry_emptyPreEncodedAscii =
    {
        0x08, 0x00, 0x01, 0x0b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x11, 0x00, //   ....ABCDEFGHIJK..
    };

    public static readonly byte[] DATA_002_refreashMsg =
    {
        0x00, 0x9F, 0x02, 0x06, 0x7F, 0xFF, 0xFF, 0xFF, 0x9B, 0xFB, 0x04, 0x49, 0x96, 0x02, 0xD2, 0x09, //   ........ ...I....
        0x00, 0x0E, 0x73, 0x6F, 0x6D, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x20, 0x69, 0x6E, 0x66, 0x6F, //   ..some t ext info
        0x08, 0x31, 0x30, 0x32, 0x30, 0x33, 0x30, 0x34, 0x30, 0x04, 0x10, 0x11, 0x12, 0x13, 0x22, 0x80, //   .1020304 0.....".
        0x56, 0x27, 0xFE, 0x7F, 0x7F, 0x05, 0x54, 0x52, 0x49, 0x2E, 0x4E, 0x01, 0x05, 0x80, 0x48, 0x08, //   V'....TR I.N...H.
        0x00, 0x03, 0x0D, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, 0x64, //   ...Appli cationId
        0x11, 0x03, 0x32, 0x35, 0x36, 0x0F, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, //   ..256.Ap plicatio
        0x6E, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x0C, 0x72, 0x73, 0x73, 0x6C, 0x43, 0x6F, 0x6E, 0x73, 0x75, //   nName..r sslConsu
        0x6D, 0x65, 0x72, 0x08, 0x50, 0x6F, 0x73, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x09, 0x6C, 0x6F, //   mer.Posi tion..lo
        0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73, 0x74, 0x0F, 0x45, 0x58, 0x54, 0x45, 0x4E, 0x44, 0x45, 0x44, //   calhost. EXTENDED
        0x20, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52, 0xFF, 0xFF, 0xFF, 0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //    HEADER. ........
        0xFF, 0x09, 0x03, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0A, 0x00, 0x00, 0xAF, 0x07, 0x41, 0x42, //   ........ ......AB
        0x43, 0x44, 0x45, 0x46, 0x47, 0x00, 0x20, 0x03, 0x08, 0x75, 0xC1, 0x00, 0x6F, 0x04, 0x0A, 0x0D, //   CDEFG. . .u..o...
        0x3C, 0xEC,                                          //    <.
    };

    public static readonly byte[] DATA_003_closeMsg =
    {
        0x00, 0x18, 0x05, 0x06, 0x7F, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0x0F, 0x45, 0x58, 0x54, 0x45, 0x4E, //  ........ ...EXTEN
        0x44, 0x45, 0x44, 0x20, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52,                                     //  DED HEAD ER
    };

    public static readonly byte[] DATA_003_elementList_oneEntry_preencodedAscii =
    {
        0x08, 0x00, 0x01, 0x0b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x11, 0xfe, 0x01, 0x04, 0x31, //   ....ABCDEFGHIJK....1
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, //   23456789012345678901
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30,       //   2345678901234567890
    };

    public static readonly byte[] DATA_004_elementList_wthrreEntries_middleEntryHasElementList =
    {
        0x08, 0x00, 0x03, 0x09, 0x75, 0x69, 0x6e, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x04, 0x02, 0x04, 0xd2, 0x0e, 0x63, 0x6f, //   ....uint type.....co
        0x6e, 0x74, 0x61, 0x69, 0x6e, 0x65, 0x72, 0x20, 0x74, 0x79, 0x70, 0x65, 0x85, 0xfe, 0x00, 0x2b, 0x08, 0x00, 0x01, 0x0b, //   ntainer type...+....
        0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 0x11, 0x1a, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, //   string type..ABCDEFG
        0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x0f, //   HIJKLMNOPQRSTUVWXYZ.
        0x61, 0x6e, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x20, 0x65, 0x6c, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x04, 0x04, 0x3a, 0xde, 0x68, //   another element..:.h
        0xb1,                                                                                                                   //   .
    };

    public static readonly byte[] DATA_004_updateMsg =
    {
        0x00, 0x86, 0x04, 0x01, 0x7F, 0xED, 0xCB, 0xA9, 0x83, 0xFB, 0x04, 0x01, 0x49, 0x96, 0x02, 0xD2, //  ........ ....I...
        0x0A, 0x01, 0xF4, 0x04, 0x10, 0x11, 0x12, 0x13, 0x80, 0x56, 0x27, 0xFE, 0x7F, 0x7F, 0x05, 0x54, //  ........ .V'....T
        0x52, 0x49, 0x2E, 0x4E, 0x01, 0x05, 0x80, 0x48, 0x08, 0x00, 0x03, 0x0D, 0x41, 0x70, 0x70, 0x6C, //  RI.N...H ....Appl
        0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, 0x64, 0x11, 0x03, 0x32, 0x35, 0x36, 0x0F, 0x41, //  icationI d..256.A
        0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x0C, //  pplicati onName..
        0x72, 0x73, 0x73, 0x6C, 0x43, 0x6F, 0x6E, 0x73, 0x75, 0x6D, 0x65, 0x72, 0x08, 0x50, 0x6F, 0x73, //  rsslCons umer.Pos
        0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x09, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73, 0x74, //  ition..l ocalhost
        0x0F, 0x45, 0x58, 0x54, 0x45, 0x4E, 0x44, 0x45, 0x44, 0x20, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52, //  .EXTENDE D HEADER
        0xFF, 0xFF, 0xFF, 0xFA, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x03, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, //  ........ ........
        0x0A, 0x00, 0x00, 0xAF, 0x07, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x00, 0x20, 0x03, 0x08, //  .....ABC DEFG. ..
        0x75, 0xC1, 0x00, 0x6F, 0x04, 0x0A, 0x0D, 0x3C, 0xEC,                                           //  u..o...< .
    };

    public static readonly byte[] DATA_005_elementList_secondEntryElementList_rollback =
    {
        0x08, 0x00, 0x01, 0x09, 0x75, 0x69, 0x6e, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x04, 0x02, 0x04, 0xd2, //  ....uint type....
    };

    public static readonly byte[] DATA_005_statusMsg =
    {
        0x00, 0x45, 0x03, 0x04, 0x00, 0x00, 0x00, 0x18, 0x81, 0x7b, 0x00, 0x1a, 0x0d, 0x12, 0x65, 0x6E,
        0x63, 0x6F, 0x64, 0x65, 0x53, 0x74, 0x61, 0x74, 0x65, 0x4D, 0x73, 0x67, 0x54, 0x65, 0x73, 0x74,
        0x06, 0x0b, 0x7b, 0x08, 0x03, 0x4c, 0x02, 0x04, 0x0a, 0x05, 0x03, 0x09, 0x80, 0x05, 0x08, 0x00,
        0x00, 0x00, 0x07, 0x0b, 0x43, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x43, 0x00,
        0x00, 0x04, 0xD2, 0x00, 0x00, 0x02, 0x37,
    };

    public static readonly byte[] DATA_006_encodeFieldList_wEntries_andRollBack =
    {
        0x09, 0x03, 0x02, 0x00, 0x03, 0x00, 0x05, 0x00, 0x0a, 0x00, 0x00, 0xaf, 0x07, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, //   .............ABCDEFG
        0x00, 0x20, 0x03, 0x08, 0x75, 0xc1, 0x00, 0x6f, 0x04, 0x0a, 0x0d, 0x3c, 0xec, 0x00, 0x36, 0x00,                         //   . ..u..o...<..6.
     };

    public static readonly byte[] DATA_007_elementList_entries_wdate_time_enum =
   {
        0x08, 0x00, 0x05, 0x09, 0x44, 0x61, 0x74, 0x65, 0x20, 0x74, 0x65, 0x73, 0x74, 0x09, 0x04, 0x17, 0x05, 0x07, 0xdc, 0x0f, //   ....Date test.......
        0x42, 0x6c, 0x61, 0x6e, 0x6b, 0x20, 0x44, 0x61, 0x74, 0x65, 0x20, 0x74, 0x65, 0x73, 0x74, 0x09, 0x04, 0x00, 0x00, 0x00, //   Blank Date test.....
        0x00, 0x09, 0x54, 0x69, 0x6d, 0x65, 0x20, 0x74, 0x65, 0x73, 0x74, 0x0a, 0x05, 0x0b, 0x1e, 0x0f, 0x00, 0x07, 0x0f, 0x42, //   ..Time test........B
        0x6c, 0x61, 0x6e, 0x6b, 0x20, 0x54, 0x69, 0x6d, 0x65, 0x20, 0x74, 0x65, 0x73, 0x74, 0x0a, 0x08, 0xff, 0xff, 0xff, 0xff, //   lank Time test......
        0xff, 0x3f, 0xff, 0xff, 0x09, 0x45, 0x6e, 0x75, 0x6d, 0x20, 0x74, 0x65, 0x73, 0x74, 0x0e, 0x01, 0x7f,                   //   ........Enum test...
    };

    public static readonly byte[] DATA_008_array_entries_ascii =
    {
        0x11, 0x0A, 0x00, 0x03, 0x50, 0x72, 0x65, 0x73, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x65, 0x56, 0x69, //  ....PresidenteVi
        0x63, 0x65, 0x50, 0x72, 0x65, 0x73, 0x69, 0x64, 0x61, 0x53, 0x65, 0x63, 0x72, 0x65, 0x74, 0x61, //  cePresidaSecreta
        0x72, 0x79,                                                                                     //  ry
    };

    public static readonly byte[] DATA_009_array_entries_uint =
    {
        0x04, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,                                     //  ..........
    };

    public static readonly byte[] DATA_010_array_entries_encData_len2 =
    {
        0x10, 0x05, 0x00, 0x03, 0x61, 0x62, 0x63, 0x64, 0x65, 0x30, 0x31, 0x32, 0x33, 0x34, 0x41, 0x42, //  ....abcde01234AB
        0x43, 0x44, 0x45,                                                                               //  CDE
    };

    public static readonly byte[] DATA_011_array_entries_encData_len0 =
    {
        0x10, 0x00, 0x00, 0x03, 0x05, 0x61, 0x62, 0x63, 0x64, 0x65, 0x0A, 0x30, 0x31, 0x32, 0x33, 0x34, //  .....abcde.01234
        0x35, 0x36, 0x37, 0x38, 0x39, 0x07, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,                   //  56789.ABCDEFG
    };

    public static readonly byte[] DATA_012_array_entry_blankEncData_len4 =
    {
        0x10, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // .......
    };

    public static readonly byte[] DATA_013_array_entry_blankEncData_len0 =
    {
        0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,                                                       //  .......
    };

    public static readonly byte[] DATA_014_encodePrimitive_int_len0 =
    {
        0x03, 0x00, 0x00, 0x0D, 0x01, 0x00, 0x01, 0xFF, 0x01, 0x7F, 0x01, 0x80, 0x02, 0x00, 0x80, 0x02, //  ................
        0x7F, 0xFF, 0x02, 0x80, 0x00, 0x03, 0x00, 0x80, 0x00, 0x03, 0x7F, 0xFF, 0xFF, 0x03, 0x80, 0x00, //  ................
        0x00, 0x04, 0x00, 0x80, 0x00, 0x00, 0x04, 0x7F, 0xFF, 0xFF, 0xFF, 0x04, 0x80, 0x00, 0x00, 0x00, //  ................
    };

    public static readonly byte[] DATA_015_filter_list_wEntries =
    {
        0x03, 0x03, 0x04, 0x04, 0x32, 0x01, 0x02, 0x04, 0x12, 0x34, 0x56, 0x78, 0x0b, 0x42, 0x55, 0x46,
        0x46, 0x45, 0x52, 0x20, 0x44, 0x41, 0x54, 0x41, 0x11, 0x0a, 0x02, 0x11, 0x11, 0x04, 0x01, 0x02,
        0x03, 0x04, 0x03, 0x80, 0x22, 0xff, 0x02, 0x0a, 0x41, 0x53, 0x43, 0x49, 0x49, 0x20, 0x44, 0x41,
        0x54, 0x41,
    };

    public static readonly byte[] DATA_016_array_entries_ls_uint =
    {
        0x04, 0x00, 0x00, 0x03, 0x01, 0x00, 0x01, 0xFF, 0x02, 0xFF, 0xFF,                               //   .......... ...
    };

    public static readonly byte[] DATA_017_map_wEntry =
    {
        0x1f, 0x04, 0x04, 0x00, 0x01, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31,
        0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x1a, 0x41,
        0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x02, 0x00, 0x03, 0x12, 0x04, 0x50, 0x45,
        0x52, 0x4d, 0x08, 0x4b, 0x45, 0x59, 0x30, 0x30, 0x30, 0x30, 0x31, 0x1a, 0x5a, 0x59, 0x58, 0x57,
        0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x47,
        0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x01, 0x02, 0x4b, 0x32, 0x1a, 0x41, 0x41, 0x41, 0x41, 0x41,
        0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
        0x41, 0x41, 0x41, 0x41, 0x41, 0x03, 0x02, 0x4b, 0x33, 0x00,
    };

    public static readonly byte[] DATA_018_genericMsg =
    {
        0x00, 0x7F, 0x07, 0x01, 0x7F, 0xED, 0xCB, 0xA9, 0x7F, 0x04, 0x49, 0x96, 0x02, 0xD2, 0x42, 0xE5, //   ........ ..I...B.
        0x76, 0xF7, 0x04, 0x10, 0x11, 0x12, 0x13, 0x80, 0x56, 0x27, 0xFE, 0x7F, 0x7F, 0x05, 0x54, 0x52, //   v....... V'....TR
        0x49, 0x2E, 0x4E, 0x01, 0x05, 0x80, 0x48, 0x08, 0x00, 0x03, 0x0D, 0x41, 0x70, 0x70, 0x6C, 0x69, //   I.N...H. ...Appli
        0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, 0x64, 0x11, 0x03, 0x32, 0x35, 0x36, 0x0F, 0x41, 0x70, //   cationId ..256.Ap
        0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x0C, 0x72, //   plicatio nName..r
        0x73, 0x73, 0x6C, 0x43, 0x6F, 0x6E, 0x73, 0x75, 0x6D, 0x65, 0x72, 0x08, 0x50, 0x6F, 0x73, 0x69, //   sslConsu mer.Posi
        0x74, 0x69, 0x6F, 0x6E, 0x11, 0x09, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73, 0x74, 0x0F, //   tion..lo calhost.
        0x45, 0x58, 0x54, 0x45, 0x4E, 0x44, 0x45, 0x44, 0x20, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52, 0xB0, //   EXTENDED  HEADER.
        0x39, 0x09, 0x03, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0A, 0x00, 0x00, 0xAF, 0x07, 0x41, 0x42, //   9....... ......AB
        0x43, 0x44, 0x45, 0x46, 0x47, 0x00, 0x20, 0x03, 0x08, 0x75, 0xC1, 0x00, 0x6F, 0x04, 0x0A, 0x0D, //   CDEFG. . .u..o...
        0x3C, 0xEC //                                                                                        <.
    };

    public static readonly byte[] DATA_022_ackMsg =
    {
        0x00, 0x82, 0x06, 0x01, 0x7F, 0xED, 0xCB, 0xA9, 0x3F, 0x04, 0x00, 0x00, 0x30, 0x39, 0x0B, 0x08, //  ........ ?...09..
        0x41, 0x43, 0x4B, 0x20, 0x54, 0x45, 0x58, 0x54, 0x49, 0x96, 0x02, 0xD2, 0x80, 0x56, 0x27, 0xFE, //  ACK TEXT I....V'.
        0x7F, 0x7F, 0x05, 0x54, 0x52, 0x49, 0x2E, 0x4E, 0x01, 0x05, 0x80, 0x48, 0x08, 0x00, 0x03, 0x0D, //  ...TRI.N ...H....
        0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, 0x64, 0x11, 0x03, 0x32, //  Applicat ionId..2
        0x35, 0x36, 0x0F, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x4E, 0x61, //  56.Appli cationNa
        0x6D, 0x65, 0x11, 0x0C, 0x72, 0x73, 0x73, 0x6C, 0x43, 0x6F, 0x6E, 0x73, 0x75, 0x6D, 0x65, 0x72, //  me..rssl Consumer
        0x08, 0x50, 0x6F, 0x73, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x09, 0x6C, 0x6F, 0x63, 0x61, 0x6C, //  .Positio n..local
        0x68, 0x6F, 0x73, 0x74, 0x0F, 0x45, 0x58, 0x54, 0x45, 0x4E, 0x44, 0x45, 0x44, 0x20, 0x48, 0x45, //  host.EXT ENDED HE
        0x41, 0x44, 0x45, 0x52, 0x09, 0x03, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0A, 0x00, 0x00, 0xAF, //  ADER.... ........
        0x07, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x00, 0x20, 0x03, 0x08, 0x75, 0xC1, 0x00, 0x6F, //  .ABCDEFG . ..u..o
        0x04, 0x0A, 0x0D, 0x3C, 0xEC,                                                                   //  ...<.
    };

    public static readonly byte[] DATA_023_postMsg =
    {
        0x00, 0x89, 0x08, 0x01, 0x7F, 0xED, 0xCB, 0xA9, 0x83, 0xEF, 0x04, 0xFF, 0xFF, 0xFF, 0xFA, 0xFF, //  ........ ........
        0xFF, 0xFF, 0xFF, 0x49, 0x96, 0x02, 0xD2, 0x00, 0x00, 0x30, 0x39, 0x04, 0x10, 0x11, 0x12, 0x13, //  ...I.... .09.....
        0x80, 0x56, 0x27, 0xFE, 0x7F, 0x7F, 0x05, 0x54, 0x52, 0x49, 0x2E, 0x4E, 0x01, 0x05, 0x80, 0x48, //  .V'....T RI.N...H
        0x08, 0x00, 0x03, 0x0D, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, //  ....Appl icationI
        0x64, 0x11, 0x03, 0x32, 0x35, 0x36, 0x0F, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, //  d..256.A pplicati
        0x6F, 0x6E, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x0C, 0x72, 0x73, 0x73, 0x6C, 0x43, 0x6F, 0x6E, 0x73, //  onName.. rsslCons
        0x75, 0x6D, 0x65, 0x72, 0x08, 0x50, 0x6F, 0x73, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x09, 0x6C, //  umer.Pos ition..l
        0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73, 0x74, 0x0F, 0x45, 0x58, 0x54, 0x45, 0x4E, 0x44, 0x45, //  ocalhost .EXTENDE
        0x44, 0x20, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52, 0xDB, 0xA0, 0x04, 0x09, 0x03, 0x02, 0x00, 0x03, //  D HEADER ........
        0x00, 0x04, 0x00, 0x0A, 0x00, 0x00, 0xAF, 0x07, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x00, //  ........ ABCDEFG.
        0x20, 0x03, 0x08, 0x75, 0xC1, 0x00, 0x6F, 0x04, 0x0A, 0x0D, 0x3C, 0xEC,                         //  ..u..o. ..<.
    };



    /// This tests the encodeFilterListInit() method. It contains three test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Negative test for unsupported container type
    /// 3. Positive test for everything good
    ///
    [Fact]
    public void EncodeFilterListInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(1));
        Buffer buf = new();
        buf.Data(new ByteBuffer(32));
        FilterList filterList = new();
        EncodeIterator iter = new();

        filterList.ApplyHasPerEntryPermData();
        filterList.ApplyHasTotalCountHint();
        filterList.TotalCountHint = 5;
        Buffer txt = new();
        txt.Data("encodeFilterListInitTest");
        filterList.EncodedEntries.Data(txt.Data());
        filterList.ContainerType = DataTypes.FIELD_LIST;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, filterList.EncodeInit(iter));

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Negative test for unsupported container type
        filterList.ContainerType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, filterList.EncodeInit(iter));

        // reset container type
        filterList.ContainerType = DataTypes.FIELD_LIST;

        // 3. Positive test for everything good
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(4, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
    }

    /// This tests the encodeFilterListComplete() method. It contains two test cases.
    ///
    /// 1. Positive test for everything good with success flag true
    /// 2. Positive test for everything good with success flag false
    ///
    [Fact]
    public void EncodeFilterListComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(32));
        FilterList filterList = new();
        EncodeIterator iter = new();

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        filterList.ApplyHasPerEntryPermData();
        filterList.ApplyHasTotalCountHint();
        filterList.TotalCountHint = 5;
        Buffer txt = new();
        txt.Data("encodeFilterListCompleteTest");
        filterList.EncodedEntries.Data(txt.Data());
        filterList.ContainerType = DataTypes.FIELD_LIST;

        // 1. Positive test for everything good with success flag true
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(iter, true));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(4, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(0, buf.Data().Contents[3]);

        // 2. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(iter, false));
        Assert.Equal(0, buf.Data().Position);
    }

    /// This tests the encodeFilterEntryInit() method. It contains five test cases.
    ///
    /// 1. Negative test for buffer too small to set id
    /// 2. Negative test for buffer too small to set data format
    /// 3. Positive test branch where filterList has container type
    ///    and filter entry does not
    /// 4. Positive test branch where filterList has container type
    ///    and filter entry has container type and action different then Clear
    /// 5. Positive test branch where filterList does not have container type
    ///
    [Fact]
    public void EncodeFilterEntryInit_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(6));
        FilterList filterList = new();
        EncodeIterator iter = new();

        filterList.ApplyHasPerEntryPermData();
        filterList.ApplyHasTotalCountHint();
        filterList.TotalCountHint = 5;
        filterList.ContainerType = DataTypes.FIELD_LIST;

        FilterEntry entry = new();
        entry.Id = 2;

        // 1. Buffer too small to set id
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, entry.EncodeInit(iter, 100));

        // 2. Buffer too small to set data format

        buf = new();
        buf.Data(new ByteBuffer(6));
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, entry.EncodeInit(iter, 100));

        // 3. Positive test case of
        // filterList._containerType != RsslDataTypes.NO_DATA) &&
        // !((entry._flags & RsslFilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
        buf = new();
        buf.Data(new ByteBuffer(9));
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(iter, 100));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(4, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(2, buf.Data().Contents[5]);

        // 4. Positive test case of
        // (entry._containerType != RsslDataTypes.NO_DATA) &&
        // ((entry._flags & RsslFilterEntryFlags.HAS_CONTAINER_TYPE) > 0)) &&
        // (entry._action != RsslFilterEntryActions.CLEAR)
        buf = new();
        buf.Data(new ByteBuffer(10));
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        entry.ApplyHasContainerType();
        entry.ContainerType = DataTypes.MAP;
        entry.Action = FilterEntryActions.SET;
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(iter, 100));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(4, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(0, buf.Data().Contents[3]);
        Assert.Equal(34, buf.Data().Contents[4]);
        Assert.Equal(2, buf.Data().Contents[5]);
        Assert.Equal(9, buf.Data().Contents[6]);

        // 5. Positive test case of ~3 & ~4
        filterList.ContainerType = DataTypes.NO_DATA;
        buf = new();
        buf.Data(new ByteBuffer(10));
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        entry.ApplyHasContainerType();
        entry.ContainerType = DataTypes.MAP;
        entry.Action = FilterEntryActions.SET;
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(iter, 100));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(0, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(0, buf.Data().Contents[3]);
        Assert.Equal(34, buf.Data().Contents[4]);
        Assert.Equal(2, buf.Data().Contents[5]);
        Assert.Equal(9, buf.Data().Contents[6]);

        // 6. Buffer too small when permData is specified.
        buf = new();
        buf.Data(new ByteBuffer(9));
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Buffer permData = new();
        permData.Data(new ByteBuffer(20));
        entry.ApplyHasPermData();
        entry.PermData = permData;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, entry.EncodeInit(iter, 100));

        filterList.ContainerType = DataTypes.NO_DATA;
        buf = new();
        buf.Data(new ByteBuffer(10));
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        entry.Clear();
        entry.Id = 2;
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(iter, 100));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(0, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(0, buf.Data().Contents[3]);
        Assert.Equal(0, buf.Data().Contents[4]);
        Assert.Equal(2, buf.Data().Contents[5]);
        Assert.Equal(0, buf.Data().Contents[6]);
    }

    /// This tests the encodeFilterEntry() method. It contains four test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good without permData
    /// 3. Positive test for everything good without permData but permData flag set
    /// 4. Positive test for everything good with permData
    ///
    [Fact]
    public void EncodeFilterEntry_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(6));
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        FilterList filterList = new();
        FilterEntry filterEntry = new();
        EncodeIterator iter = new();
        byte[] bufBytes;

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        filterList.ApplyHasPerEntryPermData();
        filterList.ApplyHasTotalCountHint();
        filterList.TotalCountHint = 5;
        Buffer txt = new();
        txt.Data("encodeFilterEntryTest");
        filterList.EncodedEntries.Data(txt.Data());
        filterList.ContainerType = DataTypes.FIELD_LIST;

        filterEntry.ApplyHasContainerType();
        filterEntry.Action = FilterEntryActions.SET;
        filterEntry.ContainerType = DataTypes.FIELD_LIST;
        filterEntry.Id = 1;
        txt.Data("encodeFilterEntryTest3");
        filterEntry.EncodedData = txt;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter) );
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, filterEntry.Encode(iter) );

        CodecReturnCode ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(smallBuf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion())
                == CodecReturnCode.SUCCESS)
        {
            ret = filterEntry.Encode(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(iter, true));

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Positive test for everything good without permData
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterEntry.Encode(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(iter, true));

        byte[] expectedDataNoPermData =
            { 0x02, 0x04, 0x05, 0x01, 0x22, 0x01, 0x04, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64,
              0x65, 0x46, 0x69, 0x6c, 0x74, 0x65, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54,
              0x65, 0x73, 0x74, 0x33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        bufBytes = new byte[expectedDataNoPermData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedDataNoPermData, bufBytes);

        // 3. Positive test for everything good without permData but permData flag set
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        filterEntry.ApplyHasPermData();
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterEntry.Encode(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(iter, true));

        byte[] expectedDataNoPermDataButFlagSet =
            { 0x03, 0x04, 0x05, 0x01, 0x32, 0x01, 0x04, 0x00, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64,
              0x65, 0x46, 0x69, 0x6c, 0x74, 0x65, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65,
              0x73, 0x74, 0x33 };

        Assert_ArrayEqual(expectedDataNoPermDataButFlagSet, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 4. Positive test for everything good with permData
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        filterEntry.ApplyHasPermData();
        txt.Data("encodeFilterEntryTest2");
        filterEntry.PermData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterEntry.Encode(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeComplete(iter, true));

        byte[] expectedDataPermData =
            { 3, 4, 5, 1, 50, 1, 4, 22, 101, 110, 99, 111, 100, 101, 70, 105, 108, 116, 101, 114,
              69, 110, 116, 114, 121, 84, 101, 115, 116, 50, 22, 101, 110, 99, 111, 100, 101, 70,
              105, 108, 116, 101, 114, 69, 110, 116, 114, 121, 84, 101, 115, 116, 51 };

        Assert_ArrayEqual(expectedDataPermData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /// This tests the encodeFilterEntryComplete() method. It contains two test cases.
    ///
    /// 1. Positive test for everything good with success flag true
    /// 2. Positive test for everything good with success flag false
    ///
    [Fact]
    public void EncodeFilterEntryComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(100));
        FilterList filterList = new();
        EncodeIterator iter = new();
        FilterEntry entry = new();

        // 1. introduce an error into the RsslEncodeSizeMark in order to execute
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        filterList.ContainerType = DataTypes.FIELD_LIST;
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        EncodingLevel levelInfo = iter._levelInfo[iter._encodingLevel];
        int origPos = levelInfo._internalMark._sizePos;
        levelInfo._internalMark._sizePos = 99;
        Assert.Equal(CodecReturnCode.INVALID_DATA, entry.EncodeComplete(iter, true));
        levelInfo._internalMark._sizePos = origPos;

        filterList.ApplyHasPerEntryPermData();
        filterList.ApplyHasTotalCountHint();
        filterList.TotalCountHint = 5;

        // 2. a simple no data test with success flag true
        filterList.ContainerType = DataTypes.FIELD_LIST;
        entry.Id = 2;
        buf.Data().Clear();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(iter, 100));
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeComplete(iter, true));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(4, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(2, buf.Data().Contents[5]);

        // 3. a simple no data test with success flag false
        buf.Data().Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeInit(iter, 100));
        Assert.Equal(CodecReturnCode.SUCCESS, entry.EncodeComplete(iter, false));

        Assert.Equal(2, buf.Data().Contents[0]);
        Assert.Equal(4, buf.Data().Contents[1]);
        Assert.Equal(5, buf.Data().Contents[2]);
        Assert.Equal(2, buf.Data().Contents[5]);

        // 4. cause RsslEncoders.finishU16Mark() to fail in order to execute
        iter._curBufPos = 1000;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, entry.EncodeComplete(iter, true));
    }

    /// This tests the encodeMapInit() method. It contains eleven test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Negative test for unsupported container type
    /// 3. Negative test for unsupported key primitive type
    /// 4. Negative test for buffer too small without summaryData but flag set
    /// 5. Positive test for everything good without summaryData but flag set
    /// 6. Negative test for buffer too small without setDefs but flag set
    /// 7. Positive test for everything good without setDefs but flag set
    /// 8. Positive test for everything good
    /// 9. Positive test for everything good and summary data complete with success true
    /// 10.Positive test for everything good and summary data complete with success false
    /// 11.Negative test for buffer too small and summary data complete with success true
    /// 11.Negative test for buffer that is big enough for map header but not summary data
    ///
    [Fact]
    public void EncodeMapInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(1));
        Buffer mediumBuf = new();
        mediumBuf.Data(new ByteBuffer(6));
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Buffer summaryDataSmallBuf = new();
        summaryDataSmallBuf.Data(new ByteBuffer(26));
        Eta.Codec.Map map = new();
        EncodeIterator iter = new();

        map.ApplyHasKeyFieldId();
        map.ApplyHasPerEntryPermData();
        map.ApplyHasTotalCountHint();
        map.TotalCountHint = 5;
        Buffer txt = new();
        txt.Data("encodeMapInitTest");
        map.EncodedEntries.Data(txt.Data());
        map.ContainerType = DataTypes.FIELD_LIST;
        map.KeyPrimitiveType = DataTypes.INT;
        map.KeyFieldId = 22;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, map.EncodeInit(iter, 0, 0));

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Negative test for unsupported container type
        map.ContainerType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, map.EncodeInit(iter, 0, 0));

        // reset container type
        map.ContainerType = DataTypes.FIELD_LIST;

        // 3. Negative test for unsupported key primitive type
        map.KeyPrimitiveType = DataTypes.INT_1;
        Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, map.EncodeInit(iter, 0, 0));

        // reset key primitive type
        map.KeyPrimitiveType = DataTypes.INT;

        // 4. Negative test for buffer too small without summaryData but flag set
        iter.SetBufferAndRWFVersion(mediumBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        map.ApplyHasSummaryData();
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, map.EncodeInit(iter, 0, 0));

        // 5. Positive test for everything good without summaryData but flag set
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));

        Assert.Equal(0x1A, buf.Data().Contents[0]);
        Assert.Equal(0x03, buf.Data().Contents[1]);
        Assert.Equal(0x04, buf.Data().Contents[2]);
        Assert.Equal(0x00, buf.Data().Contents[3]);
        Assert.Equal(0x16, buf.Data().Contents[4]);

        // 6. Negative test for buffer too small without setDefs but flag set
        mediumBuf.Data().Rewind();
        iter.SetBufferAndRWFVersion(mediumBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        map.ApplyHasSetDefs();
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, map.EncodeInit(iter, 0, 0));

        // 7. Positive test for everything good without setDefs but flag set
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));

        Assert.Equal(0x1B, buf.Data().Contents[0]);
        Assert.Equal(0x03, buf.Data().Contents[1]);
        Assert.Equal(0x04, buf.Data().Contents[2]);
        Assert.Equal(0x00, buf.Data().Contents[3]);
        Assert.Equal(0x16, buf.Data().Contents[4]);

        // 8. Positive test for everything good
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        txt.Data("encodeMapInitTest");
        map.EncodedSetDefs = txt;
        txt.Data("encodeMapInitTest");
        map.EncodedSummaryData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));

        byte[] expectedData =
            {27, 3, 4, 0, 22, 17, 101, 110, 99, 111,
             100, 101, 77, 97, 112, 73, 110, 105, 116, 84,
             101, 115, 116, 17, 101, 110, 99, 111, 100, 101,
             77, 97, 112, 73, 110, 105, 116, 84, 101, 115,
             116, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);

        // 9. Positive test for everything good and summary data complete with success true
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 64; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        map.EncodedSummaryData.Clear();
        map.ApplyHasSummaryData();
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeSummaryDataComplete(iter, true));

        byte[] expectedData2 =
            { 27, 3, 4, 0, 22, 17, 101, 110, 99, 111, 100, 101, 77, 97, 112, 73,
              110, 105, 116, 84, 101, 115, 116, 128, 0, 5 };

        Assert.Equal(28, buf.Data().Position);
        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, expectedData2.Length));

        // 10. Positive test for everything good and summary data complete with success false
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 64; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeSummaryDataComplete(iter, false));

        byte[] expectedData3 =
            { 27, 3, 4, 0, 22, 17, 101, 110, 99, 111, 100, 101, 77, 97, 112, 73,
              110, 105, 116, 84, 101, 115, 116 };

        // Should be at start of summary data encoding.
        Assert.Equal(25, buf.Data().Position);
        Assert_ArrayEqual(expectedData3, new System.Span<byte>(buf.Data().Contents, 0, expectedData3.Length));

        // 11. Negative test for buffer too small and summary data complete with success true
        iter.SetBufferAndRWFVersion(summaryDataSmallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, map.EncodeInit(iter, 0, 0));
    }

    /// This tests the encodeMapComplete() method. It contains two test cases.
    ///
    /// 1. Positive test for everything good with success flag true
    /// 2. Positive test for everything good with success flag false
    ///
    [Fact]
    public void EncodeMapComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Map map = new();
        EncodeIterator iter = new();
        byte[] bufBytes;
        Buffer txt = new();

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        map.ApplyHasKeyFieldId();
        map.ApplyHasPerEntryPermData();
        map.ApplyHasTotalCountHint();
        map.ApplyHasSummaryData();
        map.ApplyHasSetDefs();
        map.TotalCountHint = 5;
        txt.Data("encodeMapCompleteTest");
        map.EncodedEntries.Data(txt.Data());
        txt.Data("encodeMapCompleteTest");
        map.EncodedSetDefs = txt;
        map.EncodedSummaryData = txt;
        map.ContainerType = DataTypes.FIELD_LIST;
        map.KeyPrimitiveType = DataTypes.INT;
        map.KeyFieldId = 22;

        // 1. Positive test for everything good with success flag true
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, true));

        byte[] expectedData =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x15, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54,
              0x65, 0x73, 0x74, 0x15, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61,
              0x70, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73,
              0x74, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);

        // 2. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, false));
        Assert.Equal(0, buf.Data().Position);
    }

    /// This tests the encodeMapEntryInit() method. It contains five test cases.
    ///
    /// 1. Positive test for everything good without permData but flag set
    /// 2. Negative test for buffer too small
    /// 3. Positive test for everything good with DELETE entry
    /// 4. Positive test for everything good without pre-encoded data
    /// 5. Positive test for everything good with pre-encoded data
    ///
    [Fact]
    public void EncodeMapEntryInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(64));
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Eta.Codec.Map map = new();
        MapEntry mapEntry = new();
        Eta.Codec.Int intKey = new();
        EncodeIterator iter = new();
        Buffer txt = new();

        map.ApplyHasKeyFieldId();
        map.ApplyHasPerEntryPermData();
        map.ApplyHasTotalCountHint();
        map.ApplyHasSummaryData();
        map.ApplyHasSetDefs();
        map.TotalCountHint = 5;
        txt.Data("encodeMapEntryInitTest");
        map.EncodedEntries.Data(txt.Data());
        map.EncodedSetDefs = txt;
        map.EncodedSummaryData = txt;
        map.ContainerType = DataTypes.FIELD_LIST;
        map.KeyPrimitiveType = DataTypes.INT;
        map.KeyFieldId = 22;
        mapEntry.ApplyHasPermData();
        mapEntry.Action = MapEntryActions.ADD;
        intKey.Value(33);

        // 1. Positive test for everything good without permData but flag set
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, intKey, 0));

        byte[] expectedData =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
              0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x00, 0x01, 0x21, 0x00, 0x00,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0 };
        byte[] bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);

        // 2. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.PermData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, mapEntry.EncodeInit(iter, intKey, 0));

        // 3. Positive test for everything good with DELETE entry
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.Action = MapEntryActions.DELETE;
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, intKey, 0));

        byte[] expectedData2 =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
              0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x13, 0x16, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x01, 0x21 };

        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 4. Positive test for everything good without pre-encoded data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.Action = MapEntryActions.ADD;
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, intKey, 0));

        byte[] expectedData3 =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
              0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x16, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x01, 0x21 };

        Assert_ArrayEqual(expectedData3, new System.Span<byte>(buf.Data().Contents, 0, expectedData3.Length));

        // 5. Positive test for everything good with pre-encoded data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        mapEntry.EncodedKey = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, 0));

        byte[] expectedData4 =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54,
              0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x16, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64,
              0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69,
              0x74, 0x54, 0x65, 0x73, 0x74 };

        Assert_ArrayEqual(expectedData4, new System.Span<byte>(buf.Data().Contents, 0, expectedData4.Length));
    }

    /// This tests the encodeMapEntryComplete() method. It contains three test cases.
    ///
    /// 1. Positive test for everything good with success flag true
    /// 2. Positive test for everything good with success flag false
    /// 3. Positive test for everything good with DELETE entry
    ///
    [Fact]
    public void EncodeMapEntryComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Buffer buf2 = new();
        buf2.Data(new ByteBuffer(128));
        Map map = new();
        MapEntry mapEntry = new();
        Int intKey = new();
        EncodeIterator iter = new();
        byte[] bufBytes;
        Buffer txt = new();

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        map.ApplyHasKeyFieldId();
        map.ApplyHasPerEntryPermData();
        map.ApplyHasTotalCountHint();
        map.ApplyHasSummaryData();
        map.ApplyHasSetDefs();
        map.TotalCountHint = 5;
        txt.Data("encodeMapEntryComplete");
        map.EncodedEntries.Data(txt.Data());
        map.EncodedSetDefs = txt;
        map.EncodedSummaryData = txt;
        map.ContainerType = DataTypes.FIELD_LIST;
        map.KeyPrimitiveType = DataTypes.INT;
        map.KeyFieldId = 22;
        mapEntry.ApplyHasPermData();
        mapEntry.PermData = txt;
        mapEntry.Action = MapEntryActions.ADD;
        intKey.Value(33);

        // 1. Positive test for everything good with success flag true
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, intKey, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeComplete(iter, true));

        byte[] expectedData =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70,
              0x6c, 0x65, 0x74, 0x65, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c,
              0x65, 0x74, 0x65, 0x05, 0x00, 0x00, 0x12, 0x16, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
              0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x01, 0x21, (byte)0xfe, 0x00, 0x00,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);

        // 2. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, intKey, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeComplete(iter, false));

        byte[] expectedData2 =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70,
              0x6c, 0x65, 0x74, 0x65, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c,
              0x65, 0x74, 0x65, 0x05, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0 };

        // compare only first 54 bytes here even though more was encoded
        // success = false moves position back to 54
        Assert.Equal(54, buf.Data().Position);
        Assert_ArrayEqual(new System.Span<byte>(expectedData2, 0, 54),
            new System.Span<byte>(buf.Data().Contents, 0, 54));

        // 3. Positive test for everything good with DELETE entry
        iter.SetBufferAndRWFVersion(buf2, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.Action = MapEntryActions.DELETE;
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeInit(iter, intKey, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.EncodeComplete(iter, true));

        byte[] expectedData3 =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70,
              0x6c, 0x65, 0x74, 0x65, 0x16, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d,
              0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c,
              0x65, 0x74, 0x65, 0x05, 0x00, 0x00, 0x13, 0x16, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
              0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x01, 0x21, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData3.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf2.Copy(bufBytes));

        Assert_ArrayEqual(expectedData3, bufBytes);
    }

    /// This tests the encodeMapEntry() method. It contains four test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good without permData
    /// 3. Positive test for everything good without permData but permData flag set
    /// 4. Positive test for everything good with permData
    ///
    [Fact]
    public void EncodeMapEntry_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(49));
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Map map = new();
        MapEntry mapEntry = new();
        EncodeIterator iter = new();
        Int intKey = new();
        Buffer txt = new();
        txt.Data("encodeMapEntryTest");

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        map.ApplyHasKeyFieldId();
        map.ApplyHasPerEntryPermData();
        map.ApplyHasTotalCountHint();
        map.ApplyHasSummaryData();
        map.ApplyHasSetDefs();
        map.TotalCountHint = 5;
        map.EncodedEntries.Data(txt.Data());
        map.EncodedSetDefs = txt;
        map.EncodedSummaryData = txt;
        map.ContainerType = DataTypes.FIELD_LIST;
        map.KeyPrimitiveType = DataTypes.INT;
        map.KeyFieldId = 22;
        mapEntry.Action = MapEntryActions.ADD;
        mapEntry.EncodedData.Data(new ByteBuffer(0));
        intKey.Value(33);

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, mapEntry.Encode(iter, intKey));

        CodecReturnCode ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(smallBuf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = mapEntry.Encode(iter, intKey);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        Assert.Equal(CodecReturnCode.INVALID_DATA, mapEntry.EncodeComplete(iter, true));

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Positive test for everything good without permData
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Encode(iter, intKey));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, true));

        byte[] expectedDataNoPermData =
            { 0x1b, 0x03, 0x04, 0x00, 0x16, 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74,
              0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e,
              0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x01, 0x02, 0x01,
              0x21, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        Assert_ArrayEqual(expectedDataNoPermData,
            new System.Span<byte>(buf.Data().Contents, 0, expectedDataNoPermData.Length));

        // 3. Positive test for everything good without permData but permData flag set
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.ApplyHasPermData();
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Encode(iter, intKey));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, true));

        byte[] expectedDataNoPermDataButFlagSet =
            { 0x1f, 0x03, 0x04, 0x00, 0x16, 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74,
              0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e,
              0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x01, 0x12, 0x00,
              0x01, 0x21, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        Assert_ArrayEqual(expectedDataNoPermDataButFlagSet,
            new System.Span<byte>(buf.Data().Contents, 0, expectedDataNoPermDataButFlagSet.Length));

        // 4. Positive test for everything good with permData
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.ApplyHasPermData();
        mapEntry.PermData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Encode(iter, intKey));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, true));

        byte[] expectedDataPermData =
            { 0x1f, 0x03, 0x04, 0x00, 0x16, 0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65,
              0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74,
              0x12, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e,
              0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x01, 0x12, 0x12,
              0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x4d, 0x61, 0x70, 0x45, 0x6e, 0x74,
              0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x01, 0x21 };

        Assert_ArrayEqual(expectedDataPermData,
            new System.Span<byte>(buf.Data().Contents, 0, expectedDataPermData.Length));

        // 5. Positive test to encode a Entry Key as ASCII to excercise RsslEncoders.encBuffer
        // with RsslEncodeIteratorStates.PRIMITIVE_U15.
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        mapEntry.Clear();
        map.KeyPrimitiveType = DataTypes.ASCII_STRING;
        mapEntry.Action = MapEntryActions.ADD;
        mapEntry.EncodedData.Data(new ByteBuffer(0));
        Buffer asciiKey = new();
        asciiKey.Data("test ascii key");
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Encode(iter, asciiKey));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, true));
    }

    /// This tests the encodeElementListInit method.
    ///
    /// <ol>
    /// <li>Negative case with Buffer to small.</li>
    /// <li>Negative case with Buffer to small with info specified.</li>
    /// <li>Positive case with Info specified. Encoded bytes verified.</li>
    /// <li>Negative test with SetData BUFFER_TO_SMALL</li>
    /// <li>Negative test with SetData with negative set id</li>
    /// <li>Negative test with SetData, setId, but no setDef</li>
    /// <li>Negative test with SetData, no setId, no setDef</li>
    /// <li>Positive test with setData, with an empty setDef</li>
    /// <li>Positive test with setData, setId and with a setDef with zero entries.</li>
    /// <li>Positive test with setData, setId and with a setDef with one entry.</li>
    /// <li>Positive test with setDef and standard data</li>
    /// <li>Positive test with setDef with count=0 and standard data</li>
    /// <li>Negative test with setDef with count=1, standard data and encodedSetData too large, BUFFER_TO_SMALL.</li>
    /// <li>Positive test with setDef with count=1, standard data and encodedSetData</li>
    /// <li>Positive test with setdef count=1, w/o standard data, w/ encodedSetData.</li>
    /// <li>Negative test with setdef count=1, w/o standard data, w/ encodedSetData, BUFFER_TO_SMALL.</li>
    /// <li>Negative test with standard data, buffer_TO_SMALL.</li>
    /// <li>Positive test with standard data. Encoded bytes verified.</li>
    /// </ol>
    ///
    [Fact]
    public void EncodeElementListInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(2));
        Buffer buf = new();
        buf.Data(new ByteBuffer(32));
        ElementList elementList = new();
        EncodeIterator iter = new();

        // 1. Neg test BUFFER_TO_SMALL
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementList.EncodeInit(iter, null, 2));
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 2));

        // 2. Neg test BUFFER_TO_SMALL with Info
        elementList.ApplyHasInfo();
        elementList.ElementListNum = 1;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementList.EncodeInit(iter, null, 2));

        // 3. test with Info (which was previously set)
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));

        // verification data taken from ETAC DataTest
        Assert.Equal(1, buf.Data().Contents[0]);
        Assert.Equal(2, buf.Data().Contents[1]);
        Assert.Equal(0, buf.Data().Contents[2]);
        Assert.Equal(1, buf.Data().Contents[3]);

        // 4. Neg test with SetData BUFFER_TO_SMALL
        smallBuf.Data().Rewind();
        elementList.Clear();
        elementList.ApplyHasSetData();
        iter.Clear();
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementList.EncodeInit(iter, null, 2));

        // 6. Neg test with SetData, setId, but no setDef
        buf.Data().Rewind();
        elementList.Clear();
        elementList.ApplyHasSetData();
        elementList.ApplyHasSetId();
        elementList.SetId = 0;
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SET_DEF_NOT_PROVIDED, elementList.EncodeInit(iter, null, 32));

        // 7. Neg test with SetData, no setId, no setDef
        buf.Data().Rewind();
        elementList.Clear();
        elementList.ApplyHasSetData();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SET_DEF_NOT_PROVIDED, elementList.EncodeInit(iter, null, 32));

        // 8. Neg test with setData, with an empty setDef
        buf.Data().Rewind();
        iter.Clear();
        LocalElementSetDefDb elementSetDefDb = new();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SET_DEF_NOT_PROVIDED, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 9. test with setData, setId and with a setDef with zero entries.
        ElementSetDef elementSetDef = new();
        elementSetDefDb.Definitions[0].Count = elementSetDef.Count;
        elementSetDefDb.Definitions[0].SetId = elementSetDef.SetId;
        if (elementSetDef.Entries != null)
        {
            elementSetDefDb.Definitions[0].Entries = elementSetDef.Entries;
        }
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 10. test with setData, setId and with a setDef with one entry.
        elementSetDefDb.Definitions[0].Count = 1;
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 11. test with setDef and standard data
        elementList.ApplyHasStandardData();
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 12. test with setDef with count=0 and standard data
        elementSetDefDb.Definitions[0].Count = 0;
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 13. neg test with setDef with count=1, standard data and encodedSetData too large, BUFFER_TO_SMALL.
        elementSetDefDb.Definitions[0].Count = 1;
        Buffer encodedSetData = new();
        encodedSetData.Data(new ByteBuffer(40), 0, 30);
        elementList.EncodedSetData = encodedSetData;
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 14. test with setDef with count=1, standard data and encodedSetData
        encodedSetData.Data(encodedSetData.Data(), 0, 10);
        elementList.EncodedSetData = encodedSetData;
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 15. test with setdef count=1, w/o standard data, w/ encodedSetData.
        elementList.Flags &= ~ElementListFlags.HAS_STANDARD_DATA;
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 16. Neg test with setdef count=1, w/o standard data, w/ encodedSetData, BUFFER_TO_SMALL.
        encodedSetData.Data().Compact(); // reset everything for this test
        Assert.Equal(CodecReturnCode.SUCCESS, encodedSetData.Data(encodedSetData.Data(), 0, 40));
        elementList.EncodedSetData = encodedSetData;
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementList.EncodeInit(iter, elementSetDefDb, 40));

        // 17. neg test with standard data, buffer_TO_SMALL.
        elementList.Clear();
        elementList.ApplyHasStandardData();
        smallBuf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // 18. test with standard data.
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, elementSetDefDb, 32));

        // verification data taken from ETAC DataTest
        Assert.Equal(0x08, buf.Data().Contents[0]);
        // Note that bytes 1 and 2 are skipped and not written, so they are not checked.
    }

    /// This tests the encodeElementListComplete method.
    ///
    /// <ol>
    /// <li>Positive case with standard data with boolean true. Encoded bytes verified.</li>
    /// <li>Positive case with standard data with boolean false. Encoded bytes verified.</li>
    /// </ol>
    ///
    [Fact]
    public void EncodeElementListComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(32));
        ElementList elementList = new();
        EncodeIterator iter = new();

        // 1. Positive case with standard data with boolean true.
        elementList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        // verification data taken from ETAC DataTest
        Assert.Equal(0x08, buf.Data().Contents[0]);
        // Note that bytes 1 and 2 are skipped and not written, so they are not checked.

        // 2. Positive case with standard data with boolean false.
        iter.Clear();
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, false)); // rollback

        // verification data taken from ETAC DataTest
        Assert.Equal(0x08, buf.Data().Contents[0]);
        // Note that bytes 1 and 2 are skipped and not written, so they are not checked.
    }

    /// This tests the encodeElementEntry method.
    ///
    /// <ol>
    /// <li>Positive case - encode one element entry into an element list. Encoded bytes verified.</li>
    /// <li>Negative case - encode one element entry into an element list with BUFFER_TOO_SMALL.</li>
    /// <li>Negative case - encode one element entry into an element list with entry.name too big.</li>
    /// <li>Negative case - encode one preencoded element entry into an element list.</li>
    /// <li>Positive case - encode one preencoded element entry into an element list. Encoded bytes verified.</li>
    /// <li>Positive case - encode one empty preencoded element entry into an element list. Encoded bytes verified.</li>
    /// <li>Positive Case - verify encDate (and blank date) EncTime and encEnum. Encoded bytes verified.</li>
    /// <li>Negative case - encode one empty preencoded element entry into an element list.</li>
    /// </ol>
    ///
    [Fact]
    public void EncodeElementEntry_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(32));
        ElementList elementList = new();
        ElementEntry elementEntry = new();
        EncodeIterator iter = new();

        // 1. Positive case - encode one element entry into an element list.
        elementList.ApplyHasStandardData();
        elementList.ApplyHasInfo();
        elementList.ElementListNum = 29731; // 0x7423
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));

        Buffer name = new();
        name.Data("uint");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.UINT;
        Eta.Codec.UInt uInt = new();
        uInt.Value(1234);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, uInt));

        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        byte[] expectedData = DATA_001_elementList_one_entry;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData,
            new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 2. Neg case - encode one element entry into an element list with BUFFER_TOO_SMALL.
        elementList.Clear();
        elementList.ApplyHasStandardData();
        buf.Data().Rewind();
        buf.Data(buf.Data(), 0, 8);
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(iter, uInt));

        CodecReturnCode ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(buf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = elementEntry.Encode(iter, uInt);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 3. Neg case - encode one element entry into an element list with entry.name too big.
        buf.Data().Rewind();
        buf.Data(buf.Data(), 0, 10);
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        name.Data("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // over 0x80
        elementEntry.Name = name;
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(iter, uInt));

        ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(buf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = elementEntry.Encode(iter, uInt);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 4. Neg case - encode one preencoded element entry into an element list.
        Buffer bigBuf = new();
        bigBuf.Data(new ByteBuffer(300));
        iter.Clear();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));
        Buffer preencoded = new();
        preencoded.Data("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // len=260
        elementEntry.EncodedData = preencoded;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(iter));

        ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(bigBuf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = elementEntry.Encode(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 5. Positive case - encode one preencoded element entry into an element list.
        bigBuf.Data(bigBuf.Data(), 0, bigBuf.Data().Position);
        bigBuf.Data().Rewind();
        name.Data("ABCDEFGHIJK");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ASCII_STRING;
        iter.Clear();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 300));
        elementEntry.EncodedData = preencoded;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        expectedData = DATA_003_elementList_oneEntry_preencodedAscii;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(bigBuf.Data().Contents, 0, bigBuf.Length));

        // 6. Positive case - encode one empty preencoded element entry into an element list.
        bigBuf.Data(bigBuf.Data(), 0, bigBuf.Data().Position);
        bigBuf.Data().Rewind();
        name.Data("ABCDEFGHIJK");
        elementEntry.Name = name;
        preencoded.Clear();
        iter.Clear();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 300));
        elementEntry.EncodedData = preencoded;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        expectedData = DATA_002_elementList_oneEntry_emptyPreEncodedAscii;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(bigBuf.Data().Contents, 0, bigBuf.Length));

        // 7. Positive Case - verify encDate (and blank date) EncTime and encEnum
        bigBuf.Data().Rewind();
        bigBuf.Data().Limit = bigBuf.Data().Capacity;
        bigBuf.Data(bigBuf.Data(), 0, 300);
        iter.Clear();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 300));
        // test RsslEncoders.encDate
        name.Data("Date test");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.DATE;
        Eta.Codec.Date date = new();
        date.Day(23);
        date.Month(5);
        date.Year(2012);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, date));

        // test RsslEncoders.encDate blank
        name.Data("Blank Date test");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.DATE;
        date = new();
        date.Blank();
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, date));

        // test RsslEncoders.encTime
        name.Data("Time test");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.TIME;
        Eta.Codec.Time time = new();
        time.Hour(11);
        time.Minute(30);
        time.Second(15);
        time.Millisecond(7);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, time));

        // test RsslEncoders.encTime blank
        name.Data("Blank Time test");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.TIME;
        time = new();
        time.Blank();
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, time));

        //test RsslEncoders.encEum
        name.Data("Enum test");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ENUM;
        Eta.Codec.Enum enumeration = new();
        enumeration.Value(127);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, enumeration));
        // encode element list complete
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        // verify encoded data against ETAC.
        expectedData = DATA_007_elementList_entries_wdate_time_enum;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(bigBuf.Data().Contents, 0, bigBuf.Length));

        // 8. Neg case - encode one empty preencoded element entry into an element list.
        bigBuf.Data().Rewind();
        bigBuf.Data(bigBuf.Data(), 0, 32);
        name.Data("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // over 0x80
        elementEntry.Name = name;
        preencoded.Clear();
        iter.Clear();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 32));
        elementEntry.EncodedData = preencoded;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.Encode(iter));

        ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(buf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = elementEntry.Encode(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);
    }

    /// This tests the encodeStatusMsg() method. It contains four test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good without permData
    /// 3. Positive test for everything good without permData but permData flag set
    /// 4. Positive test for everything good with permData
    ///
    [Fact]
    public void EncodeStatusMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(11));
        IStatusMsg msg = (IStatusMsg)new Msg();
        msg.DomainType = 4;
        msg.MsgClass = MsgClasses.STATUS;
        msg.StreamId = 24;
        EncodeIterator iter = new();

        // 1. None of the data is set
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData = { 0, 8, 3, 4, 0, 0, 0, 24, 0, 0 };
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, expectedData.Length));

        // 2. Has state and buffer too small
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.ApplyHasState();
        msg.State.Code(StateCodes.TOO_MANY_ITEMS);
        msg.State.DataState(DataStates.SUSPECT);
        msg.State.StreamState(StreamStates.CLOSED_RECOVER);
        Buffer textBuffer = new();
        textBuffer.Data("encodeStateMsgTest");
        msg.State.Text(textBuffer);
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.Encode(iter));

        // 3. Has group and buffer too small
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasGroupId();
        byte[] gib = { 11, 123, 8, 3, 76, 2 };
        Buffer groupId = new();
        groupId.Data(ByteBuffer.Wrap(gib));
        msg.GroupId = groupId;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.Encode(iter));

        // 4. Has perm data and buffer too small
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasPermData();
        byte[] pb = { 10, 5, 3, 9 };
        Buffer permData = new();
        permData.Data(ByteBuffer.Wrap(pb));
        msg.PermData = permData;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.Encode(iter));

        // 4. Has key and buffer too small
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasFilter();
        msg.MsgKey.Filter = 7;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.Encode(iter));


        // 5. Has ext header and buffer too small
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasExtendedHdr();
        byte[] ehb = { 67, 1, 2, 3, 4, 5, 6, 7, 8, 9, 67 };
        Buffer extHeader = new();
        extHeader.Data(ByteBuffer.Wrap(ehb));
        msg.ExtendedHeader = extHeader;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.Encode(iter));

        // 5. Has post user info and buffer too small
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasPostUserInfo();
        long userAddr = 1234L;
        msg.PostUserInfo.UserAddr = userAddr;
        long userId = 567L;
        msg.PostUserInfo.UserId = userId;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.Encode(iter));

        // 4. Has state and ok
        buf = new();
        buf.Data(new ByteBuffer(31));
        msg.Flags = 0;
        msg.ApplyHasState();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData1 =
              { 0, 29, 3, 4, 0, 0, 0, 24, 32, 0, 26, 13, 18, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
                0x74,  0x61, 0x74, 0x65, 0x4D, 0x73, 0x67, 0x54, 0x65, 0x73, 0x74 };
        Assert_ArrayEqual(expectedData1, new System.Span<byte>(buf.Data().Contents, 0, expectedData1.Length));

        // 5. Has group id and ok
        ClearBuffer(buf.Data());
        msg.Flags = 0;
        msg.ApplyHasGroupId();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData2 =
            { 0, 15, 3, 4, 0, 0, 0, 24, 16, 0, 6, 11, 123, 8, 3, 76, 2 };
        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, expectedData2.Length));

        // 6. Has perm data and ok
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasPermData();
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData3 =
            { 0, 13, 3, 4, 0, 0, 0, 24, 2, 0, 4, 10, 5, 3, 9 };
        Assert_ArrayEqual(expectedData3, new System.Span<byte>(buf.Data().Contents, 0, expectedData3.Length));

        // 7. Has key and ok
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasMsgKey();
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData4 =
            { 0, 15, 3, 4, 0, 0, 0, 24, 8, 0, 128, 5, 8, 0, 0, 0, 7 };
        Assert_ArrayEqual(expectedData4, new System.Span<byte>(buf.Data().Contents, 0, expectedData4.Length));

        // 8. Has ext header and ok
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasExtendedHdr();
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData5 =
            { 0, 20, 3, 4, 0, 0, 0, 24, 1, 0, 11, 67, 1, 2, 3, 4, 5, 6, 7, 8, 9, 67 };
        Assert_ArrayEqual(expectedData5, new System.Span<byte>(buf.Data().Contents, 0, expectedData5.Length));

        // 9. Has post user info and ok
        ClearBuffer(buf.Data());
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.Flags = 0;
        msg.ApplyHasPostUserInfo();
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData6 =
            { 0, 17, 3, 4, 0, 0, 0, 24, 129, 0, 0, 0, 0, 4, 210, 0, 0, 2, 55 };
        Assert_ArrayEqual(expectedData6, new System.Span<byte>(buf.Data().Contents, 0, expectedData6.Length));

        // 10. Has all
        buf = new();
        buf.Data(new ByteBuffer(71));
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        msg.ApplyHasPostUserInfo();
        msg.ApplyHasExtendedHdr();
        msg.ApplyHasGroupId();
        msg.ApplyHasMsgKey();
        msg.ApplyHasPermData();
        msg.ApplyHasState();
        msg.ApplyClearCache();

        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(iter));

        byte[] expectedData7 =
            { 0, 0x45, 0x03, 0x04, 0x00, 0x00, 0x00, 0x18,
                 0x81, 0x7b, 0x00,
                 0x1a, 0x0d, 0x12, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
                 0x74, 0x61, 0x74, 0x65, 0x4D, 0x73, 0x67, 0x54, 0x65, 0x73, 0x74,
                 0x06, 0x0b, 0x7b, 0x08, 0x03, 0x4c, 0x02,
                 0x04, 0x0a, 0x05, 0x03, 0x09,
                 0x80, 0x05, 0x08, 0x00, 0x00, 0x00, 0x07,
                 0x0b, 0x43, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x43,
                 0x00, 0x00, 0x04, 0xd2, 0x00, 0x00, 0x02, 0x37 };
        Assert_ArrayEqual(expectedData7, new System.Span<byte>(buf.Data().Contents, 0, expectedData7.Length));
    }

    /// This tests the encodeElementEntryInit and encodeElementEntryComplete method.
    ///
    /// <ol>
    /// <li>Positive case - encode three element entry into an element list. Middle element is an element list. Encoded bytes verified.</li>
    /// <li>Negative case - test elementEntryInit() with an element name that is too big (BUFFER_TO_SMALL)</li>
    /// <li>Positive case - test elementEntryInit() with entry with a container of NO_DATA.</li>
    /// <li>Positive case - test encodeElementEntryComplete with false. Encoded bytes verified.</li>
    /// <li>Negative case - test encodeElementEntryComplete with internalMerk._sizeBytes > 0.</li>
    /// </ol>
    ///
    [Fact]
    public void EncodeElementEntryInitComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(105));
        ElementList elementList = new();
        ElementEntry elementEntry = new();
        EncodeIterator iter = new();

        /* 1. Positive case - encode three element entry into an element list.
         * Middle element is an element list. Encoded bytes verified.
         *    The first entry is a UINT.
         *    The second entry is an element list with an ASCII string as an element.
         *    The third entry is a UINT.
         *      ElementListInit
         *        entry1 - "uint type" UINT 1234
         *        entry2 - "container type" ElementList
         *          ElementListInit
         *            entry1: "string type" ASCII "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
         *          ElementListComplete\
         *        entry3 - "uint type" UINT 987654321
         *      ElementListComplete
         */
        elementList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 105));

        // first entry - uint 1234
        Buffer name = new();
        name.Data("uint type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.UINT;
        Eta.Codec.UInt uInt = new();
        uInt.Value(1234);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, uInt));

        // second entry - element list
        name.Data("container type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ELEMENT_LIST;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(iter, 0));
        // has standard data
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 50));
        name.Data("string type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ASCII_STRING;
        Buffer stringBuffer = new();
        stringBuffer.Data("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, stringBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeComplete(iter, true));

        // third entry - uint 987654321
        name.Data("another element");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.UINT;
        uInt.Value(987654321);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, uInt));

        // complete the list
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        byte[] expectedData = DATA_004_elementList_wthrreEntries_middleEntryHasElementList;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 2. Negative case - test elementEntryInit() with an element name that is too big (BUFFER_TO_SMALL).
        elementList.Clear();
        buf.Data().Rewind();
        buf.Data().Limit = buf.Data().Capacity;
        buf.Data(buf.Data(), 0, 25);
        iter.Clear();
        elementList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 25));

        name.Data("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"); // 180
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ELEMENT_LIST;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.EncodeInit(iter, 0));

        // 3. Positive case - test elementEntryInit() with entry with a container of NO_DATA.
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 25));

        name.Data("no data type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.NO_DATA;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(iter, 0));

        // 4. Positive case - test encodeElementEntryComplete with false. Encoded bytes verified.
        buf.Data().Rewind();
        buf.Data(buf.Data(), 0, 105);
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        elementList.Clear();
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 105));

        // first entry - uint 1234
        name.Data("uint type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.UINT;
        uInt.Value(1234);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, uInt));

        // second entry - element list
        name.Data("container type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ELEMENT_LIST;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(iter, 0));
        // has standard data
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 50));
        name.Data("string type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ASCII_STRING;
        stringBuffer.Data("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, stringBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeComplete(iter, false));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));

        expectedData = DATA_005_elementList_secondEntryElementList_rollback;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 5. Negative case - test encodeElementEntryComplete with internalMerk._sizeBytes > 0.
        Buffer bigBuf = new();
        bigBuf.Data(new ByteBuffer(312));
        iter.Clear();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        elementList.Clear();
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 105));

        // first entry - uint 1234
        name.Data("uint type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.UINT;
        uInt.Value(1234);
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, uInt));

        // second entry - element list
        name.Data("container type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ELEMENT_LIST;
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.EncodeInit(iter, 0));
        // has standard data
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(iter, null, 50));
        name.Data("string type");
        elementEntry.Name = name;
        elementEntry.DataType = DataTypes.ASCII_STRING;
        stringBuffer.Data("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZA"); // 209
        Assert.Equal(CodecReturnCode.SUCCESS, elementEntry.Encode(iter, stringBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(iter, true));
        // force encodeElementEntryComplete() to call finishU16Mark with dataLength >= 0xFE,
        // by setting sizeBytes to 1.
        iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 1;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, elementEntry.EncodeComplete(iter, true));
    }

    /// This tests the encodeFieldListInit method.
    ///
    /// <ol>
    /// <li>Negative case - field list without anything, BUFFER_TO_SMALL.</li>
    /// <li>Negative case - field list with info, BUFFER_TO_SMALL.</li>
    /// <li>Negative case - dictionaryID with negative number - INVALID_DATA.</li>
    /// </ol>
    /// Note that the positive case is covered in encodeFieldListTest().
    ///
    [Fact]
    public void EncodeFieldListInit_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(105));
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(0));
        FieldList fieldList = new();
        EncodeIterator iter = new();

        // 1. Negative case - field list without anything, BUFFER_TO_SMALL.
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldList.EncodeInit(iter, null, 105));


        // 2. Negative case - field list with info, BUFFER_TO_SMALL.
        fieldList.ApplyHasInfo();
        smallBuf.Data(new ByteBuffer(4));

        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldList.EncodeInit(iter, null, 105));

        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 10;
        fieldList.ApplyHasStandardData();
        smallBuf.Data(new ByteBuffer(6));
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldList.EncodeInit(iter, null, 105));
    }

    /// This tests the encodeFieldListComplete method.
    ///
    /// <ol>
    /// <li>encodeFieldListComplete with false and verify iterator roll back.</li>
    /// </ol>
    /// Note that the positive case is covered in encodeFieldListTest().
    ///
    [Fact]
    public void EncodeFieldListComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(105));
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();
        EncodeIterator iter = new();

        // 1. encodeFieldListComplete with false and verify iterator roll back.
        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 0;
        fieldList.FieldListNum = 0;
        fieldList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 105));

        fieldEntry.FieldId = 22;
        fieldEntry.DataType = DataTypes.REAL;
        Real real = new();
        real.Value(123456789, RealHints.EXPONENT_4);
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter, real));

        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(iter, false));
        Assert.Equal(0, iter._curBufPos); // verify rollback
    }

    /// This tests the encodeFieldEntry method.
    ///
    /// <ol>
    /// <li>test with buffer_to_small at encodeFieldEntry() data!=null.</li>
    /// <li>test with unsupported data type in entry.</li>
    /// <li>test with encodedData.</li>
    /// <li>test with encdata, buffer_to_small.</li>
    /// <li>test with encoding blank.</li>
    /// <li>test with encoding blank, buffer_to_small.</li>
    /// </ol>
    /// Note that the positive case is covered in encodeFieldListTest().
    ///
    [Fact]
    public void EncodeFieldEntry_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(7));
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();
        EncodeIterator iter = new();

        // 1. test with buffer_to_small at encodeFieldEntry() data!=null.
        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 0;
        fieldList.FieldListNum = 0;
        fieldList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 7));

        fieldEntry.FieldId = 22;
        fieldEntry.DataType = DataTypes.UINT;
        UInt uInt = new();
        uInt.Value(554433);
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(iter, uInt));

        CodecReturnCode ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(buf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = fieldEntry.Encode(iter, uInt);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 2. test with unsupported data type in entry.
        buf = new();
        buf.Data(new ByteBuffer(60));
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 60));
        fieldEntry.DataType = 254;
        Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, fieldEntry.Encode(iter, uInt));

        // 3. test with encodedData
        buf.Data().Clear();
        fieldEntry.Clear();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 60));
        Buffer encodedData = new();
        encodedData.Data("abcdefg");
        fieldEntry.FieldId = 100;
        fieldEntry.DataType = DataTypes.OPAQUE;
        fieldEntry.EncodedData = encodedData;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter));

        // 4. test with encdata, buffer_to_small
        buf.Data().Clear();
        fieldEntry.Clear();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 256));
        encodedData = new();
        encodedData.Data("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ0123");
        fieldEntry.FieldId = 100;
        fieldEntry.DataType = DataTypes.OPAQUE;
        fieldEntry.EncodedData = encodedData;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(iter));

        ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(buf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = fieldEntry.Encode(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 5. test with encoding blank.
        buf.Data().Clear();
        fieldEntry.Clear();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 256));
        fieldEntry.FieldId = 30;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeBlank(iter));

        // 6. test with encoding blank, buffer_to_small
        iter.Clear();
        buf.Data().Clear();
        buf.Data(buf.Data(), 0, 8);
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 8));
        fieldEntry.FieldId = 30;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.EncodeBlank(iter));

        ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(buf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = fieldEntry.EncodeBlank(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);
    }

    /// This tests the encodeFieldEntryInit() method. It contains two test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good
    ///
    [Fact]
    public void EncodeFieldEntryInit_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(7));
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();
        EncodeIterator iter = new();

        // 1. Negative test for buffer too small.
        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 0;
        fieldList.FieldListNum = 0;
        fieldList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 7));

        fieldEntry.FieldId = 22;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer txt = new();
        txt.Data("encodeFieldEntryInitTest");
        fieldEntry.EncodedData = txt;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.EncodeInit(iter, 0));

        // 2. Positive test for everything good.
        buf = new();
        buf.Data(new ByteBuffer(60));
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 60));
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeInit(iter, 0));

        Assert.Equal(9, buf.Data().Contents[0]);
        Assert.Equal(3, buf.Data().Contents[1]);
        Assert.Equal(0x16, buf.Data().Contents[8]);
        Assert.Equal(12, buf.Data().Position);
    }


    /// This tests the encodeFieldEntryComplete() method. It contains two test cases.
    ///
    /// 1. Positive test for everything good, success flag true
    /// 2. Positive test for everything good, success flag false
    ///
    [Fact]
    public void EncodeFieldEntryComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(60));
        Buffer buf2 = new();
        buf2.Data(new ByteBuffer(60));
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeFieldEntryInitTest");

        // 1. Positive test for everything good, success flag true.
        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 0;
        fieldList.FieldListNum = 0;
        fieldList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        fieldEntry.FieldId = 22;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        fieldEntry.EncodedData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 60));
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeComplete(iter, true));

        Assert.Equal(9, buf.Data().Contents[0]);
        Assert.Equal(3, buf.Data().Contents[1]);
        Assert.Equal(0x16, buf.Data().Contents[8]);
        Assert.Equal(0xfe, buf.Data().Contents[9]);
        Assert.Equal(12, buf.Data().Position);

        // 2. Positive test for everything good, success flag false.
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf2, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 60));
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeComplete(iter, false));

        Assert.Equal(9, buf2.Data().Contents[0]);
        Assert.Equal(3, buf2.Data().Contents[1]);
        Assert.Equal(7, buf2.Data().Position);
    }


    /// Positive test. Encode a field list as follows, and verify encoded data.
    /// <ul>
    /// <li>FieldListInit</li>
    /// <ul><li>FieldEntry - (22)  UINT 12345</li></ul>
    /// <li>FieldListComplete false (roll-back)</li>
    /// <li>FieldListInit</li>
    /// <ul>
    /// <li>FieldEntry - (10)  REAL Blank - blank to encoder.</li>
    /// <li>FieldEntry - (175) pre-encoded data. (ABCDEFG)</li>
    /// <li>FieldEntry - (32)  UINT 554433</li>
    /// <li>FieldEntry - (111) REAL 867564 EXPONENT_4.</li>
    /// <li>FieldEntry - (54)  REAL Blank - real.isBlank</li>
    /// </ul>
    /// <li>FieldListComplete</li>
    ///
    [Fact]
    public void EncodeFieldList_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(40));
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();
        EncodeIterator iter = new();

        /* 1. Positive test. Encode a field list as follows, and verify encoded data.
         *    FieldListInit
         *      FieldEntry - (22)  UINT 12345
         *    FieldListComplete false (roll-back)
         *    FieldListInit
         *      FieldEntry - (10)  REAL Blank - blank to encoder.
         *      FieldEntry - (175) pre-encoded data. (ABCDEFG)
         *      FieldEntry - (32)  UINT 554433
         *      FieldEntry - (111) REAL 867564 EXPONENT_4.
         *      FieldEntry - (54)  REAL Blank - real.isBlank
         *    FieldListComplete
         */

        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 40));

        // encode (22) UINT 12345
        fieldEntry.FieldId = 22;
        fieldEntry.DataType = DataTypes.UINT;
        Eta.Codec.UInt uInt = new();
        uInt.Value(12345);
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter, uInt));

        // roll-back.
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(iter, false));

        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(iter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter));

        // encode (32) UINT 554433
        uInt.Clear();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Eta.Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter, real));

        // encode (54) REAL Blank - real.isBlank
        real.Clear();
        real.Blank();
        fieldEntry.FieldId = 54;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(iter, real));

        // encodeFieldListComplete
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(iter, true));

        // verify data with ETAC
        Assert_ArrayEqual(DATA_006_encodeFieldList_wEntries_andRollBack,
            new System.Span<byte>(buf.Data().Contents, 0, DATA_006_encodeFieldList_wEntries_andRollBack.Length));
    }

    /// This tests the encodeArrayInit, encodeArrayEntry and
    /// encodeArrayComplete methods.
    ///
    /// <ol>
    /// <li>Negative case - set primitive array type to an unsupported data type.</li>
    /// <li>Negative case - use a buffer that is too small.</li>
    /// <li>Negative case - invalid item length (negative value).</li>
    /// <li>Positive case - valid item length.</li>
    /// <li>Positive case - encodeArrayComplete with true</li>
    /// <li>Positive case - encodeArrayComplete with false.</li>
    /// <li>Negative case - encodeArrayEntry with length = 0 and an invalid primitive type.</li>
    /// <li>Negative case - encodeArrayEntry with length = 0</li>
    /// <li>Negative case - encodeArrayEntry with length = 0 and unsupported dataType of UTF8 (returned by encodePrimitive()).</li>
    /// <li>Negative Case - set item length, a valid primitive type, but a buffer too small.</li>
    /// <li>Positive Case - set item length and a valid primitive type. Verify encoded data.</li>
    /// <li>Positive case - Array with encodedData, itemLength > 0. Verify encoded data.</li>
    /// <li>Positive case - Array with encodedData, itemLength=0. Verify encoded data.</li>
    /// <li>Negative case - ArrayEntry with encodedData=null and data=null.</li>
    /// <li>Positive case - Array populated with UInts. verify encoded data.</li>
    /// </ol>
    /// Note that the positive case is covered in encodeFieldListTest().
    ///
    [Fact]
    public void EncodeArrayInitEntryComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(3));
        Eta.Codec.Array array = new();
        Eta.Codec.Int Int = new();

        // 1. Negative case - set primitive array type to an unsupported data type.
        EncodeIterator iter = new();
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        // Assert.Throws(() => array.PrimitiveType = DataTypes.INT_2);
        // Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, Encoders.EncodeArrayInit(iter, array));

        // 2. Negative case - use a buffer that is too small.
        array.PrimitiveType = DataTypes.INT;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, Encoders.EncodeArrayInit(iter, array));

        // 4. Positive case - valid item length.
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.ItemLength = 1;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));

        // 5. Positive case - encodeArrayComplete with true.
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // 6. Positive case - encodeArrayComplete with false.
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, false));

        // 7. Negative case - encodeArrayEntry with length = 0 and an invalid primitive type.
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.RMTES_STRING + 1;
        array.ItemLength = 0;
        Int.Clear();
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, Encoders.EncodeArrayEntry(iter, Int));

        // 8. Negative case - encodeArrayEntry with length = 0
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.RMTES_STRING + 1;
        array.ItemLength = 0;
        Int.Clear();
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, Encoders.EncodeArrayEntry(iter, Int));

        // 9. Negative Case - set item length, a valid primitive type, but a buffer too small.
        smallBuf.Data(new ByteBuffer(10));
        iter.Clear();
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.ASCII_STRING;
        array.ItemLength = 11;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, Encoders.EncodeArrayEntry(iter, smallBuf));

        // 10. Positive Case - set item length and a valid primitive type. Verify encoded data.
        smallBuf.Data("Presidente");
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.ASCII_STRING;
        array.ItemLength = 10;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        smallBuf.Data("VicePresid");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        smallBuf.Data("aSecretary");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify data with ETAC
        byte[] expectedData = DATA_008_array_entries_ascii;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 11. Positive case - Array with encodedData, itemLength > 0. Verify encoded data.
        //     ArrayInit primitiveType=Buffer itemLength =
        buf.Data(buf.Data(), 0, buf.Data().Position);
        buf.Data().Rewind();
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.BUFFER;
        array.ItemLength = 5;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        smallBuf.Data("abcde");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        smallBuf.Data("01234");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        smallBuf.Data("ABCDE");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = DATA_010_array_entries_encData_len2;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 12. Positive case - Array with encodedData, itemLength=0. Verify encoded data.
        buf.Data().Rewind();
        buf.Data().Limit = buf.Data().Capacity;
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.Clear();
        array.PrimitiveType = DataTypes.BUFFER;
        array.ItemLength = 0;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        smallBuf.Data("abcde");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        smallBuf.Data("0123456789");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        smallBuf.Data("ABCDEFG");
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, smallBuf));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = DATA_011_array_entries_encData_len0;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 13. Positive case - ArrayEntry with itemLength > 0 and encodedData.Length=0. Verify encoded data.
        buf.Data().Rewind();
        buf.Data().Limit = buf.Data().Capacity;
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.Clear();
        array.PrimitiveType = DataTypes.BUFFER;
        array.ItemLength = 4;
        Buffer emptyBuffer = new();
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, emptyBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, emptyBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, emptyBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = DATA_012_array_entry_blankEncData_len4;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 14. Positive case - ArrayEntry with itemLength > 0 and encodedData.Length=0. Verify encoded data.
        buf.Data().Rewind();
        buf.Data().Limit = buf.Data().Capacity;
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.Clear();
        array.PrimitiveType = DataTypes.BUFFER;
        array.ItemLength = 0;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, emptyBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, emptyBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, emptyBuffer));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = DATA_013_array_entry_blankEncData_len0;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));

        // 15. Positive case - Array populated with UInts. Verify encoded data.
        //     ArrayInit primitiveType=UInt, itemLength=2;
        //       ArrayEntry 0
        //       ArrayEntry 255        // one byte
        //       ArrayEntry 65535      // two bytes
        //     ArrayComplete
        buf.Data().Rewind();
        buf.Data().Limit = buf.Data().Capacity;
        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.UINT;
        array.ItemLength = 2;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        Eta.Codec.UInt uInt = new UInt();
        uInt.Value(0);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, uInt));
        uInt.Value(255);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, uInt));
        uInt.Value(65535);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, uInt));
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify encoded data
        expectedData = DATA_009_array_entries_uint;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    [Fact]
    public void EncodePrimitive_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        EncodeIterator iter = new();
        Eta.Codec.Array array = new();
        Int Int = new();

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        array.PrimitiveType = DataTypes.INT;
        array.ItemLength = 0;
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayInit(iter, array));
        // one byte needed
        Int.Value(0);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(-1);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(127);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(-128);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        // two byte needed
        Int.Value(128);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(32767);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(-32768);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        // three bytes needed
        Int.Value(32768);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(8388607);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(-8388608);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        // four bytes needed
        Int.Value(8388608);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(2147483647);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        Int.Value(-2147483648);
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayEntry(iter, Int));
        // encodeArrayComplete
        Assert.Equal(CodecReturnCode.SUCCESS, Encoders.EncodeArrayComplete(iter, true));

        // verify encoded data
        byte[] expectedData = DATA_014_encodePrimitive_int_len0;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData,
            new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /// encode a request message and compare encoded contexts with ETAC.
    ///
    [Fact]
    public void EncodeRequestMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IRequestMsg msg = (IRequestMsg)new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new(), applicationName = new();
        Buffer position = new();
        // RsslBuffer password = Rsslnew(), instanceId =
        // Rsslnew();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.REQUEST;
        msg.StreamId = 100;
        msg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.ELEMENT_LIST;
        msg.ApplyStreaming();
        msg.ApplyConfInfoInUpdates();
        msg.ApplyMsgKeyInUpdates();
        msg.ApplyHasBatch();
        msg.ApplyHasView();

        /* set priority */
        msg.ApplyHasPriority();
        msg.Priority.PriorityClass = 3;
        msg.Priority.Count = 4;

        /* set QoS and Worst QoS */
        msg.ApplyHasQos();
        msg.ApplyHasWorstQos();
        msg.Qos.Timeliness(QosTimeliness.REALTIME);
        msg.Qos.Rate(QosRates.TICK_BY_TICK);
        msg.WorstQos.Timeliness(QosTimeliness.DELAYED);
        msg.WorstQos.TimeInfo(65532);
        msg.WorstQos.Rate(QosRates.TIME_CONFLATED);
        msg.WorstQos.RateInfo(65533);

        /* set msgKey members */
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();

        msg.MsgKey.Name.Data("Batch_Request");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
                     msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload

        ElementList eList = new();
        ElementEntry eEntry = new();
        Array elementArray = new();
        ArrayEntry ae = new();
        Buffer itemName = new();

        eList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.Name = ElementNames.BATCH_ITEM_LIST;
        eEntry.DataType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.PrimitiveType = DataTypes.ASCII_STRING;
        elementArray.ItemLength = 0;

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));
        itemName.Data("TRI.N");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));
        itemName.Data("IBM.N");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));
        itemName.Data("CSCO.O");
        Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, itemName));

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));

        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));

        // encode View (22 = BID, 25 = ASK)
        int[] viewList = { 22, 25 };
        int viewListCount = 2;
        int i;
        Eta.Codec.UInt tempUInt = new();

        eEntry.Clear();
        eEntry.Name = ElementNames.VIEW_TYPE;
        eEntry.DataType = DataTypes.UINT;
        tempUInt.Value(ViewTypes.FIELD_ID_LIST);
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.Encode(encodeIter, tempUInt));

        eEntry.Clear();
        eEntry.Name = ElementNames.VIEW_DATA;
        eEntry.DataType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeInit(encodeIter, 0));

        elementArray.Clear();
        elementArray.PrimitiveType = DataTypes.UINT;
        elementArray.ItemLength = 2; // fixed length values

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeInit(encodeIter));

        for (i = 0; i < viewListCount; i++)
        {
            tempUInt.Value(viewList[i]);
            Assert.Equal(CodecReturnCode.SUCCESS, ae.Encode(encodeIter, tempUInt));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, elementArray.EncodeComplete(encodeIter, true));

        /* complete encoding of complex element entry. */
        Assert.Equal(CodecReturnCode.SUCCESS, eEntry.EncodeComplete(encodeIter, true));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, eList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_001_requestMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData,
            new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /// Encode a refresh message and compare encoded contexts with ETAC.
    ///
    [Fact]
    public void EncodeRefreshMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IRefreshMsg msg = (IRefreshMsg)new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new(), applicationName = new();
        Buffer position = new();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.REFRESH;
        msg.StreamId = int.MaxValue;
        msg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.FIELD_LIST;
        msg.ApplyRefreshComplete();
        msg.ApplySolicited();
        msg.ApplyClearCache();
        msg.ApplyDoNotCache();

        /* set state */
        msg.State.StreamState(StreamStates.OPEN);
        msg.State.DataState(DataStates.OK);
        msg.State.Code(StateCodes.NONE);
        Buffer text = new();
        text.Data("some text info");
        msg.State.Text(text);

        /* set groupId */
        Buffer groupId = new();
        groupId.Data("10203040");
        msg.GroupId = groupId;

        /* set QoS */
        msg.ApplyHasQos();
        msg.Qos.Timeliness(QosTimeliness.REALTIME);
        msg.Qos.Rate(QosRates.TICK_BY_TICK);

        /* set part number */
        msg.ApplyHasPartNum();
        msg.PartNum = 32767;

        /* set sequence number */
        msg.ApplyHasSeqNum();
        msg.SeqNum = 1234567890L;

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* set permData */
        msg.ApplyHasPermData();
        byte[] ba = { 0x10, 0x11, 0x12, 0x13 };
        Buffer permData = new();
        permData.Data(ByteBuffer.Wrap(ba));
        msg.PermData = permData;

        /* set post user info */
        msg.ApplyHasPostUserInfo();
        msg.PostUserInfo.UserAddr = 4294967290L; // 0xFFFFFFFA
        msg.PostUserInfo.UserId = 4294967295L; // 0xFFFFFFFF

        /* set msgKey members */
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasServiceId();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
        msg.MsgKey.ServiceId = 32639;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
                     msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */


        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter));

        // encode (32) UINT 554433
        Eta.Codec.UInt uInt = new();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Eta.Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, real));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_002_refreashMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData,
            new System.Span<byte>(buf.Data().Contents, 0, expectedData.Length));
    }

    /// Encode a close message and compare encoded contexts with ETAC.
    ///
    [Fact]
    public void EncodeCloseMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        ICloseMsg msg = (ICloseMsg)new Msg();

        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.CLOSE;
        msg.StreamId = int.MaxValue;
        msg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.NO_DATA;
        msg.ApplyAck();

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encodeIter));

        byte[] expectedData = DATA_003_closeMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData,
            new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /// Encode an update message and compare encoded contexts with ETAC.
    ///
    [Fact]
    public void EncodeUpdateMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IUpdateMsg msg = (IUpdateMsg)new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new(), applicationName = new();
        Buffer position = new();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.UPDATE;
        msg.StreamId = 2146290601; // 0x7FEDCBA9
        msg.DomainType = (int)DomainType.LOGIN;
        msg.ContainerType = DataTypes.FIELD_LIST;
        msg.UpdateType = UpdateEventTypes.QUOTE;
        msg.ApplyDoNotCache();
        msg.ApplyDoNotConflate();
        msg.ApplyDoNotRipple();

        /* set sequence number */
        msg.ApplyHasSeqNum();
        msg.SeqNum = 1234567890L;

        /* conflation Count and Conflation Time */
        msg.ApplyHasConfInfo();
        msg.ConflationCount = 10;
        msg.ConflationTime = 500; // ms

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* set permData */
        msg.ApplyHasPermData();
        byte[] pd = { 0x10, 0x11, 0x12, 0x13 };
        Buffer permData = new();
        permData.Data(ByteBuffer.Wrap(pd));
        msg.PermData = permData;

        /* set post user info */
        msg.ApplyHasPostUserInfo();
        msg.PostUserInfo.UserAddr = 4294967290L; // 0xFFFFFFFA
        msg.PostUserInfo.UserId = 4294967295L; // 0xFFFFFFFF

        /* set msgKey members */
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasServiceId();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
        msg.MsgKey.ServiceId = 32639;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
                     msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter));

        // encode (32) UINT 554433
        Eta.Codec.UInt uInt = new();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Eta.Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, real));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_004_updateMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    [Fact]
    public void EncodeRequestWithPrivateStream_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IRequestMsg msg = (IRequestMsg)new Msg();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.REQUEST;
        msg.StreamId = 100;
        msg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.NO_DATA;
        msg.ApplyPrivateStream();

        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encodeIter));

        byte[] expectedData = new byte[] { 0x00, 0x0C, 0x01, 0x06, 0x00, 0x00, 0x00, 0x64, 0x81, 0x00, 0x00, 0x80, 0x01, 0x00 };
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    [Fact]
    public void EncodeRequestWithIdAndEncAttrib_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IRequestMsg msg = (IRequestMsg)new Msg();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.REQUEST;
        msg.StreamId = 100;
        msg.DomainType = (int)DomainType.MARKET_PRICE;
        msg.ContainerType = DataTypes.NO_DATA;

        /* set msgKey members */
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasIdentifier();
        msg.MsgKey.ApplyHasAttrib();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.Identifier = 0x7fff;

        Buffer encodedAttrib = new();
        encodedAttrib.Data("ENCODED ATTRIB");
        msg.MsgKey.AttribContainerType = DataTypes.OPAQUE;
        msg.MsgKey.EncodedAttrib = encodedAttrib;

        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, msg.Encode(encodeIter));

        // prepare ByteBuffer for printing.
        buf.Data(buf.Data(), 0, buf.Data().Position);

        byte[] expectedData = DATA_001_requestMsgWithIdAndEncAttrib;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /*
     * Test RsslMsgKey.Copy() and RsslMsgKey.copyReferences().
     */
    [Fact]
    public void MsgKeyImplCopyReferences_Test()
    {
        MsgKey msgKey = new();
        MsgKey msgKeyCopy = new();

        // set msgKey members
        msgKey.ApplyHasNameType();
        msgKey.ApplyHasName();
        msgKey.ApplyHasIdentifier();
        msgKey.ApplyHasAttrib();

        Buffer name = new();
        name.Data("TRI.N");
        msgKey.Name = name;
        msgKey.NameType = InstrumentNameTypes.RIC;
        msgKey.Identifier = 0x7fff;

        Buffer encodedAttrib = new();
        encodedAttrib.Data("ENCODED ATTRIB");
        msgKey.AttribContainerType = DataTypes.OPAQUE;
        msgKey.EncodedAttrib = encodedAttrib;

        // perform the copyReferences
        msgKeyCopy.CopyReferences(msgKey);

        // verify
        Assert.Equal(msgKey.Flags, msgKeyCopy.Flags);
        Assert.True(msgKeyCopy.CheckHasName());
        Assert.True(msgKeyCopy.CheckHasNameType());
        Assert.True(msgKeyCopy.CheckHasIdentifier());
        Assert.True(msgKeyCopy.CheckHasAttrib());
        Assert.Equal(DataTypes.OPAQUE, msgKeyCopy.AttribContainerType);

        // name references should match.
        Assert.Equal(msgKey.Name.Data(), msgKeyCopy.Name.Data());
        Assert.Equal(msgKey.NameType, msgKeyCopy.NameType);
        Assert.Equal(msgKey.Identifier, msgKeyCopy.Identifier);
        Assert.Equal(msgKey.AttribContainerType, msgKeyCopy.AttribContainerType);
        // encodedAttrib references should match.
        Assert.Equal(msgKey.EncodedAttrib.Data(), msgKeyCopy.EncodedAttrib.Data());
    }

    /*
     * Test RsslMsgImpl.MsgKey (which tests RsslMsgKey.copyReferences()).
     */
    [Fact]
    public void MsgMsgKey_Test()
    {
        IRefreshMsg msg = (IRefreshMsg)new Msg();
        msg.MsgClass = MsgClasses.REFRESH;
        msg.ApplyHasMsgKey();

        MsgKey msgKey = msg.MsgKey;
        MsgKey msgKeyFromRsslMsg;

        // set msgKey members
        msgKey.ApplyHasNameType();
        msgKey.ApplyHasName();
        msgKey.ApplyHasIdentifier();
        msgKey.ApplyHasAttrib();

        Buffer name = new();
        name.Data("TRI.N");
        msgKey.Name = name;
        msgKey.NameType = InstrumentNameTypes.RIC;
        msgKey.Identifier = 0x7fff;

        Buffer encodedAttrib = new();
        encodedAttrib.Data("ENCODED ATTRIB");
        msgKey.AttribContainerType = DataTypes.OPAQUE;
        msgKey.EncodedAttrib = encodedAttrib;

        // get the msgKey from the RsslMsg.
        msgKeyFromRsslMsg = msg.MsgKey;

        // verify that the msgKey from the RsslMsg has the references to our msgKey.
        Assert.Equal(msgKey.Flags, msgKeyFromRsslMsg.Flags);
        Assert.True(msgKeyFromRsslMsg.CheckHasName());
        Assert.True(msgKeyFromRsslMsg.CheckHasNameType());
        Assert.True(msgKeyFromRsslMsg.CheckHasIdentifier());
        Assert.True(msgKeyFromRsslMsg.CheckHasAttrib());
        Assert.Equal(DataTypes.OPAQUE, msgKeyFromRsslMsg.AttribContainerType);

        // name references should match.
        Assert.Equal(msgKey.Name.Data(), msgKeyFromRsslMsg.Name.Data());
        Assert.Equal(msgKey.NameType, msgKeyFromRsslMsg.NameType);
        Assert.Equal(msgKey.Identifier, msgKeyFromRsslMsg.Identifier);
        Assert.Equal(msgKey.AttribContainerType, msgKeyFromRsslMsg.AttribContainerType);
        // encodedAttrib references should match.
        Assert.Equal(msgKey.EncodedAttrib.Data(), msgKeyFromRsslMsg.EncodedAttrib.Data());
    }

    [Fact]
    public void tmp_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Map map = new();
        MapEntry mapEntry = new();
        EncodeIterator iter = new();
        Eta.Codec.Int intKey = new();

        map.ApplyHasKeyFieldId();
        map.ApplyHasPerEntryPermData();
        map.ApplyHasTotalCountHint();
        map.ApplyHasSummaryData();
        map.ApplyHasSetDefs();
        map.TotalCountHint = 5;
        Buffer txt = new();
        txt.Data("encodeMapEntryTest");
        map.EncodedEntries.Data(txt.Data());
        map.EncodedSetDefs = txt;
        map.EncodedSummaryData = txt;
        map.ContainerType = DataTypes.FIELD_LIST;
        map.KeyPrimitiveType = DataTypes.INT;
        map.KeyFieldId = 22;
        mapEntry.Action = MapEntryActions.ADD;
        mapEntry.EncodedData.Data(new ByteBuffer(0));
        intKey.Value(33);

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Positive test for everything good without permData
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, mapEntry.Encode(iter, intKey));
        Assert.Equal(CodecReturnCode.SUCCESS, map.EncodeComplete(iter, true));
    }

    /// This tests the encodeSeriesInit() method. It contains five test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Negative test for unsupported container type
    /// 3. Positive test for everything good without summaryData but flag set
    /// 4. Positive test for everything good without setDefs but flag set
    /// 5. Positive test for everything good
    ///
    [Fact]
    public void EncodeSeriesInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(1));
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Series series = new();
        EncodeIterator iter = new();
        Buffer txt = new();

        series.ApplyHasTotalCountHint();
        series.TotalCountHint = 5;
        txt.Data("encodeSeriesInitTest");
        series.EncodedEntries.Data(txt.Data());
        series.ContainerType = DataTypes.FIELD_LIST;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, series.EncodeInit(iter, 0, 0));

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Negative test for unsupported container type
        series.ContainerType = DataTypes.ARRAY;
        Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, series.EncodeInit(iter, 0, 0));

        // reset container type
        series.ContainerType = DataTypes.FIELD_LIST;

        // 3. Positive test for everything good without summaryData but flag set
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        series.ApplyHasSummaryData();
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));

        Assert.Equal(0x06, buf.Data().Contents[0]);
        Assert.Equal(0x04, buf.Data().Contents[1]);
        Assert.Equal(0x00, buf.Data().Contents[2]);
        Assert.Equal(0x00, buf.Data().Contents[3]);
        Assert.Equal(4, buf.Data().Position);

        // 4. Positive test for everything good without setDefs but flag set
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        series.ApplyHasSetDefs();
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));

        Assert.Equal(0x07, buf.Data().Contents[0]);
        Assert.Equal(0x04, buf.Data().Contents[1]);
        Assert.Equal(0x00, buf.Data().Contents[2]);
        Assert.Equal(0x00, buf.Data().Contents[3]);
        Assert.Equal(4, buf.Data().Position);

        // 5. Positive test for everything good
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        series.EncodedSetDefs = txt;
        series.EncodedSummaryData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));

        byte[] expectedData =
            { 0x07, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x49,
              0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65,
              0x72, 0x69, 0x65, 0x73, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        byte[] bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(47, buf.Data().Position);
    }

    /// This tests the encodeSeriesComplete() method. It contains two test cases.
    ///
    /// 1. Positive test for everything good with success flag true
    /// 2. Positive test for everything good with success flag false
    ///
    [Fact]
    public void EncodeSeriesComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Series series = new();
        EncodeIterator iter = new();
        byte[] bufBytes;
        Buffer txt = new();
        txt.Data("encodeSeriesCompleteTest");

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        series.ApplyHasTotalCountHint();
        series.ApplyHasSummaryData();
        series.ApplyHasSetDefs();
        series.TotalCountHint = 5;
        series.EncodedEntries.Data(txt.Data());
        series.EncodedSetDefs = txt;
        series.EncodedSummaryData = txt;
        series.ContainerType = DataTypes.FIELD_LIST;

        // 1. Positive test for everything good with success flag true
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeComplete(iter, true));

        byte[] expectedData =
            { 0x07, 0x04, 0x18, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x43,
              0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x18, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65,
              0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(55, buf.Data().Position);

        // 2. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeComplete(iter, false));
        Assert.Equal(0, buf.Data().Position);
    }

    /// This tests the encodeSeriesEntryInit() method. It contains three test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good without pre-encoded data
    /// 3. Positive test for everything good with pre-encoded data
    ///
    [Fact]
    public void EncodeSeriesEntryInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(60));
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Series series = new();
        SeriesEntry seriesEntry = new();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeSeriesEntryInitTest");

        series.ApplyHasTotalCountHint();
        series.ApplyHasSummaryData();
        series.ApplyHasSetDefs();
        /* Use a larger totalCountHint. The seriesEntry.EncodeInit will write a 3-byte length,
         * but series.EncodeInit's overrun check for totalCount assumes the worst-case (4 bytes),
         * so seriesEntry.EncodeInit's fail case can only be checked if at least two bytes were
         * used to write totalCountHint. */
        series.TotalCountHint = 255;
        series.EncodedEntries.Data(txt.Data());
        series.EncodedSetDefs = txt;
        series.EncodedSummaryData = txt;
        series.ContainerType = DataTypes.FIELD_LIST;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, seriesEntry.EncodeInit(iter, 0));

        // 2. Positive test for everything good without pre-encoded data
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(iter, 0));

        byte[] expectedData =
            { 0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x80, 0xFF };

        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, expectedData.Length));
        Assert.Equal(61, buf.Data().Position);

        // 3. Positive test for everything good with pre-encoded data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        seriesEntry.EncodedData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(iter, 0));

        byte[] expectedData2 =
            { 0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x80, 0xFF };

        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, expectedData2.Length));
        Assert.Equal(61, buf.Data().Position);
    }

    /// This tests the encodeSeriesEntryComplete() method. It contains four test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Negative test for invalid data
    /// 3. Positive test for everything good with success flag true
    /// 4. Positive test for everything good with success flag false
    ///
    [Fact]
    public void EncodeSeriesEntryComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Series series = new();
        SeriesEntry seriesEntry = new();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeSeriesEntryComplete");

        series.ApplyHasTotalCountHint();
        series.ApplyHasSummaryData();
        series.ApplyHasSetDefs();
        series.TotalCountHint = 5;
        series.EncodedEntries.Data(txt.Data());
        series.EncodedSetDefs = txt;
        series.EncodedSummaryData = txt;
        series.ContainerType = DataTypes.FIELD_LIST;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(iter, 0));
        iter._curBufPos = 100000;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, seriesEntry.EncodeComplete(iter, true));

        // 2. Negative test for invalid data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(iter, 0));
        iter._curBufPos = 100000;
        iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 0;
        Assert.Equal(CodecReturnCode.INVALID_DATA, seriesEntry.EncodeComplete(iter, true));

        // 3. Positive test for everything good with success flag true
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeComplete(iter, true));

        byte[] expectedData =
                { 0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
                  0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x19, 0x65, 0x6e, 0x63,
                  0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
                  0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x05, 0, 0, 0xfe };

        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, expectedData.Length));
        Assert.Equal(60, buf.Data().Position);

        // 4. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.EncodeComplete(iter, false));

        byte[] expectedData2 =
            { 0x07, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f,
              0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x05, 0x00, 0x00 };

        // compare only first 57 bytes here even though more was encoded
        // success = false moves position back to 54
        Assert.Equal(57, buf.Data().Position);
        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, 57));
    }

    /// This tests the encodeSeriesEntry() method. It contains five test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good without pre-encoded data
    /// 3. Positive test for everything good with pre-encoded data
    /// 4. Positive test for everything good with big pre-encoded data
    /// 5. Negative test for everything good with too big pre-encoded data
    ///
    [Fact]
    public void EncodeSeriesEntry_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(52));
        Buffer buf = new();
        buf.Data(new ByteBuffer(80));
        Buffer bigBuf = new();
        Buffer bigBuf2 = new();
        bigBuf.Data(new ByteBuffer(65700));
        bigBuf2.Data(new ByteBuffer(65700));
        Series series = new();
        SeriesEntry seriesEntry = new();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeSeriesEntryTest");

        series.ApplyHasTotalCountHint();
        series.ApplyHasSummaryData();
        series.ApplyHasSetDefs();

        /* Use a larger totalCountHint. The seriesEntry.Encode will write a 1-byte length
         * (payload is empty), but series.EncodeInit's overrun check for totalCount assumes the
         * worst-case (4 bytes), so seriesEntry.EncodeInit's fail case can only be checked if
         * four bytes were used to write totalCountHint. */
        series.TotalCountHint = 555555555;
        series.EncodedEntries.Data(txt.Data());
        series.EncodedSetDefs = txt;
        series.EncodedSummaryData = txt;
        series.ContainerType = DataTypes.FIELD_LIST;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, seriesEntry.Encode(iter));

        // need to copy into new buffer
        CodecReturnCode ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(smallBuf), Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion()) == CodecReturnCode.SUCCESS)
        {
            ret = seriesEntry.Encode(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 2. Positive test for everything good without pre-encoded data
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.Encode(iter));

        byte[] expectedData =
            { 0x07, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
              0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
              0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0xE1, 0x1D,
              0x1A, 0xE3 };

        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, expectedData.Length));
        Assert.Equal(53, buf.Data().Position);

        // 3. Positive test for everything good with pre-encoded data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        seriesEntry.EncodedData.Data(txt.Data());
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.Encode(iter));

        byte[] expectedData2 =
            { 0x07, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65, 0x73, 0x45,
              0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53,
              0x65, 0x72, 0x69, 0x65, 0x73, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0xE1, 0x1D,
              0x1A, 0xE3, 0x00, 0x00, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x53, 0x65, 0x72, 0x69, 0x65,
              0x73, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74 };

        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, expectedData2.Length));
        Assert.Equal(74, buf.Data().Position);

        // 4. Positive test for everything good with big pre-encoded data
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        ByteBuffer bb = bigBuf2.Data();
        for (int i = 0; i < 65535; i++)
        {
            bb.Write((byte)i);
        }
        seriesEntry.Clear();
        seriesEntry.EncodedData = bigBuf2;
        Assert.Equal(CodecReturnCode.SUCCESS, seriesEntry.Encode(iter));

        // 5. Negative test for everything good with too big pre-encoded data
        bigBuf.Data().Rewind();
        bigBuf2.Data().Rewind();
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, series.EncodeInit(iter, 0, 0));
        bb = bigBuf2.Data();
        for (int i = 0; i < 65536; i++)
        {
            bb.Write((byte)i);
        }
        seriesEntry.Clear();
        seriesEntry.EncodedData = bigBuf2;
        Assert.Equal(CodecReturnCode.INVALID_DATA, seriesEntry.Encode(iter));
    }

    /// This tests the encodeVectorInit() method. It contains eight test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Negative test for unsupported container type
    /// 3. Positive test for everything good without summaryData but flag set
    /// 4. Positive test for everything good without setDefs but flag set
    /// 5. Positive test for everything good
    /// 6. Positive test for everything good and summary data complete with success true
    /// 7. Positive test for everything good and summary data complete with success false
    /// 8. Negative test for buffer that is big enough for vector header but not summary data
    ///
    [Fact]
    public void EncodeVectorInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(1));
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Buffer summaryDataSmallBuf = new();
        summaryDataSmallBuf.Data(new ByteBuffer(26));
        Vector vector = new Eta.Codec.Vector();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeVectorInitTest");

        vector.ApplyHasPerEntryPermData();
        vector.ApplyHasTotalCountHint();
        vector.ApplySupportsSorting();
        vector.TotalCountHint = 5;
        vector.EncodedEntries.Data(txt.Data());
        vector.ContainerType = DataTypes.FIELD_LIST;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, vector.EncodeInit(iter, 0, 0));

        // reset to bigger buffer
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        // 2. Negative test for unsupported container type
        // vector.ContainerType = DataTypes.ARRAY;
        // Assert.Equal(CodecReturnCode.UNSUPPORTED_DATA_TYPE, vector.EncodeInit(iter, 0, 0));

        // reset container type
        vector.ContainerType = DataTypes.FIELD_LIST;

        // 3. Positive test for everything good without summaryData but flag set
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vector.ApplyHasSummaryData();
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));

        Assert.Equal(0x1a, buf.Data().Contents[0]);
        Assert.Equal(0x04, buf.Data().Contents[1]);
        Assert.Equal(0x00, buf.Data().Contents[2]);
        Assert.Equal(0x00, buf.Data().Contents[3]);
        Assert.Equal(4, buf.Data().Position);

        // 4. Positive test for everything good without setDefs but flag set
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vector.ApplyHasSetDefs();
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));

        Assert.Equal(0x1b, buf.Data().Contents[0]);
        Assert.Equal(0x04, buf.Data().Contents[1]);
        Assert.Equal(0x00, buf.Data().Contents[2]);
        Assert.Equal(0x00, buf.Data().Contents[3]);
        Assert.Equal(4, buf.Data().Position);

        // 5. Positive test for everything good
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vector.EncodedSetDefs = txt;
        vector.EncodedSummaryData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));

        byte[] expectedData =
            { 0x1b, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x49,
              0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65,
              0x63, 0x74, 0x6f, 0x72, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 5, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        byte[] bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(47, buf.Data().Position);

        // 6. Positive test for everything good and summary data complete with success true
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 64; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vector.EncodedSummaryData.Clear();
        vector.ApplyHasSummaryData();
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeSummaryDataComplete(iter, true));

        byte[] expectedData2 =
            { 0x1b, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x49,
              0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x80, 0, 5, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        Assert.Equal(28, buf.Data().Position);
        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, expectedData2.Length));

        // 7. Positive test for everything good and summary data complete with success false
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 64; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeSummaryDataComplete(iter, false));

        byte[] expectedData3 =
            { 0x1b, 0x04, 0x14, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x49,
              0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        // Should be at start of summary data encoding.
        Assert.Equal(25, buf.Data().Position);
        Assert_ArrayEqual(expectedData3, new System.Span<byte>(buf.Data().Contents, 0, expectedData3.Length));

        // 8. Negative test for buffer that is big enough for vector header but not summary data
        iter.SetBufferAndRWFVersion(summaryDataSmallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, vector.EncodeInit(iter, 0, 0));
    }

    /// This tests the encodeVectorComplete() method. It contains two test cases.
    ///
    /// 1. Positive test for everything good with success flag true
    /// 2. Positive test for everything good with success flag false
    ///
    [Fact]
    public void EncodeVectorComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(64));
        Vector vector = new Eta.Codec.Vector();
        EncodeIterator iter = new();
        byte[] bufBytes;
        Buffer txt = new();
        txt.Data("encodeVectorCompleteTest");

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        vector.ApplyHasPerEntryPermData();
        vector.ApplyHasTotalCountHint();
        vector.ApplySupportsSorting();
        vector.ApplyHasSummaryData();
        vector.ApplyHasSetDefs();
        vector.TotalCountHint = 5;
        vector.EncodedEntries.Data(txt.Data());
        vector.EncodedSetDefs = txt;
        vector.EncodedSummaryData = txt;
        vector.ContainerType = DataTypes.FIELD_LIST;

        // 1. Positive test for everything good with success flag true
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeComplete(iter, true));

        byte[] expectedData =
            { 0x1b, 0x04, 0x18, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x43,
              0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x18, 0x65, 0x6e, 0x63, 0x6f,
              0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65,
              0x54, 0x65, 0x73, 0x74, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(55, buf.Data().Position);

        // 2. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeComplete(iter, false));
        Assert.Equal(0, buf.Data().Position);
    }

    /// This tests the encodeVectorEntryInit() method. It contains five test cases.
    ///
    /// 1. Positive test for everything good without permData but flag set
    /// 2. Negative test for buffer too small
    /// 3. Positive test for everything good with DELETE entry
    /// 4. Positive test for everything good without pre-encoded data
    /// 5. Positive test for everything good with pre-encoded data
    ///
    [Fact]
    public void EncodeVectorEntryInit_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(64));
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Eta.Codec.Vector vector = new();
        Eta.Codec.VectorEntry vectorEntry = new();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeVectorEntryInitTest");

        vector.ApplySupportsSorting();
        vector.ApplyHasPerEntryPermData();
        vector.ApplyHasTotalCountHint();
        vector.ApplyHasSummaryData();
        vector.ApplyHasSetDefs();
        vector.TotalCountHint = 5;
        vector.EncodedEntries.Data(txt.Data());
        vector.EncodedSetDefs = txt;
        vector.EncodedSummaryData = txt;
        vector.ContainerType = DataTypes.FIELD_LIST;
        vectorEntry.ApplyHasPermData();
        vectorEntry.Action = VectorEntryActions.SET;
        vectorEntry.Index = 11;

        // 1. Positive test for everything good without permData but flag set
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));

        byte[] expectedData =
            { 0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00, 0x00, 0x12, 0x0b, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        byte[] bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(63, buf.Data().Position);

        // 2. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vectorEntry.PermData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, vectorEntry.EncodeInit(iter, 0));

        // 3. Positive test for everything good with DELETE entry
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 128; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vectorEntry.Action = VectorEntryActions.DELETE;
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));

        byte[] expectedData2 =
            { 0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0x15, 0x0b, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64,
              0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74 };

        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
        Assert.Equal(85, buf.Data().Position);

        // 4. Positive test for everything good without pre-encoded data
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 128; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vectorEntry.Action = VectorEntryActions.SET;
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));

        byte[] expectedData3 =
            { 0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0x12, 0x0b, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64,
              0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00 };

        Assert_ArrayEqual(expectedData3, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
        Assert.Equal(88, buf.Data().Position);

        // 5. Positive test for everything good with pre-encoded data
        buf.Data().Rewind();
        // clear buffer
        for (int i = 0; i < 128; i++)
        {
            buf.Data().Write((byte)0);
        }
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        vectorEntry.EncodedData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));

        byte[] expectedData4 =
            { 0x1b, 0x04, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x19, 0x65, 0x6e, 0x63,
              0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e,
              0x69, 0x74, 0x54, 0x65, 0x73, 0x74, 0x05, 0, 0, 0x12, 0x0b, 0x19, 0x65, 0x6e, 0x63, 0x6f, 0x64,
              0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74, 0x72, 0x79, 0x49, 0x6e, 0x69, 0x74,
              0x54, 0x65, 0x73, 0x74, 0, 0, 0 };

        Assert_ArrayEqual(expectedData4, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
        Assert.Equal(88, buf.Data().Position);
    }

    /// This tests the encodeVectorEntryComplete() method. It contains five test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Negative test for invalid data
    /// 3. Positive test for everything good with success flag true
    /// 4. Positive test for everything good with success flag false
    /// 5. Positive test for everything good with DELETE entry
    ///
    [Fact]
    public void EncodeVectorEntryComplete_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(128));
        Buffer buf2 = new();
        buf2.Data(new ByteBuffer(128));
        Vector vector = new();
        VectorEntry vectorEntry = new();
        EncodeIterator iter = new();
        byte[] bufBytes;
        Buffer txt = new();
        txt.Data("encodeVectorEntryCompleteTest");

        vector.ApplySupportsSorting();
        vector.ApplyHasPerEntryPermData();
        vector.ApplyHasTotalCountHint();
        vector.ApplyHasSummaryData();
        vector.ApplyHasSetDefs();
        vector.TotalCountHint = 5;
        vector.EncodedEntries.Data(txt.Data());
        vector.EncodedSetDefs = txt;
        vector.EncodedSummaryData = txt;
        vector.ContainerType = DataTypes.FIELD_LIST;
        vectorEntry.ApplyHasPermData();
        vectorEntry.PermData = txt;
        vectorEntry.Action = VectorEntryActions.SET;
        vectorEntry.Index = 11;
        vectorEntry.EncodedData = txt;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));
        iter._curBufPos = 100000;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, vectorEntry.EncodeComplete(iter, true));

        // 2. Negative test for invalid data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));
        iter._curBufPos = 100000;
        iter._levelInfo[iter._encodingLevel]._internalMark._sizeBytes = 0;
        Assert.Equal(CodecReturnCode.INVALID_DATA, vectorEntry.EncodeComplete(iter, true));

        // 3. Positive test for everything good with success flag true
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeComplete(iter, true));

        byte[] expectedData =
            { 0x1b, 0x04, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74,
              0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74,
              0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
              0x00, 0x12, 0x0b, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72,
              0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73,
              0x74, 0xfe, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(100, buf.Data().Position);

        // 4. Positive test for everything good with success flag false
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeComplete(iter, false));

        byte[] expectedData2 =
            { 0x1b, 0x04, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74,
              0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74,
              0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        // compare only first 65 bytes here even though more was encoded
        // success = false moves position back to 65
        Assert.Equal(65, buf.Data().Position);
        Assert_ArrayEqual(new System.Span<byte>(expectedData2, 0, 65),
            new System.Span<byte>(buf.Data().Contents, 0, 65));

        // 5. Positive test for everything good with DELETE entry
        iter.SetBufferAndRWFVersion(buf2, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        vectorEntry.Action = VectorEntryActions.DELETE;
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeInit(iter, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.EncodeComplete(iter, true));

        byte[] expectedData3 =
            { 0x1b, 0x04, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45,
              0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74,
              0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x45, 0x6e, 0x74,
              0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
              0x00, 0x15, 0x0b, 0x1d, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6f, 0x72,
              0x45, 0x6e, 0x74, 0x72, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x6c, 0x65, 0x74, 0x65, 0x54, 0x65, 0x73,
              0x74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        bufBytes = new byte[expectedData3.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf2.Copy(bufBytes));

        Assert_ArrayEqual(expectedData3, bufBytes);
        Assert.Equal(97, buf2.Data().Position);
    }

    /// This tests the encodeVectorEntryTest() method. It contains five test cases.
    ///
    /// 1. Negative test for buffer too small
    /// 2. Positive test for everything good without pre-encoded data
    /// 3. Positive test for everything good with pre-encoded data
    /// 4. Positive test for everything good with big pre-encoded data
    /// 5. Negative test for everything good with too big pre-encoded data
    ///
    [Fact]
    public void EncodeVectorEntry_Test()
    {
        Buffer smallBuf = new();
        smallBuf.Data(new ByteBuffer(64));
        Buffer buf = new();
        buf.Data(new ByteBuffer(100));
        Buffer bigBuf = new();
        Buffer bigBuf2 = new();
        bigBuf.Data(new ByteBuffer(65700));
        bigBuf2.Data(new ByteBuffer(65700));
        Vector vector = new();
        VectorEntry vectorEntry = new();
        EncodeIterator iter = new();
        Buffer txt = new();
        txt.Data("encodeVectorEntryTest");

        vector.ApplySupportsSorting();
        vector.ApplyHasPerEntryPermData();
        vector.ApplyHasTotalCountHint();
        vector.ApplyHasSummaryData();
        vector.ApplyHasSetDefs();
        vector.TotalCountHint = 5;
        vector.EncodedEntries.Data(txt.Data());
        vector.EncodedSetDefs = txt;
        vector.EncodedSummaryData = txt;
        vector.ContainerType = DataTypes.FIELD_LIST;
        vectorEntry.ApplyHasPermData();
        vectorEntry.Action = VectorEntryActions.SET;
        vectorEntry.Index = 11;
        vectorEntry.PermData = txt;

        // 1. Negative test for buffer too small
        iter.SetBufferAndRWFVersion(smallBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, vectorEntry.Encode(iter));

        // 2. Try to grow the buffer to hit the edge cases
        CodecReturnCode ret = CodecReturnCode.BUFFER_TOO_SMALL;
        while ((ret == CodecReturnCode.BUFFER_TOO_SMALL) &&
                iter.SetBufferAndRWFVersion(GrowByOneAndCopy(smallBuf),
                                            Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion())
                == CodecReturnCode.SUCCESS)
        {
            ret = vectorEntry.Encode(iter);
        }
        Assert.Equal(CodecReturnCode.SUCCESS, ret);

        // 3. Positive test for everything good without pre-encoded data
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.Encode(iter));

        byte[] expectedData =
            { 0x1B, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45,
              0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56,
              0x65, 0x63, 0x74, 0x6F, 0x72, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
              0x00, 0x12, 0x0B, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72,
              0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        byte[] bufBytes = new byte[expectedData.Length];
        Assert.Equal(CodecReturnCode.SUCCESS, buf.Copy(bufBytes));

        Assert_ArrayEqual(expectedData, bufBytes);
        Assert.Equal(74, buf.Data().Position);

        // 3. Positive test for everything good with pre-encoded data
        buf.Data().Rewind();
        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        vectorEntry.EncodedData = txt;
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.Encode(iter));

        byte[] expectedData2 =
            { 0x1B, 0x04, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45,
              0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56,
              0x65, 0x63, 0x74, 0x6F, 0x72, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x05, 0x00,
              0x00, 0x12, 0x0B, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x56, 0x65, 0x63, 0x74, 0x6F, 0x72,
              0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x15, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65,
              0x56, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x54, 0x65, 0x73, 0x74, 0x00,
              0x00, 0x00, 0x00, 0x00 };

        Assert_ArrayEqual(expectedData2, new System.Span<byte>(buf.Data().Contents, 0, expectedData2.Length));
        Assert.Equal(95, buf.Data().Position);

        // 4. Positive test for everything good with big pre-encoded data
        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));
        ByteBuffer bb = bigBuf2.Data();
        for (int i = 0; i < 65535; i++)
        {
            bb.Write((byte)i);
        }
        vectorEntry.Clear();
        vectorEntry.EncodedData = bigBuf2;
        Assert.Equal(CodecReturnCode.SUCCESS, vectorEntry.Encode(iter));

        // 5. Negative test for everything good with too big pre-encoded data
        bigBuf.Data().Rewind();
        bigBuf2.Data().Rewind();

        iter.SetBufferAndRWFVersion(bigBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        Assert.Equal(CodecReturnCode.SUCCESS, vector.EncodeInit(iter, 0, 0));

        bb = bigBuf2.Data();
        for (int i = 0; i < 65536; i++)
        {
            bb.Write((byte)i);
        }

        vectorEntry.Clear();
        vectorEntry.EncodedData = bigBuf2;
        Assert.Equal(CodecReturnCode.INVALID_DATA, vectorEntry.Encode(iter));
    }

    /// Encode a generic message and compare encoded contents with ETAC.
    ///
    [Fact]
    public void EncodeGenericMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IGenericMsg msg = (IGenericMsg)new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new(), applicationName = new();
        Buffer position = new();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.GENERIC;
        msg.StreamId = 2146290601; // 0x7FEDCBA9
        msg.DomainType = (int)DomainType.LOGIN;
        msg.ContainerType = DataTypes.FIELD_LIST;
        msg.ApplyMessageComplete();

        /* set sequence number */
        msg.ApplyHasSeqNum();
        msg.SeqNum = 1234567890L;

        /* set secondary sequence number */
        msg.ApplyHasSecondarySeqNum();
        msg.SecondarySeqNum = 1122334455L;

        /* set part number */
        msg.ApplyHasPartNum();
        msg.PartNum = 12345;

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* set permData */
        msg.ApplyHasPermData();
        byte[] pd = { 0x10, 0x11, 0x12, 0x13 };
        Buffer permData = new();
        permData.Data(ByteBuffer.Wrap(pd));
        msg.PermData = permData;

        /* set msgKey members */
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasServiceId();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
        msg.MsgKey.ServiceId = 32639;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
                     msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter));

        // encode (32) UINT 554433
        Eta.Codec.UInt uInt = new();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Eta.Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, real));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_018_genericMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /// Encode an ack message and compare encoded contents with ETAC.
    ///
    [Fact]
    public void EncodeAckMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IAckMsg msg = (IAckMsg)new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new(), applicationName = new();
        Buffer position = new();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.ACK;
        msg.StreamId = 2146290601; // 0x7FEDCBA9
        msg.DomainType = (int)DomainType.LOGIN;
        msg.ContainerType = DataTypes.FIELD_LIST;
        msg.AckId = 12345;

        /* set sequence number */
        msg.ApplyHasSeqNum();
        msg.SeqNum = 1234567890L;

        /* set nak code */
        msg.ApplyHasNakCode();
        msg.NakCode = NakCodes.NOT_OPEN;

        /* set private stream flag */
        msg.ApplyPrivateStream();

        /* set ack text */
        msg.ApplyHasText();
        Buffer ackText = new();
        ackText.Data("ACK TEXT");
        msg.Text = ackText;

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* set msgKey members */
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasServiceId();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
        msg.MsgKey.ServiceId = 32639;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
                     msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter));

        // encode (32) UINT 554433
        Eta.Codec.UInt uInt = new();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Eta.Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, real));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_022_ackMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData,
            new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    /// Encode a post message and compare encoded contexts with ETAC.
    ///
    [Fact]
    public void EncodePostMsg_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(1024));
        IPostMsg msg = (IPostMsg)new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new(), applicationName = new();
        Buffer position = new();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.POST;
        msg.StreamId = 2146290601; // 0x7FEDCBA9
        msg.DomainType = (int)DomainType.LOGIN;
        msg.ContainerType = DataTypes.FIELD_LIST;
        msg.ApplyAck();
        msg.ApplyPostComplete();

        // set sequence number
        msg.ApplyHasSeqNum();
        msg.SeqNum = 1234567890L;

        // set post id
        msg.ApplyHasPostId();
        msg.PostId = 12345;

        // set part number
        msg.ApplyHasPartNum();
        msg.PartNum = 23456;

        // set post user rights
        msg.ApplyHasPostUserRights();
        msg.PostUserRights = PostUserRights.MODIFY_PERM;

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* set permData */
        msg.ApplyHasPermData();
        byte[] pd = { 0x10, 0x11, 0x12, 0x13 };
        Buffer permData = new();
        permData.Data(ByteBuffer.Wrap(pd));
        msg.PermData = permData;

        /* set post user info */
        msg.PostUserInfo.UserAddr = 4294967290L; // 0xFFFFFFFA
        msg.PostUserInfo.UserId = 4294967295L; // 0xFFFFFFFF

        /* set msgKey members */
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasServiceId();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
        msg.MsgKey.ServiceId = 32639;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
        /*
         * since our msgKey has opaque that we want to encode, we need to useEncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
                     msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, (Real)null));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter));

        // encode (32) UINT 554433
        Eta.Codec.UInt uInt = new();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Eta.Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, real));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_023_postMsg;
        Assert.NotNull(expectedData);
        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf.Data().Contents, 0, buf.Length));
    }

    [Fact]
    public void EncodeInNon0PosBuffer_Test()
    {
        Buffer buf = new();
        buf.Data(new ByteBuffer(35), 3, 32);
        FilterList filterList = new();
        EncodeIterator iter = new();

        filterList.ApplyHasPerEntryPermData();
        filterList.ApplyHasTotalCountHint();
        filterList.TotalCountHint = 5;
        Buffer txt = new();
        txt.Data("encodeFilterListInitTest");
        filterList.EncodedEntries.Data(txt.Data());
        filterList.ContainerType = DataTypes.FIELD_LIST;

        iter.SetBufferAndRWFVersion(buf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        filterList.ContainerType = DataTypes.FIELD_LIST;

        // 3. Positive test for everything good
        Assert.Equal(CodecReturnCode.SUCCESS, filterList.EncodeInit(iter));
    }

    /// <summary>
    /// Change buffer during encoding a generic message and compare encoded contents with ETAC.
    /// </summary>
    [Fact]
    public void RealignBuffer_Test()
    {
        Buffer buf1 = new();
        buf1.Data(new ByteBuffer(10));
        IGenericMsg msg = new Msg();
        ElementEntry element = new();
        ElementList elementList = new();
        Buffer applicationId = new();
        Buffer applicationName = new();
        Buffer position = new();
        EncodeIterator encodeIter = new();

        /* clear encode iterator */
        encodeIter.Clear();

        /* set-up message */
        msg.MsgClass = MsgClasses.GENERIC;
        msg.StreamId = 2146290601; // 0x7FEDCBA9
        msg.DomainType = (int)DomainType.LOGIN;
        msg.ContainerType = DataTypes.FIELD_LIST;
        msg.ApplyMessageComplete();

        /* set sequence number */
        msg.ApplyHasSeqNum();
        msg.SeqNum = 1234567890L;

        /* set secondary sequence number */
        msg.ApplyHasSecondarySeqNum();
        msg.SecondarySeqNum = 1122334455L;

        /* set part number */
        msg.ApplyHasPartNum();
        msg.PartNum = 12345;

        /* extended header */
        msg.ApplyHasExtendedHdr();
        Buffer extendedHeader = new();
        extendedHeader.Data("EXTENDED HEADER");
        msg.ExtendedHeader = extendedHeader;

        /* set permData */
        msg.ApplyHasPermData();
        byte[] pd = { 0x10, 0x11, 0x12, 0x13 };
        Buffer permData = new();
        permData.Data(ByteBuffer.Wrap(pd));
        msg.PermData = permData;

        /* set msgKey members */
        msg.ApplyHasMsgKey();
        msg.MsgKey.ApplyHasAttrib();
        msg.MsgKey.ApplyHasNameType();
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasServiceId();

        msg.MsgKey.Name.Data("TRI.N");
        msg.MsgKey.NameType = InstrumentNameTypes.RIC;
        msg.MsgKey.AttribContainerType = DataTypes.ELEMENT_LIST;
        msg.MsgKey.ServiceId = 32639;

        /* encode message */
        encodeIter.SetBufferAndRWFVersion(buf1, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

        /*
         * since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit
         * EncodeMsgInit should return and inform us to encode our key opaque
         */
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, msg.EncodeInit(encodeIter, 0));
        Buffer buf2 = new();
        buf2.Data(new ByteBuffer(100));

        encodeIter.RealignBuffer(buf2);
        Assert.Equal(CodecReturnCode.ENCODE_MSG_KEY_ATTRIB, msg.EncodeInit(encodeIter, 0));

        /* encode our msgKey opaque */
        /* encode the element list */
        elementList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeInit(encodeIter, null, 3));

        /* ApplicationId */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPID;
        applicationId.Data("256");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationId));

        /* ApplicationName */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.APPNAME;
        applicationName.Data("rsslConsumer");
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, applicationName));

        /* Position */
        element.DataType = DataTypes.ASCII_STRING;
        element.Name = ElementNames.POSITION;
        position.Data("localhost");
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, element.Encode(encodeIter, position));
        Buffer buf3 = new();
        buf3.Data(new ByteBuffer(150));
        encodeIter.RealignBuffer(buf3);
        Assert.Equal(CodecReturnCode.SUCCESS, element.Encode(encodeIter, position));

        /* complete encode element list */
        Assert.Equal(CodecReturnCode.SUCCESS, elementList.EncodeComplete(encodeIter, true));

        /* complete encode key */
        /*
         * EncodeMsgKeyAttribComplete finishes our key opaque, so it should
         * return and indicate for us to encode our container/msg payload
         */
        Assert.Equal(CodecReturnCode.ENCODE_CONTAINER,
            msg.EncodeKeyAttribComplete(encodeIter, true));

        // encode payload
        FieldList fieldList = new();
        FieldEntry fieldEntry = new();

        /*
         * 1. Positive test. Encode a field list as follows, and verify encoded
         * data. FieldListInit FieldEntry - (10) REAL Blank - blank to encoder.
         * FieldEntry - (175) pre-encoded data. (ABCDEFG) FieldEntry - (32) UINT
         * 554433 FieldEntry - (111) REAL 867564 EXPONENT_4. FieldListComplete
         */

        fieldList.ApplyHasInfo();
        fieldList.DictionaryId = 2;
        fieldList.FieldListNum = 3;
        fieldList.ApplyHasStandardData();
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeInit(encodeIter, null, 40));

        // encode (10) REAL as Blank - blank to encoder.
        fieldEntry.FieldId = 10;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.EncodeBlank(encodeIter));

        // encode (175) pre-encoded data "ABDCEFG"
        fieldEntry.FieldId = 175;
        fieldEntry.DataType = DataTypes.ASCII_STRING;
        Buffer preEncoded = new();
        preEncoded.Data("ABCDEFG");
        fieldEntry.EncodedData = preEncoded;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter));

        // encode (32) UINT 554433
        Codec.UInt uInt = new();
        uInt.Value(554433);
        fieldEntry.FieldId = 32;
        fieldEntry.DataType = DataTypes.UINT;
        Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, fieldEntry.Encode(encodeIter, uInt));
        Buffer buf4 = new();
        buf4.Data(new ByteBuffer(200));
        encodeIter.RealignBuffer(buf4);
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, uInt));

        // encode (111) REAL 867564 Exponent_4
        Codec.Real real = new();
        real.Value(867564, RealHints.EXPONENT_4);
        fieldEntry.FieldId = 111;
        fieldEntry.DataType = DataTypes.REAL;
        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Encode(encodeIter, real));

        /* complete the encoding of the payload */
        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.EncodeComplete(encodeIter, true));

        /* complete encode message */
        Assert.Equal(CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));

        byte[] expectedData = DATA_018_genericMsg;

        Assert_ArrayEqual(expectedData, new System.Span<byte>(buf4.Data().Contents, 0, buf4.Length));
    }

    #region Helper methods

    private void Assert_ArrayEqual(System.Span<byte> expectedData, System.Span<byte> actualData)
    {
        Assert.Equal(expectedData.Length, actualData.Length);

        for (int pos = 0; pos < expectedData.Length; pos++)
        {
            Assert.True(expectedData[pos] == actualData[pos],
                $"Mismatch at position {pos}: {expectedData[pos]} != {actualData[pos]}");
        }
    }

    private void ClearBuffer(ByteBuffer buffer)
    {
        buffer.Rewind();
        for (int i = 0; i < buffer.Capacity; i++)
        {
            buffer.Write((byte)0);
        }
        buffer.Rewind();
    }

    private Buffer GrowByOneAndCopy(Buffer src)
    {
        ByteBuffer newBuf = new ByteBuffer(src.Data().Capacity + 1);
        src.Data().Flip();
        newBuf.Put(src.Data());
        src.Data(newBuf);
        return src;
    }
    #endregion
}