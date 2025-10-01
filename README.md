# Baby Redis

A minimal Redis clone written in C++20.
Implements a subset of Redis functionality, including RESP protocol parsing/encoding, a command dispatcher, an in-memory key–value store with TTL support, and a simple TCP server for client connections.

## Features

- RESP2 Protocol Support

  - Encode/decode Simple Strings, Errors, Integers, Bulk Strings, and Arrays.
  - Compatible with redis-cli.

- Core Commands

  - PING → PONG
  - SET key value [EX seconds] → stores a string value with optional expiration
  - GET key → retrieves value or (nil)
  - DEL key → delete key, returns number deleted
  - TTL key → reports time-to-live in seconds (-2 missing, -1 no TTL)

- Expiration

  - Keys can have an expireAt timestamp (std::chrono::steady_clock).
  - purge_if_expired is called on access to remove expired keys.

- Command Dispatcher

  - Maps command names (uppercase) to handler functions.
  - Ensures all handlers return a valid RESP reply.

- Server

  - TCP server listening on a configurable port (default 6380).
  - Accepts redis-cli connections.
  - Protocol errors return -ERR.
 
## Build & Run
### Prerequisites

- CMake 3.16+

- g++-10+ or clang++-12+ (C++20 required)

- Catch2 is fetched automatically with CMake

### Build
```
mkdir build && cd build
cmake ..
make -j
```

### Run Server
```
./bin/baby_redis
```

Default port is 6380.

### Minimal redis-cli test
```
redis-cli -p 6380 PING
redis-cli -p 6380 SET foo bar EX 5
redis-cli -p 6380 GET foo
redis-cli -p 6380 TTL foo
```

## Testing

Unit tests use Catch2.
```
ctest --output-on-failure
```
