package com.refinitiv.eta.examples.codec;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;

/**
 * This example is intended to serve as a high-level, basic demonstration of
 * encoding and decoding using the UPA API. The user can set break points and
 * step through this code to get a look and feel for how to use the API.
 * <p>
 * <em>Summary</em>
 * <p>
 * Encode/decode examples for all containers (ElementList, FieldList,
 * FilterList, Map, Message, Series, Vector) are provided. There are two Message
 * examples that show how to encode/decode a nested Map or Series inside the
 * message.
 * <p>
 * This application is written as a basic example. Some of the techniques used
 * can be modified for best performance as needed.
 * <p>
 * Because both encode and decode sides know what was encoded, some checking is
 * not occurring. For safety, message and container masks should be checked
 * before accessing members.
 * <ul>
 * <li>FieldList encode/decode examples are located in FieldListCodec.java.
 * <li>ElementList encode/decode examples are located in ElementListCodec.java.
 * <li>Series encode/decode examples are located in SeriesCodec.java.
 * <li>Map encode/decode examples are located in MapCodec.java.
 * <li>Message encode/decode examples are located in MsgCodec.java.
 * <li>FilterList encode/decode examples are located in FilterListCodec.java.
 * <li>Vector encode/decode examples are located in VectorCodec.java.
 * </ul>
 * <p>
 * <em>Setup Environment</em>
 * <p>
 * No special setup is required.
 * <p>
 * <em>Running the application:</em>
 * <p>
 * This example is intended to run within an IDE but can also be ran
 * stand-alone.
 * <p>
 * Change directory to the <i>Java</i> directory and issue the following <i>Gradle</i> command.
 * <p>
 * Linux: ./gradlew runCodecExample<br>
 * Windows: gradlew.bat runCodecExample
 */
public class CodecExample
{	
    public static void main(String[] args)
    {
        /* display product version information */
        System.out.println(Codec.queryVersion().toString() + "\nCodecExample starting ...");
    	CodecExample codecExample = new CodecExample();
    	codecExample.run();
    }
	
	int run()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* perform the example for encoding/decoding a FieldList Container Type */
		if ((retVal = exampleEncodeDecodeFieldList()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a FieldList Container Type!\n");
			return retVal;
		}
		
		/* perform the example for encoding/decoding a Map Container Type containing nested FieldLists */
		if ((retVal = exampleEncodeDecodeMap()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a Map Container Type!\n");
			return retVal;
		}
	
		/* perform the example for encoding/decoding a RefreshMsg message containing a nested Map */
		if ((retVal = exampleEncodeDecodeMsgWithMap()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a RefreshMsg message containing a Map!\n");
			return retVal;
		}
		/* perform the example for encoding/decoding a ElementList Container Type */
		if ((retVal = exampleEncodeDecodeElementList()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a ElementList Container Type!\n");
			return retVal;
		}
	
		/* perform the example for encoding/decoding a Series Container Type containing a nested ElementList */
		if ((retVal = exampleEncodeDecodeSeries()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a Series Container Type!\n");
			return retVal;
		}
	
		/* perform the example for encoding/decoding a Vector Container Type containing a nested ElementList */
		if ((retVal = exampleEncodeDecodeVector()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a Vector Container Type!\n");
			return retVal;
		}
	
		/* perform the example for encoding/decoding a FilterList Container Type containing a nested ElementList */
		if ((retVal = exampleEncodeDecodeFilterList()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a FilterList Container Type!\n");
			return retVal;
		}

		/* perform the example for encoding/decoding a RefreshMsg message containing a nested Series */
		if ((retVal = exampleEncodeDecodeMsgWithSeries()) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding/decoding a RefreshMsg message containing a Series!\n");
			return retVal;
		}
	
		return CodecReturnCodes.SUCCESS;
	}

	/* perform the example for encoding/decoding a FieldList Container Type */
	int exampleEncodeDecodeFieldList()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
	
		/* perform the example for encoding a FieldList Container Type */
		/* We pass in the buffer to this method with the 
		   total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer
		   for the encDecBuffer.length member */
		if ((retVal = exampleEncodeFieldList(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a FieldList Container Type!\n");
			return retVal;
		}
		
		/* perform the example for decoding a FieldList Container Type */
		/* We will pass the same buffer directly to the 
		   decode method which will then decode it */
		if ((retVal = exampleDecodeFieldList(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a FieldList Container Type!\n");
			return retVal;
		}
	
		return CodecReturnCodes.SUCCESS;
	}
	
	
	/* perform the example for encoding/decoding a Map Container Type */
	int exampleEncodeDecodeMap()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a Map Container Type */
		/* We pass in the buffer to this method with the 
		   total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer
		   for the encDecBuffer.length member */
		if ((retVal = exampleEncodeMap(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a Map Container Type!\n");
			return retVal;
		}
		
		/* perform the example for decoding a Map Container Type */
		/* We will pass the same buffer directly to the 
		   decode method which will then decode it */
		if ((retVal = exampleDecodeMap(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a Map Container Type!\n");
			return retVal;
		}
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* perform the example for encoding/decoding a ElementList Container Type */
	int exampleEncodeDecodeElementList()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a ElementList Container Type */
		/* We pass in the buffer to this method with the 
		   total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer
		   for the encDecBuffer.length member */
		if ((retVal = exampleEncodeElementList(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a Element Container Type!\n");
			return retVal;
		}
		
		/* perform the example for decoding a ElementList Container Type */
		/* We will pass the same buffer directly to the decode method which will then decode it */
		if ((retVal = exampleDecodeElementList(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a ElementList Container Type!\n");
			return retVal;
		}
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* perform the example for encoding/decoding a Series Container Type */
	int exampleEncodeDecodeSeries()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a Series Container Type */
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = exampleEncodeSeries(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a Series Container Type!\n");
			return retVal;
		}
		
		/* perform the example for decoding a Series Container Type */
		/* We will pass the same buffer directly to the decode method which will then decode it */
		if ((retVal = exampleDecodeSeries(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a Series Container Type!\n");
			return retVal;
		}
		return CodecReturnCodes.SUCCESS;
	}
	
	/* perform the example for encoding/decoding a Vector Container Type */
	int exampleEncodeDecodeVector()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a Vector Container Type */
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = exampleEncodeVector(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a Vector Container Type!\n");
			return retVal;
		}
		
		/* perform the example for decoding a Vector Container Type */
		/* We will pass the same buffer directly to the decode method which will then decode it */
		if ((retVal = exampleDecodeVector(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a Vector Container Type!\n");
			return retVal;
		}
		return CodecReturnCodes.SUCCESS;
	}
	
	/* perform the example for encoding/decoding a FilterList Container Type */
	int exampleEncodeDecodeFilterList()
	{
		int retVal;  //used to store and check the return value from the method calls
		
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a FilterList Container Type */
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = exampleEncodeFilterList(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a FilterList Container Type!\n");
			return retVal;
		}
		
		/* perform the example for decoding a FilterList Container Type */
		/* We will pass the same buffer directly to the decode method which will then decode it */
		if ((retVal = exampleDecodeFilterList(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a FilterList Container Type!\n");
			return retVal;
		}
		return CodecReturnCodes.SUCCESS;
	}
	
	/* perform the example for encoding/decoding a RefreshMsg message with a Map */
	int exampleEncodeDecodeMsgWithMap()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a RefreshMsg simple message */
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = exampleEncodeMsgWithMap(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a RefreshMsg with a map!\n");
			return retVal;
		}
		
		/* perform the example for decoding a RefreshMsg simple message */
		/* We will pass the same buffer directly to the decode method which will then decode it */
		if ((retVal = exampleDecodeMsgWithMap(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a RefreshMsg message with map!\n");
			return retVal;
		}
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* perform the example for encoding/decoding a RefreshMsg message with a Series */
	int exampleEncodeDecodeMsgWithSeries()
	{
		int retVal;  //used to store and check the return value from the method calls
	
		/* create a Buffer to encode into */
		/* Typically, for performance, the transport layer can provide a pool of buffers
		 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
		/* For this example I am heap allocating the buffer (500 bytes is large enough for
		 * the message I am encoding). */
		Buffer encDecBuffer = CodecFactory.createBuffer();
		encDecBuffer.data(ByteBuffer.allocate(500));
		
		/* perform the example for encoding a RefreshMsg simple message */
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
		   it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = exampleEncodeMsgWithSeries(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for encoding a RefreshMsg message with Series!\n");
			return retVal;
		}
		
		/* perform the example for decoding a RefreshMsg simple message */
		/* We will pass the same buffer directly to the decode method which will then decode it */
		if ((retVal = exampleDecodeMsgWithSeries(encDecBuffer)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error running the example for decoding a RefreshMsg with a Series!\n");
			return retVal;
		}
	
		return CodecReturnCodes.SUCCESS;
	}
	
	
	/* Example for encoding a FieldList Container Type. This method will
	 * encode a basic field list with several primitives embedded in it. */
	int exampleEncodeFieldList(Buffer encBuf)
	{
		FieldListCodec fieldListCodec = new FieldListCodec(); // the field list codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the simple message */
		System.out.printf("\nBegin UPA Fieldlist Encoding Example\n");
	
		if ((retVal = fieldListCodec.exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			
			System.out.printf("Error encoding field list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("\nUPA Fieldlist Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for encoding a Map Container Type. This method will
	 * encode a basic Map which contains nested field lists. */
	int exampleEncodeMap(Buffer encBuf)
	{
		MapCodec mapCodec = new MapCodec(); // the map codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the simple message */
		System.out.printf("\nBegin UPA Map Encoding Example\n");
	
		if ((retVal = mapCodec.exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding map.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeMap.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Map Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for encoding a ElementList Container Type. This method will
	 * encode a basic ElementList which contains nested field lists. */
	int exampleEncodeElementList(Buffer encBuf)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the simple message */
		System.out.printf("\nBegin UPA ElementList Encoding Example\n");
		
		System.out.printf("\tElementList Encoding Begin\n");
	
		if ((retVal = elementListCodec.exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding ElementList.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeElementList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("\nUPA ElementList Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for encoding a Series Container Type. This method will
	 * encode a basic Series which contains nested field lists. */
	int exampleEncodeSeries(Buffer encBuf)
	{
		SeriesCodec seriesCodec = new SeriesCodec(); // the series codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the simple message */
		System.out.printf("\nBegin UPA Series Encoding Example\n");
	
		if ((retVal = seriesCodec.exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding Series.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeSeries.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Series Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for encoding a Vector Container Type. This method will
	 * encode a basic Vector which contains nested element lists. */
	int exampleEncodeVector(Buffer encBuf)
	{
		VectorCodec vectorCodec = new VectorCodec();  // the vector codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the simple message */
		System.out.printf("\nBegin UPA Vector Encoding Example\n");
	
		if ((retVal = vectorCodec.exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding Vector.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeVector.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Vector Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for encoding a FilterList Container Type. This method will
	 * encode a basic FilterList which contains 2 nested element list and
	 * a field list. */
	int exampleEncodeFilterList(Buffer encBuf)
	{
		FilterListCodec filterListCodec = new FilterListCodec(); // the filter list codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the simple message */
		System.out.printf("\nBegin UPA FilterList Encoding Example\n");
	
		if ((retVal = filterListCodec.exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding FilterList.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeFilterList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA FilterList Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	
	/* Example for encoding a RefreshMsg with map. This method will
	 * encode a simple RefreshMsg message which contains a MAP containerType. */
	int exampleEncodeMsgWithMap(Buffer encBuf)
	{
		MsgCodec msgCodec = new MsgCodec(); // the message codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the message */
		System.out.printf("\nBegin RefreshMsg message with Map Encoding Example\n");
	
		if ((retVal = msgCodec.exampleEncodeRefreshWithMap(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding map.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeMsg.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("RefreshMsg message with map Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for encoding a RefreshMsg with a series. This method will
	 * encode a simple RefreshMsg message which contains a SERIES containerType. */
	int exampleEncodeMsgWithSeries(Buffer encBuf)
	{
		MsgCodec msgCodec = new MsgCodec(); // the message codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encBuf.data() points to sufficient memory and encBuf.length()
		 * indicates number of bytes available in encBuf.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		/* Perform all content encoding now that iterator is prepared.  */
	
		/* Encode the message */
		System.out.printf("\nBegin RefreshMsg message with Series Encoding Example\n");
	
		if ((retVal = msgCodec.exampleEncodeRefreshWithSeries(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding series.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeMsgWithSeries.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("RefreshMsg message with series Encoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	
	/* Example for decoding a FieldList Container Type. This method will
	 * decode a basic field list with several primitives embedded in it. */
	int exampleDecodeFieldList(Buffer decBuf)
	{
		FieldListCodec fieldListCodec = new FieldListCodec(); // the field list codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both 
		   as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		/* Perform all content decoding now that iterator is prepared.  */
	
		/* Decode the FieldList */
		System.out.printf("\nBegin UPA Fieldlist Decoding Example\n");
	
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = fieldListCodec.exampleDecode(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding field list.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeFieldList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Fieldlist Decoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for decoding a Map Container Type. This method will
	 * decode a basic Map which contains nested field lists. */
	int exampleDecodeMap(Buffer decBuf)
	{
		MapCodec mapCodec = new MapCodec(); // the map codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both 
		   as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		/* Decode the Map */
		System.out.printf("\nBegin UPA Map Decoding Example\n");
	
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = mapCodec.exampleDecode(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding map.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeMap.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Map Decoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	
	/* Example for decoding a ElementList Container Type. This method will
	 * decode a basic ElementList which contains nested field lists. */
	int exampleDecodeElementList(Buffer decBuf)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both 
		   as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		/* Decode the ElementList */
		System.out.printf("\nBegin UPA ElementList Decoding Example\n");
	
		/* We pass in the buffer to this method with the 
	       total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer
	       for the encDecBuffer.length member */
		if ((retVal = elementListCodec.exampleDecode(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding ElementList.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeElementList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA ElementList Decoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for decoding a Series Container Type. This method will
	 * decode a basic Series which contains nested field lists. */
	int exampleDecodeSeries(Buffer decBuf)
	{
		SeriesCodec seriesCodec = new SeriesCodec(); // the series codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		System.out.printf("\nBegin UPA Series Decoding Example\n");
	
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = seriesCodec.exampleDecode(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding Series.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeSeries.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Series Decoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for decoding a Vector Container Type. This method will
	 * decode a basic Vector which contains nested element lists. */
	int exampleDecodeVector(Buffer decBuf)
	{
		VectorCodec vectorCodec = new VectorCodec(); // the vector codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		System.out.printf("\nBegin UPA Vector Decoding Example\n");
	
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = vectorCodec.exampleDecode(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding Vector.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeVector.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA Vector Decoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for decoding a FilterList Container Type. This method will
	 * decode a basic FilterList which contains 2 nested element list and
	 * a field list. */
	int exampleDecodeFilterList(Buffer decBuf)
	{
		FilterListCodec filterListCodec = new FilterListCodec(); // the filter list codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		System.out.printf("\nBegin UPA FilterList Decoding Example\n");
	
		/* We pass in the buffer to this method with the total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer for the encDecBuffer.length member */
		if ((retVal = filterListCodec.exampleDecode(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding FilterList.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeFilterList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("UPA FilterList Decoding Example Complete\n");
	
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for decoding a RefreshMsg message with a map. This method will
	 * decode a simple RefreshMsg message which contains a MAP containerType. */
	int exampleDecodeMsgWithMap(Buffer decBuf)
	{
		MsgCodec msgCodec = new MsgCodec(); // the message codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both 
		   as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		/* Decode the message with map */
		System.out.printf("\nBegin RefreshMsg message with Map Decoding Example\n");
	
		/* We pass in the buffer to this method with the 
	       total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer
	       for the encDecBuffer.length member */
		if ((retVal = msgCodec.exampleDecodeRefreshWithMap(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding map.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeMsg.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("RefreshMsg message with Map Decoding Example Complete\n");
		return CodecReturnCodes.SUCCESS;
	}
	
	/* Example for decoding a RefreshMsg message with a series. This method will
	 * decode a simple RefreshMsg message which contains a SERIES containerType. */
	int exampleDecodeMsgWithSeries(Buffer decBuf)
	{
		MsgCodec msgCodec = new MsgCodec(); // the message codec
		int retVal;  //used to store and check the return value from the method calls
	
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		/* create and initialize decode iterator */
		/* Note: Iterator must be initialized before decoding begins - 
		   Here, I will use the clear method later on when decoding begins.
		   Typically, one method would not perform both the encode and the decode,
		   but as there is no network in between this example will do both 
		   as a demonstration */
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
	
		/* clear the decode iterator with the clear method */
		decodeIter.clear();
	
		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that decBuf.data() points to encoded contents to decode */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = decodeIter.setBufferAndRWFVersion(decBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with SetDecodeIteratorBuffer.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		/* Perform all content decoding now that iterator is prepared.  */
	
		/* Decode the message with series */
		System.out.printf("\nBegin RefreshMsg message with Series Decoding Example\n");
	
		/* We pass in the buffer to this method with the 
	       total length available.  When the method finishes,
	       it will set the actual length encoded into the buffer
	       for the encDecBuffer.length member */
		if ((retVal = msgCodec.exampleDecodeRefreshWithSeries(decodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error decoding series.\n");
			System.out.printf("Error %s (%d) encountered with exampleDecodeMsgWithSeries.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
	
		System.out.printf("RefreshMsg message with Series Decoding Example Complete\n");
		return CodecReturnCodes.SUCCESS;
	}
}
