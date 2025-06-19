/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.codec;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.ElementListFlags;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.Float;
import com.refinitiv.eta.codec.Double;

/**
 * This is used for encoding and decoding an ElementList Container Type.
*/
public class ElementListCodec
{
    /*
     * this method will encode a basic Element list with several primitives
     * embedded in it
     */
    int exampleEncode(EncodeIterator encIter)
    {
        /* create a single ElementEntry and reuse for each entry */
        ElementEntry elemEntry = CodecFactory.createElementEntry();

        /* used to store and check return values */
        int retVal;

        /* create and initialize element list structure */
        ElementList elementList = CodecFactory.createElementList();

        /* various data types used during encoding */
        Time time = CodecFactory.createTime();
        time.hour(10);
        time.minute(21);
        time.second(16);
        time.millisecond(777);
        UInt uInt = CodecFactory.createUInt();
        uInt.value(17);
        Int Int = CodecFactory.createInt();
        Int.value(13);

        Float floatVal = CodecFactory.createFloat();
        floatVal.value(1.34f);

        Double doubleVal = CodecFactory.createDouble();
        doubleVal.value(1.345);

        /*
         * populate element list structure prior to call to
         * EncodeElementListInit
         */

        /*
         * indicate that standard data will be encoded and that elementListNum
         * is included
         */
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA | ElementListFlags.HAS_ELEMENT_LIST_INFO);

        /* populate elementListNum with info needed to cache */
        elementList.elementListNum(7);

        /*
         * begin encoding of element list - assumes that (*encIter) is already
         * populated with buffer and version information
         */

        /*
         * Please note: here for simplicity, we did not use success parameter
         * for EncodeElementListInit/EncodeElementListComplete calls. We are
         * just simply displaying an error if it occurs and exit, thus true is
         * used in replacement for success parameter
         */

        if ((retVal = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            /* print out message with return value string, value, and text */
            System.out.printf("Error %s (%d) encountered with EncodeElementListInit.  Error Text: %s\n",
                              CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
            return retVal;
        }

        /* FIRST Element Entry: encode entry from the Time primitive type */
        /*
         * populate and encode element entry with name and dataType information
         * for this element
         */
        elemEntry.name().data("Element - Time");
        elemEntry.dataType(DataTypes.TIME);
        System.out.printf("\tEncoding Element Entry (name: %s) \n", elemEntry.name());
        retVal = elemEntry.encode(encIter, time);
        System.out.printf("\t\tEncoded Time: %d:%d:%d\n", time.hour(), time.minute(), time.second());

        /* SECOND Element Entry: encode entry from the Int primitive type */
        /*
         * populate and encode element entry with name and dataType information
         * for this element
         */
        elemEntry.name().data("Element - Int");
        elemEntry.dataType(DataTypes.INT);
        System.out.printf("\tEncoding Element Entry (name: %s) \n", elemEntry.name());
        retVal = elemEntry.encode(encIter, Int);
        System.out.printf("\t\tEncoded signed Integer: %d\n", Int.toLong());

        /* THIRD Element Entry: encode entry from the UInt primitive type */
        /*
         * populate and encode element entry with name and dataType information
         * for this element
         */
        elemEntry.name().data("Element - UInt");
        elemEntry.dataType(DataTypes.UINT);
        System.out.printf("\tEncoding Element Entry (name: %s) \n", elemEntry.name());
        retVal = elemEntry.encode(encIter, uInt);
        System.out.printf("\t\tEncoded Unsigned Integer: %d\n", uInt.toLong());

        /* FOURTH Element Entry: encode entry from the Real primitive type */
        /*
         * populate and encode element entry with name and dataType information
         * for this element
         */
        elemEntry.clear(); // clear this to ensure a blank field
        elemEntry.name().data("Element - Real - Blank");
        elemEntry.dataType(DataTypes.REAL);
        System.out.printf("\tEncoding Element Entry (name: %s) \n", elemEntry.name());
        elemEntry.encodeBlank(encIter); /* this encodes a blank */
        System.out.printf("\t\tEncoded Real: Blank\n");

        /* FIFTH Element Entry: encode entry from the Float primitive type */
        /*
         * populate and encode element entry with name and dataType information
         * for this element
         */
        elemEntry.name().data("Element - Float");
        elemEntry.dataType(DataTypes.FLOAT);
        System.out.printf("\tEncoding Element Entry (name: %s) \n", elemEntry.name());
        retVal = elemEntry.encode(encIter, floatVal);
        System.out.printf("\t\tEncoded Float: %f\n", floatVal.toFloat());

        /* SIXTH Element Entry: encode entry from the Double primitive type */
        /*
         * populate and encode element entry with name and dataType information
         * for this element
         */
        elemEntry.name().data("Element - Double");
        elemEntry.dataType(DataTypes.DOUBLE);
        System.out.printf("\tEncoding Element Entry (name: %s) \n", elemEntry.name());
        retVal = elemEntry.encode(encIter, doubleVal);
        System.out.printf("\t\tEncoded Float: %f\n", doubleVal.toDouble());

        /*
         * complete elementList encoding. If success parameter is true, this
         * will finalize encoding. If success parameter is false, this will roll
         * back encoding prior to EncodeElementListInit
         */

        if ((retVal = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            System.out.printf("Error %s (%d) encountered with EncodeElementListComplete.  Error Text: %s\n",
                              CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
            return retVal;
        }

        System.out.printf("\tElementList Encoding Complete");
        return CodecReturnCodes.SUCCESS;
    }

    /*
     * this method will encode a basic element list with several primitives
     * embedded in it
     */
    int exampleDecode(DecodeIterator decIter)
    {
        /* create and initialize element list structure */
        ElementList elementList = CodecFactory.createElementList();

        /* create a single ElementEntry and reuse for each entry */
        ElementEntry elemEntry = CodecFactory.createElementEntry();

        /* used to store and check return codes */
        int retVal;

        /* structures for decoding various data types */
        Time time = CodecFactory.createTime();
        Real real = CodecFactory.createReal();
        Int Int = CodecFactory.createInt();
        UInt uInt = CodecFactory.createUInt();
        Float floatVal = CodecFactory.createFloat();
        Double doubleVal = CodecFactory.createDouble();
        
        /* decode into the element list structure */
        if ((retVal = elementList.decode(decIter, null)) < CodecReturnCodes.SUCCESS)
        {
            /* decoding failure tends to be unrecoverable */
            System.out.printf("Error %s (%d) encountered with DecodeElementList.  Error Text: %s\n",
                              CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
            return CodecReturnCodes.FAILURE;
        }
        System.out.printf("\tElementList Header Decoded\n");

        /* decode each element entry */
        while ((retVal = elemEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (retVal < CodecReturnCodes.SUCCESS)
            {
                /* decoding failure tends to be unrecoverable */
                System.out.printf("Error %s (%d) encountered with DecodeElementEntry.  Error Text: %s\n",
                                  CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                return CodecReturnCodes.FAILURE;
            }

            /* Continue decoding field entries. */

            System.out.printf("\tDecoding Element Entry (name: %s) \n", elemEntry.name());

            /* use elemEntry.dataType to call correct primitive decode method */
            switch (elemEntry.dataType())
            {
                case DataTypes.REAL:
                    if ((retVal = real.decode(decIter)) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error %s (%d) encountered with DecodeReal().  Error Text: %s\n",
                                          CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                        return CodecReturnCodes.FAILURE;
                    }
                    if (real.isBlank())
                        System.out.printf("\t\tDecoded Real: Blank\n");
                    else
                        System.out.printf("\t\tReal Decoded: hint: %d  value: %d\n", real.hint(), real.toLong());
                    break;

                case DataTypes.TIME:
                    if ((retVal = time.decode(decIter)) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error %s (%d) encountered with DecodeTime().  Error Text: %s\n",
                                          CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                        return CodecReturnCodes.FAILURE;
                    }
                    System.out.printf("\t\tTime Decoded: %d:%d:%d\n", time.hour(), time.minute(), time.second());
                    break;

                case DataTypes.INT:
                    if ((retVal = Int.decode(decIter)) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error %s (%d) encountered with DecodeInt().  Error Text: %s\n",
                                          CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                        return CodecReturnCodes.FAILURE;
                    }
                    System.out.printf("\t\tInt Decoded: %d\n", Int.toLong());
                    break;

                case DataTypes.UINT:
                    if ((retVal = uInt.decode(decIter)) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error %s (%d) encountered with DecodeUInt().  Error Text: %s\n",
                                          CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                        return CodecReturnCodes.FAILURE;
                    }
                    System.out.printf("\t\tUInt Decoded: %d\n", uInt.toLong());
                    break;
                    
                case DataTypes.FLOAT:
                    if ((retVal = floatVal.decode(decIter)) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error %s (%d) encountered with DecodeUInt().  Error Text: %s\n",
                                          CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                        return CodecReturnCodes.FAILURE;
                    }
                    System.out.printf("\t\tFloat Decoded: %f\n", floatVal.toFloat());
                    break;
                    
                case DataTypes.DOUBLE:
                    if ((retVal = doubleVal.decode(decIter)) < CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("Error %s (%d) encountered with DecodeUInt().  Error Text: %s\n",
                                          CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
                        return CodecReturnCodes.FAILURE;
                    }
                    System.out.printf("\t\tDouble Decoded: %f\n", doubleVal.toDouble());
                    break;
                    
                default:
                    System.out.printf("Error: unexpected datatype(%d) found in elementList\n", elemEntry.dataType());
                    break;
            }
        }

        System.out.printf("\tElementList Decoding Complete\n");
        return CodecReturnCodes.SUCCESS;
    }
}
