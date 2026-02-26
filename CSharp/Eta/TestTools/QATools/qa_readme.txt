vaconsumer-OAuthV2-001: Alter VAConsumer to create 1 reactor, 2 connections, both for OAuth V2 but can identify either same / diff credential.
vaconsumer-PH-001
     Alter VAConsumer to add command-line options for testing Preferred host feature.
    -reconnectAttemptLimit <integer value> specifies the maximum number of times the RsllReactor will attempt to reconnect a channel. If set to -1, there is no limit
    -reconnectMinDelay <milliseconds> specifies the minimum time the RsslReactor will wait before attempting to reconnect
    -reconnectMaxDelay <milliseconds> specifies the maximum time the RsslReactor will wait before attempting to reconnect
consumer-RealValue-001
    Alter Consumer to test Real.Value() return code.
