/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import java.nio.ByteBuffer;

public class Hashers
{
    private static long hashCrcTable[] = { 
        0x00000000L, 0xBC5C9CBFL, 0x9F32D42DL, 0x236E4892L, 0xD9EE4509L, 0x65B2D9B6L, 
        0x46DC9124L, 0xFA800D9BL, 0x54576741L, 0xE80BFBFEL, 0xCB65B36CL, 0x77392FD3L, 
        0x8DB92248L, 0x31E5BEF7L, 0x128BF665L, 0xAED76ADAL, 0xA8AECE82L, 0x14F2523DL, 
        0x379C1AAFL, 0x8BC08610L, 0x71408B8BL, 0xCD1C1734L, 0xEE725FA6L, 0x522EC319L, 
        0xFCF9A9C3L, 0x40A5357CL, 0x63CB7DEEL, 0xDF97E151L, 0x2517ECCAL, 0x994B7075L, 
        0xBA2538E7L, 0x0679A458L, 0xB6D67057L, 0x0A8AECE8L, 0x29E4A47AL, 0x95B838C5L, 
        0x6F38355EL, 0xD364A9E1L, 0xF00AE173L, 0x4C567DCCL, 0xE2811716L, 0x5EDD8BA9L, 
        0x7DB3C33BL, 0xC1EF5F84L, 0x3B6F521FL, 0x8733CEA0L, 0xA45D8632L, 0x18011A8DL, 
        0x1E78BED5L, 0xA224226AL, 0x814A6AF8L, 0x3D16F647L, 0xC796FBDCL, 0x7BCA6763L, 
        0x58A42FF1L, 0xE4F8B34EL, 0x4A2FD994L, 0xF673452BL, 0xD51D0DB9L, 0x69419106L, 
        0x93C19C9DL, 0x2F9D0022L, 0x0CF348B0L, 0xB0AFD40FL, 0x8A270DFDL, 0x367B9142L, 
        0x1515D9D0L, 0xA949456FL, 0x53C948F4L, 0xEF95D44BL, 0xCCFB9CD9L, 0x70A70066L, 
        0xDE706ABCL, 0x622CF603L, 0x4142BE91L, 0xFD1E222EL, 0x079E2FB5L, 0xBBC2B30AL, 
        0x98ACFB98L, 0x24F06727L, 0x2289C37FL, 0x9ED55FC0L, 0xBDBB1752L, 0x01E78BEDL, 
        0xFB678676L, 0x473B1AC9L, 0x6455525BL, 0xD809CEE4L, 0x76DEA43EL, 0xCA823881L, 
        0xE9EC7013L, 0x55B0ECACL, 0xAF30E137L, 0x136C7D88L, 0x3002351AL, 0x8C5EA9A5L, 
        0x3CF17DAAL, 0x80ADE115L, 0xA3C3A987L, 0x1F9F3538L, 0xE51F38A3L, 0x5943A41CL, 
        0x7A2DEC8EL, 0xC6717031L, 0x68A61AEBL, 0xD4FA8654L, 0xF794CEC6L, 0x4BC85279L, 
        0xB1485FE2L, 0x0D14C35DL, 0x2E7A8BCFL, 0x92261770L, 0x945FB328L, 0x28032F97L, 
        0x0B6D6705L, 0xB731FBBAL, 0x4DB1F621L, 0xF1ED6A9EL, 0xD283220CL, 0x6EDFBEB3L, 
        0xC008D469L, 0x7C5448D6L, 0x5F3A0044L, 0xE3669CFBL, 0x19E69160L, 0xA5BA0DDFL, 
        0x86D4454DL, 0x3A88D9F2L, 0xF3C5F6A9L, 0x4F996A16L, 0x6CF72284L, 0xD0ABBE3BL, 
        0x2A2BB3A0L, 0x96772F1FL, 0xB519678DL, 0x0945FB32L, 0xA79291E8L, 0x1BCE0D57L, 
        0x38A045C5L, 0x84FCD97AL, 0x7E7CD4E1L, 0xC220485EL, 0xE14E00CCL, 0x5D129C73L, 
        0x5B6B382BL, 0xE737A494L, 0xC459EC06L, 0x780570B9L, 0x82857D22L, 0x3ED9E19DL, 
        0x1DB7A90FL, 0xA1EB35B0L, 0x0F3C5F6AL, 0xB360C3D5L, 0x900E8B47L, 0x2C5217F8L, 
        0xD6D21A63L, 0x6A8E86DCL, 0x49E0CE4EL, 0xF5BC52F1L, 0x451386FEL, 0xF94F1A41L, 
        0xDA2152D3L, 0x667DCE6CL, 0x9CFDC3F7L, 0x20A15F48L, 0x03CF17DAL, 0xBF938B65L, 
        0x1144E1BFL, 0xAD187D00L, 0x8E763592L, 0x322AA92DL, 0xC8AAA4B6L, 0x74F63809L, 
        0x5798709BL, 0xEBC4EC24L, 0xEDBD487CL, 0x51E1D4C3L, 0x728F9C51L, 0xCED300EEL, 
        0x34530D75L, 0x880F91CAL, 0xAB61D958L, 0x173D45E7L, 0xB9EA2F3DL, 0x05B6B382L, 
        0x26D8FB10L, 0x9A8467AFL, 0x60046A34L, 0xDC58F68BL, 0xFF36BE19L, 0x436A22A6L, 
        0x79E2FB54L, 0xC5BE67EBL, 0xE6D02F79L, 0x5A8CB3C6L, 0xA00CBE5DL, 0x1C5022E2L, 
        0x3F3E6A70L, 0x8362F6CFL, 0x2DB59C15L, 0x91E900AAL, 0xB2874838L, 0x0EDBD487L, 
        0xF45BD91CL, 0x480745A3L, 0x6B690D31L, 0xD735918EL, 0xD14C35D6L, 0x6D10A969L, 
        0x4E7EE1FBL, 0xF2227D44L, 0x08A270DFL, 0xB4FEEC60L, 0x9790A4F2L, 0x2BCC384DL, 
        0x851B5297L, 0x3947CE28L, 0x1A2986BAL, 0xA6751A05L, 0x5CF5179EL, 0xE0A98B21L, 
        0xC3C7C3B3L, 0x7F9B5F0CL, 0xCF348B03L, 0x736817BCL, 0x50065F2EL, 0xEC5AC391L, 
        0x16DACE0AL, 0xAA8652B5L, 0x89E81A27L, 0x35B48698L, 0x9B63EC42L, 0x273F70FDL, 
        0x0451386FL, 0xB80DA4D0L, 0x428DA94BL, 0xFED135F4L, 0xDDBF7D66L, 0x61E3E1D9L, 
        0x679A4581L, 0xDBC6D93EL, 0xF8A891ACL, 0x44F40D13L, 0xBE740088L, 0x02289C37L, 
        0x2146D4A5L, 0x9D1A481AL, 0x33CD22C0L, 0x8F91BE7FL, 0xACFFF6EDL, 0x10A36A52L, 
        0xEA2367C9L, 0x567FFB76L, 0x7511B3E4L, 0xC94D2F5BL
    };

    /**
     * Calculates the hash value for a buffer by calculating the crc32 using the polynomial (0xF3C5F6A9)
     *
     * @param buffer specifies a buffer containing data to calculate hash code.
     * @param position specifies a start position of the buffer.
     * @param length specifies a length of data
     * @return a hash value
     */
    public static long polyHash(ByteBuffer buffer, int position, int length)
    {
        long crc = 0;
        int len = position + length;

        for (int i = position; i < len; i++)
            crc = hashCrcTable[(int)(crc ^ buffer.get(i)) & 0xFF] ^ (crc >> 8);

        return crc;
    }

    /**
     * Calculates a hashing entity ID
     * 
     * @param buffer specifies a buffer containing data to calculate hash code.
     * @param position specifies a start position of the buffer.
     * @param length specifies a length of data
     * @param numBuckets specifies a number of buckets
     * @return a hash code
     */
    public static int hashingEntityId(ByteBuffer buffer, int position, int length, int numBuckets)
    {
    	long totalRange = 0x100000000L;
    	long sum = ((polyHash(buffer, position, length) * numBuckets) / totalRange) + 1; //add 1 to make it 1-based

        return (int)sum;
    }
}
