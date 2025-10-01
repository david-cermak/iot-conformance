# MQTT Conformance (Overview)

Two parts:
- Host client prototype (`host/`): ESP-IDF’s `esp-mqtt` targeting Linux with a UDP-backed transport.
- TTCN-3 suite (`suite/`): Minimal CONNECT/CONNACK validation using TITAN IPL4asp over UDP.

Layout
- `host/`: IDF project; optional for early TTCN testing if you have another client.
- `suite/`: Ready-to-build TTCN-3 suite; includes a build helper.

Build TTCN-3 suite
- Prereqs: `sudo apt install eclipse-titan`; `git submodule update --init --recursive`.
- Build from `mqtt/suite`:
  - `./make.sh`
  - This vendors IPL4asp from `deps/titan.TestPorts.IPL4asp` and links `IPL4asp_PT.cc`.

Run
- Send an MQTT CONNECT to `127.0.0.1:7777` (any client/source port).
- Execute the suite: `ttcn3_start mqtt_connect mqtt_connect.cfg`.
- The suite replies CONNACK to the sender’s source port.

Clean
- `cd mqtt/suite && ./make.sh clean` removes generated `.cc/.hh`, objects, depfiles, logs, binary, and Makefile.

Host client (optional)
- If you want to drive `esp-mqtt` over UDP, see `mqtt/host` (Linux target: `idf.py --preview set-target linux`).
