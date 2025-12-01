from PyQt6.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QPushButton, QComboBox, QTextEdit, QLabel, QHBoxLayout
from PyQt6.QtCore import Qt, QTimer
from .hud_widget import HUDWidget
from ..comms.udp_link import UDPLink

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('AthenaPilot GCS')
        self.rc = {
            'roll': 1500,
            'pitch': 1500,
            'yaw': 1500,
            'throttle': 1000
        }
        self.key_pressed = set()
        self._init_ui()

        # UDP link
        self.link = UDPLink(listen_port=9003, send_port=9000)
        self.link.telemetry_received.connect(self._on_telemetry)
        self.link.console_received.connect(self._on_console)
        self.link.start_listening()

        # timer - send RC periodically
        self.send_timer = QTimer(self)
        self.send_timer.timeout.connect(self._send_rc)
        self.send_timer.start(50)

    def _init_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        layout = QHBoxLayout()
        central.setLayout(layout)

        left = QVBoxLayout()
        right = QVBoxLayout()
        layout.addLayout(left, 3)
        layout.addLayout(right, 1)

        # HUD
        self.hud = HUDWidget(self)
        self.hud.setMinimumSize(600,400)
        left.addWidget(self.hud)

        # Map placeholder
        self.map_placeholder = QLabel('Map (placeholder)')
        self.map_placeholder.setAlignment(Qt.AlignmentFlag.AlignCenter)
        left.addWidget(self.map_placeholder)

        # Controls
        self.arm_btn = QPushButton('Arm')
        self.arm_btn.setCheckable(True)
        self.arm_btn.clicked.connect(self._on_arm)
        right.addWidget(self.arm_btn)

        self.mode_combo = QComboBox()
        self.mode_combo.addItems(['STABILIZE','LOITER','LAND'])
        self.mode_combo.currentTextChanged.connect(self._on_mode_change)
        right.addWidget(self.mode_combo)

        self.console = QTextEdit()
        self.console.setReadOnly(True)
        right.addWidget(self.console)

    def _on_telemetry(self, roll, pitch, yaw):
        # update HUD
        self.hud.update_attitude(roll, pitch)
        self.console.append(f"Telemetry: R={roll:.2f}, P={pitch:.2f}, Y={yaw:.2f}")

    def _on_console(self, text):
        self.console.append(text)

    def _on_arm(self, checked):
        if checked:
            self.arm_btn.setText('Disarm')
            self.link.send('ARM,1')
        else:
            self.arm_btn.setText('Arm')
            self.link.send('ARM,0')

    def _on_mode_change(self, mode_text):
        self.link.send(f"MODE,{mode_text}")

    def _send_rc(self):
        msg = f"RC,{self.rc['roll']},{self.rc['pitch']},{self.rc['yaw']},{self.rc['throttle']}"
        self.link.send(msg)

    # Key presses: W/S (pitch), A/D (roll), Z/X (yaw), F/V (throttle)
    def keyPressEvent(self, event):
        k = event.key()
        step = 10
        if k == Qt.Key.Key_W:
            self.rc['pitch'] -= step
            self.key_pressed.add('W')
        elif k == Qt.Key.Key_S:
            self.rc['pitch'] += step
            self.key_pressed.add('S')
        elif k == Qt.Key.Key_A:
            self.rc['roll'] -= step
            self.key_pressed.add('A')
        elif k == Qt.Key.Key_D:
            self.rc['roll'] += step
            self.key_pressed.add('D')
        elif k == Qt.Key.Key_Z:
            self.rc['yaw'] -= step
            self.key_pressed.add('Z')
        elif k == Qt.Key.Key_X:
            self.rc['yaw'] += step
            self.key_pressed.add('X')
        elif k == Qt.Key.Key_F:
            self.rc['throttle'] = min(2000, self.rc['throttle'] + step)
            self.key_pressed.add('F')
        elif k == Qt.Key.Key_V:
            self.rc['throttle'] = max(1000, self.rc['throttle'] - step)
            self.key_pressed.add('V')
        self._clamp_rc()

    def keyReleaseEvent(self, event):
        k = event.key()
        if k == Qt.Key.Key_W:
            self.key_pressed.discard('W')
            self.rc['pitch'] = 1500
        elif k == Qt.Key.Key_S:
            self.key_pressed.discard('S')
            self.rc['pitch'] = 1500
        elif k == Qt.Key.Key_A:
            self.key_pressed.discard('A')
            self.rc['roll'] = 1500
        elif k == Qt.Key.Key_D:
            self.key_pressed.discard('D')
            self.rc['roll'] = 1500
        elif k == Qt.Key.Key_Z:
            self.key_pressed.discard('Z')
            self.rc['yaw'] = 1500
        elif k == Qt.Key.Key_X:
            self.key_pressed.discard('X')
            self.rc['yaw'] = 1500
        elif k == Qt.Key.Key_F:
            self.key_pressed.discard('F')
            # throttle persists
        elif k == Qt.Key.Key_V:
            self.key_pressed.discard('V')
        self._clamp_rc()

    def _clamp_rc(self):
        for ch in ['roll','pitch','yaw']:
            self.rc[ch] = max(1000, min(2000, self.rc[ch]))
        self.rc['throttle'] = max(1000, min(2000, self.rc['throttle']))
