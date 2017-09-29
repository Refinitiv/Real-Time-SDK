Definition of QATools: The purpose of QATools is to test variations of examples (see Applications/Examples). For each example that is altered to run a test, there will be a directory here to represent that variation. Example: If VAConsumer was altered 2 times to do 2 different tests, there will be directories here, such as, "vaconsumer*01" and "vaconsumer*02". In each directory are the files that have been altered. These directories contain ONLY the files that were altered for that example.

How to use QATools: For each QATool direcotry, user must copy or overlay the source files from that directory into the original location in Applications/Examples where the entire source for an example exists.  The user must re-build the code to run the altered example. 

Disclaimer:  Please note that this is not a comprehensive list of all test variations used in test.

List of altered code directories:
-----------------------------------------------------------------------------------------

Module:  Consumer 
-----------------

consumer-LoginReissue-001:  Alter Consumer to send a login reissue with PAUSE-ALL after some item updates and a RESUME-ALL after receiving a source directory update.

consumer-Dict-001: Altered the code of the Consumer application to both read the dictionary from file and download from the network. Once the dictionaries are loded they are compared to see if they are the same.  This is testing the encoding of dictionary messages as multi parts refreshes.

Module:  Value Add Provider 
---------------------------

vaprovider-TsFrag-001:  Alter VAProvider to validate incoming tunnel stream message content to have "1, 2, 3...255".  If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction with vaconsumer-TsFrag-001.

vaprovider-TsFrag-002:  Alter VAProvider to validate incoming tunnel stream message as a generic message with opaque buffer data body of "1, 2, 3....255" repeated.  If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction with vaconsumer-TsFrag-002.

vaprovider-GenM-001:   Alters VAProvider to send genericMsg on login, directory and market price streams. 
GenericMsg contains ElementList with one ElementEntry. 
Also added is the ability to decode genericMsg from a consumer. 
This VAProvider works in conjuction with the watchlist consumer code change from wlconsumer-GenM-001
$

Module:  Value Add Consumer 
---------------------------

vaconsumer-LoginReissue-001:  Alter VAConsumer to send a login reissue with PAUSE-ALL after some item updates and a RESUME-ALL after receiving a source directory update.

vaconsumer-TsFrag-001:  Alter VAConsumer to accept -bufSize and -fillSize as inputs.  The bufSize is the size of buffer requested by getBuffer().  The fillSize is is the number of bytes written.  For example, if bufSize is 1000 and fillSize is 900, then the VAConsumer gets a buffer of 1000 bytes but only writes 900 bytes before sending. The filled buffer is sent to the provider/ads in fragments if larger than max fragment size.  This code change fills the buffer with "1, 2, 3....255" and repeats this pattern of data up to the fillSize.  The provider will then need to check for the same pattern in the fully assembled message and print "TEST PASSED".

vaconsumer-TsFrag-002:  Alter VAConsumer to accept -msgSize as an input.  This is the message size application gets and then writes data into.  Note that the message is a generic message with an opaque buffer as its data body.  Typically, the opaque buffer data body is 10 bytes less than the msgSize with the generic message header being the other 10 bytes.  This size of message is then sent to the provider/ads in fragments if larger than max fragment size.  This code change fills the opaque buffer data body with "1, 2, 3....255" and repeats this pattern of data.
The provider will then need to check for the same pattern in the fully assembled message and print "TEST PASSED".

Module:  Watchlist Consumer 
---------------------------

wlconsumer-LoginReissue-001:  Alter WLConsumer to send a login reissue with PAUSE-ALL after some item updates and a RESUME-ALL after receiving a source directory update.

wlconsumer-TrepAuth-001:  Alter WLConsumer to take a "-at2" input that permits user to specify a
2nd token used in a token renewal.

wlconsumer-TrepAuth-002:  Alter WLConsumer to send PAUSE_ALL after 5 update and a RESUME_ALL after receiving a source directory update. Also WLConsumer is altered to take a "-at2" input that permits user to specify a 2nd token used in a token renewal.

wlconsumer-TrepAuth-003:  Alter WLConsumer to send item pause after 5 updates on stream id 5 and item pause after 10 updates on stream id 6. Altered code does an item resume after 20 updates on stream id 6. Also WLConsumer is altered to take a "-at2" input that permits user to specify a 2nd token used in a token renewal.  NOTE:  This test must be done with two like items and one different item. Example:  "-mp TRI -mp TRI -mp IBM"

wlconsumer-TrepAuth-004:  Alter WLConsumer to send PAUSE_ALL after 5 updates. Also WLConsumer is altered to take a "-at2" input that permits user to specify a 2nd token used in a token renewal. NOTE: This is a slight variation on wlconsumer-TrepAuth-001 with the RESUME-ALL code removed.

wlconsumer-ConsFunc-001:  Altered WLConsumer to accept new command line arguments.
MP Items can now be requested as snapshot, private stream, and/or view. 
Example: "-mp IBM:SPV3" is IBM with all three and viewId 3, 
"-mp GOOG:V5P" requests GOOG with viewId 5 and private stream 
(Order of SPV is not significant as long as viewId number immediatly follows the 'V'). 
If neither snapshot, private stream, or view are needed, it can be written as "-mp IBM"
List of MP Items in command line are treated as if indexed(starting at index 0), 
and can be specified to be requested after a certain number of events.
(3 events are defined: -e1 is for source directory updates, -e2 is for item update messages,
-e3 is for channel_down_reconnecting events)
Example: doing "-mp IBM -mp GOOG:PV3 -mp TRI:S -mp NFLX:PSV7 -AAPL -e2 7::1:2 -e2 9::4 -e3 8::3 " 
will initially only request IBM, and after 7 update messages, 
it will request mp items indexed 1 through 2, so GOOG and TRI. 
After 8 channel-down-reconnecting events, it will request item indexed 3 only 
since there is no end index given, it is not treated a a range (so just NFLX)
Similarily, after 9 update events, it will request item indexed 4 (AAPL)
Initial requests can be delayed until source dir refresh by specifying '-delayIntialRequest'
Single open and allow suspect data can be specified by setting '-singleOpen 0(or 1)'
and '-allowSuspect 0(or 1)' (default set to 1)

wlconsumer-GenM-001:  Alters WLConsumer to send genericMsg on login, directory and market price streams. 
GenericMsg contains ElementList with one ElementEntry. 
Also added is the ability to decode genericMsg from a provider. 
This works in conjuction with the VAProvider code change from vaprovider-GenM-001
