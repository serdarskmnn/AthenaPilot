import socket
import threading
import time
from PyQt6.QtCore import QObject, pyqtSignal


class UDPLink(QObject):
    telemetry_received = pyqtSignal(float, float, float)
    console_received = pyqtSignal(str)

    def __init__(self, listen_port=9003, send_port=9000, parent=None):
        super().__init__(parent)
        self.listen_port = listen_port
        self.send_port = send_port
        self.local_addr = ('127.0.0.1', self.send_port)
        # Send socket
        self.send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        # Listen socket
        self.recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.recv_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.recv_sock.bind(('127.0.0.1', self.listen_port))
        self.recv_sock.setblocking(False)
        self._running = False

    def send(self, message: str):
        try:
            self.send_sock.sendto(message.encode('utf-8'), self.local_addr)
            self.console_received.emit(f"Sent: {message}")
        except Exception as ex:
            self.console_received.emit(f"Send error: {ex}")

    def start_listening(self):
        self._running = True
        threading.Thread(target=self._listen_loop, daemon=True).start()

    def stop_listening(self):
        self._running = False

    def _listen_loop(self):
        while self._running:
            try:
                data, addr = self.recv_sock.recvfrom(4096)
                s = data.decode('utf-8')
                self.console_received.emit(f"Recv: {s}")
                if s.startswith('TELEM,'):
                    parts = s.split(',')
                    try:
                        r = float(parts[1]); p = float(parts[2]); y = float(parts[3])
                        self.telemetry_received.emit(r, p, y)
                    except Exception as ex:
                        self.console_received.emit(f"Telemetry parse error: {ex}")
            except BlockingIOError:
                # nothing to read
                time.sleep(0.02)
            except Exception as ex:
                self.console_received.emit(f"Recv error: {ex}")
                time.sleep(0.1)
