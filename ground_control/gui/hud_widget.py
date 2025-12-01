from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QColor, QPen
from PyQt6.QtCore import Qt, QRectF

class HUDWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.roll = 0.0
        self.pitch = 0.0

    def update_attitude(self, roll, pitch):
        self.roll = roll
        self.pitch = pitch
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        w = self.width()
        h = self.height()
        # fill sky and ground with a horizon line that moves based on pitch
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        # Background sky
        painter.save()
        painter.translate(w/2, h/2)
        painter.rotate(-self.roll)  # Roll rotates the horizon
        # Pitch translates the horizon up/down; pitch positive means nose up
        pitch_offset = (self.pitch / 30.0) * (h/2)  # scale pitch
        painter.translate(0, pitch_offset)

        # Draw sky
        painter.fillRect(-w, -h*2, w*2, h*2, QColor('#87CEEB'))
        # Draw ground
        painter.fillRect(-w, 0, w*2, h*2, QColor('#8B4513'))

        # Horizon line
        pen = QPen(Qt.GlobalColor.black)
        pen.setWidth(2)
        painter.setPen(pen)
        painter.drawLine(-w, 0, w, 0)

        painter.restore()

        # Center crosshair (use integer coordinates to satisfy QPainter methods)
        pen = QPen(Qt.GlobalColor.white)
        pen.setWidth(2)
        painter.setPen(pen)
        cx = int(w/2)
        cy = int(h/2)
        painter.drawLine(cx - 20, cy, cx + 20, cy)
        painter.drawLine(cx, cy - 20, cx, cy + 20)

        painter.end()
