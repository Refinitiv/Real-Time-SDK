package com.thomsonreuters.proxy.authentication;

import java.io.File;
import java.io.FileNotFoundException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivilegedActionException;
import java.util.HashMap;

import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.login.AppConfigurationEntry;
import javax.security.auth.login.Configuration;
import javax.security.auth.login.LoginContext;

import org.apache.commons.codec.binary.Base64;
import org.apache.http.util.EncodingUtils;

public class KerberosAuthenticationScheme implements IAuthenticationScheme
{
    public final IProxyAuthenticator _proxyAuthenticator;
    public static final String[] RequiredCredentials = {
        CredentialName.DOMAIN, CredentialName.USERNAME,
        CredentialName.PASSWORD, CredentialName.KRB5_CONFIG_FILE };
	
    // Example krb5 config file:
    //	[libdefaults]
    //		default_realm = EXAMPLE.COM
    //		default_tkt_enctypes = aes128-cts rc4-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc
    //		default_tgs_enctypes = aes128-cts rc4-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc
    //		permitted_enctypes   = aes128-cts rc4-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc
    //
    //	[realms]
    //		EXAMPLE.COM  = {
    //			kdc = kdc1.example.com 
    //			default_domain = EXAMPLE.COM 
    //	}
    //
    //	[domain_realm]
    //		.EXAMPLE.COM = EXAMPLE.COM


    public static final String PROXY_AUTHORIZATION_PREFIX = "Proxy-Authorization: ";
    public static final String AUTHORIZATION_PREFIX = "Authorization: ";
    private static final String KERBEROS_RESPONSE_PREFIX = "KERBEROS ";
    public static final String EOL = "\r\n";
	
    public byte[] serviceTicket;

    @SuppressWarnings("unused")
    private int ntlmResponseCount = 0;
    boolean stopScheme = false;
    String db; // debug env variable

    private static HashMap<String, String> loginConfigOptions = new HashMap<String, String>();

    
    public KerberosAuthenticationScheme(IProxyAuthenticator proxyAuthenticator) throws NullPointerException
    {
        if (proxyAuthenticator == null)
        {
            throw new NullPointerException(String.format("%s: a valid proxyAuthenticator is required.",
                                                         this.getClass().getName()));
        }

        _proxyAuthenticator = proxyAuthenticator;
    }
	
    @Override
    public IProxyAuthenticator getProxyAuthenicator()
    {
        return _proxyAuthenticator;
    }

    /* Processes a response from the proxy server
     * and returns a (http) "Proxy-authorization: " value (e.g. "KERBEROS dfdfakajas...") with a trailing \r\n
     * or returns an empty string if a "Proxy-authorization: " value does not need to be sent back to the proxy
     * 
     * httpResponseCode is the http response code to handle (e.g. 407)
     * proxyServerResponse is a response from the proxy server to process (may be null)
     *            
     * Throws ProxyAuthenticationException (an exception that halted the authentication process occurred)
     */
    @Override
    public String processResponse(int httpResponseCode, String proxyServerResponse)
            throws ProxyAuthenticationException
    {
        // System.out.println("KERBEROS  KerberosAuthenticationScheme::processResponse(int httpResponseCode, String proxyServerResponse)   start");
        StringBuilder proxyAuthorizationValue = new StringBuilder();

        String credentials = EncodingUtils.getAsciiString(Base64.encodeBase64(serviceTicket));

        if (httpResponseCode == 407)
            proxyAuthorizationValue.append(PROXY_AUTHORIZATION_PREFIX);
        else
            proxyAuthorizationValue.append(AUTHORIZATION_PREFIX);

        proxyAuthorizationValue.append(KERBEROS_RESPONSE_PREFIX);
        proxyAuthorizationValue.append(credentials);
        proxyAuthorizationValue.append(EOL);
        return proxyAuthorizationValue.toString();
    }
	
    /* Name of this authentication scheme (KERBEROS) */
    public String name()
    {
        return "KERBEROS";
    }	
	
    public boolean stopScheme()
    {
        return stopScheme;
    }

    /* Throws a ProxyAuthenticationException if the credentials required for Kerberos5 authentication are invalid */
    @Override
    public void validateCredentials() throws ProxyAuthenticationException
    {
        for (String credentialName : RequiredCredentials)
        {
            if (!_proxyAuthenticator.getCredentials().isSet(credentialName))
            {
                StringBuilder sb = new StringBuilder();
                sb.append(this.getClass().getName());
                sb.append(": The \"");
                sb.append(credentialName);
                sb.append("\" credential is required for Kerberos5 authentication. ( The full list of required credentials is: ");

                for (String required : RequiredCredentials)
                {
                    sb.append(required);
                    sb.append(" ");
                }

                sb.append(")");

                throw new ProxyAuthenticationException(sb.toString());
            }
        }

        // do AS and TGS exchanges upfront here
        try
        {
            // the final outcome of this method is getting the Kerberos service ticket
            // after doing the necessary AS and TGS exchanges according to the Kerberos specification
            doASandTGSexchanges();
        }
        catch (Exception e)
        {
            stopScheme = true;
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("Error while attempting to get Kerberos service ticket:  " + e.toString());
            throw new ProxyAuthenticationException("Error while attempting to get Kerberos service ticket:  " + e.toString());
        }
    }

    /* Use JAAS to do Authentication Server (AS) exchange, i.e. get the Ticket Granting Ticket (TGT)
     * and use GSSAPI(=Generic Security Service API) for to do Ticket-Granting Server (TGS) exchange, i.e get Kerberos service ticket
     */
    public void doASandTGSexchanges() throws Exception
    {
        // domain (pre-authentication) account
        final String domain = _proxyAuthenticator.getCredentials().get(CredentialName.DOMAIN);

        // userName (pre-authentication) account
        final String userName = _proxyAuthenticator.getCredentials().get(CredentialName.USERNAME);

        // Password for the pre-auth acct.
        final String password = _proxyAuthenticator.getCredentials().get(CredentialName.PASSWORD);

        // Name of krb5 config file
        final String krbfile = _proxyAuthenticator.getCredentials().get(CredentialName.KRB5_CONFIG_FILE);

        // Name of login module (name of LoginContext in JAAS config file)
        final String module = "upaj-kerberos-client";

        // set necessary system properties
        System.setProperty("java.security.krb5.conf", krbfile);
        // System.setProperty("sun.security.krb5.debug", true);
        // System.setProperty("javax.security.auth.useSubjectCredsOnly", "false");

        // confirm all the above
        krb5Validate(userName, password, krbfile, module);

        final CallbackHandler handler = getUsernamePasswordHandler(userName, password);

        final LoginContext loginContext = new LoginContext(module, handler);

        // attempt to login,
        // i.e. perform Kerberos Authentication Seerver (AS) exchange
        // This is the "AS-REQ and AS-REP" Kerberos handshake
        loginContext.login();
        // we now have the TGT, which we can get from the authenticated Subject loginContext.subject(),
        // i.e. credentials are populated into Subject
        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            System.out.println("Kerberos Authentication Server (AS) exchange completed\n");

        // now get Kerberos service ticket for service 'HTTP' from Ticket-Granting Server
        // (the Kerberos service ticket is an array of encrypted bytes)
        serviceTicket = getKerberosServiceTicket(userName, domain, "HTTP", _proxyAuthenticator.getProxyHost(), loginContext);
        if (serviceTicket != null)
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            {
                System.out.println("serviceTicket size == " + serviceTicket.length);
                // System.out.println("serviceTicket (hex) ==");
                // printHex(serviceTicket);
                System.out.println("Kerberos Ticket-Granting Server (TGS) exchange completed\n");
            }
        }
        else
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("failed getting serviceTicket");
        }

        // logout
        loginContext.logout();
    }
	
    public static void krb5Validate(final String username, final String password, final String krbfile, final String moduleName)
                    throws FileNotFoundException, NoSuchAlgorithmException
    {
        // confirm username was provided
        if (null == username || username.isEmpty())
        {
            throw new IllegalArgumentException("Must provide a username");
        }

        // confirm password was provided
        if (null == password || password.isEmpty())
        {
            throw new IllegalArgumentException("Must provide a password");
        }

        // confirm krb5.conf file exists
        if (null == krbfile || krbfile.isEmpty())
        {
            throw new IllegalArgumentException("Must provide a krb5 file");
        }
        else
        {
            final File file = new File(krbfile);
            if (!file.exists())
            {
                throw new FileNotFoundException(krbfile);
            }
        }

        // load Kerberos login config manually (not using login.conf)
        loadLoginConfig();

        // confirm that runtime loaded the Kerberos login config
        final Configuration config = Configuration.getConfiguration();

        // confirm that the login module name exists
        AppConfigurationEntry[] appConfigurationEntry = config.getAppConfigurationEntry(moduleName);
        if (null == appConfigurationEntry)
        {
            throw new IllegalArgumentException("The Kerberos login module " + moduleName + " was not loaded");
        }
        else
        {
            System.out.println("Got Kerberos login config for login module " + moduleName);
        }
    }
    
    /* Load Kerberos login config manually (not using login.conf) */
    private static void loadLoginConfig()
    {
        Configuration.setConfiguration(new Configuration()
        {
            @Override
            public AppConfigurationEntry[] getAppConfigurationEntry(String cname)
            {
                 String name = com.sun.security.auth.module.Krb5LoginModule.class.getName();
                 
                 loginConfigOptions.put("com.sun.security.auth.module.Krb5LoginModule", "required");
                             
                 AppConfigurationEntry ace = new AppConfigurationEntry(name, 
                                                                       AppConfigurationEntry.LoginModuleControlFlag.REQUIRED,
                                                                       loginConfigOptions);
                 AppConfigurationEntry[] entry = {ace};
                 return entry;
            }
        });     
    }
 
    public static CallbackHandler getUsernamePasswordHandler(final String username, final String password)
    {
        final CallbackHandler handler = new CallbackHandler()
        {
            public void handle(final Callback[] callback)
            {
                for (int i = 0; i < callback.length; i++)
                {
                    if (callback[i] instanceof NameCallback)
                    {
                        final NameCallback nameCallback = (NameCallback)callback[i];
                        nameCallback.setName(username);
                    }
                    else if (callback[i] instanceof PasswordCallback)
                    {
                        final PasswordCallback passCallback = (PasswordCallback)callback[i];
                        passCallback.setPassword(password.toCharArray());
                    }
                    else
                        System.err.println("Unsupported Callback: " + callback[i].getClass().getName());
                }
            }
        };

        return handler;
    }

    public byte[] getKerberosServiceTicket(String userName, String domain, String service, String server, LoginContext loginContext)
            throws PrivilegedActionException
    {
        //
        // 1st: get TGT (Ticket Granting Ticket that should now be in loginContext)
        Subject subject = loginContext.getSubject(); // TGT (Ticket Granting Ticket)
        // output Subject info
        // System.out.println("\nSubject (Ticket Granting Ticket) ==" + subject);

        //
        // 2nd: get Kerberos service ticket,
        byte[] serviceTicket = null;
        @SuppressWarnings("unchecked")
        Object doAs = Subject.doAs(subject, new KerberosServiceTicketGenerator("Kerberos5", userName, domain, service, server));
        serviceTicket = (byte[])doAs; // doAs returns the Kerberos service ticket as an array of encrypted bytes

        return serviceTicket;
    }

    public static void printHex(byte[] data)
    {
        int i = 0, j = 0, k = 0; // loop counters
        int line_addr = 0;       // memory address printed on the left
        @SuppressWarnings("unused")
        String line_to_print = "";

        if (data.length == 0)
            return;

        StringBuilder _sbbuffer = new StringBuilder();

        // loop through every input byte
        String _hexLine = "";
        String _asciiLine = "";
        for (i = 0, line_addr = 0; i < data.length; i++, line_addr++, k++)
        {
            // print the line numbers at the beginning of the line
            if ((i % 16) == 0)
            {
                if (i != 0)
                {
                    k = 0;
                    _sbbuffer.append(_hexLine);
                    // _sbbuffer.append("\t...\t");
                    _sbbuffer.append(" ");
                    _sbbuffer.append(_asciiLine + "\n");
                }

                _asciiLine = "";
                _hexLine = String.format("%04X: ", line_addr);
            }

            _hexLine = _hexLine.concat(String.format("%02X ", data[i]));
            if (k == 7)
                _hexLine = _hexLine.concat("  ");

            if (data[i] > 31 && data[i] < 127)
                _asciiLine = _asciiLine.concat(String.valueOf((char)data[i]));
            else
                _asciiLine = _asciiLine.concat(".");
        }

        // handle the ASCII for the final line, which may not be completely filled.
        if (i % 16 > 0)
        {
            for (j = 0; j < 16 - (i % 16); j++)
            {
                _hexLine = _hexLine.concat("     ");
            }

            _sbbuffer.append(_hexLine);
            _sbbuffer.append("  ");
            _sbbuffer.append(_asciiLine);
        }
        System.out.println(_sbbuffer.toString());
    }

}
