import sys
from PyQt6.QtWidgets import QApplication
from ground_control.gui.main_window import MainWindow
from ground_control.comms.udp_link import UDPLink

def main():
    app = QApplication(sys.argv)
    udp_link = UDPLink(listen_port=9003)
    udp_link.start()
    window = MainWindow(udp_link=udp_link)
    window.show()
    try:
        result = app.exec()
    finally:
        udp_link.stop()
    sys.exit(result)

if __name__ == '__main__':
    main()