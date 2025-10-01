# MQTT TTCN-3 Suite

Minimal TTCN-3 suite for MQTT CONNECT/CONNACK over UDP, using TITAN’s IPL4asp test port. The suite listens on UDP `7777`, checks an incoming CONNECT, and replies with a CONNACK to the sender.

Files
- `mqtt_connect.ttcn`: Testcase `tc_mqtt_connect_conack()` using `IPL4asp_PT`.
- `mqtt_connect.cfg`: Basic runtime configuration.
- `make.sh`: Helper to generate a Makefile and build against a vendored IPL4asp.

Prerequisites
- TITAN (Ubuntu): `sudo apt install eclipse-titan`
- Submodules initialized: `git submodule update --init --recursive`
- Vendored IPL4asp present at `deps/titan.TestPorts.IPL4asp/` (already added).

Build
- From this directory:
  - `./make.sh`
  - This script prefers the vendored IPL4asp and falls back to the system TITAN paths. It generates code, builds `IPL4asp_PT.cc`, and links the executable `mqtt_connect`.

Run
- Start your MQTT client to send a CONNECT to UDP `127.0.0.1:7777`.
- Execute the suite:
  - `ttcn3_start mqtt_connect mqtt_connect.cfg`
- The testcase responds with CONNACK (`20 02 00 00`) to the sender’s source port (for example, `7771` if the client used that source port).

Clean
- Remove generated `.cc/.hh`, objects, depfiles, logs, binary, and Makefile:
  - `./make.sh clean`

Notes
- We compile only `IPL4asp_Types` and `IPL4asp_PortType` plus `Socket_API_Definitions.ttcn` to avoid extra dependencies. No `IPL4asp_Functions` required.
- If TITAN is installed under `/usr/include/titan`, no manual `TTCN3_DIR` export is necessary. Otherwise, set `TTCN3_DIR` accordingly (e.g., `/usr/lib/titan`).
