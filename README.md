# Redis

# TODOs

* Add a proper logger
* Try to use zero copy in message processing, custom allocators?
* Disable Nagle's algorithm in TCP socket?
* Exceptions should be handled
* Add latency measurement metrics
* Handle requests based on their arrival time.
* Replace callbacks with futures and promises in the TCP server. (The program is sequential now)
