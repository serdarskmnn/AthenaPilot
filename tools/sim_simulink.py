"""
Small simulator that mimics Simulink behavior for the lock-step: send SENSOR packets to firmware (port 9002)
and wait for MOTOR responses on port 9001.

Usage:
    python3 tools/sim_simulink.py
"""
import socket
import time

SIM_SEND_PORT = 9002  # Simulink -> Firmware (sensor)
SIM_RECV_PORT = 9001  # Firmware -> Simulink (motors)
HOST = '127.0.0.1'

def main():
    send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind receive socket to motor port
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    recv_sock.bind((HOST, SIM_RECV_PORT))
    print('Simulink simulator started. Sending Sensor to 127.0.0.1:9002, waiting for motor reply on 9001')

    roll = 0.0
    pitch = 0.0
    yaw = 0.0
    dt = 0.02
    seq = 0
    try:
        while True:
            seq += 1
            msg = f"SENSOR,{roll:.2f},{pitch:.2f},{yaw:.2f},{dt:.3f}"
            send_sock.sendto(msg.encode('utf-8'), (HOST, SIM_SEND_PORT))
            print(f"Sent: {msg}")
            # Wait for motor reply (block until received)
            data, addr = recv_sock.recvfrom(4096)
            s = data.decode('utf-8')
            print(f"Recv from firmware: {s}")
            # Simple physics: adjust roll/pitch/yaw based on small delta to show changes
            roll += 0.1
            pitch += -0.05
            yaw += 0.02
            time.sleep(dt)
    except KeyboardInterrupt:
        print('\nSim stopped by user')

if __name__ == '__main__':
    main()
