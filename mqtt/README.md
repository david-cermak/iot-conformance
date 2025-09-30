# MQTT Conformance Suite (Host)

This suite runs MQTT client logic on Linux using ESP-IDF’s `esp-mqtt` and a mock TCP transport over UDP. No target firmware is used in this step.

## Layout
- `host/`: IDF project targeting Linux. Builds `esp-mqtt` and a mock `tcp_transport` that forwards bytes via UDP (`7771` in, `7777` out).
- `suite/`: TTCN-3 suite placeholder (to be added in the next step).

## Build (Linux target)
1) Ensure submodules are initialized and `IDF_PATH` points to `deps/idf`:
   - `git submodule update --init --recursive`
   - `export IDF_PATH="$(pwd)/deps/idf" && . "$IDF_PATH/export.sh"`
2) Configure Linux target (from `mqtt/host`):
   - `idf.py --preview set-target linux`
3) Build:
   - `idf.py build`

Notes
- Linux compatibility layers are used from `deps/protocols/common_components/linux_compat/{freertos,esp_timer}`.
- The project overrides IDF’s `tcp_transport` via a local component providing UDP-based transport.

## Run
In one terminal (server replying CONNACK):
- `python3 host/scripts/mqtt_udp_server.py`  (binds `127.0.0.1:7777`)

In another terminal (client):
- `idf.py monitor`  (from `mqtt/host`) or run the built ELF in `build/`.

Environment overrides
- `MQTT_UDP_DST` (default `127.0.0.1`)
- `MQTT_UDP_OUT` (default `7777`, client→server)
- `MQTT_UDP_IN`  (default `7771`, server→client)

Expected result
- Client logs `MQTT_EVENT_CONNECTED` after exchanging CONNECT/CONNACK via UDP.

Next step
- Add TTCN-3 suite in `suite/` to drive scenarios against the host client.
