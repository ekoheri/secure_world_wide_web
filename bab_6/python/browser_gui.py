# pip3.10 install PyQt6 PyQt6-WebEngine

import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QLineEdit, QPushButton, QHBoxLayout
from PyQt6.QtWebEngineWidgets import QWebEngineView
from PyQt6.QtCore import QUrl
from PyQt6.QtWebEngineCore import QWebEnginePage

class CustomWebEnginePage(QWebEnginePage):
    def javaScriptConsoleMessage(self, level, message, lineNumber, sourceID):
        pass  # Jangan tampilkan pesan JavaScript di terminal

class WebBrowser(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Simple Web Browser")
        self.setGeometry(100, 100, 900, 600)

        # Widget utama
        self.browser = QWebEngineView()
        self.browser.setPage(CustomWebEnginePage(self))  # Pakai custom page
        #self.browser.setUrl(QUrl("https://www.google.com"))

        # Input URL
        self.url_bar = QLineEdit()
        self.url_bar.setPlaceholderText("Enter URL here...")
        self.url_bar.returnPressed.connect(self.load_url)

        # Tombol Go / Loading
        self.go_button = QPushButton("Go")
        self.go_button.clicked.connect(self.load_url)

        # Layout atas (URL bar + tombol)
        top_layout = QHBoxLayout()
        top_layout.addWidget(self.url_bar)
        top_layout.addWidget(self.go_button)

        # Layout utama
        main_layout = QVBoxLayout()
        main_layout.addLayout(top_layout)
        main_layout.addWidget(self.browser)

        # Set widget utama
        central_widget = QWidget()
        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

        # Event Loading
        self.browser.loadStarted.connect(self.start_loading)
        self.browser.loadFinished.connect(self.finish_loading)
        self.browser.titleChanged.connect(self.update_title)

    def load_url(self):
        url_text = self.url_bar.text()
        if not url_text.startswith("http"):
            url_text = "https://" + url_text  # Tambahkan https jika tidak ada
        self.browser.setUrl(QUrl(url_text))

    def start_loading(self):
        self.go_button.setText("Loading...")  # Ubah tombol menjadi "Loading..."
        self.go_button.setEnabled(False)  # Nonaktifkan tombol sementara

    def finish_loading(self):
        self.go_button.setText("Go")  # Kembalikan tombol ke "Go"
        self.go_button.setEnabled(True)  # Aktifkan kembali tombol

    def update_title(self, title):
        self.setWindowTitle(title)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = WebBrowser()
    window.show()
    sys.exit(app.exec())
