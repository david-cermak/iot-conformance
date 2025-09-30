# TTCN-3 Build & Run (src)

This directory contains TTCN-3 suites and build glue for Eclipse TITAN. Follow these steps on Ubuntu to build and run the suites locally, or bridge them to an ESP32 target.

## Install (Ubuntu)

```
sudo apt update
sudo apt install --no-install-recommends \
  eclipse-titan g++ make libxml2-dev libssl-dev expect
# Optional (serial bridge):
sudo apt install python3-pip && pip3 install pyserial
```

Verify tools:
```
which ttcn3_makefilegen && ttcn3_makefilegen -v
which ttcn3_start && ttcn3_start -v
```

## Build

From this `src/` directory:
```
./make.sh        # generates Makefile and builds `test_suite`
# or incremental
make -j$(nproc)
```
Artifacts: `test_suite` (executable), generated `*.cc/*.hh`, `*.o`.

## Run (PC/local)

Default UDP endpoints (configurable in `tcp_suite.cfg`):
- Suite: 127.0.0.1:7777
- SUT bridge: 0.0.0.0:7771

Run suites from `src/`:
```
# Sample TCP black-box suite
ttcn3_start test_suite tcp_suite.cfg
# TCP2 sanity suite
ttcn3_start test_suite tcp2_check_3_runs.cfg
# Single test
ttcn3_start test_suite tcp2_check_3_runs.cfg tcp2_check.test_tcp_connect_data_close
```

## Run with ESP32 over UART (optional)

1) Build/flash the harness in `network_tests/` using ESP-IDF (`idf.py build && idf.py -p PORT flash`).
2) Start the serial bridge (converts UDP<->hex lines on UART):
```
python3 net_suite_test.py --port /dev/ttyUSB0 --baud 115200 \
  --udp-in 7771 --udp-dst 127.0.0.1 --udp-out 7777
```
3) Run the suite as above. The device prints `PacketOut:[..]` lines; the bridge forwards them to UDP:7777 and injects UDP:7771 payloads as hex to the device.

Tip: add your user to the `dialout` group for serial access:
```
sudo usermod -aG dialout $USER  # re-login required
```

## Troubleshooting
- Command not found: install `eclipse-titan` and recheck `$PATH`.
- Build includes TITAN via system paths; override with `TTCN3_DIR=/usr` if needed.
- Ensure UDP ports 7771/7777 are free and loopback-accessible.

