package com.refinitiv.eta.transport;

import java.io.FileInputStream;
import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;

import javax.net.ssl.SSLContext;

// This class is here to manage an ssl context for a server.  That way, we only need to configure the context once, and can then spawn additioanl 
// sslEngine sessions at will from the server.
public class EncryptedContextHelper {
    private final SSLContext context;
    
    EncryptedContextHelper(BindOptions options) throws IOException
    {
        assert (options != null) : "options cannot be null";

        KeyStore serverKS;
        javax.net.ssl.KeyManagerFactory serverKMF;
        javax.net.ssl.TrustManagerFactory serverTMF;

        String keystorePassword;
        String keystoreFile;
        String keystoreType;
        String securityProvider;
        String trustManagerAlgorithm;
        String securityProtocol;
        String keyManagerAlgorithm;
        
        ServerEncryptionOptionsImpl encOpts = (ServerEncryptionOptionsImpl)options.encryptionOptions();

        keystorePassword = encOpts.keystorePasswd();
		keystoreFile = encOpts.keystoreFile();
		keystoreType = encOpts.keystoreType();
		securityProvider = encOpts.securityProvider();
		trustManagerAlgorithm = encOpts.trustManagerAlgorithm();
		securityProtocol = encOpts.securityProtocol();
		keyManagerAlgorithm = encOpts.keyManagerAlgorithm();
        
        char[] keystorePasswordChars = keystorePassword != null ? keystorePassword.toCharArray() : null;
        if (keystoreFile != null && !keystoreFile.isEmpty())
        {

        	 try
             {
                 // ClientKS=KeyStore.getInstance("JKS"); //JKS=JavaKeyStore
                 if (keystoreType == null|| keystoreType.equals(""))
                 {
                     // get JavaKeyStore from java.security file (default: keystore.type=jks)
                	 serverKS = KeyStore.getInstance(encOpts._defaultKeystoreType);
                 } else
                 {
                	 serverKS = KeyStore.getInstance(keystoreType);
                 }
             } catch (KeyStoreException e)
             {
                 throw new IOException("Error when getting keystore type  " + e.getMessage());
             }

             // load the keystore from the keystore file
             try
             {
                 FileInputStream _fstream = new FileInputStream(keystoreFile);

                 serverKS.load(_fstream, keystorePassword.toCharArray());
             }
             catch (IOException | NoSuchAlgorithmException e)
             {
                 throw new IOException("Error when loading keystore from certificate file  " + e.getMessage());
             }
             catch (CertificateException e)
             {
                 throw new IOException("CertificateException when loading keystore from certificate file  " + e.getMessage());
             }
        }
        else
        {
            //If KeyStore is set to null then TrustManagerFactory will be initialized with certificates from
            //default trusted keystore
            serverKS = null;
        }
        // create a TrustManagerFactory
        try
        {
            if (trustManagerAlgorithm == null || trustManagerAlgorithm.equals(""))
            {
                // get default trust management algorithm for security provider
                // (default: PKIX for security provider SunJSSE)
                serverTMF = javax.net.ssl.TrustManagerFactory.getInstance(encOpts._defaultTrustManagerAlgorithm, securityProvider);
            }
            else
                serverTMF = javax.net.ssl.TrustManagerFactory.getInstance(trustManagerAlgorithm, securityProvider);
        }
        catch (NoSuchAlgorithmException | NoSuchProviderException e)
        {
            throw new IOException("Error when creating TrustManagerFactory: " + e.getMessage());
        }

        // initialize the above TrustManagerFactory
        try
        {
            serverTMF.init(serverKS);
        }
        catch (KeyStoreException e)
        {
            throw new IOException("Error when initializing TrustManagerFactory:  " + e.getMessage());
        }

        // create a Java SSLContext object
        try
        {
        	if(securityProtocol == null || securityProtocol.equals(""))
        		context = SSLContext.getInstance(encOpts._defaultSecurityProtocol);
        	else
        		context = SSLContext.getInstance(securityProtocol);
            if (keyManagerAlgorithm == null || keyManagerAlgorithm.equals(""))
            {
            	if(securityProvider == null || securityProvider.equals(""))
	                // get default key management algorithm for security provider
	                // (default: SunX509 for security provider SunJSSE)
	                serverKMF = javax.net.ssl.KeyManagerFactory.getInstance(encOpts._defaultKeyManagerAlgorithm, encOpts._defaultSecurityProvider);
            	else
            		 serverKMF = javax.net.ssl.KeyManagerFactory.getInstance(encOpts._defaultKeyManagerAlgorithm, securityProvider);
            }
            else
            {
                serverKMF = javax.net.ssl.KeyManagerFactory.getInstance(keyManagerAlgorithm, securityProvider);
            }
            serverKMF.init(serverKS, keystorePasswordChars);

            /* Need the key manager for private key and certificate */
            context.init(serverKMF.getKeyManagers(), serverTMF.getTrustManagers(), null);
        }
        catch (NoSuchAlgorithmException | NoSuchProviderException | KeyStoreException e)
        {
            throw new IOException("Error when initializing SSLContext:  " + e.getMessage());
        } catch (UnrecoverableKeyException e)
        {
            throw new IOException("UnrecoverableKeyException when initializing SSLContext:  " + e.getMessage());
        }
        catch (KeyManagementException e)
        {
            throw new IOException("KeyManagementException when initializing SSLContext:  " + e.getMessage());
        }
    }
    
    SSLContext getContext() {
    	return context;
    }
}
