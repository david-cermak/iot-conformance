import argparse
import os
import re
import socket
import sys
import time
from threading import Event, Thread

try:
    import serial  # pyserial
except ImportError as e:
    serial = None

stop_sock_listener = Event()
stop_io_listener = Event()
sock = None


def io_listener(ser: "serial.Serial", udp_out_addr: str, udp_out_port: int):
    """Read UART lines, detect PacketOut, forward bytes to UDP."""
    global sock
    pattern = re.compile(r"PacketOut:\[([a-fA-F0-9]+)\]")
    while not stop_io_listener.is_set():
        try:
            line = ser.readline()  # reads until '\n'
        except Exception:
            time.sleep(0.1)
            continue
        if not line:
            continue
        try:
            text = line.decode(errors="ignore").strip()
        except Exception:
            continue
        m = pattern.search(text)
        if not m:
            continue
        hex_payload = m.group(1)
        try:
            data = bytearray.fromhex(hex_payload)
        except ValueError:
            continue
        try:
            if sock is not None:
                sock.sendto(data, (udp_out_addr, udp_out_port))
        except Exception:
            pass


def sock_listener(ser: "serial.Serial", bind_addr: str, udp_in_port: int):
    """Listen on UDP, forward incoming bytes to UART as hex line terminated by CR."""
    global sock
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.5)
    sock.bind((bind_addr, udp_in_port))
    try:
        while not stop_sock_listener.is_set():
            try:
                payload, client = sock.recvfrom(2048)
            except socket.timeout:
                continue
            except Exception:
                break
            # Convert bytes to hex string with spaces; device expects hex per byte on a single line
            packet_hex = " ".join(f"{b:02x}" for b in payload)
            try:
                ser.write(packet_hex.encode("ascii") + b"\r")  # CR ends line on target
            except Exception:
                time.sleep(0.05)
                continue
    finally:
        try:
            sock.close()
        finally:
            sock = None


def parse_args():
    p = argparse.ArgumentParser(description="Bridge TTCN-3 UDP <-> serial for ESP32 net suite")
    p.add_argument("--port", default=os.environ.get("NET_SUITE_SERIAL", "/dev/ttyUSB0"), help="Serial port path")
    p.add_argument("--baud", type=int, default=int(os.environ.get("NET_SUITE_BAUD", "115200")), help="Serial baud rate")
    p.add_argument("--bind", default=os.environ.get("NET_SUITE_BIND", "0.0.0.0"), help="UDP bind address for incoming packets")
    p.add_argument("--udp-in", type=int, default=int(os.environ.get("NET_SUITE_UDP_IN", "7771")), help="UDP port to receive from suite tools")
    p.add_argument("--udp-out", type=int, default=int(os.environ.get("NET_SUITE_UDP_OUT", "7777")), help="UDP port to send PacketOut to")
    p.add_argument("--udp-dst", default=os.environ.get("NET_SUITE_UDP_DST", "127.0.0.1"), help="UDP destination address for PacketOut")
    return p.parse_args()


def main():
    if serial is None:
        print("pyserial is required. Install with: pip install pyserial", file=sys.stderr)
        sys.exit(1)

    args = parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.2)
    except Exception as e:
        print(f"Failed to open serial port {args.port}: {e}", file=sys.stderr)
        sys.exit(2)

    # Start bridge threads
    stop_sock_listener.clear()
    stop_io_listener.clear()
    t_sock = Thread(target=sock_listener, args=(ser, args.bind, args.udp_in), daemon=True)
    t_io = Thread(target=io_listener, args=(ser, args.udp_dst, args.udp_out), daemon=True)
    t_sock.start()
    t_io.start()

    print(f"Serial bridge running on {args.port} @ {args.baud} | UDP in {args.bind}:{args.udp_in} -> UART | UART PacketOut -> {args.udp_dst}:{args.udp_out}")
    try:
        while t_sock.is_alive() and t_io.is_alive():
            time.sleep(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        stop_io_listener.set()
        stop_sock_listener.set()
        t_sock.join(timeout=1.0)
        t_io.join(timeout=1.0)
        try:
            ser.close()
        except Exception:
            pass


if __name__ == '__main__':
    main()
