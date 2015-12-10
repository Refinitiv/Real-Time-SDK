# EAP ETA Java Readme

This repository contains early access ETA Java code and example applications.

Much of this parallels what was relesed on the customer zone for UPAJ 8.0.0.L1.rrg, however there are some interim bug fixes and Reactor Watchlist enhancements that are present in this repository.

This is provided as early access and is not intended to be production or supported code.  The code is not buildable at this time and is provided to serve as a reference.  This functionality will change and the location and structure may also change prior to becoming official.

Regarding ETA Java Reactor Watchlist functionality, the following are functional in this prototype:

•	Request Timeout for all streams
•	Login Stream
•	Login AAA Token Update
•	Login Pause All
•	Login Support to Send Off-stream Posts
•	Directory Stream & Service Cache
•	Item Request Aggregation & Response Fanout
•	Item Request Service Matching
•	Item Aggregation Priority
•	Item Request QoS Ranges
•	Optimize MSG_KEY_IN_UPDATES Flag
•	Open-Window Throttling

The following ETA Java Reactor functionality is provided but has not completed QA test cycles:

•	Directory Request Fanout
•	Recovery of Closed Item Streams
•	Support to send On-Stream Posts
•	Watchlist Consumer Example
