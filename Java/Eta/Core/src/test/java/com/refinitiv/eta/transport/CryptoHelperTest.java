/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import static org.junit.Assert.assertEquals;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;
import java.security.KeyStore;
import java.security.Security;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.function.Consumer;

import javax.net.ssl.*;

import com.refinitiv.eta.transport.crypto.CryptoHelper;
import com.refinitiv.eta.transport.crypto.CryptoHelperFactory;
import org.conscrypt.Conscrypt;

import org.junit.After;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

@RunWith(Parameterized.class)
public class CryptoHelperTest
{
	public static final String LOCALHOST = "localhost";
	public static final int PORT = 14005;
	public static final String KEYSTORE_PASSWORD = "changeit";
	public static final String RESOURCE_PATH = "src/test/resources/com/refinitiv/eta/transport/CryptoHelperJunit/";
	public static final String VALID_CERTIFICATE = RESOURCE_PATH + "localhost.jks";
	public static final String INVALID_CERTIFICATE = RESOURCE_PATH + "invalid_certificate.jks";
	public static final String TLSv12 = "TLSv1.2";
	public static final String TLSv13 = "TLSv1.3";

	public static String[] client_protocol_versions = {TLSv12, TLSv13};
	private CryptoHelper cryptoHelper;
	private SSLServerSocket serverSocket;

	private String clientSecurityProvider;
	private String engineClassName;

	@After
	public void tearDown() throws Exception
	{
		if(serverSocket != null && !serverSocket.isClosed()){
			serverSocket.close();
		}
	}

	@Parameterized.Parameters
	public static Object[][] data()
	{
		return new Object[][]{
				{"SunJSSE", "sun.security.ssl.SSLEngineImpl"},
				{"Conscrypt", "org.conscrypt.Java8EngineWrapper"}
		};
	}

	public CryptoHelperTest(String clientSecurityProvider, String engineClassName)
	{
		this.clientSecurityProvider = clientSecurityProvider;
		this.engineClassName = engineClassName;
	}

	@Test()
	public void shouldDoHandshakeWhenCertificateCommonNameIsValid() throws IOException
	{
		startServer(VALID_CERTIFICATE);
		createClientCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
	}

	@Test(expected = SSLHandshakeException.class)
	public void shouldFailHandshakeWhenCertificateCommonNameIsInvalid() throws IOException
	{
		startServer(INVALID_CERTIFICATE);
		createClientCryptoHelper(INVALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
	}

	@Test
	public void shouldUseSecurityProvider() throws IOException, ExecutionException, InterruptedException
	{
		startServer(INVALID_CERTIFICATE);
		createClientCryptoHelper(INVALID_CERTIFICATE);
		assertEquals(engineClassName, cryptoHelper._engine.getClass().getName());
	}

	@Test
	public void canUseTLS1_2Version() throws IOException, ExecutionException, InterruptedException
	{
		String[] protocols = {"TLSv1.2"};
		CompletableFuture<String> protocolFuture = new CompletableFuture<>();
		startServer(VALID_CERTIFICATE, protocols, socket -> protocolFuture.complete(((SSLSocket) socket).getSession().getProtocol()));
		createClientCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
		assertEquals("TLSv1.2", protocolFuture.get());
	}

	@Test
	public void shouldUseTLS1_3Version() throws IOException, ExecutionException, InterruptedException
	{
		CompletableFuture<String> protocolFuture = new CompletableFuture<>();
		startServer(VALID_CERTIFICATE, client_protocol_versions, socket -> protocolFuture.complete(((SSLSocket) socket).getSession().getProtocol()));
		createClientCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
		assertEquals("TLSv1.3", protocolFuture.get());
	}

	@Test(expected = IOException.class)
	public void shouldFailHandshakeIfServerDoesntSupportTLS1_2() throws IOException, ExecutionException, InterruptedException
	{
		String[] protocols = {"TLSv1.1"};
		CompletableFuture<String> protocolFuture = new CompletableFuture<>();
		startServer(VALID_CERTIFICATE, protocols, socket -> protocolFuture.complete(((SSLSocket) socket).getSession().getProtocol()));
		createClientCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
		assertEquals("TLSv1.2", protocolFuture.get());
	}
	
	private void startServer(String keystoreFile) throws IOException
	{
		startServer(keystoreFile, null, null);
	}
	
	private void startServer(String keystoreFile, String[] protocolVersions, Consumer<Socket> socketCallback) throws IOException
	{
		SSLServerSocketFactory serverSocketFactory = initServerSSLContext(keystoreFile).getServerSocketFactory();
		
		serverSocket = (SSLServerSocket) serverSocketFactory.createServerSocket(PORT);

		if(protocolVersions != null)
		{
			SSLParameters sslParameters = new SSLParameters();
			sslParameters.setProtocols(protocolVersions);
			serverSocket.setSSLParameters(sslParameters);
		}
		
		new Thread(() ->
		{
			try
			{
				Socket socket = serverSocket.accept();
				if(socketCallback != null){
					socketCallback.accept(socket);
				}
				if(socket.isClosed())
				{
					return;
				}
				BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
				String line;
				while ((line = reader.readLine()) != null)
				{
					System.out.println("Received line: " + line);
				}
				socket.close();
			}
			catch (Exception ex){
				ex.printStackTrace();
			}
		}).start();
	}

	private void createClientCryptoHelper(String keystoreFile) throws IOException {
		ConnectOptionsImpl options = new ConnectOptionsImpl();
		options.encryptionOptions().KeystoreFile(keystoreFile);
		options.encryptionOptions().KeystorePasswd(KEYSTORE_PASSWORD);
		options.encryptionOptions().KeystoreType("JKS");
		options.encryptionOptions().TrustManagerAlgorithm("");
		options.encryptionOptions().KeyManagerAlgorithm("SunX509");
		options.encryptionOptions().SecurityProtocol("TLS");
		options.encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
		options.encryptionOptions().SecurityProvider(clientSecurityProvider);
		options.unifiedNetworkInfo().address(LOCALHOST);
		options.unifiedNetworkInfo().serviceName(Integer.toString(PORT));

		SocketChannel socketChannel = SocketChannel.open();
		socketChannel.connect(new InetSocketAddress(LOCALHOST, PORT));
		socketChannel.configureBlocking(false);

		cryptoHelper = CryptoHelperFactory.createClient(options);
		cryptoHelper.initializeEngine(socketChannel);
	}

	static {
		if(Security.getProvider("Conscrypt") == null)
		{
			Security.addProvider(Conscrypt.newProvider());
		}
	}

	public static SSLContext initServerSSLContext(String keystoreFile)
	{
		try
		{
			KeyStore keyStore = KeyStore.getInstance("JKS");
			SSLContext context = SSLContext.getInstance("TLS");
			keyStore.load(new FileInputStream(keystoreFile), KEYSTORE_PASSWORD.toCharArray());

			KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
			keyManagerFactory.init(keyStore, KEYSTORE_PASSWORD.toCharArray());

			context.init(keyManagerFactory.getKeyManagers(), null, null);
			return context;
		} catch (Exception ex)
		{
			throw new IllegalStateException("Failed to initialize server ssl context", ex);
		}
	}

	private void writeLine(CryptoHelper cryptoHelper) throws IOException
	{
		ByteBuffer buffer = ByteBuffer.allocate(1000);
		buffer.put("Test line\n".getBytes());
		buffer.flip();
		cryptoHelper.write(buffer);
	}
}