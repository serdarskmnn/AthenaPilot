from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QBrush, QColor, QPen
from PyQt6.QtCore import Qt

class HUDWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.roll = 0.0  # degrees
        self.pitch = 0.0 # degrees

    def set_attitude(self, roll: float, pitch: float):
        self.roll = roll
        self.pitch = pitch
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        w = self.width()
        h = self.height()
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Fill background with sky
        painter.fillRect(0, 0, w, h, QColor(60, 140, 240))

        # Save and center
        painter.save()
        painter.translate(w/2, h/2)

        # Apply roll rotation
        painter.rotate(-self.roll)

        # Translate by pitch (vertical) - scale pitch degrees to pixels
        pitch_pixels = (self.pitch / 45.0) * (h/4)
        painter.translate(0, pitch_pixels)

        # Draw ground (brown) rectangle below horizon
        painter.fillRect(-w, 0, w*2, h*2, QColor(140, 90, 40))

        # Draw horizon line
        pen = QPen(QColor(255, 255, 255))
        pen.setWidth(2)
        painter.setPen(pen)
        painter.drawLine(-w, 0, w, 0)

        # Draw some pitch markers
        pen.setWidth(1)
        painter.setPen(pen)
        for off in range(-3, 4):
            y = off * 20
            painter.drawLine(-40, y, 40, y)

        # Restore transform
        painter.restore()

        # Draw aircraft symbol
        pen = QPen(QColor(255, 255, 255))
        pen.setWidth(2)
        painter.setPen(pen)
        painter.drawLine(w/2 - 40, h/2, w/2 - 10, h/2)
        painter.drawLine(w/2 + 40, h/2, w/2 + 10, h/2)
        painter.drawLine(w/2 - 10, h/2, w/2 + 10, h/2)
        painter.drawLine(w/2, h/2 - 10, w/2, h/2 + 10)
