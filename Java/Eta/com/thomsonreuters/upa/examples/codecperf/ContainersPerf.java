package com.thomsonreuters.upa.examples.codecperf;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.FieldListFlags;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.FilterEntryFlags;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.FilterListFlags;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.MapEntryFlags;
import com.thomsonreuters.upa.codec.MapFlags;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.examples.common.CommandLine;
import com.thomsonreuters.upa.transport.TransportFactory;

/**
 * <p>
 * This is a main class to run UPA Containers Perf application. The purpose of
 * this application is to benchmark/measuring performance for various UPA
 * containers. It is a single-threaded client application. Sample UPA Containers
 * include UpaArray, UpaElementList, UpaElementEntry, UpaFieldList,
 * UpaFieldEntry, UpaFilterList, UpaFilterEntry, UpaMap, UpaMapEntry, UpaSeries,
 * UpaSeriesEntry.
 * </p>
 * <p>
 * <H2>Setup Environment</H2>
 * <p>
 * The RDMFieldDictionary and enumtype.def files could be located in the
 * directory of execution or this application will request dictionary from
 * provider.
 * <p>
 * <H2>Running the application:</H2>
 * <p>
 * Pass the number of loops as the command line argument, and default is
 * 1000000.
 * <ul>
 * <li>-nl Number of loops for Perf testing. Default is <i>1000000</i>.
 * </ul>
 * 
 */
public class ContainersPerf
{
    /* Containers Perf testing default number of loops */
    private static final int defaultNumOfLoops = 1000000;

    // UpaArray, UpaElementList, UpaElementEntry, UpaFieldList, UpaFieldEntry,
    // UpaFilterList, UpaFilterEntry
    // UpaMap, UpaMapEntry, UpaSeries, UpaSeriesEntry

    EncodeIterator _encIter = CodecFactory.createEncodeIterator();
    DecodeIterator _decIter = CodecFactory.createDecodeIterator();
    Buffer _buffer = CodecFactory.createBuffer();

    public void run(String[] args)
    {
        long startTime = System.currentTimeMillis();
        int loop;

        /* set the number of loops for Perf testing */
        if (CommandLine.hasArg("nl"))
        {
            loop = CommandLine.intValue("nl");
        }
        else
        {
            loop = defaultNumOfLoops;
        }

        for (int i = 0; i < loop; i++)
        {
            nestedEncDecTest();
        }
        System.out.println("finished nestedEncDecTest ");

        for (long i = 0; i < loop; i++)
        {
            basicFieldListNestingTest();
        }
        System.out.println("finished basicFieldListNestingTest ");

        for (long i = 0; i < loop; i++)
        {
            FilterListUnitTests();
        }
        System.out.println("finished FilterListUnitTests ");

        for (long i = 0; i < loop; i++)
        {
            MapUnitTests();
        }
        System.out.println("finished MapUnitTests ");

        for (long i = 0; i < loop; i++)
        {
            FieldListUnitTests();
        }
        System.out.println("finished FieldListUnitTests ");

        for (long i = 0; i < loop; i++)
        {
            elementListUnitTests();
        }
        System.out.println("finished elementListUnitTests ");

        for (long i = 0; i < loop; i++)
        {
            BufferEntryTest();
        }
        System.out.println("finished BufferEntryTest ");

        for (long i = 0; i < loop; i++)
        {
            arrayUnitTests();
        }
        System.out.println("finished arrayUnitTests ");

        long stopTime = System.currentTimeMillis();
        long diff = stopTime - startTime;
        System.out.println("loops " + loop + ", containers encoding/decoding time " + diff
                + " milliseconds.");
    }

    void testBufferArray(Buffer[] testBuffers, int count, int itemLength)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */
        for (int type = DataTypes.BUFFER; type <= DataTypes.RMTES_STRING; type++)
        {
            _encIter.clear();
            _buffer.data(ByteBuffer.allocate(100));
            _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            Array array = CodecFactory.createArray();
            ArrayEntry ae = CodecFactory.createArrayEntry();
            array.itemLength(itemLength);
            array.primitiveType(type);
            array.encodeInit(_encIter);
            for (int i = 0; i < count; i++)
            {
                ae.encode(_encIter, testBuffers[i]);
            }
            array.encodeComplete(_encIter, true);

            _buffer.data().flip();
            _buffer.data(_buffer.data(), 0, _buffer.data().limit());

            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            Array array1 = CodecFactory.createArray();
            array1.decode(_decIter);

            int ret = 0;
            Buffer tmpBuffer = CodecFactory.createBuffer();
            while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    tmpBuffer.clear();
                    ret = tmpBuffer.decode(_decIter);
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    //
                }
            }
        }
    }

    void testEnumArray(Enum[] testEnums, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.ENUM);
        array.encodeInit(_encIter);
        for (int i = 0; i < count; i++)
        {
            ae.encode(_encIter, testEnums[i]);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        Enum tmpEnum = CodecFactory.createEnum();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                tmpEnum.clear();
                ret = tmpEnum.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testStateArray(State[] testStates, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.STATE);
        array.encodeInit(_encIter);
        for (int i = 0; i < count; i++)
        {
            ae.encode(_encIter, testStates[i]);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        State tmpState = CodecFactory.createState();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                tmpState.clear();
                ret = tmpState.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testQosArray(Qos[] testQoss, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.QOS);
        array.encodeInit(_encIter);
        for (int i = 0; i < count; i++)
        {
            ae.encode(_encIter, testQoss[i]);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        Qos tmpQos = CodecFactory.createQos();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpQos.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testDateArray(Date[] testDates, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.DATE);
        array.encodeInit(_encIter);
        for (int i = 0; i < count; i++)
        {
            ae.encode(_encIter, testDates[i]);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        Date tmpDate = CodecFactory.createDate();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpDate.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testTimeArray(Time[] testTimes, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.TIME);
        array.encodeInit(_encIter);
        for (int i = 0; i < count; i++)
        {
            ae.encode(_encIter, testTimes[i]);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);
        ;

        int ret = 0;
        Time tmpTime = CodecFactory.createTime();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpTime.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testRealArray(Real[] testReals, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.REAL);
        array.encodeInit(_encIter);
        for (int i = 0; i < count; i++)
        {
            ae.encode(_encIter, testReals[i]);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        Real tmpReal = CodecFactory.createReal();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpReal.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testUIntArray(long[] testUInts, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.UINT);
        array.encodeInit(_encIter);
        UInt data = CodecFactory.createUInt();
        for (int i = 0; i < count; i++)
        {
            data.clear();
            data.value(testUInts[i]);
            ae.encode(_encIter, data);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        UInt tmpUInt = CodecFactory.createUInt();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpUInt.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    void testIntArray(long[] testInts, int count, int itemLength, boolean blank)
    {
        /*
         * Encode and decode the primitive types given by 'primitives' into and
         * out of an array. Encode as blank if specified. Use a set definition
         * if given.
         */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.INT);
        array.encodeInit(_encIter);
        Int data = CodecFactory.createInt();
        for (int i = 0; i < count; i++)
        {
            data.clear();
            data.value(testInts[i]);
            ae.encode(_encIter, data);
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        Int tmpInt = CodecFactory.createInt();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpInt.decode(_decIter);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
        }
    }

    public void arrayUnitTests()
    {
        /*
         * Tests encoding arrays of different types and itemLengths, ensuring
         * that it succeeds or fails as appropriate
         */

        /* Indexed by encoded size(e.g. testInts[4] is a 4-byte value) */
        long testInts[] = { 0x0, 0x7f, 0x7fff, 0x7fffff, 0x7fffffff, 0x7fffffffffL,
                0x7fffffffffffL, 0x7fffffffffffffL, 0x7fffffffffffffffL };
        // testIntArray(testInts, 9, 8, false);
        testIntArray(testInts, 9, 0, false);

        long testUInts[] = { 0x0, 0xff, 0xffff, 0xffffff, 0xffffffff, 0xffffffffffL,
                0xffffffffffffL, 0xffffffffffffffL, 0xffffffffffffffffL };
        // testUIntArray(testUInts, 9, 8, false);
        testUIntArray(testUInts, 9, 0, false);

        Real testReals[] = new Real[testInts.length];
        int hint = RealHints.EXPONENT5;
        for (int i = 0; i < testInts.length; i++)
        {
            testReals[i] = CodecFactory.createReal();
            testReals[i].value(testInts[i], hint);
        }
        // testRealArray(testReals, 9, 16, false);
        testRealArray(testReals, 9, 0, false);

        int testDT[][] = new int[3][7]; // test for 3 entries
        DateTime testDateTime[] = new DateTime[3];
        Date testDate[] = new Date[3];
        Time testTime[] = new Time[3];

        for (int i = 0; i < 3; i++)
        {
            testDT[i][0] = 1990 + 2 * i; // year
            testDT[i][1] = 1 + 3 * i; // month
            testDT[i][2] = 7 * i + 2; // day
            testDT[i][3] = 11 - 2 * i; // hour
            testDT[i][4] = 60 - 10 * i; // minute
            testDT[i][5] = 60 - 11 * i; // second
            testDT[i][6] = 1000 - 60 * i; // milisecond

            testDateTime[i] = CodecFactory.createDateTime();
            testDateTime[i].year(testDT[i][0]);
            testDateTime[i].month(testDT[i][1]);
            testDateTime[i].day(testDT[i][2]);
            testDateTime[i].hour(testDT[i][3]);
            testDateTime[i].minute(testDT[i][4]);
            testDateTime[i].second(testDT[i][5]);
            testDateTime[i].millisecond(testDT[i][6]);

            testDate[i] = CodecFactory.createDate();
            testDate[i].year(testDT[i][0]);
            testDate[i].month(testDT[i][1]);
            testDate[i].day(testDT[i][2]);

            testTime[i] = CodecFactory.createTime();
            testTime[i].hour(testDT[i][3]);
            testTime[i].minute(testDT[i][4]);
            testTime[i].second(testDT[i][5]);
            testTime[i].millisecond(testDT[i][6]);
        }
        testDateArray(testDate, 3, 0, false);
        // testDateArray(testDate, 3, 4, false);
        testTimeArray(testTime, 3, 0, false);
        // testTimeArray(testTime, 3, 8, false);

        Qos testQos3 = CodecFactory.createQos();
        testQos3.dynamic(false);
        testQos3.rate(QosRates.TICK_BY_TICK);
        testQos3.timeliness(QosTimeliness.REALTIME);

        Qos testQos1 = CodecFactory.createQos();
        testQos1.dynamic(true);
        testQos1.rate(QosRates.TICK_BY_TICK);
        testQos1.timeliness(QosTimeliness.DELAYED);
        testQos1.timeInfo(567);

        Qos testQos2 = CodecFactory.createQos();
        testQos2.dynamic(false);
        testQos2.rate(QosRates.TIME_CONFLATED);
        testQos2.rateInfo(897);
        testQos2.timeliness(QosTimeliness.REALTIME);

        Qos[] testQos = { testQos2, testQos1, testQos3 };
        testQosArray(testQos, 3, 0, false);

        State testState1 = CodecFactory.createState();
        testState1.code(StateCodes.INVALID_VIEW);
        testState1.dataState(DataStates.NO_CHANGE);
        testState1.streamState(StreamStates.OPEN);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("Illinois");
        testState1.text(stateText);

        State testState2 = CodecFactory.createState();
        testState2.code(StateCodes.NONE);
        testState2.dataState(DataStates.NO_CHANGE);
        testState2.streamState(StreamStates.OPEN);

        State testState3 = CodecFactory.createState();
        testState3.code(StateCodes.INVALID_ARGUMENT);
        testState3.dataState(DataStates.NO_CHANGE);
        testState3.streamState(StreamStates.NON_STREAMING);
        Buffer stateText3 = CodecFactory.createBuffer();
        stateText3.data("bla bla");
        testState3.text(stateText3);

        State[] testState = { testState1, testState2, testState3 };
        testStateArray(testState, 3, 0, false);

        Enum[] testEnum = new Enum[3];
        for (int i = 0; i < 3; i++)
        {
            testEnum[i] = CodecFactory.createEnum();
            testEnum[i].value((short)testInts[i]);
        }
        testEnumArray(testEnum, 3, 0, false);

        Buffer testBuffer1 = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(7);
        byte[] bts = { 1, 2, 0, 10, 57, 0x7F, 4 };
        bb.put(bts);
        testBuffer1.data(bb, 0, 7);

        Buffer testBuffer2 = CodecFactory.createBuffer();
        bb = ByteBuffer.allocate(5);
        byte[] bts1 = { 27, 48, 0, 10, 57 };
        bb.put(bts1);
        testBuffer2.data(bb, 0, 5);

        Buffer testBuffer3 = CodecFactory.createBuffer();
        bb = ByteBuffer.allocate(4);
        byte[] bts2 = { 10, 57, 0x7F, 4 };
        bb.put(bts2);
        testBuffer3.data(bb, 0, 4);

        Buffer[] testBuffer = { testBuffer1, testBuffer2, testBuffer3 };
        testBufferArray(testBuffer, 3, 0);
    }

    public void BufferEntryTest()
    {
        Buffer buffer1 = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(7);
        byte[] bts = { 41, 42, 40, 50, 57, 32, 33 };
        bb.put(bts);
        buffer1.data(bb, 0, 7);

        Buffer buffer2 = CodecFactory.createBuffer();
        bb = ByteBuffer.allocate(6);
        byte[] bts1 = { 37, 48, 40, 50, 57, 32 };
        bb.put(bts1);
        buffer2.data(bb, 0, 6);

        Buffer buffer3 = CodecFactory.createBuffer();
        bb = ByteBuffer.allocate(4);
        byte[] bts2 = { 50, 57, 48, 34 };
        bb.put(bts2);
        buffer3.data(bb, 0, 4);

        Buffer buffer4 = CodecFactory.createBuffer();
        bb = ByteBuffer.allocate(4);
        byte[] bts3 = { 10, 57, 0x7F, 4 };
        bb.put(bts3);
        buffer4.data(bb, 0, 0);

        Buffer[] buffers = { buffer1, buffer2, buffer3, buffer4 };

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(6);
        array.primitiveType(DataTypes.BUFFER);
        array.encodeInit(_encIter);
        for (int i = 0; i < 4; i++)
        {
            ae.encode(_encIter, buffers[i]);
            if (i == 3)
            {
                //
            }
        }
        array.encodeComplete(_encIter, true);

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);

        int ret = 0;
        int i = 0;
        Buffer tmpBuffer = CodecFactory.createBuffer();
        ae.clear();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                tmpBuffer.clear();
                ret = tmpBuffer.decode(_decIter);
                if (array.itemLength() <= buffers[i].length())
                {
                    for (int j = 0; j < array.itemLength(); j++)
                    {
                        //
                    }
                }
                else
                {
                    for (int j = 0; j < buffers[i].length(); j++)
                    {
                        //
                    }
                    for (int j = buffers[i].length(); j < array.itemLength(); j++)
                    {
                        //
                    }
                }
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                //
            }
            i++;
        }
    }

    private void decodeElementList(int elements, int fi)
    {
        int[] flags = { ElementListFlags.HAS_STANDARD_DATA,
                ElementListFlags.HAS_ELEMENT_LIST_INFO + ElementListFlags.HAS_STANDARD_DATA };
        // These arrays for fids and dataTypes make up our "Dictionary"

        ElementList decElementList = CodecFactory.createElementList();
        ElementEntry decEntry = CodecFactory.createElementEntry();
        Buffer[] names = { CodecFactory.createBuffer(), CodecFactory.createBuffer(),
                CodecFactory.createBuffer(), CodecFactory.createBuffer(),
                CodecFactory.createBuffer(),
        // CodecFactory.createBuffer(),
        // CodecFactory.createBuffer()
        };
        names[0].data("INT");
        names[1].data("UINT");
        names[2].data("REAL");
        names[3].data("DATE");
        names[4].data("TIME");

        Int decInt = CodecFactory.createInt();
        UInt decUInt = CodecFactory.createUInt();
        Real decReal = CodecFactory.createReal();
        Date decDate = CodecFactory.createDate();
        Time decTime = CodecFactory.createTime();
        DateTime decDateTime = CodecFactory.createDateTime();
        Array decArray = CodecFactory.createArray();
        ArrayEntry decArrayEntry = CodecFactory.createArrayEntry();

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        decElementList.decode(_decIter, null);
        if (flags[fi] == ElementListFlags.HAS_ELEMENT_LIST_INFO)
        {
            //
        }
        while (decEntry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
        {
            switch (decEntry.dataType())
            {
                case DataTypes.INT:
                    decInt.decode(_decIter);
                    break;
                case DataTypes.UINT:
                    decUInt.decode(_decIter);
                    break;
                case DataTypes.REAL:
                    decReal.decode(_decIter);
                    break;
                case DataTypes.DATE:
                    decDate.decode(_decIter);
                    break;
                case DataTypes.TIME:
                    decTime.decode(_decIter);
                    break;
                case DataTypes.DATETIME:
                    decDateTime.decode(_decIter);
                    break;
                case DataTypes.ARRAY:
                    decArray.decode(_decIter);

                    while (decArrayEntry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        decInt.decode(_decIter);
                        decArrayEntry.clear();
                    }
                    break;
            }
            decEntry.clear();
        }
    }

    public void elementListUnitTests()
    {
        ElementList container = CodecFactory.createElementList();
        ElementEntry entry = CodecFactory.createElementEntry();

        int[] flags = { ElementListFlags.HAS_STANDARD_DATA,
                ElementListFlags.HAS_ELEMENT_LIST_INFO + ElementListFlags.HAS_STANDARD_DATA };

        Buffer[] names = { CodecFactory.createBuffer(), CodecFactory.createBuffer(),
                CodecFactory.createBuffer(), CodecFactory.createBuffer(),
                CodecFactory.createBuffer(),
        // CodecFactory.createBuffer(),
        // CodecFactory.createBuffer()
        };
        names[0].data("INT");
        names[1].data("UINT");
        names[2].data("REAL");
        names[3].data("DATE");
        names[4].data("TIME");

        int dataTypes[] = { DataTypes.INT, DataTypes.UINT, DataTypes.REAL, DataTypes.DATE,
                DataTypes.TIME,
        // DataTypes.DATETIME,
        // DataTypes.ARRAY
        };

        Int paylInt = CodecFactory.createInt();
        paylInt.value(-2049);

        UInt paylUInt = CodecFactory.createUInt();
        paylUInt.value(2049);

        Real paylReal = CodecFactory.createReal();
        paylReal.value(0xFFFFF, 1);

        Date paylDate = CodecFactory.createDate();
        paylDate.day(3);
        paylDate.month(8);
        paylDate.year(1892);

        Time paylTime = CodecFactory.createTime();
        paylTime.hour(23);
        paylTime.minute(59);
        paylTime.second(59);
        paylTime.millisecond(999);

        DateTime paylDateTime = CodecFactory.createDateTime();
        paylDateTime.day(3);
        paylDateTime.month(8);
        paylDateTime.year(1892);
        paylDateTime.hour(23);
        paylDateTime.minute(59);
        paylDateTime.second(59);
        paylDateTime.millisecond(999);

        /* Pre-encode array */
        Buffer preencodedArray = CodecFactory.createBuffer();
        preencodedArray.data(ByteBuffer.allocate(20));

        _encIter.clear();
        _encIter.setBufferAndRWFVersion(preencodedArray, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(0);
        array.primitiveType(DataTypes.INT);
        array.encodeInit(_encIter);
        Int data = CodecFactory.createInt();
        data.value(0xDEEEDEEE);
        for (int i = 0; i < 3; i++)
        {
            ae.encode(_encIter, data);
        }
        array.encodeComplete(_encIter, true);

        for (int i = 0; i < dataTypes.length; i++)
        {
            for (int k = 0; k < flags.length; k++)
            {
                _encIter.clear();
                _buffer.data(ByteBuffer.allocate(100));
                _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
                container.flags(flags[k]);
                if ((flags[k] & ElementListFlags.HAS_ELEMENT_LIST_INFO) > 0)
                    container.setId(256);
                container.encodeInit(_encIter, null, 0);
                for (int j = 0; j <= i; j++)
                {
                    entry.clear();
                    entry.name(names[j]);
                    entry.dataType(dataTypes[j]);
                    switch (dataTypes[j])
                    {
                        case DataTypes.INT:
                            entry.encode(_encIter, paylInt);
                            break;
                        case DataTypes.UINT:
                            entry.encode(_encIter, paylUInt);
                            break;
                        case DataTypes.REAL:
                            entry.encode(_encIter, paylReal);
                            break;
                        case DataTypes.DATE:
                            entry.encode(_encIter, paylDate);
                            break;
                        case DataTypes.TIME:
                            entry.encode(_encIter, paylTime);
                            break;
                        case DataTypes.DATETIME:
                            entry.encode(_encIter, paylDateTime);
                            break;
                        case DataTypes.ARRAY:
                            // encode two entries with the same array but
                            // different way of encoding
                            preencodedArray.data().flip();
                            preencodedArray.data(preencodedArray.data(), 0, preencodedArray.data()
                                    .limit());

                            entry.encode(_encIter, preencodedArray);

                            entry.clear();
                            entry.name(names[j - 1]);
                            entry.dataType(DataTypes.ARRAY);
                            entry.encodeInit(_encIter, 0);
                            array.itemLength(0);
                            array.primitiveType(DataTypes.INT);
                            array.encodeInit(_encIter);
                            for (int l = 0; l < 3; l++)
                            {
                                ae.encode(_encIter, data);
                            }
                            array.encodeComplete(_encIter, true);
                            entry.encodeComplete(_encIter, true);
                            break;
                    }
                }
                container.encodeComplete(_encIter, true);

                // decode encoded element list and compare
                decodeElementList(i + 1, k);
            }
        }
    }

    private void decodeFieldList(int elements, int fi)
    {
        int[] flags = { FieldListFlags.HAS_STANDARD_DATA,
                FieldListFlags.HAS_FIELD_LIST_INFO + FieldListFlags.HAS_STANDARD_DATA };

        FieldList decFieldList = CodecFactory.createFieldList();
        FieldEntry decEntry = CodecFactory.createFieldEntry();

        Int decInt = CodecFactory.createInt();
        UInt decUInt = CodecFactory.createUInt();
        Real decReal = CodecFactory.createReal();
        Date decDate = CodecFactory.createDate();
        Time decTime = CodecFactory.createTime();
        DateTime decDateTime = CodecFactory.createDateTime();
        Array decArray = CodecFactory.createArray();
        ArrayEntry decArrayEntry = CodecFactory.createArrayEntry();

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        decFieldList.decode(_decIter, null);
        if ((flags[fi] & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
        {
            //
        }

        while (decEntry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
        {
            switch (decEntry.dataType())
            {
                case DataTypes.INT:
                    decInt.decode(_decIter);
                    break;
                case DataTypes.UINT:
                    decUInt.decode(_decIter);
                    break;
                case DataTypes.REAL:
                    decReal.decode(_decIter);
                    break;
                case DataTypes.DATE:
                    decDate.decode(_decIter);
                    break;
                case DataTypes.TIME:
                    decTime.decode(_decIter);
                    break;
                case DataTypes.DATETIME:
                    decDateTime.decode(_decIter);
                    break;
                case DataTypes.ARRAY:
                    decArray.decode(_decIter);

                    while (decArrayEntry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        decInt.decode(_decIter);
                        decArrayEntry.clear();
                    }
                    break;
            }
            decEntry.clear();
        }
    }

    public void FieldListUnitTests()
    {
        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        int[] flags = { FieldListFlags.HAS_STANDARD_DATA,
                FieldListFlags.HAS_FIELD_LIST_INFO + FieldListFlags.HAS_STANDARD_DATA };

        int fids[] = { 1, 2, 3, 4, 5 };

        int dataTypes[] = { DataTypes.INT, DataTypes.UINT, DataTypes.REAL, DataTypes.DATE,
                DataTypes.TIME,
        // DataTypes.DATETIME,
        // DataTypes.ARRAY
        };

        Int paylInt = CodecFactory.createInt();
        paylInt.value(-2049);

        UInt paylUInt = CodecFactory.createUInt();
        paylUInt.value(2049);

        Real paylReal = CodecFactory.createReal();
        paylReal.value(0xFFFFF, 1);

        Date paylDate = CodecFactory.createDate();
        paylDate.day(3);
        paylDate.month(8);
        paylDate.year(1892);

        Time paylTime = CodecFactory.createTime();
        paylTime.hour(23);
        paylTime.minute(59);
        paylTime.second(59);
        paylTime.millisecond(999);

        DateTime paylDateTime = CodecFactory.createDateTime();
        paylDateTime.day(3);
        paylDateTime.month(8);
        paylDateTime.year(1892);
        paylDateTime.hour(23);
        paylDateTime.minute(59);
        paylDateTime.second(59);
        paylDateTime.millisecond(999);

        /* Pre-encode array */
        Buffer preencodedArray = CodecFactory.createBuffer();
        preencodedArray.data(ByteBuffer.allocate(20));

        _encIter.clear();
        _encIter.setBufferAndRWFVersion(preencodedArray, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(0);
        array.primitiveType(DataTypes.INT);
        array.encodeInit(_encIter);
        Int data = CodecFactory.createInt();
        data.value(0xDEEEDEEE);
        for (int i = 0; i < 3; i++)
        {
            ae.encode(_encIter, data);
        }
        array.encodeComplete(_encIter, true);

        for (int i = 0; i < dataTypes.length; i++)
        {
            for (int k = 0; k < flags.length; k++)
            {
                _encIter.clear();
                _buffer.data(ByteBuffer.allocate(100));
                _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
                container.flags(flags[k]);
                if ((flags[k] & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
                {
                    container.dictionaryId(257);
                    container.fieldListNum(256);
                }
                container.encodeInit(_encIter, null, 0);
                for (int j = 0; j <= i; j++)
                {
                    entry.clear();
                    entry.fieldId(fids[j]);
                    entry.dataType(dataTypes[j]);

                    switch (dataTypes[j])
                    {
                        case DataTypes.INT:
                            entry.encode(_encIter, paylInt);
                            break;
                        case DataTypes.UINT:
                            entry.encode(_encIter, paylUInt);
                            break;
                        case DataTypes.REAL:
                            entry.encode(_encIter, paylReal);
                            break;
                        case DataTypes.DATE:
                            entry.encode(_encIter, paylDate);
                            break;
                        case DataTypes.TIME:
                            entry.encode(_encIter, paylTime);
                            break;
                        case DataTypes.DATETIME:
                            entry.encode(_encIter, paylDateTime);
                            break;
                        case DataTypes.ARRAY:
                            // encode two entries with the same array but
                            // different way of encoding
                            preencodedArray.data().flip();
                            preencodedArray.data(preencodedArray.data(), 0, preencodedArray.data()
                                    .limit());

                            entry.encode(_encIter, preencodedArray);

                            entry.clear();
                            entry.fieldId(fids[j - 1]);
                            entry.dataType(DataTypes.ARRAY);
                            entry.encodeInit(_encIter, 0);
                            array.itemLength(0);
                            array.primitiveType(DataTypes.INT);
                            array.encodeInit(_encIter);
                            for (int l = 0; l < 3; l++)
                            {
                                ae.encode(_encIter, data);
                            }
                            array.encodeComplete(_encIter, true);
                            entry.encodeComplete(_encIter, true);
                            break;
                    }
                }
                container.encodeComplete(_encIter, true);

                // decode encoded element list and compare
                decodeFieldList(i + 1, k);
            }
        }
    }

    private void decodeFieldList()
    {
        FieldList decFieldList = CodecFactory.createFieldList();
        FieldEntry decEntry = CodecFactory.createFieldEntry();

        Int decInt = CodecFactory.createInt();
        UInt decUInt = CodecFactory.createUInt();
        Real decReal = CodecFactory.createReal();
        Date decDate = CodecFactory.createDate();
        Time decTime = CodecFactory.createTime();

        decFieldList.decode(_decIter, null);

        while (decEntry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
        {
            switch (decEntry.dataType())
            {
                case DataTypes.INT:
                    decInt.decode(_decIter);
                    break;
                case DataTypes.UINT:
                    decUInt.decode(_decIter);
                    break;
                case DataTypes.REAL:
                    decReal.decode(_decIter);
                    break;
                case DataTypes.DATE:
                    decDate.decode(_decIter);
                    break;
                case DataTypes.TIME:
                    decTime.decode(_decIter);
                    break;
            }
            decEntry.clear();
        }
    }

    private void encodeFieldList()
    {
        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        int fids[] = { 1, 2, 3, 4, 5 };

        int dataTypes[] = { DataTypes.INT, DataTypes.UINT, DataTypes.REAL, DataTypes.DATE,
                DataTypes.TIME, };

        Int paylInt = CodecFactory.createInt();
        paylInt.value(-2049);

        UInt paylUInt = CodecFactory.createUInt();
        paylUInt.value(2049);

        Real paylReal = CodecFactory.createReal();
        paylReal.value(0xFFFFF, 1);

        Date paylDate = CodecFactory.createDate();
        paylDate.day(3);
        paylDate.month(8);
        paylDate.year(1892);

        Time paylTime = CodecFactory.createTime();
        paylTime.hour(23);
        paylTime.minute(59);
        paylTime.second(59);
        paylTime.millisecond(999);

        container.flags(FieldListFlags.HAS_STANDARD_DATA);
        container.encodeInit(_encIter, null, 0);

        // for (int j = 0; j< 5; j++)
        for (int j = 0; j < 1; j++)
        {
            entry.clear();
            entry.fieldId(fids[j]);
            entry.dataType(dataTypes[j]);

            switch (dataTypes[j])
            {
                case DataTypes.INT:
                    entry.encode(_encIter, paylInt);
                    break;
                case DataTypes.UINT:
                    entry.encode(_encIter, paylUInt);
                    break;
                case DataTypes.REAL:
                    entry.encode(_encIter, paylReal);
                    break;
                case DataTypes.DATE:
                    entry.encode(_encIter, paylDate);
                    break;
                case DataTypes.TIME:
                    entry.encode(_encIter, paylTime);
                    break;
            }
        }
        container.encodeComplete(_encIter, true);
    }

    private void decodeMap(int ai, int fi, boolean hasPermData)
    {
        Map container = CodecFactory.createMap();
        MapEntry entry = CodecFactory.createMapEntry();
        Buffer deckey = CodecFactory.createBuffer();
        deckey.data(ByteBuffer.allocate(10));

        int[] flags = { MapFlags.HAS_SUMMARY_DATA, MapFlags.HAS_PER_ENTRY_PERM_DATA,
                MapFlags.HAS_TOTAL_COUNT_HINT, MapFlags.HAS_KEY_FIELD_ID };

        Buffer keyData = CodecFactory.createBuffer();
        keyData.data("keyData");
        Buffer permissionData = CodecFactory.createBuffer();
        permissionData.data("permission");
        Buffer summaryData = CodecFactory.createBuffer();
        summaryData.data("$");
        Buffer delName = CodecFactory.createBuffer();
        delName.data("Del");

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        container.decode(_decIter);
        if ((fi == MapFlags.HAS_SUMMARY_DATA) || (fi == MapFlags.HAS_TOTAL_COUNT_HINT)
                || (hasPermData) || (fi == MapFlags.HAS_KEY_FIELD_ID))
        {
            //
        }
        if ((flags[fi] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
        {
            //
        }

        if ((flags[fi] & MapFlags.HAS_SUMMARY_DATA) > 0)
        {
            //
        }

        while (entry.decode(_decIter, deckey) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (hasPermData)
            {
                //
            }
            else
            {
                //
            }

            if (entry.action() == MapEntryActions.DELETE)
            {
                //
            }
            else
            {
                decodeFieldList();
            }
            entry.clear();
        }
    }

    public void MapUnitTests()
    {
        Map container = CodecFactory.createMap();
        MapEntry entry = CodecFactory.createMapEntry();

        int[] flags = { MapFlags.HAS_SUMMARY_DATA, MapFlags.HAS_PER_ENTRY_PERM_DATA,
                MapFlags.HAS_TOTAL_COUNT_HINT, MapFlags.HAS_KEY_FIELD_ID };

        int[] entryActions = { MapEntryActions.ADD, MapEntryActions.DELETE, MapEntryActions.UPDATE };
        Buffer keyData = CodecFactory.createBuffer();
        keyData.data("keyData");
        Buffer permissionData = CodecFactory.createBuffer();
        permissionData.data("permission");
        Buffer summaryData = CodecFactory.createBuffer();
        summaryData.data("$");
        Buffer delName = CodecFactory.createBuffer();
        delName.data("Del");

        for (int i = 0; i < entryActions.length; i++)
        {
            for (int k = 0; k < flags.length; k++)
            {
                // for each entryAction/mapFlag test with entry with and with no
                // permission data
                _encIter.clear();
                _buffer.data(ByteBuffer.allocate(500));
                _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
                container.clear();
                container.flags(flags[k]);
                if ((flags[k] & MapFlags.HAS_SUMMARY_DATA) > 0)
                {
                    container.encodedSummaryData(summaryData);
                }
                if ((flags[k] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
                {
                    container.totalCountHint(5);
                }
                container.containerType(DataTypes.FIELD_LIST);
                container.keyPrimitiveType(DataTypes.ASCII_STRING);

                container.encodeInit(_encIter, 0, 0);

                // encode entries
                for (int j = 0; j <= k * i; j++)
                {
                    entry.clear();
                    entry.flags(MapEntryFlags.NONE);
                    entry.action(entryActions[i]);
                    if (entry.action() == MapEntryActions.DELETE)
                    {
                        entry.encode(_encIter, delName);
                    }
                    else
                    {
                        entry.encodeInit(_encIter, keyData, 0);
                        encodeFieldList();
                        entry.encodeComplete(_encIter, true);
                    }
                }
                container.encodeComplete(_encIter, true);

                // decode encoded element list and compare
                decodeMap(i, k, false);
                container.clear();

                if ((flags[k] & MapFlags.HAS_PER_ENTRY_PERM_DATA) > 0)
                {
                    // for each entryAction/mapFlag test with entry with and
                    // with no permission data
                    _encIter.clear();
                    _buffer.data(ByteBuffer.allocate(500));
                    _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
                                                    Codec.minorVersion());
                    container.flags(flags[k]);
                    if ((flags[k] & MapFlags.HAS_SUMMARY_DATA) > 0)
                    {
                        container.encodedSummaryData(summaryData);
                    }
                    if ((flags[k] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
                    {
                        container.totalCountHint(5);
                    }
                    container.containerType(DataTypes.FIELD_LIST);
                    container.keyPrimitiveType(DataTypes.ASCII_STRING);

                    container.encodeInit(_encIter, 0, 0);

                    // encode entries
                    for (int j = 0; j <= k * i; j++)
                    {
                        entry.clear();
                        entry.flags(MapEntryFlags.HAS_PERM_DATA);
                        entry.permData(permissionData);
                        entry.action(entryActions[i]);
                        if (entry.action() == MapEntryActions.DELETE)
                        {
                            entry.encode(_encIter, delName);
                        }
                        else
                        {
                            entry.encodeInit(_encIter, keyData, 0);
                            encodeFieldList();
                            entry.encodeComplete(_encIter, true);
                        }
                    }
                    container.encodeComplete(_encIter, true);

                    // decode encoded element list and compare
                    decodeMap(i, k, true);
                    container.clear();
                }
            }
        }
    }

    private void decodeFilterList(int ai, int fi, int ef, boolean hasPermData)
    {
        FilterList container = CodecFactory.createFilterList();
        FilterEntry entry = CodecFactory.createFilterEntry();

        int[] flags = { FilterListFlags.HAS_PER_ENTRY_PERM_DATA,
                FilterListFlags.HAS_TOTAL_COUNT_HINT };

        Buffer permissionData = CodecFactory.createBuffer();
        permissionData.data("permission");

        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        container.decode(_decIter);
        if ((fi == FilterListFlags.HAS_TOTAL_COUNT_HINT) || (hasPermData))
        {
            //
        }
        if ((flags[fi] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
        {
            //
        }

        while (entry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (hasPermData)
            {
                int f = FilterEntryFlags.HAS_PERM_DATA;
                if (ef == 1)
                    f = f + FilterEntryFlags.HAS_CONTAINER_TYPE;
            }
            else
            {
                //
            }

            if (entry.action() != FilterEntryActions.CLEAR)
            {
                decodeFieldList();
            }
            entry.clear();
        }
    }

    public void FilterListUnitTests()
    {
        FilterList container = CodecFactory.createFilterList();
        FilterEntry entry = CodecFactory.createFilterEntry();

        int[] flags = { FilterListFlags.HAS_PER_ENTRY_PERM_DATA,
                FilterListFlags.HAS_TOTAL_COUNT_HINT };

        int[] entryActions = { FilterEntryActions.CLEAR, FilterEntryActions.SET,
                FilterEntryActions.UPDATE };
        int[] entryFlags = { FilterEntryFlags.NONE, FilterEntryFlags.HAS_CONTAINER_TYPE };
        Buffer permissionData = CodecFactory.createBuffer();
        permissionData.data("permission");

        for (int i = 0; i < entryActions.length; i++)
        {
            for (int k = 0; k < flags.length; k++)
            {
                for (int l = 0; l < entryFlags.length; l++)
                {
                    // for each entryAction/flag test with entry with no
                    // permission data
                    _encIter.clear();
                    _buffer.data(ByteBuffer.allocate(200));
                    _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
                                                    Codec.minorVersion());
                    container.clear();
                    container.flags(flags[k]);
                    container.containerType(DataTypes.FIELD_LIST);
                    if ((flags[k] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
                    {
                        container.totalCountHint(5);
                    }
                    container.containerType(DataTypes.FIELD_LIST);
                    container.encodeInit(_encIter);

                    // encode entries
                    for (int j = 0; j < (k + 1) * (i + 1) * (l + 1); j++)
                    {
                        entry.clear();
                        entry.flags(entryFlags[l]);
                        entry.action(entryActions[i]);
                        entry.id(k * i + 1);
                        if ((entryFlags[l] & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
                        {
                            entry.containerType(DataTypes.FIELD_LIST);
                        }
                        entry.encodeInit(_encIter, 0);
                        if (entry.action() != FilterEntryActions.CLEAR)
                        {
                            encodeFieldList();
                        }
                        entry.encodeComplete(_encIter, true);
                    }
                    container.encodeComplete(_encIter, true);

                    // decode encoded element list and compare
                    decodeFilterList(i, k, l, false);
                    container.clear();

                    // for each entryAction/flag test with entry with permission
                    // data
                    if (k == 0)
                    {
                        _encIter.clear();
                        _buffer.data(ByteBuffer.allocate(200));
                        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
                                                        Codec.minorVersion());
                        container.clear();
                        container.flags(flags[k]);
                        if ((flags[k] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
                        {
                            container.totalCountHint(5);
                        }
                        container.containerType(DataTypes.FIELD_LIST);
                        container.encodeInit(_encIter);

                        // encode entries
                        for (int j = 0; j < (k + 1) * (i + 1) * (l + 1); j++)
                        {
                            entry.clear();
                            entry.flags(entryFlags[l] + FilterEntryFlags.HAS_PERM_DATA);
                            entry.permData(permissionData);
                            entry.action(entryActions[i]);
                            entry.id(k * i + 1);
                            if ((entryFlags[l] & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
                            {
                                entry.containerType(DataTypes.FIELD_LIST);
                            }
                            entry.encodeInit(_encIter, 0);
                            if (entry.action() != FilterEntryActions.CLEAR)
                            {
                                encodeFieldList();
                            }
                            entry.encodeComplete(_encIter, true);
                        }
                        container.encodeComplete(_encIter, true);

                        // decode encoded element list and compare
                        decodeFilterList(i, k, l, true);
                        container.clear();
                    }
                }
            }
        }
    }

    public void fieldDictionaryTest()
    {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName = "enumtype.def";
        com.thomsonreuters.upa.transport.Error error = TransportFactory.createError();

        dictionary.clear();
        dictionary.loadEnumTypeDictionary(dictionaryFileName, error);

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(200));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        // TODO dictionary.encodeEnumTypeDictionary not implemented
        // dictionary.encodeEnumTypeDictionary(_encIter,
        // DictionaryVerbosityValues.VERBOSE, error);

        // TODO do the same for field dictionary - not implemented
    }

    public void dataDictionaryTest()
    {
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName1 = "TestData/com/thomsonreuters/upa/data/Codec/enumtype1.def";
        final String dictionaryFileName2 = "TestData/com/thomsonreuters/upa/data/Codec/enumtype2.def";
        com.thomsonreuters.upa.transport.Error error = TransportFactory.createError();

        dictionary.clear();
        dictionary.loadEnumTypeDictionary(dictionaryFileName1, error);

        dictionary.clear();
        dictionary.loadEnumTypeDictionary(dictionaryFileName2, error);
    }

    // TODO realignBuffer not implemented
    public void bufferRealignTest()
    {
        Buffer bigBuffer = CodecFactory.createBuffer();
        bigBuffer.data(ByteBuffer.allocate(10000));

        FieldList container = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();

        FieldList decContainer = CodecFactory.createFieldList();
        FieldEntry decEntry = CodecFactory.createFieldEntry();
        UInt decUInt = CodecFactory.createUInt();

        UInt paylUInt = CodecFactory.createUInt();
        paylUInt.value(0xfaabd00ddeeedeeel);

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(200));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        container.applyHasStandardData();
        container.encodeInit(_encIter, null, 0);

        for (int i = -256; i < 256; i++)
        {
            entry.clear();
            entry.dataType(DataTypes.UINT);
            entry.fieldId(i);
            int ret = entry.encode(_encIter, paylUInt);
            if (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
            {
                _encIter.realignBuffer(bigBuffer);
            }
            else
            {
                //
            }
        }

        container.encodeComplete(_encIter, true);

        bigBuffer.data().flip();
        bigBuffer.data(bigBuffer.data(), 0, bigBuffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(bigBuffer, Codec.majorVersion(), Codec.minorVersion());

        decContainer.decode(_decIter, null);
        for (int i = -256; i < 256; i++)
        {
            decEntry.clear();
            decUInt.clear();
            decEntry.decode(_decIter);
            decUInt.decode(_decIter);
        }
        decEntry.clear();
        decEntry.decode(_decIter);

    }

    public void basicFieldListNestingTest()
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(200));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        FieldList container = CodecFactory.createFieldList();
        FieldList container1 = CodecFactory.createFieldList();
        FieldEntry entry = CodecFactory.createFieldEntry();
        FieldEntry entry1 = CodecFactory.createFieldEntry();

        Real paylReal = CodecFactory.createReal();
        paylReal.value(97, 2);

        Date paylDate = CodecFactory.createDate();
        paylDate.day(21);
        paylDate.month(10);
        paylDate.year(1978);

        container.applyHasStandardData();
        container.setId(0);
        container.encodeInit(_encIter, null, 0);

        entry.clear();
        entry.dataType(DataTypes.DATE);
        entry.fieldId(1);
        entry.encode(_encIter, paylDate);

        entry.clear();
        entry.fieldId(2);
        entry.encodeInit(_encIter, 100);

        container1.applyHasStandardData();
        container1.encodeInit(_encIter, null, 0);
        entry1.clear();
        entry1.fieldId(3);
        entry1.dataType(DataTypes.REAL);
        entry1.encode(_encIter, paylReal);
        container1.encodeComplete(_encIter, true);
        entry.encodeComplete(_encIter, true);

        entry.clear();
        entry.fieldId(4);
        entry.dataType(DataTypes.DATE);
        entry.encode(_encIter, paylDate);

        container.encodeComplete(_encIter, true);

        // now decode
        _buffer.data().flip();
        _buffer.data(_buffer.data(), 0, _buffer.data().limit());

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        // reuse the containers
        container.clear();
        container.decode(_decIter, null);

        entry.clear();
        entry1.clear();
        while (entry.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if ((entry.fieldId() == 1) || (entry.fieldId() == 4))
            {
                paylDate.clear();
                paylDate.decode(_decIter);
            }
            else if (entry.fieldId() == 2)
            {
                container1.clear();
                container1.decode(_decIter, null);

                entry1.clear();
                while (entry1.decode(_decIter) != CodecReturnCodes.END_OF_CONTAINER)
                {
                    if (entry1.fieldId() == 3)
                    {
                        paylReal.clear();
                        paylReal.decode(_decIter);
                    }
                }
            }
            entry.clear();
        }
    }

    private void encodeNestedStructure(int type, int nested)
    {
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        UInt key = CodecFactory.createUInt();
        key.value(nested);

        if (nested == 0)
        {
            encodeFieldList();
        }
        else
        {
            switch (type)
            {
                case DataTypes.FIELD_LIST:
                    fieldList.applyHasStandardData();
                    fieldList.encodeInit(_encIter, null, 0);
                    fieldEntry.clear();
                    fieldEntry.fieldId(2);
                    fieldEntry.encodeInit(_encIter, 100);
                    encodeNestedStructure(DataTypes.FIELD_LIST, nested - 1);
                    fieldEntry.encodeComplete(_encIter, true);
                    fieldList.encodeComplete(_encIter, true);
                    break;
                case DataTypes.MAP:
                    map.containerType(DataTypes.MAP);
                    if (nested == 1)
                        map.containerType(DataTypes.FIELD_LIST);
                    map.keyPrimitiveType(DataTypes.UINT);
                    map.encodeInit(_encIter, 0, 0);
                    mapEntry.clear();
                    mapEntry.action(MapEntryActions.ADD);
                    key.value(nested * 2);
                    mapEntry.encodeInit(_encIter, key, 100);
                    encodeNestedStructure(DataTypes.MAP, nested - 1);
                    mapEntry.encodeComplete(_encIter, true);
                    map.encodeComplete(_encIter, true);
                    break;
                case DataTypes.FILTER_LIST:
                    filterList.containerType(DataTypes.MAP);
                    filterList.encodeInit(_encIter);
                    filterEntry.clear();
                    filterEntry.applyHasContainerType();
                    if (nested == 1)
                        filterEntry.containerType(DataTypes.FIELD_LIST);
                    else
                        filterEntry.containerType(DataTypes.FILTER_LIST);
                    filterEntry.action(FilterEntryActions.SET);
                    filterEntry.id(nested + 1);
                    filterEntry.encodeInit(_encIter, 100);
                    encodeNestedStructure(DataTypes.FILTER_LIST, nested - 1);
                    filterEntry.encodeComplete(_encIter, true);
                    filterList.encodeComplete(_encIter, true);
                    break;
                default:
                    System.out.println("not implemented ");
            }
        }

    }

    private void decodeNestedStructure(int type, int nested)
    {
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        UInt key = CodecFactory.createUInt();
        key.value(nested);

        if (nested == 0)
        {
            decodeFieldList();
        }
        else
        {
            switch (type)
            {
                case DataTypes.FIELD_LIST:
                    fieldList.decode(_decIter, null);
                    fieldList.checkHasStandardData();
                    fieldEntry.clear();
                    fieldEntry.decode(_decIter);
                    decodeNestedStructure(DataTypes.FIELD_LIST, nested - 1);
                    break;
                case DataTypes.MAP:
                    map.decode(_decIter);
                    if (nested == 1)
                    {
                        //
                    }
                    else
                    {
                        //
                    }
                    ;
                    mapEntry.clear();
                    mapEntry.decode(_decIter, key);
                    decodeNestedStructure(DataTypes.MAP, nested - 1);
                    break;
                case DataTypes.FILTER_LIST:
                    filterList.decode(_decIter);
                    filterEntry.clear();
                    filterEntry.decode(_decIter);
                    if (nested == 1)
                    {
                        //
                    }
                    else
                    {
                        //
                    }
                    decodeNestedStructure(DataTypes.FILTER_LIST, nested - 1);
                    break;
                default:
                    System.out.println("not implemented ");
            }
        }

    }

    public void nestedEncDecTest()
    {
        int nested = 5;
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(200));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        // TODO add for rrg
        // Vector vector = CodecFactory.createVector();
        // Series series = CodecFactory.createSeries();

        for (int i = 0; i < nested; i++)
        {
            _encIter.clear();
            _buffer.data(ByteBuffer.allocate(200));
            _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            encodeNestedStructure(DataTypes.FIELD_LIST, nested);
            // now decode
            _buffer.data().flip();
            _buffer.data(_buffer.data(), 0, _buffer.data().limit());

            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            decodeNestedStructure(DataTypes.FIELD_LIST, nested);

            _encIter.clear();
            _buffer.data(ByteBuffer.allocate(200));
            _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            encodeNestedStructure(DataTypes.MAP, nested);
            // now decode
            _buffer.data().flip();
            _buffer.data(_buffer.data(), 0, _buffer.data().limit());

            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            decodeNestedStructure(DataTypes.MAP, nested);

            _encIter.clear();
            _buffer.data(ByteBuffer.allocate(200));
            _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            encodeNestedStructure(DataTypes.FILTER_LIST, nested);
            // now decode
            _buffer.data().flip();
            _buffer.data(_buffer.data(), 0, _buffer.data().limit());

            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            decodeNestedStructure(DataTypes.FILTER_LIST, nested);

            // TODO implement for rrg
            // _encIter.clear();
            // _buffer.data(ByteBuffer.allocate(200));
            // _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
            // Codec.minorVersion());
            //
            // encodeNestedStructure(DataTypes.VECTOR, nested);
            // // now decode
            // _buffer.data().flip();
            // _buffer.data(_buffer.data(), 0, _buffer.data().limit());
            //
            // _decIter.clear();
            // _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
            // Codec.minorVersion());
            //
            // decodeNestedStructure(DataTypes.VECTOR, nested);
            //
            //
            // _encIter.clear();
            // _buffer.data(ByteBuffer.allocate(200));
            // _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
            // Codec.minorVersion());
            //
            // encodeNestedStructure(DataTypes.SERIES, nested);
            // // now decode
            // _buffer.data().flip();
            // _buffer.data(_buffer.data(), 0, _buffer.data().limit());
            //
            // _decIter.clear();
            // _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(),
            // Codec.minorVersion());
            //
            // decodeNestedStructure(DataTypes.SERIES, nested);
        }

    }

    private static void addCommandLineArgs()
    {
        CommandLine.programName("ContainersPerf");
        CommandLine.addOption("nl", defaultNumOfLoops, "The number of loops. Default is:'"
                + defaultNumOfLoops + "'");
    }

    /**
     * Initializes consumer application.
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments.
     * 
     * @param args
     */
    public void init(String[] args)
    {
        /* process command line args */
        addCommandLineArgs();
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }
    }

    public static void main(String[] args)
    {
        ContainersPerf containersPerf = new ContainersPerf();
        containersPerf.init(args);
        containersPerf.run(args);
    }
}