# Kung-Fu-Chess

A chess variant with no turns. Both players can move any of their pieces at any
time, and moves take real time to travel across the board instead of resolving
instantly — two moves racing for the same cell can collide, land at different
times, or get dodged with an in-place jump. It's chess, but live.

See [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) for the engine's architecture,
class-by-class breakdown, and the real-time rules in detail.

## Layout

| Directory | What it is |
|---|---|
| `Kung-Fu-Chess/` | The engine — pure game logic, no graphics, driven by a text protocol on stdin/stdout. Headers in `include/`, sources in `src/`. |
| `Kung-Fu-Chess.Net/` | Networking primitives: WebSocket framing/handshake, Base64, SHA-1, the client/server socket wrappers, and the wire format (JSON) shared between client and server. |
| `Kung-Fu-Chess.Server/` | The multiplayer server — connection management, matchmaking, game rooms, user accounts (SQLite-backed, PBKDF2 password hashing), rating, and `Kung-Fu-Chess.ServerApp`, the server executable (single-threaded game loop, §4.3). |
| `Kung-Fu-Chess.Client/` | Client-side networked state: `ClientState` (mirrors the server's GAME_STATE broadcast) and `ViewModel` (selection + non-authoritative move checking). No UI yet — see "What's not built yet" below. |
| `Kung-Fu-Chess.UI/` | Graphical front end (OpenCV, vendored under `deps/opencv/`) for the local/offline windowed game. Not yet wired up to the networked client. |
| `Kung-Fu-Chess.Tests/` | doctest unit tests, mirroring the source layout of the other projects one folder per class. |

### What's not built yet

The server side (rooms, auth, matchmaking, rating, the wire protocol, the
WebSocket transport) is complete and end-to-end tested — see "Running the
server" below. What's still missing is the **networked client's UI**: login
screen, lobby/menu, room join flow, and wiring `Kung-Fu-Chess.Client` +
`Kung-Fu-Chess.UI`'s renderer into one windowed app that actually talks to
`Kung-Fu-Chess.ServerApp` over the wire. Today `Kung-Fu-Chess.UI` only drives
the offline, non-networked engine (`Kung-Fu-Chess.exe`).

## Build & test

CMake project (root `CMakeLists.txt`), MSVC toolset, Debug config. Pass
`-DKFC_BUILD_UI=OFF` to skip `Kung-Fu-Chess.UI` and its OpenCV/vcpkg dependency
when you only need the engine, networking, server, and tests.

```bash
# Configure (adjust cmake.exe path to your VS 2022 install if not on PATH)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DKFC_BUILD_UI=OFF

# Build
cmake --build build --config Debug -j

# Run the doctest suite
./build/Kung-Fu-Chess.Tests/Debug/Kung-Fu-Chess.Tests.exe
```

## Running the server

```bash
./build/Kung-Fu-Chess.Server/Debug/Kung-Fu-Chess.ServerApp.exe
```

Listens on `ws://127.0.0.1:9001`, creates `kungfuchess.db` (SQLite, user
accounts) in the working directory on first run, and logs nothing beyond a
startup line — Ctrl-C to stop it. There's no client UI yet (see above), so
exercising it means speaking the wire protocol directly, e.g. with a small
`WsClient` program or any WebSocket client, sending JSON messages:

```json
{"type":"register","username":"alice","password":"hunter2"}
{"type":"login","username":"alice","password":"hunter2"}
{"type":"join_room","room_name":"my-room"}
{"type":"move","start":{"x":0,"y":6},"dest":{"x":0,"y":4}}
{"type":"jump","cell":{"x":0,"y":6}}
{"type":"resign"}
{"type":"quick_match"}
```

The server replies to `register`/`login` with `{"type":"..._result","success":...}`,
to `join_room`/`quick_match` with `{"type":"join_room_result","role":"opponent"|"spectator","color":"w"|"b"}`
(color omitted for spectators), and broadcasts `{"type":"game_state", ...}` to
every room member on every tick (~30ms) for the lifetime of the match.