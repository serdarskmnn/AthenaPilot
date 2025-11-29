from PyQt6.QtCore import QObject, pyqtSignal
import socket
import threading
import time

class UDPLink(QObject):
    telem_received = pyqtSignal(dict)

    def __init__(self, listen_port=9003, parent=None):
        super().__init__(parent)
        self.listen_port = listen_port
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(("127.0.0.1", self.listen_port))
        self._running = False
        self._thread = None

    def parse_telemetry(self, data: str):
        # data like: TELEM,alt=...,gs=...,batt=...,r=...,p=...,y=...,m1=...,m2=...
        fields = data.strip().split(',')
        result = {}
        for f in fields:
            if '=' in f:
                k, v = f.split('=', 1)
                try:
                    result[k] = float(v)
                except ValueError:
                    result[k] = v
            else:
                # First token 'TELEM' or others
                pass
        return result

    def _run_receiver(self):
        self._sock.setblocking(True)
        while self._running:
            try:
                data, addr = self._sock.recvfrom(2048)
                text = data.decode('utf-8')
                parsed = self.parse_telemetry(text)
                self.telem_received.emit(parsed)
            except OSError:
                break
            except Exception as e:
                # Don't kill thread on parse error
                print("UDPLink error:", e)
                time.sleep(0.01)

    def start(self):
        if self._running: return
        self._running = True
        self._thread = threading.Thread(target=self._run_receiver, daemon=True)
        self._thread.start()

    def stop(self):
        self._running = False
        try:
            self._sock.close()
        except Exception:
            pass
        if self._thread:
            self._thread.join(timeout=0.5)

    def __del__(self):
        self.stop()