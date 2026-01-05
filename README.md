# Message-Relay

Client-server message relay in C.

## Features
- Supports connections of multiple clients
- Displays all sent messages in ProcessingServer
- Broadcasts messages sent from one client to all others

## Architecture
- ProcessingServer: TCP server, accepts client connections, receives clients messages, writes them in console and broadcasts them to all connected clients
- Client: TCP client, connects to ProcessingServer and sends text messages

## Build
```bash
git clone https://github.com/n3tw4lk3r/Message-Relay
mkdir build && cd build
cmake ..
make -j$(nproc)
```
## Usage example
```bash
# in build/
src/run_ProcessingServer 8080
src/run_Client 127.0.0.1 8080
```

## Features To Implement
- Use epoll instead of select
- Implement some sort of processing messages in ProcessingServer
