import sys
import os
from PyQt6.QtWidgets import QApplication

# Allow running this file either from repository root or from inside the ground_control
# directory by ensuring the repo root is on sys.path when needed. This makes both
# `python3 ground_control/main.py` (from repo root) and `cd ground_control && python3 main.py` work.
if __package__ is None:
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    if repo_root not in sys.path:
        sys.path.insert(0, repo_root)

from ground_control.gui.main_window import MainWindow

def main():
    app = QApplication(sys.argv)
    w = MainWindow()
    w.show()
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
