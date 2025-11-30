import socket
import threading
import time

class UDPLink:
    """Simple UDP link between GCS and C++ firmware.

    - Sends RC commands to 127.0.0.1:9000
    - Listens for telemetry on 127.0.0.1:9003
    """
    def __init__(self, rc_host='127.0.0.1', rc_port=9000, telemetry_port=9003, telemetry_callback=None):
        self.rc_host = rc_host
        self.rc_port = rc_port
        self.telemetry_port = telemetry_port
        self.telemetry_callback = telemetry_callback

        self.sock_out = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.sock_in = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock_in.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock_in.bind((rc_host, telemetry_port))
        self.running = True
        self._listener = threading.Thread(target=self._listener_thread, daemon=True)
        self._listener.start()

    def close(self):
        self.running = False
        try:
            self.sock_in.close()
        except Exception:
            pass

    def _listener_thread(self):
        while self.running:
            try:
                data, addr = self.sock_in.recvfrom(4096)
                text = data.decode('utf-8')
                if self.telemetry_callback:
                    self.telemetry_callback(text)
            except Exception:
                time.sleep(0.01)

    def _send(self, text: str):
        self.sock_out.sendto(text.encode('utf-8'), (self.rc_host, self.rc_port))

    def send_rc(self, roll: int, pitch: int, yaw: int, throttle: int, arm: int = 0, mode: str = 'STABILIZE'):
        # Compose CSV message: "ROLL:1500,PITCH:1500,YAW:1500,THROTTLE:1000,ARM:0,MODE:STABILIZE"
        text = f"ROLL:{roll},PITCH:{pitch},YAW:{yaw},THROTTLE:{throttle},ARM:{arm},MODE:{mode}"
        self._send(text)

    def send_arm(self, arm: bool):
        self._send(f"ARM:{1 if arm else 0}")

    def send_mode(self, mode: str):
        self._send(f"MODE:{mode}")

    def send_custom(self, msg: str):
        self._send(msg)
