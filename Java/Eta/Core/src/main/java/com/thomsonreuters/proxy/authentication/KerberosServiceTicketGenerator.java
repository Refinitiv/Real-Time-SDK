package com.thomsonreuters.proxy.authentication;

import java.security.PrivilegedExceptionAction;

import org.ietf.jgss.GSSContext;
import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;
import org.ietf.jgss.GSSManager;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.Oid;

/* KerberosServiceTicketGenerator involves the creation of a PrivilegedExceptionAction.
 * The java.security.PrivilegedExceptionAction is similar to a Runnable interface,
 * because it simply requires the implementation of a run() method.
 * The intent of a PrivilegedExceptionAction is that it can be run by an AccessController,
 * which can grant or deny access based on credentials.
 * In the case of creating a Kerberos service ticket, the AccesController will actually be called by
 * a javax.security.auth.Subject, which provides its credentials in the form of a TGT (Ticket-Granting Ticket).
 */

@SuppressWarnings("rawtypes")
public class KerberosServiceTicketGenerator implements PrivilegedExceptionAction
{
    private String _authenticationScheme; // "SPNEGO" or "Kerberos5"
    private String _userName;
    @SuppressWarnings("unused")
    private String _password;
    private String _domain;
    private String _service; // e.g. HTTP
    private String _server;  // e.g. proxy hostname
	
    private Oid oid;
    String db; // debug env variable
	
    KerberosServiceTicketGenerator(String authenticationScheme, String userName, String domain, String service, String server)
    {
        _userName = userName;
        _domain = domain;
        _service = service;
        _server = server;
        _authenticationScheme = authenticationScheme;
    }
	
    public Object run() throws Exception
    {
        // GSSAPI is generic, but have to give the appropriate Object ID, in order to create Kerberos 5 service tickets
        if (_authenticationScheme != null)
            setOid(_authenticationScheme);

        // create a GSSManager, which will do the work
        GSSManager gssManager = GSSManager.getInstance();

        // tell the GSSManager the Kerberos name of the client and service
        GSSName clientName = gssManager.createName(_userName + "@" + _domain, GSSName.NT_USER_NAME);
        GSSName serviceName = gssManager.createName(_service + "/" + _server + "@" + _domain, null);

        // get the client's credentials
        // this run() method will be called by Subject.doAs(),
        // so the client's credentials (Kerberos TGT or Ticket-Granting Ticket) are already available in the Subject

        GSSCredential clientCredentials = gssManager.createCredential(clientName, GSSCredential.INDEFINITE_LIFETIME, oid, GSSCredential.INITIATE_ONLY);
        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            System.out.println("gssManager.createCredential completed");

        // create a security context between the client and the service
        GSSContext gssContext = gssManager.createContext(serviceName, oid, clientCredentials, GSSContext.DEFAULT_LIFETIME);
        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            System.out.println("gssManager.createContext completed");

        // Initialize the security context.
        // This operation will cause a Kerberos request of Active Directory,
        // to create a service ticket for the client to use the service (e.g. HTTP)
        // i.e. perform Kerberos Ticket-Granting Server (TGS) exchange.
        // This is the "TGS-REQ and TGS-REP" Kerberos handshake.
        byte[] serviceTicket = gssContext.initSecContext(new byte[0], 0, 0);
        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            System.out.println("gssManager.initSecContext completed");
        gssContext.dispose();

        // return the Kerberos service ticket as an array of encrypted bytes
        return serviceTicket;
    }
	
    private void setOid(String authenticationScheme)
    {
        // give GSSAPI the following Object ID
        // (the unique object identifier is used by GSSAPI to select the underlying security mechanism)
        try
        {
            if (authenticationScheme.equals("SPNEGO"))
                oid = new Oid("1.3.6.1.5.5.2"); // SPNEGO Oid
            else if (authenticationScheme.equals("Kerberos5"))
                oid = new Oid("1.2.840.113554.1.2.2"); // Kerberos5 Oid
            else
                System.out.println("Unsupported authentication scheme: " + authenticationScheme);
        }
        catch (GSSException e)
        {
            System.out.println("Oid GSSException: " + e.getMessage());
        }
    }

}
