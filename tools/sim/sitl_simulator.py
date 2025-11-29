#!/usr/bin/env python3
"""
Simple SITL simulator that sends sensor packets (CSV) to firmware on UDP 127.0.0.1:9001
then blocks waiting for motor commands on port 9002 (simulating Simulink lock-step).

It emits telemetry in a format the firmware understands: roll,pitch,yaw,thrust,alt,gs,batt,timestamp
and prints motor commands received from firmware.
"""
import socket
import time
import math
import sys
import argparse

def run(port_send=9001, port_recv=9002, host='127.0.0.1', hz=20):
    sock_send = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_recv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_recv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock_recv.bind((host, port_recv))
    print(f"SITL simulator: sending to {host}:{port_send}, receiving at {host}:{port_recv}")
    t = 0.0
    dt = 1.0 / hz
    try:
        while True:
            # create a simple oscillating sensor signal
            roll = 0.05 * math.sin(t)
            pitch = 0.05 * math.cos(t * 0.5)
            yaw = 0.02 * math.sin(t * 0.2)
            thrust = 0.6 + 0.1 * math.sin(t * 0.1)
            alt = 100.0 + 0.5 * math.sin(t * 0.3)
            gs = 20.0 + 0.5 * math.cos(t * 0.2)
            batt = 95.0 - 0.01 * t
            timestamp = time.time()
            msg = f"{roll},{pitch},{yaw},{thrust},{alt},{gs},{batt},{timestamp}"
            # send
            sock_send.sendto(msg.encode('utf-8'), (host, port_send))
            # now block for motor commands from firmware
            data, addr = sock_recv.recvfrom(4096)
            mstr = data.decode('utf-8')
            print(f"SITL got motors: {mstr}")
            # advance time
            time.sleep(dt)
            t += dt
    except KeyboardInterrupt:
        print("SITL simulator stopping")
    finally:
        sock_send.close()
        sock_recv.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='SITL Simulator: send sensors to firmware')
    parser.add_argument('--send-port', type=int, default=9001)
    parser.add_argument('--recv-port', type=int, default=9002)
    parser.add_argument('--host', type=str, default='127.0.0.1')
    parser.add_argument('--hz', type=int, default=20)
    args = parser.parse_args()
    run(port_send=args.send_port, port_recv=args.recv_port, host=args.host, hz=args.hz)
