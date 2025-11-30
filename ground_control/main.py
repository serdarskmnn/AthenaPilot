import sys
import os
try:
    from PyQt6.QtWidgets import QApplication
except Exception as e:
    print('PyQt6 is required to run the GCS. Install with: python -m pip install PyQt6')
    raise

# When run as a script from the package directory (or elsewhere), ensure the repo root
# is on sys.path so the package imports will work whether run as a module or directly.
if __package__ is None:
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    if repo_root not in sys.path:
        sys.path.insert(0, repo_root)
    __package__ = 'ground_control'

from .comms.udp_link import UDPLink
from .gui.main_window import MainWindow


def main():
    app = QApplication(sys.argv)
    # Create UDP link and pass to UI
    udp = UDPLink(rc_host='127.0.0.1', rc_port=9000, telemetry_port=9003)
    win = MainWindow(udp)
    win.show()
    return app.exec()

if __name__ == '__main__':
    exit(main())
