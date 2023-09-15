package com.refinitiv.eta.transport;

import static org.junit.Assert.assertEquals;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;
import java.security.KeyStore;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.function.Consumer;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLHandshakeException;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLServerSocket;
import javax.net.ssl.SSLServerSocketFactory;
import javax.net.ssl.SSLSocket;

import org.junit.After;
import org.junit.Test;

public class CryptoHelperTest
{
	public static final String LOCALHOST = "localhost";
	public static final int PORT = 14005;
	public static final String KEYSTORE_PASSWORD = "changeit";
	public static final String RESOURCE_PATH = "src/test/resources/com/refinitiv/eta/transport/CryptoHelperJunit/";
	public static final String VALID_CERTIFICATE = RESOURCE_PATH + "localhost.jks";
	public static final String INVALID_CERTIFICATE = RESOURCE_PATH + "invalid_certificate.jks";
	private CryptoHelper cryptoHelper;
	private SSLServerSocket serverSocket;

	@After
	public void tearDown() throws Exception
	{
		if(serverSocket != null && !serverSocket.isClosed()){
			serverSocket.close();
		}
	}
	
	@Test
	public void shouldDoHandshakeWhenCertificateCommonNameIsValid() throws IOException
	{
		startServer(VALID_CERTIFICATE);
		createCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
	}

	@Test(expected = SSLHandshakeException.class)
	public void shouldFailHandshakeWhenCertificateCommonNameIsInvalid() throws IOException
	{
		startServer(INVALID_CERTIFICATE);
		createCryptoHelper(INVALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
	}

	@Test
	public void shouldUseTLS1_2Version() throws IOException, ExecutionException, InterruptedException
	{
		CompletableFuture<String> protocolFuture = new CompletableFuture<>();
		startServer(VALID_CERTIFICATE, null, socket -> protocolFuture.complete(((SSLSocket) socket).getSession().getProtocol()));
		createCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
		assertEquals("TLSv1.2", protocolFuture.get());
	}

	@Test(expected = IOException.class)
	public void shouldFailHandshakeIfServerDoesntSupportTLS1_2() throws IOException, ExecutionException, InterruptedException
	{
		CompletableFuture<String> protocolFuture = new CompletableFuture<>();
		startServer(VALID_CERTIFICATE, "TLSv1.1", socket -> protocolFuture.complete(((SSLSocket) socket).getSession().getProtocol()));
		createCryptoHelper(VALID_CERTIFICATE);
		cryptoHelper.doHandshake();
		writeLine(cryptoHelper);
		assertEquals("TLSv1.2", protocolFuture.get());
	}
	
	private void startServer(String keystoreFile) throws IOException
	{
		startServer(keystoreFile, null, null);
	}
	
	private void startServer(String keystoreFile, String protocolVersion, Consumer<Socket> socketCallback) throws IOException
	{
		SSLServerSocketFactory serverSocketFactory = initServerSSLContext(keystoreFile).getServerSocketFactory();
		
		serverSocket = (SSLServerSocket) serverSocketFactory.createServerSocket(PORT);

		if(protocolVersion != null)
		{
			SSLParameters sslParameters = new SSLParameters();
			sslParameters.setProtocols(new String[]{protocolVersion});
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

	private void createCryptoHelper(String keystoreFile) throws IOException {
		ConnectOptionsImpl options = new ConnectOptionsImpl();
		options.encryptionOptions().KeystoreFile(keystoreFile);
		options.encryptionOptions().KeystorePasswd(KEYSTORE_PASSWORD);
		options.encryptionOptions().KeystoreType("JKS");
		options.encryptionOptions().TrustManagerAlgorithm("");
		options.encryptionOptions().KeyManagerAlgorithm("SunX509");
		options.encryptionOptions().SecurityProtocol("TLS");
		options.encryptionOptions().SecurityProvider("SunJSSE");
		options.unifiedNetworkInfo().address(LOCALHOST);
		options.unifiedNetworkInfo().serviceName(Integer.toString(PORT));
		
		SocketChannel socketChannel = SocketChannel.open();
		socketChannel.connect(new InetSocketAddress(LOCALHOST, PORT));
		socketChannel.configureBlocking(false);

		cryptoHelper = new CryptoHelper(options);
		cryptoHelper.initializeEngine(socketChannel);
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