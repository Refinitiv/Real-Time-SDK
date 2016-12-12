package com.thomsonreuters.proxy.authentication.example;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.SocketChannel;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;

import com.thomsonreuters.proxy.authentication.CredentialName;
import com.thomsonreuters.proxy.authentication.IProxyAuthenticatorResponse;
import com.thomsonreuters.proxy.authentication.ProxyAuthenticationException;
import com.thomsonreuters.proxy.authentication.ProxyAuthenticatorFactory;
import com.thomsonreuters.proxy.authentication.CredentialsFactory;
import com.thomsonreuters.proxy.authentication.IProxyAuthenticator;
import com.thomsonreuters.proxy.authentication.ICredentials;
import com.thomsonreuters.proxy.authentication.ResponseCodeException;

public class ProxyAuthViaSocketChannelExample
{
    private enum HttpRequestType
    {
        GET, CONNECT
    }

    private final int BUFFER_SIZE = 1024 * 1000;
    private final ByteBuffer _buffer = ByteBuffer.allocateDirect(BUFFER_SIZE);
    private final CharBuffer _charBuffer = CharBuffer.allocate(BUFFER_SIZE);
    private final CharsetEncoder _encoder;
    private final CharsetDecoder _decoder;
    private final IProxyAuthenticator _authenticator;

    private final String _proxyHost;
    private final int _proxyPort;
    private final String _actualDestHost;
    private final int _actualDestPort;

    private SocketChannel _channel; // non-final because we may need to re-open the connection

    private static final String EOL = "\r\n";
    private static final String USER_AGENT = "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.2)\r\n";
    private static final String ACCEPT = "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*\r\n";
    private static final String PROXY_CONNECTION_KEEP_ALIVE = "Proxy-Connection: Keep-Alive\r\n";
    private static final String PRAGMA_NO_CACHE = "Pragma: no-cache\r\n";
    private static final String CONNECTION_KEEP_ALIVE = "Connection: Keep-Alive\r\n";
    private static final String END_OF_MESSAGE = "\r\n\r\n";

    public ProxyAuthViaSocketChannelExample(String proxyHost, int proxyPort, String actualDestHost, int actualDestPort, ICredentials credentials)
            throws IOException, ResponseCodeException, ProxyAuthenticationException, InterruptedException
    {
        _proxyHost = proxyHost;
        _authenticator = ProxyAuthenticatorFactory.create(credentials, _proxyHost);

        _proxyPort = proxyPort;
        _actualDestHost = actualDestHost;
        _actualDestPort = actualDestPort;

        // init encoder / decoder
        Charset charset = Charset.forName("ISO-8859-1");
        _encoder = charset.newEncoder();
        _decoder = charset.newDecoder();

        connect();
    }

    private void connect() throws IOException
    {
        System.out.println(String.format("Attempting to connect to proxy %s:%d...", _proxyHost, _proxyPort));

        InetSocketAddress socketAddress = new InetSocketAddress(_proxyHost, _proxyPort);
        _channel = SocketChannel.open();
        _channel.connect(socketAddress);

        System.out.println("...successfully connected to the proxy.");
    }

    public boolean authenticateNio(HttpRequestType establishConnectionVia)
            throws CharacterCodingException, IOException, ResponseCodeException, ProxyAuthenticationException
    {
        boolean authenticated = false;
        int ignoredResponses = 0;
        final int MAX_IGNORED_RESPONSES = 10;
        final StringBuilder response = new StringBuilder();

        // Send initial request
        String request = buildRequest(establishConnectionVia, false, null);
        writeToChannel(request);

        // Read response
        while ((_channel.read(_buffer)) != -1)
        {
            _buffer.flip();
            _decoder.decode(_buffer, _charBuffer, false);

            _charBuffer.flip();
            response.append(_charBuffer.toString());
            System.out.println(_charBuffer.toString()); // for debugging

            if (!_authenticator.isAuthenticated() && !response.toString().contains(END_OF_MESSAGE))
            {
                _buffer.clear();
                _charBuffer.clear();
                continue;
            }

            IProxyAuthenticatorResponse authenticatorResponse = null;
            try
            {
                authenticatorResponse = _authenticator.processResponse(response.toString());
            }
            catch (ResponseCodeException e)
            {
                // the response from the proxy may contain an HTML error message intended for uses that we should ignore
                ++ignoredResponses;
                if (ignoredResponses < MAX_IGNORED_RESPONSES)
                {
                    String ignoring = String.format("Ignoring a response from the proxy that did not contain a response code (%d/%d)",
                                                    ignoredResponses, MAX_IGNORED_RESPONSES);
                    System.out.println(ignoring);
                    _buffer.clear();
                    _charBuffer.clear();
                    continue;
                }
                else
                {
                    throw e;
                }
            }

            _buffer.clear();
            _charBuffer.clear();
            response.setLength(0);

            if (!_authenticator.isAuthenticated())
            {
                if (authenticatorResponse.isProxyConnectionClose())
                {
                    System.out.println("*** The proxy requested a close. Reconnecting. ***\n\n");
                    connect();
                }

                request = buildRequest(establishConnectionVia, false, authenticatorResponse.getProxyAuthorization());
                writeToChannel(request);
            }
            else
            {
                if (authenticated)
                {
                    break; // we don't want to read any more responses
                }

                authenticated = true;

                if (establishConnectionVia.equals(HttpRequestType.CONNECT))
                {
                    // issue an *https* GET request for content:
                    request = buildRequest(HttpRequestType.GET, true, null);

                    writeToChannel(request);
                }
                else
                {
                    break;
                }
            }
        }

        return authenticated;
    }

    private void writeToChannel(String request) throws CharacterCodingException, IOException
    {
        System.out.println(); // for debugging
        System.out.println(request); // for debugging
        ByteBuffer toWrite = _encoder.encode(CharBuffer.wrap(request));

        while (toWrite.hasRemaining())
        {
            _channel.write(toWrite);
        }
    }

    /* Returns a string containing the "common" HTTP Get request.
     * A suffix (a trailing \r\n) must be appended to the returned value (to make it a valid HTTP request.)
     * 
     * Returns a string containing the "common" HTTP Get request.
     * A suffix (a trailing \r\n) must be appended to the returned value (to make it a valid HTTP request.)
     */
    private final String buildHttpGetRequestPrefix(boolean useHttps)
    {
        StringBuilder sb = new StringBuilder();

        if (useHttps)
        {
            sb.append("GET https://");
            sb.append(_actualDestHost);
            sb.append("/");
        }
        else
        {
            sb.append("GET http://");
            sb.append(_actualDestHost);
            sb.append(":");
            sb.append(_actualDestPort);
        }

        sb.append(" HTTP/1.1\r\n");
        sb.append(ACCEPT);
        sb.append(USER_AGENT);
        sb.append("Host: ");
        sb.append(_actualDestHost);
        // sb.append(":");
        // sb.append(_actualDestPort);
        sb.append("\r\n");
        sb.append(CONNECTION_KEEP_ALIVE);

        if (!useHttps)
        {
            sb.append(PROXY_CONNECTION_KEEP_ALIVE);
        }

        return sb.toString();
    }

    /* Returns a string containing the "common" HTTP Connect request.
     * A suffix (a trailing \r\n) must be appended to the returned value (to make it a valid HTTP request.)
     * 
     * Returns a string containing the "common" HTTP Connect request.
     * A suffix (a trailing \r\n) must be appended to the returned value (to make it a valid HTTP request.)
     */
    private final String buildHttpConnectRequestPrefix()
    {
        StringBuilder sb = new StringBuilder();

        sb.append("CONNECT ");
        sb.append(_actualDestHost);
        sb.append(":");
        sb.append(_actualDestPort);
        sb.append(" HTTP/1.1\r\n");
        sb.append(USER_AGENT);
        sb.append(PROXY_CONNECTION_KEEP_ALIVE);
        sb.append("Content-Length: 0\r\n");
        sb.append("Host: ");
        sb.append(_actualDestHost);
        sb.append("\r\n");
        sb.append(PRAGMA_NO_CACHE);

        return sb.toString();
    }

    private final String buildRequest(HttpRequestType requestMode, boolean useHttps, String authorization)
    {
        StringBuilder sb = new StringBuilder();

        if (requestMode.equals(HttpRequestType.GET))
        {
            sb.append(buildHttpGetRequestPrefix(false));
        }
        else
        {
            sb.append(buildHttpConnectRequestPrefix());
        }

        if (authorization != null)
        {
            sb.append(authorization);
        }

        sb.append(EOL);

        return sb.toString();
    }

    public void close() throws Exception
    {
        if (_channel != null)
        {
            try
            {
                _channel.close();
            }
            catch (IOException ignored)
            {
            }
        }
    }

    private static String getLocalHostname()
    {
        try
        {
            return InetAddress.getLocalHost().getHostName();
        }
        catch (UnknownHostException ignored)
        {
            return "localhost"; // for the unlikely event we can't get the actual hostanme
        }
    }

    private static ICredentials getCredentials()
    {
        ICredentials credentials = CredentialsFactory.create();

        // NTLM example:
        credentials.set(CredentialName.DOMAIN, "AMERS.IME.REUTERS.COM");
        credentials.set(CredentialName.USERNAME, "tony.louras");
        credentials.set(CredentialName.PASSWORD, "Hellas02");
        credentials.set(CredentialName.LOCAL_HOSTNAME, getLocalHostname());

        // Negotiate/Kerberos or Kerberos example:
        // credentials.set(CredentialName.DOMAIN, "AMERS.IME.REUTERS.COM");
        // credentials.set(CredentialName.USERNAME, "firstname.lastname");
        // credentials.set(CredentialName.PASSWORD, "abc123");
        // credentials.set(CredentialName.KRB5_CONFIG_FILE,
        // "C:\\WINDOWS\\krb5.ini");

        return credentials;
    }


    public static void main(String[] args)
    {
        final ICredentials credentials = getCredentials();

        final String proxyHost = "myProxyHost";
        final String actualDestHost = "myProxyHost";
        HttpRequestType establishConnectionVia = HttpRequestType.GET; // http or https

        final int proxyPort;
        final int actualDestPort;

        try
        {
            if (establishConnectionVia.equals(HttpRequestType.GET))
            {
                actualDestPort = 80;
                proxyPort = 8080;
            }
            else
            {
                proxyPort = 8443;
                actualDestPort = 443;
            }

            ProxyAuthViaSocketChannelExample test = new ProxyAuthViaSocketChannelExample(proxyHost, proxyPort,
                                                                                         actualDestHost, actualDestPort,
                                                                                         credentials);

            if (!test.authenticateNio(establishConnectionVia))
            {
                System.err.println("Error: Failed to authenticate.");
            }

            System.out.println("Exiting.");

        }
        catch (ResponseCodeException e)
        {
            e.printStackTrace();
        }
        catch (ProxyAuthenticationException e)
        {
            e.printStackTrace();
        }
        catch (UnknownHostException e)
        {
            e.printStackTrace();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }
    }

}
