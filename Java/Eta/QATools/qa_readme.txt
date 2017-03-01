Targetted for the 3.1 release are two main features for which there will be code changes specific to that feature:
Below, 02001 can be read as "02" and "001".  The "02" represents 2nd feature of the release.
The "001" is the 1st code change for this feature. 


Consumer-00001:  Alter Consumer to send a login reissue with PAUSE-ALL after some item updates and a 
RESUME-ALL after receiving a source directory update.

VAConsumer-00001:  Alter VAConsumer to send a login reissue with PAUSE-ALL after some item updates 
and a RESUME-ALL after receiving a source directory update.

WLConsumer-00001:  Alter WLConsumer to send a login reissue with PAUSE-ALL after some item updates 
and a RESUME-ALL after receiving a source directory update.Targetted for the 3.1 release are two 
main features for which there will be code changes specific to that feature:

VAConsumer31-02001:  Alter VAConsumer to accept -bufSize and -fillSize as inputs.  The bufSize is the size of buffer
requested by getBuffer().  The fillSize is is the number of bytes written.  For example, if bufSize is 1000 and
fillSize is 900, then the VAConsumer gets a buffer of 1000 bytes but only writes 900 bytes before sending. The filled
buffer is sent to the provider/ads in fragments if larger than max fragment size.  This code change fills the buffer
with "1, 2, 3....255" and repeats this pattern of data up to the fillSize.  The provider will then need to check for
the same pattern in the fully assembled message and print "TEST PASSED".

VAProvider31-02001:  After VAProvider to validate incoming tunnel stream message content to have "1, 2, 3...255".
If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction
with VAConsumer31-02001.

VAConsumer31-02002:  Alter VAConsumer to accept -msgSize as an input.  This is the message size application gets
and then writes data into.  Note that the message is a generic message with an opaque buffer as its data body.
Typically, the opaque buffer data body is 10 bytes less than the msgSize with the generic message header being the
other 10 bytes.  This size of message is then sent to the provider/ads in fragments if larger than max fragment
size.  This code change fills the opaque buffer data body with "1, 2, 3....255" and repeats this pattern of data.
The provider will then need to check for the same pattern in the fully assembled message and print "TEST PASSED".

VAProvider31-02002:  Alter VAProvider to validate incoming tunnel stream message as a generic message with opaque
buffer data body of "1, 2, 3....255" repeated.  If the content is as expected, this code change prints a "TEST PASSED".
This code change works in conjunction with VAConsumer31-02002.

Consumer00-00010: Altered the code of the Consumer application to both read the dictionary from file
and download from the network. Once the dictionaries are loded they are compared to see if they are the same.
This is testing the encoding of dictionary messages as multi parts refreshes.

WLConsumer31-02001:  Alter WLConsumer to send PAUSE_ALL after 5 update and a RESUME_ALL after 
receiving a source directory update.
Also WLConsumer is altered to take a "-at2" input that permits user to specify a
2nd token used in a token renewal.

WLConsumer31-02002:  Alter WLConsumer to send item pause after 5 updates on stream id 5 
and item pause after 10 updates on stream id 6. Do a item resume after 20 updates on stream id 6  
Also WLConsumer is altered to take a "-at2" input that permits user to specify a
2nd token used in a token renewal.  NOTE:  This test must be done with two like items and one different item.
Example:  "-mp TRI -mp TRI -mp IBM"
