# Examples

The purpose of this readme is to offer an explanation for the dependencies between the various classes that make up each ETA examples provided in this section. Broadly speaking this folder contains examples which demonstrate how to use the API, to write client or server applications, which are capable of communicating over several transports (socket, websocket, multicast). 

The Enterprise Transport API (ETA) consists of a couple of layers: RSSL/Transport and ValueAdd (VA) Reactor. The Consumer, Provider and NIProvider examples are written directly to the lowest most performant layer: RSSL/Transport layer. The ValueAdd Reactor API handles setting up administrative domains on behalf of the application and is written to the RSSL layer: API calls back application as content arrives for the various domains and is driven by a dispatch mechanism. The VAConsumer, VAProvider and VANIProvider examples are written to the ValueAdd API. In addition, the client side implementation also offers a watchlist (WL) feature (this is written to ValueAdd API): the WatchlistConsumer example demonstrates the numerous features of this layer.

- Consumer Examples: 'Consumer', 'VAConsumer' and 'WatchlistConsumer'
- Providing Examples: Interactive Providers such as 'Provider' and 'VAProvider' and Non-Interactive (NI) Providers such as 'NIProvider' and 'VANIProvider'
- Other Examples: 'AnsiPage' and 'AuthLock' examples

The AnsiPage and AuthLock examples are standalone code samples. AnsiPage example demonstrates how to encode and decode AnsiPage content. AuthLock examples demonstrates how to use the DACS library to create permission locks for creating and publishing content. 

The Docs folders of Enterprise Transport API, describes in detail, each layer of the API and the message flows associated with the various roles: Consumer, Providers and NIProviders. The Open Message Model defines the interactions between client and server applications to establish connectivity and exchange content over various domains. The Domain Model describes administrative, item domains and custom domains. The ETA documentation provides an in-depth explanation regarding administrative domains such as Login, Source Directory and Dictionary, which must be setup before data may be exchanged over item domains such as MarketPrice, MarketByOrder, YieldCurve, etc.


## Examples and Handlers

Each example accepts commandline inputs to specify transport to use, connectivity parameters, types of requests, etc. To start, each example must establish administrative domains and then request/send content over items domains. Common code used to accomplish this is organized into two folders: EtaAppCommon and VACommon 

### EtaAppCommon

This folder defines common code user by multiple examples written to all layers of Enterprise Transport API: RSSL, VA and Watchlist. The following describes contents of EtaAppCommon:

- Rdm: Defines messages involved in setting up and maintaining administrative domains and is used by client and server examples
- Rdm/item: Defines messages to establish item domains over which content is exchanged.  
- ConsumerHandlers, NiproviderHandlers: Domain specific handlers that defined messages sent and received by Consumer and NIProvider examples. Note that for VA and WL examples, the API handles setup of administrative domains and other functionality such as ping handling. 
- ProviderSession: Provider session handler is used to initialize the server and handle incoming client channels for Provider examples.



### VACommon

This folder defines common code user by ValueAdd Examples written to these ETA layers: VA and Watchlist. Specifically, it defines categories of command line options.

- ConnectionArg, ICommandLineParser: defines classes to parse commandline and establish a connection between client and server for VA examples
- ItemArg: defines how to request items on a VA example commandline 
