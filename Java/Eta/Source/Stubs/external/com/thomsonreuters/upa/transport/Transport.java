package com.thomsonreuters.upa.transport;

import java.net.*;
import java.nio.ByteBuffer;



/**
 * UPA transport is used by an OMM Interactive Provider to create listening
 * sockets and by OMM consumer and NIP applications to establish outbound
 * connections to a listening socket.
 */
public class Transport
{


    /**
     * Initializes the UPA transport API and all internal members.<BR>
     * 
     * This is the first method called when using the UPA. It initializes
     * internal data structures.
     * 
     * @param initArgs Arguments for initialize
     * @param error UPA Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see InitArgs
     */
    public static int initialize(InitArgs initArgs, Error error)
    {
       return TransportReturnCodes.FAILURE;
        
    }

    /**
     * Uninitializes the UPA API and all internal members.<BR>
     * 
     * This is the last method called by an application when using the UPA. 
     * If multiple threads call initialize() on Transport, they have to
     * call uninitialize() when the thread finishes. The last uninitialize()
     * call releases all internally pooled resources to GC.
     * 
     * @return {@link TransportReturnCodes}
     */
    public static int uninitialize()
    {
    	return TransportReturnCodes.FAILURE;
    }

    /**
     * Gets the IP address of a hostname.<BR>
     * 
     * This method gets the IP address of a hostname in host byte order.
     * 
     * @param hostName Hostname to get IP address for
     * 
     * @return The IP address
     */
    public static InetSocketAddress hostByName(String hostName)
    {
        return null;
    }

    /**
     * Gets the user name.<BR>
     * 
     * This method gets the user name that the owner of the current process is
     * logged in under.
     * 
     * @return User name of user
     */
    public static String userName()
    {
        return System.getProperty("user.name");
    }

    /**
     * Initialize transport defined in opts if not initialized.<BR>
     * 
     * Connects a client to a listening server.<BR>
     * 
     * 1. Initialize ConnectOptions<BR>
     * 2. Set ConnectOptions to desired values<BR>
     * 3. Call connect to create Channel<BR>
     * 4. Read or write with the Channel<BR>
     * 
     * @param opts Options used when connecting
     * @param error UPA Error, to be populated in event of an error
     * 
     * @return Connected UPA channel or NULL
     * 
     * @see ConnectOptions
     * @see Channel
     */
    public static Channel connect(ConnectOptions opts, Error error)
    {
       return null;
    }

    /**
     * Creates a UPA Server by binding to a port.<BR>
     * 
     * 1. Initialize BindOptions<BR>
     * 2. Set BindOptions to desired values<BR>
     * 3. Call bind to create {@link Server}<BR>
     * 
     * @param opts Options used when binding
     * @param error UPA Error, to be populated in event of an error
     * 
     * @return Bound UPA server or NULL
     * 
     * @see BindOptions
     * @see Server
     */
    public static Server bind(BindOptions opts, Error error)
    {
      return null;
    }

    /**
     * Programmatically extracts library and product version information that is
     * compiled into this library.<BR>
     * 
     * User can call this method to programmatically extract version
     * information.<BR>
     * 
     * @see LibraryVersionInfo
     */
    public static LibraryVersionInfo queryVersion()
    {
        return null;
    }

    /**
     * <b>WARNING: creates garbage</b> Returns a hex representation of the data
     * in the provided {@link ByteBuffer}
     * 
     * @param buffer Contains the data to print
     * @param startPosition The starting position of the data in the buffer
     * @param length The length of the data
     * @return String the hex representation of the data.
     */
    public static String toHexString(ByteBuffer buffer, final int startPosition, final int length)
    {
        assert (buffer != null) : "buffer cannot be null";

        final int charsPerLine = 16;
        StringBuilder asString = new StringBuilder();
        StringBuilder currentLine = new StringBuilder();
        StringBuilder all = new StringBuilder();

        boolean processedFirst = false;
        int lineNo = 0;
        int currentChar = 0;

        // visit all the characters in the range
        for (int i = startPosition; i < (startPosition + length); i++)
        {
            if (!(currentChar < charsPerLine))
            {
                // complete this line:
                if (processedFirst)
                {
                    all.append(String.format("\n%04X: ", lineNo++));
                }
                else
                {
                    all.append(String.format("%04X: ", lineNo++));
                    processedFirst = true;
                }

                all.append(currentLine.toString()); // hex
                all.append("  "); // spacer
                all.append(asString.toString());

                // reset to prepare for the next line
                currentLine.setLength(0);
                asString.setLength(0);
                currentChar = 0;
            }

            byte b = buffer.get(i);
            currentLine.append(String.format("%02X ", b)); // convert the
                                                           // current byte to
                                                           // hex

            // prepare the byte to be printed as a string
            if (b > 31 && b < 127)
                asString.append((char)b);
            else
                asString.append('.');

            if (currentChar == 7)
            {
                currentLine.append(" "); // add an extra space after 8 chars
            }
            ++currentChar;
        }

        // process the last remaining line, if required
        if (currentLine.length() > 0)
        {
            if (processedFirst)
            {
                all.append("\n");
            }
            all.append(String.format("%04X: ", lineNo++)); // append the current
                                                           // line number
            // fill in any unused chars
            int fill = currentChar;
            while (fill < charsPerLine)
            {
                currentLine.append("   ");

                if (fill == 7)
                {
                    currentLine.append(" "); // add an extra space after 8 chars
                }
                ++fill;
            }

            all.append(currentLine.toString()); // hex
            all.append("  "); // spacer
            all.append(asString.toString());
        }

        return all.toString();
    }
}
