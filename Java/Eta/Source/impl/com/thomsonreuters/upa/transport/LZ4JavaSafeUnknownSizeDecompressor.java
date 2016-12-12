package com.thomsonreuters.upa.transport;

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import static com.thomsonreuters.upa.transport.LZ4Utils.COPY_LENGTH;
import static com.thomsonreuters.upa.transport.LZ4Utils.MIN_MATCH;
import static com.thomsonreuters.upa.transport.LZ4Utils.ML_BITS;
import static com.thomsonreuters.upa.transport.LZ4Utils.ML_MASK;
import static com.thomsonreuters.upa.transport.LZ4Utils.RUN_MASK;
import static com.thomsonreuters.upa.transport.LZ4Utils.safeArraycopy;
import static com.thomsonreuters.upa.transport.LZ4Utils.safeIncrementalCopy;
import static com.thomsonreuters.upa.transport.LZ4Utils.wildArraycopy;
import static com.thomsonreuters.upa.transport.LZ4Utils.wildIncrementalCopy;
import static com.thomsonreuters.upa.transport.LZ4Utils.checkRange;

/* Decompressor written in pure Java without using the unofficial sun.misc.Unsafe API.
 * 
 * LZ4 decompressor that requires the size of the compressed data to be known.
 */
class LZ4JavaSafeUnknownSizeDecompressor
{
    /* Uncompress src[srcOff:srcLen] into dest[destOff:]
     * and returns the number of decompressed bytes written into dest.
     * 
     * srcLen is the exact size of the compressed stream
     * 
     * Returns the original input size
     */
    int decompress(byte[] src, int srcOff, int srcLen, byte[] dest, int destOff)
    {
        checkRange(src, srcOff, srcLen);
        checkRange(dest, destOff);

        final int srcEnd = srcOff + srcLen;
        final int destEnd = dest.length;

        int sOff = srcOff;
        int dOff = destOff;

        while (sOff < srcEnd)
        {
            final int token = src[sOff++] & 0xFF;

            // literals
            int literalLen = token >>> ML_BITS;

            if (literalLen == RUN_MASK)
            {
                byte len;
                while ((len = src[sOff++]) == (byte)0xFF)
                {
                    literalLen += 0xFF;
                }
                literalLen += len & 0xFF;
            }

            final int literalCopyEnd = dOff + literalLen;
            if (literalCopyEnd > destEnd - COPY_LENGTH || sOff + literalLen > srcEnd - COPY_LENGTH)
            {
                if (literalCopyEnd > destEnd || sOff + literalLen > srcEnd)
                {
                    throw new CompressorException("Malformed input at " + sOff);
                }
                else
                {
                    safeArraycopy(src, sOff, dest, dOff, literalLen);
                    sOff += literalLen;
                    dOff = literalCopyEnd;
                    if (sOff < srcEnd)
                    {
                        throw new CompressorException("Malformed input at " + sOff);
                    }
                    break; // EOF
                }
            }

            wildArraycopy(src, sOff, dest, dOff, literalLen);
            sOff += literalLen;
            dOff = literalCopyEnd;

            // matchs
            final int matchDec = (src[sOff++] & 0xFF) | ((src[sOff++] & 0xFF) << 8);
            final int matchOff = dOff - matchDec;

            if (matchOff < destOff)
            {
                throw new CompressorException("Malformed input at " + sOff);
            }

            int matchLen = token & ML_MASK;
            if (matchLen == ML_MASK)
            {
                byte len;
                while ((len = src[sOff++]) == (byte)0xFF)
                {
                    matchLen += 0xFF;
                }
                matchLen += len & 0xFF;
            }
            matchLen += MIN_MATCH;

            final int matchCopyEnd = dOff + matchLen;

            if (matchCopyEnd > destEnd - COPY_LENGTH)
            {
                if (matchCopyEnd > destEnd)
                {
                    throw new CompressorException("Malformed input at " + sOff);
                }
                safeIncrementalCopy(dest, matchOff, dOff, matchLen);
            }
            else
            {
                wildIncrementalCopy(dest, matchOff, dOff, matchLen);
            }
            dOff = matchCopyEnd;
        }

        return dOff - destOff;
    }

  }
