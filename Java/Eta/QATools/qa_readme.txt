Targetted for the 3.1 release are two main features for which there will be code changes specific to that feature:
Below, 02001 can be read as "02" and "001".  The "02" represents 2nd feature of the release.
The "001" is the 1st code change for this feature. 

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
