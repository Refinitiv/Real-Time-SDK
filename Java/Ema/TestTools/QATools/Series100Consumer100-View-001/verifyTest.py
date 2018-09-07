#!/bin/env python

from __future__ import print_function

import os;
import re;
import sys;

java = False
cpp = False
outstandingRequests = {}
currentFids = []

def findNextEvent(input):
    event = re.compile("^event: (.*)$")
    line = input.readline()
    while (line):
        if event.match(line):
            return line.split()
        line = input.readline()

# we use 'requestMsg domainType=RSSL_DMT_MARKET_PRICE' messages because they contain all the FIDs
# being requested. The snapshot refresh might not contain all FIDs
#
# seems that in java if the reissue doesn't cause any changed in FIDs no request is sent.
# Use UPDATE or next event: to detect this
def findNextRequest(input):
    if java:
        beginMsg = re.compile("^<REQUEST domainType=\"MARKET_PRICE\" .*")
        endMsg = re.compile("^</REQUEST>$")
    else:
        beginMsg = re.compile("^<requestMsg domainType=\"RSSL_DMT_MARKET_PRICE\" .*")
        endMsg = re.compile("^</requestMsg>$")

    eventMsg = re.compile("^event: .*")

    streamPos = input.tell()
    line = input.readline()
    while (line):
        if eventMsg.match(line):
            input.seek(streamPos)
            return None
        if beginMsg.match(line):
            msg = line
            line = input.readline()            
            while(line):
                msg += line
                if endMsg.match(line):
                    return msg
                line = input.readline()
        streamPos = input.tell()
        line = input.readline()
    return None

def extractFidsFromRequestMsg(msg):
    viewMatcher = re.compile("^.*:ViewType.*:ViewData.*(<array .*</array>).*$", re.DOTALL)
    view = viewMatcher.match(msg)
    if view:
        arrayMatcher = re.compile("arrayEntry data=\"([A-Z0-9_]+)\"/>")
        return arrayMatcher.findall(view.group(1))
    else:
        return []

def verify(input, event, previousFids, currentFids):
    nextRequest = findNextRequest(input)
    if nextRequest == None:

        # handles the case of reissues that do not change the view; seems to occur on java
        if event[1] == "requesting" and event[4] == "reissue":
            return
        print("did not find a request for event ", event)
        sys.exit(1)
    else:
        extractedFids = extractFidsFromRequestMsg(nextRequest)
        if extractedFids == currentFids:
            return
        elif not extractedFids: # if the fids did not change, the request has no fids (containerType="RSSL_DT_NO_DATA")
            if previousFids == currentFids:
                return
            else:
                print("request message had no fids but previous fids (", previousFids, ") did not match new fids (", currentFids, ")")
                sys.exit(1)
        else:
            print("failed to matched fids: expected", currentFids, "; got ", extractedFids)
            sys.exit(1)

print("version:", sys.version)

# was our test program Java or Cpp-C
with open("out", "r") as input:
    cppLoginRequestMsg = re.compile("^<requestMsg domainType=\"RSSL_DMT_LOGIN\" .*")
    javaLoginRequestMsg = re.compile("^<REQUEST domainType=\"LOGIN\".*")
    line = input.readline()
    while (line):
        if cppLoginRequestMsg.match(line):
            cpp = True
            break
        if javaLoginRequestMsg.match(line):
            java = True
            break
        line = input.readline()

    if cpp == False and java == False:
        print("did not find login request msg")
        sys.exit(1)

    if cpp == True:
        print("CPP input")
    if java == True:
        print("JAVA input")

with open("out", "rU") as input:
    while True:
        print()

        event = findNextEvent(input)
        if event:
            print(event)

            # handle request
            if event[1] == "requesting" and event[4] == "request":
                sortedUniqueFids = sorted(set(event[6:]))
                previousFids = currentFids
                currentFids = sorted(set(currentFids + sortedUniqueFids))

                if java:
                    verify(input, event, previousFids, currentFids)
                    print("fids matched for request", event[5][:-1])

                # next event is the handle
                handleEvent = findNextEvent(input)
                if handleEvent:
                    if handleEvent[1] == "handle":
                        handle = handleEvent[2]
                        print("handle for request", event[5][:-1], "was", handle)
                    else:
                        print("expected to find handle event after request event; found this event [", handleEvent, "event instead")
                        sys.exit(1)
                else:
                    print("expected to find handle event after request event; did not find any event")
                    sys.exit(1)

                outstandingRequests[handle] = sortedUniqueFids

                if cpp:
                    verify(input, event, previousFids, currentFids)
                    print("fids matched for request", event[5][:-1])

            # reissue
            if event[1] == "requesting" and event[4] == "reissue":
                sortedUniqueFids = sorted(set(event[6:]))

                handleEvent = findNextEvent(input)
                if handleEvent:
                    if handleEvent[1] == "reissue" and handleEvent[3] == "handle":
                        previousFids = currentFids
                        handle = handleEvent[4]
                        print("reissue for handle", handle)
                        outstandingRequests[handle] = sortedUniqueFids;

                        # recreate currentFids
                        currentFids=[]
                        for h, fids in outstandingRequests.items():
                            currentFids = sorted(set(currentFids + fids))

                        verify(input, event, previousFids, currentFids)
                        print("fids matched for reissue", event[5][:-1], "( handle", handle, ")")
                    else:
                        print("expected to find handle event after reissue event; found this event [", handleEvent, "event instead")
                        sys.exit(1)
                else:
                    print("expected to find handle event after reissue event; did not find any event")
                    sys.exit(1)

            # removing handle
            if event[1] == "removing":
                handleBeingRemoved = event[-1]
                del outstandingRequests[handleBeingRemoved]

                # no requests left so closeMsg is expected
                if not outstandingRequests:
                    if java:
                        closeMsg = re.compile("^<CLOSE domainType=\"MARKET_PRICE\".*$")
                    else:
                        closeMsg = re.compile("^<closeMsg domainType=\"RSSL_DMT_MARKET_PRICE\".*$")
                    line = input.readline()
                    while line:
                        if closeMsg.match(line):
                            print("found expected closeMsg after removing handle", event[-1])
                            sys.exit(0)
                        line = input.readline()
                    print("expected to find closeMsg after removing handle", event[-1])
                    sys.exit(1)
                                
                # recreate currentFids
                previousFids = currentFids;
                currentFids=[]
                for handle, fids in outstandingRequests.items():
                    currentFids = sorted(set(currentFids + fids))
                verify(input, event, previousFids, currentFids)
                print("fids matched after removing handle", event[-1])

        else:
            for h, f in outstandingRequests.iteritems():
                print("handle", h, "has fids", f)
            sys.exit(0)
