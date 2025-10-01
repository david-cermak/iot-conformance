# iot-conformance

Conformance test workspace for IoT protocols.

Highlights
- TCP suites and ESP32 harness (`tcp/`)
- MQTT prototype and TTCN-3 suite (`mqtt/`)

Quick start (MQTT TTCN-3)
- Install TITAN: `sudo apt install eclipse-titan`
- Init submodules: `git submodule update --init --recursive`
- Build suite: `cd mqtt/suite && ./make.sh`
- Run suite: `ttcn3_start mqtt_connect mqtt_connect.cfg`

Notes
- TITAN IPL4asp is vendored at `deps/titan.TestPorts.IPL4asp` and used by the suite.
