package com.thomsonreuters.upa.transport;

import java.io.IOException;
import java.io.RandomAccessFile;

class GetServiceByName 
{
    static RandomAccessFile _raf;
    static String _servicesFile;

    static private void findFile()
    {
        try
        {
            _raf = new RandomAccessFile("/etc/services", "r");
            _servicesFile = "/etc/services";
        }
        catch (IOException ioe)
        {
            /* File doesn't exist or is otherwise not available. */
            /* Find other path. */
        }

        /* check for info in windir environment variables */
        String windir = System.getProperty("WINDIR");
        if (windir != null)
        {
            int index;
            if (((index = windir.indexOf(":\\")) != -1) ||
                    ((index = windir.indexOf(":")) != -1) ||
                    ((index = windir.indexOf("\\")) != -1))
            {
                windir = windir.substring(0, index);
            }
        }
        else
        {
            windir = "c";
        }

        try
        {
            _raf = new RandomAccessFile(windir + ":\\winnt\\system32\\drivers\\etc\\services", "r");
            _servicesFile = "windir + :\\winnt\\system32\\drivers\\etc\\services";
        }
        catch (IOException ioe)
        {
            /* File doesn't exist or is otherwise not available. */
            /* Find other path. */
        }

        try
        {
            _raf = new RandomAccessFile(windir + ":\\windows\\system32\\drivers\\etc\\services", "r");
            _servicesFile = windir + ":\\windows\\system32\\drivers\\etc\\services";
        }
        catch (IOException ioe)
        {
            /* File doesn't exist or is otherwise not available. */
        }
    }
	
    static private int getPort(String tcpipService)
    {
        try
        {
            while (true)
            {
                char c = (char)_raf.readByte();
                while (Character.isWhitespace(c))
                {
                    c = (char)_raf.readByte();
                }
                boolean match = false;
                if (tcpipService.charAt(0) == c)
                {
                    match = true;
                    for (int i = 1; i < tcpipService.length(); i++)
                    {
                        if (tcpipService.charAt(i) != (char)_raf.readByte())
                        {
                            match = false;
                            break;
                        }
                    }
                }
                if (match)
                {
                    c = (char)_raf.readByte();

                    while (Character.isWhitespace(c))
                    {
                        c = (char)_raf.readByte();
                    }

                    if (!Character.isDigit(c))
                        return -1;

                    int port = Character.digit(c, 10);
                    while (Character.isDigit(c = (char)_raf.readByte()))
                    {
                        port = port * 10 + Character.digit(c, 10);
                    }
                    if (port != 0)
                    {
                        return port;
                    }
                    else
                        return -1; // the file is formatted incorrectly
                }
                else
                {
                    do
                    {
                        c = (char)_raf.readByte();
                    }
                    while (c != '\n');
                }
            }
        }
        catch (Exception e)
        {
            return -1;
        }
    }
	
    synchronized static int getServiceByName(String tcpipService)
    {
        if (_servicesFile != null)
        {
            try
            {
                _raf = new RandomAccessFile(_servicesFile, "r");
            }
            catch (IOException ioe)
            {
            }
        }
        else
        {
            findFile();
        }

        if (_raf != null)
        {
            try
            {
                int port = getPort(tcpipService);
                _raf.close();
                if (port > 0)
                {
                    return port;
                }
            }
            catch (IOException ioe)
            {
            }
        }

        // Get well known port by service
        if (tcpipService.equals("triarch_sink"))
            return 8101;
        else if (tcpipService.equals("triarch_src"))
            return 8102;
        else if (tcpipService.equals("triarch_dbms"))
            return 8103;
        else if (tcpipService.equals("rmds_ssl_sink"))
            return 8101;
        else if (tcpipService.equals("rmds_ssl_source"))
            return 8103;
        else if (tcpipService.equals("ssl_consumer"))
            return 8101;
        else if (tcpipService.equals("ssl_provider"))
            return 8102;
        else if (tcpipService.equals("_consumer"))
            return 14002;
        else if (tcpipService.equals("_provider"))
            return 14003;
        else
            return -1;
    }

}

