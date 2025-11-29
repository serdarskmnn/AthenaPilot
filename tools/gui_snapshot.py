#!/usr/bin/env python3
import sys
import time
from PyQt6.QtWidgets import QApplication
from PyQt6.QtCore import QTimer
from ground_control.gui.main_window import MainWindow
from ground_control.comms.udp_link import UDPLink

def take_snapshot(output='gui_snapshot.png', timeout=3):
    app = QApplication(sys.argv)
    udp = UDPLink(listen_port=9003)
    udp.start()
    window = MainWindow(udp_link=udp)
    window.show()

    def save_and_exit():
        pixmap = window.grab()
        pixmap.save(output)
        print('Saved snapshot to', output)
        udp.stop()
        app.quit()

    # give time for telemetry to arrive and update GUI (if any)
    QTimer.singleShot(timeout * 1000, save_and_exit)
    app.exec()

if __name__ == '__main__':
    take_snapshot()
