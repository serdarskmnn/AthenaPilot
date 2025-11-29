from PyQt6.QtWidgets import QMainWindow, QLabel, QVBoxLayout, QWidget, QHBoxLayout, QSizePolicy
from PyQt6.QtCore import Qt, QUrl
try:
    from PyQt6.QtWebEngineWidgets import QWebEngineView
    WEB_ENGINE_AVAILABLE = True
except Exception:
    WEB_ENGINE_AVAILABLE = False

class MainWindow(QMainWindow):
    def __init__(self, udp_link=None):
        super().__init__()
        self.setWindowTitle('AthenaPilot Ground Control')
        self.resize(900, 600)
        self._udp_link = udp_link

        central = QWidget(self)
        layout = QHBoxLayout(central)

        left = QVBoxLayout()
        # Telemetry labels
        self.alt_label = QLabel('Altitude: -- m')
        self.gs_label = QLabel('Ground Speed: -- m/s')
        self.batt_label = QLabel('Battery: -- %')
        for w in (self.alt_label, self.gs_label, self.batt_label):
            w.setAlignment(Qt.AlignmentFlag.AlignLeft)
            w.setStyleSheet('font-size: 16px; padding: 6px;')
            left.addWidget(w)

        left.addStretch(1)

        # Placeholder map area
        if WEB_ENGINE_AVAILABLE:
            self.map_view = QWebEngineView()
            # Load simple placeholder (about:blank or a local html file)
            self.map_view.setUrl(QUrl('about:blank'))
            self.map_view.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            right_widget = self.map_view
        else:
            right_widget = QLabel('Map placeholder (QtWebEngine not available)')
            right_widget.setAlignment(Qt.AlignmentFlag.AlignCenter)
            right_widget.setStyleSheet('border: 1px solid #666; font-size: 18px;')

        layout.addLayout(left, 0)
        layout.addWidget(right_widget, 1)

        self.setCentralWidget(central)

        # connect to UDP link
        if self._udp_link is not None:
            self._udp_link.telem_received.connect(self.on_telemetry)

    def on_telemetry(self, data: dict):
        # Update labels safely
        # For debugging in headless mode, print telemetry to console
        try:
            print('TELEM', data)
        except Exception:
            pass
        if 'alt' in data:
            self.alt_label.setText(f"Altitude: {data['alt']:.2f} m")
        if 'gs' in data:
            self.gs_label.setText(f"Ground Speed: {data['gs']:.2f} m/s")
        if 'batt' in data:
            self.batt_label.setText(f"Battery: {data['batt']:.1f} %")