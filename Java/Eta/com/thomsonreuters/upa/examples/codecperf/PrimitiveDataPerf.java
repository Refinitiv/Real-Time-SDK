package com.thomsonreuters.upa.examples.codecperf;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Double;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.examples.common.CommandLine;

/**
 * <p>
 * This is a main class to run UPA Primitive Data Perf application. The purpose
 * of this application is to benchmark/measuring performance for various UPA
 * primitive data. It is a single-threaded client application. Sample UPA
 * primitive data include Buffer, Date, Enum, Int, Real,
 * Time, UInt, Qos, State for rrg: DateTime, Double,
 * Float
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
public class PrimitiveDataPerf
{
    /* PrimitiveData Perf testing default number of loops */
    private static final int defaultNumOfLoops = 1000000;
 
    // Buffer, Date, Enum, Int, Real, Time, UInt,
    // Qos, State, DateTime, Double, Float

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
            uIntEncodeDecodeTest();
        }
        System.out.println("finished UintTest ");

        for (int i = 0; i < loop; i++)
        {
            int64EncodeDecodeTest();
        }
        System.out.println("finished IntTest ");

        for (int i = 0; i < loop; i++)
        {
            dateEncodeDecodeTest();
        }
        System.out.println("finished dateTest ");
                       
        for (int i = 0; i < loop; i++)
        {
            dateEncodeDecodeBlankTest();
        }
        System.out.println("finished dateBlankTest ");

        for (int i = 0; i < loop; i++)
        {
            timeEncodeDecodeTest();
        }
        System.out.println("finished TimeTest ");

        
        for (int i = 0; i < loop; i++)
        {
            timeEncodeDecodeBlankTest();
        }
        System.out.println("finished timeBlankTest ");

        for (int i = 0; i < loop; i++)
        {
            dateTimeEncodeDecodeTest();
        }
        System.out.println("finished DateTimeTest ");
        
        for (int i = 0; i < loop; i++)
        {
            dateTimeEncodeDecodeBlankTest();
        }
        System.out.println("finished dateTimeBlankTest ");        
                
        for (int i = 0; i < loop; i++)
        {
            enumEncodeDecodeTest();
        }
        System.out.println("finished enumTest ");

        for (int i = 0; i < loop; i++)
        {
            realEncodeDecodeTest();
        }
        System.out.println("finished realTest ");

        for (int i = 0; i < loop; i++)
        {
            realEncodeDecodeBlankTest();
        }
        System.out.println("finished realBlankTest ");
        
        for (int i = 0; i < loop; i++)
        {
            floatEncodeDecodeTest();
        }
        System.out.println("finished FloatTest ");

        for (int i = 0; i < loop; i++)
        {
            floatEncodeDecodeBlankTest();
        }
        System.out.println("finished FloatBlankTest ");
        
        for (int i = 0; i < loop; i++)
        {
            doubleEncodeDecodeTest();
        }
        System.out.println("finished DoubleTest ");

        for (int i = 0; i < loop; i++)
        {
            doubleEncodeDecodeBlankTest();
        }
        System.out.println("finished DoubleBlankTest ");        

        for (int i = 0; i < loop; i++)
        {
            qosEncodeDecodeTest();
        }
        System.out.println("finished QosTest ");

        for (int i = 0; i < loop; i++)
        {
            stateEncodeDecodeTest();
        }
        System.out.println("finished StateTest ");

        for (int i = 0; i < loop; i++)
        {
            bufferEncodeDecodeTest();
        }
        System.out.println("finished bufferTest ");

        long stopTime = System.currentTimeMillis();
        long diff = stopTime - startTime;
        System.out.println("loops " + loop + ", primitive data encoding/decoding time " + diff);
    }

    private void uintED(UInt uint)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        uint.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        UInt uint1 = CodecFactory.createUInt();
        uint1.decode(_decIter);
    }

    public void uIntEncodeDecodeTest()
    {
        UInt uint = CodecFactory.createUInt();
        long val;

        /* leading 0's */
        for (val = 0x5555555555555555l; val != 0; val >>= 1)
        {
            uint.value(val);
            uintED(uint);
        }

        for (val = 0xAAAAAAAAAAAAAAAAl; val != -1; val >>= 1)
        {
            uint.value(val);
            uintED(uint);
        }

        /* trailing 0's */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
        {
            uint.value(val);
            uintED(uint);
        }

        /* Highest hex number */
        val = 0xFFFFFFFFFFFFFFFFl;
        uint.value(val);
        uintED(uint);

        /* 0 */
        val = 0;
        uint.value(val);
        uintED(uint);
    }

    private void intED(Int intv)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        intv.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Int int1 = CodecFactory.createInt();
        int1.decode(_decIter);
    }

    public void int64EncodeDecodeTest()
    {
        Int intv = CodecFactory.createInt();
        long val;

        /* leading 0's */
        for (val = 0x5555555555555555l; val != 0; val >>= 1)
        {
            intv.value(val);
            intED(intv);
        }

        /* Negative numbers with leading 1's */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0xFFFFFFFFFFFFFFFFl; val >>= 1)
        {
            intv.value(val);
            intED(intv);
        }

        /* Numbers with trailing zeros */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
        {
            intv.value(val);
            intED(intv);
        }

        /* Highest hex number */
        val = 0xFFFFFFFFFFFFFFFFl;
        intv.value(val);
        intED(intv);

        /* 0 */
        val = 0;
        intv.value(val);
        intED(intv);

    }

    private void dateED(Date date)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        date.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Date date1 = CodecFactory.createDate();
        date1.decode(_decIter);
    }

    public void dateEncodeDecodeTest()
    {
        Date date = CodecFactory.createDate();

        for (int i = 4095; i != 0; i >>= 1)
        {
            date.year(i);
            for (int j = 0; j <= 12; ++j)
            {
                date.month(j);
                for (int k = 0; k <= 31; ++k)
                    dateED(date);
            }
        }
    }

    public void dateEncodeDecodeBlankTest()
    {
        Date date = CodecFactory.createDate();

        date.blank();
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        date.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Date date1 = CodecFactory.createDate();
        date1.decode(_decIter);
    }

    private void timeED(Time time)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        time.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Time time1 = CodecFactory.createTime();
        time1.decode(_decIter);
    }

    public void timeEncodeDecodeTest()
    {
        Time time = CodecFactory.createTime();

        for (int hour = 23; hour >= 0; hour = hour - 2)
        {
            time.hour(hour);
            for (int min = 59; min >= 0; min = min - 10)
            {
                time.minute(min);
                for (int sec = 59; sec >= 0; sec = sec - 10)
                {
                    time.second(sec);
                    for (int msec = 59; msec >= 0; msec = msec - 10)
                    {
                        time.millisecond(msec);
                        timeED(time);
                    }
                }
            }
        }
    }

    public void timeEncodeDecodeBlankTest()
    {
        Time time = CodecFactory.createTime();

        time.blank();
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        time.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Time time1 = CodecFactory.createTime();
        time1.decode(_decIter);
    }


    private void dateTimeED(DateTime dateTime)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        dateTime.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        DateTime dateTime1 = CodecFactory.createDateTime();
        dateTime1.decode(_decIter);
    }

    public void dateTimeEncodeDecodeTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();

        for (int i = 2047; i != 0; i >>= 1)
        {
            dateTime.year(i);
            for (int j = 0; j <= 12; ++j)
            {
                dateTime.month(j);        
        
                for (int k = 0; k <= 27; ++k)
                	dateTime.day(k);
                                                  
                	for (int hour = 23; hour >= 0; hour = hour - 2)
                	{
                		dateTime.hour(hour);
                		for (int min = 59; min >= 0; min = min - 10)
                		{
                			dateTime.minute(min);
                			for (int sec = 59; sec >= 0; sec = sec - 10)
                			{
                				dateTime.second(sec);
                				for (int msec = 59; msec >= 0; msec = msec - 10)
                				{
                					dateTime.millisecond(msec);
                					dateTimeED(dateTime);
                				}
                			}
                		}
                	}
            }
        }
    }

    public void dateTimeEncodeDecodeBlankTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();

        dateTime.blank();
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        dateTime.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        DateTime dateTime1 = CodecFactory.createDateTime();
        dateTime1.decode(_decIter);
    }


    
    
    
    public void enumEncodeDecodeTest()
    {
        Enum enumv = CodecFactory.createEnum();

        for (short e = 0; e <= 0xFF; e++)
        {
            enumv.value(e);

            _encIter.clear();
            _buffer.data(ByteBuffer.allocate(15));
            _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            enumv.encode(_encIter);

            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            Enum enumv1 = CodecFactory.createEnum();
            enumv1.decode(_decIter);
        }
    }

    private void realED(Real real)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Real real1 = CodecFactory.createReal();
        real1.decode(_decIter);
    }

    public void realEncodeDecodeTest()
    {
        Real real = CodecFactory.createReal();

        for (int hint = 0; hint <= 30; ++hint)
        {
            long val;
            /* leading 0's */
            for (val = 0x5555555555555555l; val != 0; val >>= 1)
            {
                real.value(val, hint);
                realED(real);
            }

            for (val = 0xAAAAAAAAAAAAAAAAl; val != -1; val >>= 1)
            {
                real.value(val, hint);
                realED(real);
            }

            /* trailing 0's */
            for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
            {
                real.value(val, hint);
                realED(real);
            }

            real.value(0xFFFFFFFFFFFFFFFFl, hint);
            realED(real);

            real.value(0, hint);
            realED(real);
        }
    }

    public void realEncodeDecodeBlankTest()
    {
        Real real = CodecFactory.createReal();

        real.blank();
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Real real1 = CodecFactory.createReal();
        real1.decode(_decIter);
    }

    private void floatED(Float floatv)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        floatv.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Float float1 = CodecFactory.createFloat();
        float1.decode(_decIter);
    }

    public void floatEncodeDecodeTest()
    {
        Float floatv = CodecFactory.createFloat();

        long val;
        /* leading 0's */
        for (val = 0x5555555555555555l; val != 0; val >>= 1)
        {
        	floatv.value(val);
            floatED(floatv);
        }

        for (val = 0xAAAAAAAAAAAAAAAAl; val != -1; val >>= 1)
        {
        	floatv.value(val);
            floatED(floatv);
        }

        /* trailing 0's */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
        {
        	floatv.value(val);
            floatED(floatv);
        }

        floatv.value(0xFFFFFFFFFFFFFFFFl);
        floatED(floatv);

        floatv.value(0);
        floatED(floatv);
    }

    public void floatEncodeDecodeBlankTest()
    {
        Float floatv = CodecFactory.createFloat();
        floatv.blank();
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        floatv.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Float float1 = CodecFactory.createFloat();
        float1.decode(_decIter);
    }
  
// AYX
    private void doubleED(Double doublev)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        doublev.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Double double1 = CodecFactory.createDouble();
        double1.decode(_decIter);
    }

    public void doubleEncodeDecodeTest()
    {
        Double doublev = CodecFactory.createDouble();

        long val;
        /* leading 0's */
        for (val = 0x5555555555555555l; val != 0; val >>= 1)
        {
        	doublev.value(val);
            doubleED(doublev);
        }

        for (val = 0xAAAAAAAAAAAAAAAAl; val != -1; val >>= 1)
        {
        	doublev.value(val);
            doubleED(doublev);
        }

        /* trailing 0's */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
        {
        	doublev.value(val);
            doubleED(doublev);
        }

        doublev.value(0xFFFFFFFFFFFFFFFFl);
        doubleED(doublev);

        doublev.value(0);
        doubleED(doublev);
    }

    public void doubleEncodeDecodeBlankTest()
    {
        Double doublev = CodecFactory.createDouble();
        doublev.blank();
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        doublev.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Double double1 = CodecFactory.createDouble();
        double1.decode(_decIter);
    }
  
        
    
    
    private void qosED(Qos qos)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        qos.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Qos qos1 = CodecFactory.createQos();
        qos1.decode(_decIter);
    }

    public void qosEncodeDecodeTest()
    {
        int[] rates = { QosRates.UNSPECIFIED, QosRates.TICK_BY_TICK, QosRates.JIT_CONFLATED,
                QosRates.TIME_CONFLATED };
        int[] timeliness = { QosTimeliness.UNSPECIFIED, QosTimeliness.REALTIME,
                QosTimeliness.DELAYED_UNKNOWN, QosTimeliness.DELAYED };
        Qos qos = CodecFactory.createQos();
        int rateInfo = 0;
        int timeInfo = 0;

        for (int rate = 0; rate < rates.length; rate++)
        {
            for (int tm = 0; tm < timeliness.length; tm++)
            {
                if (rates[rate] == QosRates.TIME_CONFLATED)
                {
                    for (rateInfo = 0x007f; rateInfo == 0; rateInfo >>= 1)
                    {
                        if (timeliness[tm] == QosTimeliness.DELAYED)
                        {
                            for (timeInfo = 0x007f; timeInfo == 0; timeInfo >>= 1)
                            {
                                qos.rate(rates[rate]);
                                qos.timeliness(timeliness[tm]);
                                qos.rateInfo(rateInfo);
                                qos.timeInfo(timeInfo);
                                qos.dynamic(false);
                                qosED(qos);

                                qos.rate(rates[rate]);
                                qos.timeliness(timeliness[tm]);
                                qos.rateInfo(rateInfo);
                                qos.timeInfo(timeInfo);
                                qos.dynamic(true);
                                qosED(qos);
                            }
                        }
                    }
                }
            }
        }
    }

    private void stateED(State state)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        state.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        State state1 = CodecFactory.createState();
        state1.decode(_decIter);
    }

    public void stateEncodeDecodeTest()
    {
        int[] ss = { StreamStates.UNSPECIFIED, StreamStates.OPEN, StreamStates.NON_STREAMING,
                StreamStates.CLOSED_RECOVER, StreamStates.CLOSED, StreamStates.REDIRECTED };
        int[] ds = { DataStates.NO_CHANGE, DataStates.OK, DataStates.SUSPECT };

        State state = CodecFactory.createState();
        int code; // -34 - 19
        Buffer s1 = CodecFactory.createBuffer();
        s1.data("abc");
        String s2 = "qwertyu";
        Buffer s = CodecFactory.createBuffer();
        s.data(s2);

        for (int i = 0; i < ss.length; i++)
        {
            for (int j = 0; j < ds.length; j++)
            {
                for (code = -34; code == 19; code++)
                {
                    state.code(code);
                    state.dataState(ds[j]);
                    state.streamState(ss[i]);
                    stateED(state);

                    state.text(s1);
                    stateED(state);

                    state.text(s);
                    stateED(state);
                }
            }
        }
    }

    private void bufferED(Buffer buf)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        buf.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Buffer buf1 = CodecFactory.createBuffer();
        buf1.data(ByteBuffer.allocate(15));
        buf1.decode(_decIter);
    }

    public void bufferEncodeDecodeTest()
    {
        byte[] data = { 0, 1, 2, 3, 4, 5, 6, 7 };
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.wrap(data));
        bufferED(buf);

        byte[] data1 = { 0, 1, 2 };
        buf.data(ByteBuffer.wrap(data1));
        bufferED(buf);

        buf = CodecFactory.createBuffer();
        buf.data("abhytgcdty");
        bufferED(buf);

        buf = CodecFactory.createBuffer();
        buf.data("abhytgcdty");
        bufferED(buf);
    }

    private static void addCommandLineArgs()
    {
        CommandLine.programName("PrimitiveDataPerf");
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
        PrimitiveDataPerf primitiveDataPerf = new PrimitiveDataPerf();
        primitiveDataPerf.init(args);
        primitiveDataPerf.run(args);
    }
}
