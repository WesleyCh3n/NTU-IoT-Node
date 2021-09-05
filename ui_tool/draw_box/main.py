#!/usr/bin/python3
# -*- coding: utf-8 -*-

import random
import cv2
import csv
import qdarkstyle

from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure
from matplotlib.widgets  import RectangleSelector
import matplotlib.pyplot as plt

from PyQt5 import uic
from PyQt5.QtCore import (QTimer)
from PyQt5.QtWidgets import (QMainWindow, QApplication, QFileDialog, QWidget, QDesktopWidget)


plt.style.use("seaborn-dark")
for param in ['figure.facecolor', 'axes.facecolor', 'savefig.facecolor']:
    plt.rcParams[param] = '#19232d'  # bluish dark grey
for param in ['text.color', 'axes.labelcolor', 'xtick.color', 'ytick.color']:
    plt.rcParams[param] = '0.9'  # very light grey

def line_select_callback(eclick, erelease):
    'eclick and erelease are the press and release events'
    x1, y1 = eclick.xdata, eclick.ydata
    x2, y2 = erelease.xdata, erelease.ydata


def toggle_selector(event):
    print(' Key pressed.')
    if event.key in ['Q', 'q'] and toggle_selector.RS.active:
        print(' RectangleSelector deactivated.')
        toggle_selector.RS.set_active(False)
    if event.key in ['A', 'a'] and not toggle_selector.RS.active:
        print(' RectangleSelector activated.')
        toggle_selector.RS.set_active(True)


class MyCanvas(FigureCanvas):
    """Canvas"""
    def __init__(self):
        fig = Figure()
        self.axes = fig.add_subplot(111)
        self.axes.grid(color='#2A3459')
        fig.tight_layout()
        super().__init__(fig)
        toggle_selector.RS = RectangleSelector(
            self.axes, line_select_callback,
            drawtype='box', useblit=True,
            button=[1, 3],  # don't use middle button
            minspanx=5, minspany=5,
            spancoords='pixels',
            rectprops = dict(facecolor='green', edgecolor = 'green', alpha=0.5)
        )


class LogWindow(QWidget):
    """
    This "window" is a QWidget. If it has no parent, it
    will appear as a free-floating window as we want.
    """
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        uic.loadUi('./log.ui', self)


class Window(QMainWindow):
    """MainWindow"""
    def __init__(self):
        super().__init__()
        uic.loadUi('./main.ui', self)
        self.img = None
        self.modImg = None

        # bbox attribute
        self.x0 = None
        self.y0 = None
        self.x1 = None
        self.y1 = None
        self.cnt = 0
        self.bbox01 = []
        self.bbox02 = []

        # embeded matplotlib
        self.canvas = MyCanvas()
        self.toolbar = NavigationToolbar(self.canvas, self)
        self.layout.addWidget(self.toolbar)
        self.layout.addWidget(self.canvas)

        # connect signal
        self.resetBn.clicked.connect(self.reset_img)
        self.actionOpen_Image.triggered.connect(self.open_image)
        self.logBn.clicked.connect(self.open_log)
        self.csvBn.clicked.connect(self.save_csv)
        #  self.cntBox.valueChanged.connect(self.update_cnt)

        self.canvas.mpl_connect('key_press_event', toggle_selector)
        self.canvas.mpl_connect('button_release_event', self.on_release)

        self.w = LogWindow()

    def open_image(self):
        path = QFileDialog.getOpenFileName(self,"Open file","","Images(*.jpg *.bmp *.jpeg)")
        if path[0] == '':
            return
        self.img = cv2.imread(path[0])
        self.modImg = self.img.copy()
        self.display_image(self.modImg)

    def display_image(self, img):
        if img is None:
            return
        self.canvas.axes.cla()  # Clear the canvas.
        self.canvas.axes.imshow(img[:,:,::-1])
        self.canvas.draw()

    def reset_img(self):
        if self.img is None:
            return
        self.modImg = self.img.copy()
        self.display_image(self.img)
        self.w.logTB.clear()
        self.bbox01 = []
        self.bbox02 = []
        self.cnt = 0

    def on_release(self, e):
        if self.cnt >= self.cntBox.value() or self.img is None:
            return
        # convert to integer
        self.cnt += 1
        [xmin, xmax, ymin, ymax] = [int(i) for i in toggle_selector.RS.extents]
        # Append points to log
        self.w.logTB.append(f"{xmin}, {xmax}, {ymin}, {ymax}")
        self.w.logTB.append(f"{xmin}, {ymin}, {xmax-xmin}, {ymax-ymin}")
        self.bbox01.append([xmin, ymin, xmax-xmin, ymax-ymin])
        self.bbox02.append([xmin, ymin, xmax, ymax])
        # draw bbox on image
        cv2.rectangle(self.modImg,
                      (xmin, ymin),
                      (xmax, ymax),
                      (0,255,0), 2)
        self.display_image(self.modImg)

    def open_log(self):
        if self.w.isVisible():
            self.w.hide()
        else:
            self.w.show()

    def save_csv(self):
        path = QFileDialog.getSaveFileName(self,"Save file",f"node{self.nodeBox.value():02d}_fence","csv (*.csv)")
        print(path)
        if path[0] == '':
            return
        if self.rB01.isChecked():
            with open(path[0], 'w') as f:
                writer = csv.writer(f)
                writer.writerows(self.bbox01)
        if self.rB02.isChecked():
            with open(path[0], 'w') as f:
                writer = csv.writer(f)
                writer.writerows(self.bbox02)




if __name__ == '__main__':
    app = QApplication([])
    app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
    w = Window()
    w.show()
    app.exec_()
