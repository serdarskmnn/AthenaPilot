from PyQt6.QtWidgets import QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QTextEdit, QLabel, QComboBox
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QKeyEvent

from .hud_widget import HUDWidget
from ..comms.udp_link import UDPLink

class MainWindow(QMainWindow):
    def __init__(self, udp_link: UDPLink, parent=None):
        super().__init__(parent)
        self.setWindowTitle('AthenaPilot Ground Control')
        self.resize(900, 600)

        self.udp = udp_link
        self.udp.telemetry_callback = self.on_telemetry

        self.hud = HUDWidget(self)
        self.console = QTextEdit(self)
        self.console.setReadOnly(True)

        self.arm_button = QPushButton("Arm")
        self.arm_button.setStyleSheet("background-color: red; color: white; font-size: 18px")
        self.arm_button.clicked.connect(self.toggle_arm)
        self.is_armed = False

        self.mode_combo = QComboBox(self)
        self.mode_combo.addItems(['STABILIZE', 'LOITER', 'LAND'])
        self.mode_combo.currentTextChanged.connect(self.set_mode)

        # Create layout
        right_layout = QVBoxLayout()
        right_layout.addWidget(self.arm_button)
        right_layout.addWidget(QLabel('Mode'))
        right_layout.addWidget(self.mode_combo)
        right_layout.addStretch()

        top_layout = QHBoxLayout()
        top_layout.addWidget(self.hud, 2)
        right_container = QWidget()
        right_container.setLayout(right_layout)
        top_layout.addWidget(right_container, 1)

        main_layout = QVBoxLayout()
        main_layout.addLayout(top_layout, 3)
        main_layout.addWidget(QLabel('Console'))
        main_layout.addWidget(self.console, 1)

        central = QWidget()
        central.setLayout(main_layout)
        self.setCentralWidget(central)

        # RC channels - start centered
        self.rc = {'roll':1500, 'pitch':1500, 'yaw':1500, 'throttle':1000, 'arm':0, 'mode':'STABILIZE'}
        # push initial command
        self.send_rc()

    def toggle_arm(self):
        self.is_armed = not self.is_armed
        self.arm_button.setText('Disarm' if self.is_armed else 'Arm')
        self.arm_button.setStyleSheet("background-color: green; color: white; font-size: 18px" if self.is_armed else "background-color: red; color: white; font-size: 18px")
        self.rc['arm'] = 1 if self.is_armed else 0
        self.send_rc()

    def set_mode(self, mode_text):
        self.rc['mode'] = mode_text
        self.udp.send_mode(mode_text)
        self.send_rc()

    def keyPressEvent(self, event: QKeyEvent):
        key = event.key()
        step = 50
        if key == Qt.Key.Key_W:
            self.rc['pitch'] = max(1000, self.rc['pitch'] - step)
        elif key == Qt.Key.Key_S:
            self.rc['pitch'] = min(2000, self.rc['pitch'] + step)
        elif key == Qt.Key.Key_A:
            self.rc['roll'] = max(1000, self.rc['roll'] - step)
        elif key == Qt.Key.Key_D:
            self.rc['roll'] = min(2000, self.rc['roll'] + step)
        elif key == Qt.Key.Key_Up:
            self.rc['throttle'] = min(2000, self.rc['throttle'] + int(step/2))
        elif key == Qt.Key.Key_Down:
            self.rc['throttle'] = max(1000, self.rc['throttle'] - int(step/2))
        elif key == Qt.Key.Key_Left:
            self.rc['yaw'] = max(1000, self.rc['yaw'] - step)
        elif key == Qt.Key.Key_Right:
            self.rc['yaw'] = min(2000, self.rc['yaw'] + step)
        else:
            super().keyPressEvent(event)
            return
        self.send_rc()

    def send_rc(self):
        self.udp.send_rc(self.rc['roll'], self.rc['pitch'], self.rc['yaw'], self.rc['throttle'], self.rc['arm'], self.rc['mode'])
        self.append_console(f"Sent RC: R:{self.rc['roll']} P:{self.rc['pitch']} Y:{self.rc['yaw']} T:{self.rc['throttle']} ARM:{self.rc['arm']} MODE:{self.rc['mode']}")

    def on_telemetry(self, text: str):
        # parse telemetry: ROLL:0.12,PITCH:-0.05,YAW:1.2,ARMED:1,MODE:1,BATT:12.34
        parts = [p.strip() for p in text.split(',')]
        telem = {}
        for p in parts:
            if ':' in p:
                k, v = p.split(':', 1)
                telem[k.upper()] = v

        roll = float(telem.get('ROLL', 0.0))
        pitch = float(telem.get('PITCH', 0.0))
        yaw = float(telem.get('YAW', 0.0))
        armed = int(telem.get('ARMED', '0'))
        batt = float(telem.get('BATT', '0.0'))

        # update HUD
        self.hud.set_attitude(roll, pitch)
        self.append_console(f"Telemetry: R:{roll:.2f} P:{pitch:.2f} Y:{yaw:.2f} ARM:{armed} BATT:{batt:.2f}")

    def append_console(self, txt: str):
        self.console.append(txt)
