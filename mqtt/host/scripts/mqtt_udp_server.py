#!/usr/bin/env python3
import socket
import argparse


def main():
    p = argparse.ArgumentParser(description="Minimal MQTT UDP server: replies to CONNECT with CONNACK")
    p.add_argument("--bind", default="127.0.0.1", help="Bind address for incoming (client->server)")
    p.add_argument("--port", type=int, default=7777, help="Bind port for incoming (client->server)")
    args = p.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.bind, args.port))
    print(f"MQTT-UDP server listening on {args.bind}:{args.port}")

    while True:
        data, addr = sock.recvfrom(4096)
        if not data:
            continue
        # MQTT CONNECT packet starts with 0x10
        pkt_type = data[0] & 0xF0
        if pkt_type == 0x10:
            print(f"CONNECT from {addr}, len={len(data)} -> replying CONNACK")
            # MQTT 3.1.1 CONNACK: 0x20 0x02 0x00 0x00 (session present=0, return code=0)
            connack = bytes([0x20, 0x02, 0x00, 0x00])
            sock.sendto(connack, addr)
        else:
            print(f"Non-CONNECT packet type 0x{pkt_type:02x} from {addr}, ignoring")


if __name__ == "__main__":
    main()

