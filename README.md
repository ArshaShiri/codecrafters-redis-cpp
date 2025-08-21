# Redis

# TODOs

* Add a proper logger
* Better parsing of the commands (if else)
* Try to use zero copy in message processing, custom allocators?
* Disable Nagle's algorithm in TCP socket?
* Exceptions should be handled
* Add latency measurement metrics
* Replace callbacks with futures and promises in the TCP server.
* Handle signals for program terminations
* The TCP server should handle reading and writing as much as possible since it is edge triggered. (Handle large messages)
* Make packet processing faster xdp, ebpf
