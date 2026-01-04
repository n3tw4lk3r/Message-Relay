# Message-Relay

Client-server message relay in pure C.

## Architecture
- DisplayServer: TCP server, accepts client connections, receives clients' messages and writes them in console
- Client: TCP client, connects to server and send text messages

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
src/run_DisplayServer 8080
src/run_Client 127.0.0.1 8080
```

## Features To Implement
- Multi-client support
- Add middle processing server
