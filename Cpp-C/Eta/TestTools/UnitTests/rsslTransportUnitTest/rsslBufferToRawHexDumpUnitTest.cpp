/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

/************************************************************************
 *	rsslBufferToRawHexDump Unit Tests
 *
 *  Unit tests for the rsslBufferToRawHexDump function defined in
 *  Cpp-C/Eta/Impl/Transport/rsslImpl.c.
 *
 ************************************************************************/

#include "gtest/gtest.h"
#include "rtr/rsslTransport.h"

#include <cstring>
#include <cstdlib>
#include <string>

/*
 * Test fixture for rsslBufferToRawHexDump tests.
 */
class RsslBufferToRawHexDumpTests : public ::testing::Test {
protected:
    RsslError error;

    virtual void SetUp() override
    {
        memset(&error, 0, sizeof(error));
    }

    // Helper: allocate an output buffer of a given size
    static RsslBuffer makeOutputBuffer(char* storage, RsslUInt32 size)
    {
        RsslBuffer buf;
        buf.data   = storage;
        buf.length = size;
        return buf;
    }

    // Helper: build a const RsslBuffer from a C-string (length excludes NUL)
    static RsslBuffer makeInputBuffer(char* data, RsslUInt32 len)
    {
        RsslBuffer buf;
        buf.data   = data;
        buf.length = len;
        return buf;
    }
};

/*******************************************************************************
 * valuesPerLine boundary tests
 ******************************************************************************/

TEST_F(RsslBufferToRawHexDumpTests, ValuesPerLineZeroReturnsFailure)
{
    char inData[] = "hello";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 0, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE);
    EXPECT_TRUE(strstr(error.text, "rsslBufferToRawHexDump() Error: 0002 Invalid argument value of 0 for valuesPerLine.") != NULL);
}

TEST_F(RsslBufferToRawHexDumpTests, ValuesPerLineOneOddBecomesZeroReturnsFailure)
{
    // valuesPerLine == 1 is odd; the implementation decrements it to 0 which is
    // invalid and must return failure.
    char inData[] = "hello";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 1, &error);

    EXPECT_EQ(ret, RSSL_RET_FAILURE);
    EXPECT_TRUE(strstr(error.text, "rsslBufferToHexDump() Error: 0002 valuesPerLine resolved to 0 after odd number adjustment.") != NULL);
}

TEST_F(RsslBufferToRawHexDumpTests, ValuesPerLineTwo)
{
    char inData[] = "AB";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    char outData[256] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 2, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 6U) << "Output length should be updated";
    // Hex for 'A'=0x41, 'B'=0x42 with spacing: "4142 \n" (even pair, no extra space)
    std::string out(outData, output.length);
    EXPECT_NE(out.find("41"), std::string::npos) << "Should contain hex for 'A'";
    EXPECT_NE(out.find("42"), std::string::npos) << "Should contain hex for 'B'";
}

TEST_F(RsslBufferToRawHexDumpTests, ValuesPerLineOddGetsDecrementedAndSucceeds)
{
    // valuesPerLine == 3 is odd; implementation decrements to 2 and should succeed.
    char inData[] = "ABCD";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 3, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 12U);
}

TEST_F(RsslBufferToRawHexDumpTests, ValuesPerLineClampedAt70)
{
    // valuesPerLine > 70 should be clamped to 70 and succeed.
    char inData[20];
    memset(inData, 0x55, sizeof(inData));
    RsslBuffer input = makeInputBuffer(inData, sizeof(inData));

    char outData[4096] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 200, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 176U);
}

/*******************************************************************************
 * Output content / format tests
 ******************************************************************************/

TEST_F(RsslBufferToRawHexDumpTests, SingleByteProducesHexOutput)
{
    unsigned char byte = 0xAB;
    RsslBuffer input;
    input.data   = reinterpret_cast<char*>(&byte);
    input.length = 1;

    char outData[256] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 2, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 6U);
    std::string out(outData, output.length);
    EXPECT_NE(out.find("ab"), std::string::npos) << "Expected hex 'ab' in output";
}

TEST_F(RsslBufferToRawHexDumpTests, OutputContainsNewline)
{
    char inData[] = "Hello World";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 33U);
    std::string out(outData, output.length);
    EXPECT_NE(out.find('\n'), std::string::npos) << "Output should contain newline";
}

TEST_F(RsslBufferToRawHexDumpTests, InputExactlyOneLineLong)
{
    // Input length == valuesPerLine: all bytes fit on a single line.
    char inData[8];
    memset(inData, 0x0F, sizeof(inData));
    RsslBuffer input = makeInputBuffer(inData, sizeof(inData));

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 8, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 21U);
    std::string out(outData, output.length);
    // Each byte 0x0F should appear as "0f" in the output
    EXPECT_NE(out.find("0f"), std::string::npos);
}

TEST_F(RsslBufferToRawHexDumpTests, InputSpansMultipleLines)
{
    // 12 bytes with valuesPerLine=4: produces 3 lines.
    char inData[12];
    for (int i = 0; i < 12; ++i)
        inData[i] = (char)(i + 1);
    RsslBuffer input = makeInputBuffer(inData, sizeof(inData));

    char outData[1024] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 33U);
    std::string out(outData, output.length);
    // Count newlines � expect 3 (one per full line)
    size_t newlineCount = 0;
    for (char c : out)
        if (c == '\n') ++newlineCount;
    EXPECT_EQ(newlineCount, 3U);
}

TEST_F(RsslBufferToRawHexDumpTests, OutputLengthIsUpdated)
{
    char inData[] = "data";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    char outData[512] = {};
    RsslUInt32 originalLength = sizeof(outData);
    RsslBuffer output = makeOutputBuffer(outData, originalLength);

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_LT(output.length, originalLength) << "Output length should be reduced to actual written bytes";
    EXPECT_GT(output.length, 0U);
}

TEST_F(RsslBufferToRawHexDumpTests, AllZeroBytes)
{
    unsigned char inData[4] = {0x00, 0x00, 0x00, 0x00};
    RsslBuffer input;
    input.data   = reinterpret_cast<char*>(inData);
    input.length = sizeof(inData);

    char outData[256] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 11U);
    std::string out(outData, output.length);
    EXPECT_NE(out.find("00"), std::string::npos);
}

TEST_F(RsslBufferToRawHexDumpTests, AllMaxBytes)
{
    unsigned char inData[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    RsslBuffer input;
    input.data   = reinterpret_cast<char*>(inData);
    input.length = sizeof(inData);

    char outData[256] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 11U);
    std::string out(outData, output.length);
    EXPECT_NE(out.find("ff"), std::string::npos);
}

TEST_F(RsslBufferToRawHexDumpTests, OddNumberOfBytesWithEvenValuesPerLine)
{
    // 5 bytes, valuesPerLine=4: first line has 4 bytes, second line has 1 byte.
    char inData[5] = {0x10, 0x20, 0x30, 0x40, 0x50};
    RsslBuffer input = makeInputBuffer(inData, sizeof(inData));

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 22U);
    std::string out(outData, output.length);
    EXPECT_NE(out.find("10"), std::string::npos);
    EXPECT_NE(out.find("50"), std::string::npos);
}

/*******************************************************************************
 * Output buffer too small
 ******************************************************************************/

TEST_F(RsslBufferToRawHexDumpTests, OutputBufferTooSmallReturnsBufferTooSmall)
{
    char inData[] = "This is a reasonably long string for hex dumping";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));

    // Provide an output buffer that is far too small
    char outData[4] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 16, &error);

    EXPECT_EQ(ret, RSSL_RET_BUFFER_TOO_SMALL);
    EXPECT_TRUE(strstr(error.text, "rsslBufferToRawHexDump() Error: 0020 Cannot fit formatted hex dump output into output buffer of size(4)") != NULL);
}

TEST_F(RsslBufferToRawHexDumpTests, OutputBufferExactSizeSucceeds)
{
    // Use rsslCalculateHexDumpOutputSize to get the exact required size.
    char inData[] = "ABCDEF";
    RsslBuffer input = makeInputBuffer(inData, (RsslUInt32)strlen(inData));
    const RsslUInt32 vpl = 4;

    RsslUInt32 needed = rsslCalculateHexDumpOutputSize(&input, vpl);
    ASSERT_GT(needed, 0U);

    std::string outStorage(needed, '\0');
    RsslBuffer output;
    output.data   = &outStorage[0];
    output.length = needed;

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, vpl, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 22U);
    // Count newlines � expect 2
    size_t newlineCount = 0;
    std::string out(output.data, output.length);
    for (char c : out)
        if (c == '\n') ++newlineCount;
    EXPECT_EQ(newlineCount, 2U);
}

/*******************************************************************************
 * Large input
 ******************************************************************************/

TEST_F(RsslBufferToRawHexDumpTests, LargeInputBuffer)
{
    const RsslUInt32 inputSize = 1024;

    // Fill the input buffer with random bytes using a fixed seed for reproducibility.
    std::srand(42);
    std::string inStorage(inputSize, '\0');
    for (RsslUInt32 i = 0; i < inputSize; ++i)
        inStorage[i] = static_cast<char>(std::rand() & 0xFF);

    RsslBuffer input;
    input.data   = &inStorage[0];
    input.length = inputSize;

    const RsslUInt32 vpl = 16;
    RsslUInt32 needed = rsslCalculateHexDumpOutputSize(&input, vpl);
    ASSERT_GT(needed, 0U);

    std::string outStorage(needed, '\0');
    RsslBuffer output;
    output.data   = &outStorage[0];
    output.length = needed;

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, vpl, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 2624U);

    // Verify every input byte appears as its two-digit lowercase hex representation
    // somewhere in the output (spot-check first, middle and last bytes).
    const char hexChars[] = "0123456789abcdef";
    auto byteToHex = [&](unsigned char b) -> std::string {
        std::string h(2, '\0');
        h[0] = hexChars[(b >> 4) & 0x0F];
        h[1] = hexChars[b & 0x0F];
        return h;
    };

    std::string out(outStorage.data(), output.length);

    unsigned char firstByte  = static_cast<unsigned char>(inStorage[0]);
    unsigned char midByte    = static_cast<unsigned char>(inStorage[inputSize / 2]);
    unsigned char lastByte   = static_cast<unsigned char>(inStorage[inputSize - 1]);

    EXPECT_NE(out.find(byteToHex(firstByte)), std::string::npos)
        << "Hex for first byte (0x" << byteToHex(firstByte) << ") not found in output";
    EXPECT_NE(out.find(byteToHex(midByte)), std::string::npos)
        << "Hex for middle byte (0x" << byteToHex(midByte) << ") not found in output";
    EXPECT_NE(out.find(byteToHex(lastByte)), std::string::npos)
        << "Hex for last byte (0x" << byteToHex(lastByte) << ") not found in output";
}

/*******************************************************************************
 * No ASCII column (raw vs full hex dump distinction)
 ******************************************************************************/

TEST_F(RsslBufferToRawHexDumpTests, OutputDoesNotContainAsciiColumn)
{
    // rsslBufferToRawHexDump omits the ASCII character column that
    // rsslBufferToHexDump includes. For a buffer of all non-printable bytes,
    // the output should only contain hex digits, spaces, and newlines.
    unsigned char inData[4] = {0x01, 0x02, 0x03, 0x04};
    RsslBuffer input;
    input.data   = reinterpret_cast<char*>(inData);
    input.length = sizeof(inData);

    char outData[512] = {};
    RsslBuffer output = makeOutputBuffer(outData, sizeof(outData));

    RsslRet ret = rsslBufferToRawHexDump(&input, &output, 4, &error);

    EXPECT_EQ(ret, RSSL_RET_SUCCESS);
    EXPECT_EQ(output.length, 11U);

    // The raw dump should not contain the ASCII separator "   " followed by
    // printable chars that rsslBufferToHexDump adds.
    std::string out(outData, output.length);
    // Verify that non-printable chars are not reproduced literally in output
    for (unsigned char c : inData)
    {
        if (c < 0x20 || c >= 0x7f)
        {
            // The raw char itself should not appear as a literal in the output
            EXPECT_EQ(out.find((char)c), std::string::npos)
                << "Non-printable char 0x" << std::hex << (unsigned)c
                << " should not appear literally in raw hex dump output";
        }
    }
}
