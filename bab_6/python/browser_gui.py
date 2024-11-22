# sudo apt update
# Install : Instal PyGObject
# sudo apt install python3-gi python3-gi-cairo gir1.2-gtk-3.0
# Opsional install Instal GTK+ WebKit
# sudo apt install gir1.2-webkit2-4.0
# sudo apt install libcairo2-dev libgirepository1.0-dev
# python3.10 -m pip install PyGObject
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('WebKit2', '4.0')

from gi.repository import Gtk, WebKit2, Gdk

# Fungsi callback untuk memuat URL saat tombol "Enter" ditekan
def on_activate_entry(entry, web_view, spinner):
    url = entry.get_text()
    web_view.load_uri(url)

# Fungsi untuk menampilkan dan menyembunyikan spinner
def on_load_changed(web_view, load_event, spinner):
    if load_event == WebKit2.LoadEvent.STARTED:
        # Tampilkan spinner saat halaman mulai dimuat
        spinner.start()
        spinner.show()
    elif load_event == WebKit2.LoadEvent.FINISHED:
        # Sembunyikan spinner setelah halaman selesai dimuat
        spinner.stop()
        spinner.hide()

# Fungsi untuk menampilkan halaman website
def display_html():
    # Inisialisasi GTK
    Gtk.init()

    settings = Gtk.Settings.get_default()
    settings.set_property("gtk-overlay-scrolling", False)

    # Buat window utama
    window = Gtk.Window(title="My Browser")
    
    # Dapatkan ukuran layar
    display = Gdk.Display.get_default()
    monitor = display.get_primary_monitor()
    monitor_geometry = monitor.get_geometry()

    # Atur ukuran window
    screen_width = monitor_geometry.width
    screen_height = monitor_geometry.height
    window.set_default_size(screen_width, screen_height)

    # Buat widget WebKit untuk menampilkan halaman web
    web_view = WebKit2.WebView()

    # Buat entry (textbox) untuk input URL
    entry = Gtk.Entry()
    entry.set_placeholder_text("Isi URL dan tekan Enter")

    # Buat spinner untuk loading
    spinner = Gtk.Spinner()
    spinner.set_size_request(20, 20)
    spinner.hide()

    # Hubungkan sinyal "activate" (tekan Enter) pada entry
    entry.connect("activate", on_activate_entry, web_view, spinner)

    # Hubungkan sinyal untuk spinner
    web_view.connect("load-changed", on_load_changed, spinner)

    # Buat container horizontal untuk menempatkan entry dan spinner
    hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)
    hbox.pack_start(entry, True, True, 0)
    hbox.pack_start(spinner, False, False, 0)

    # Buat container vertical untuk mengatur posisi hbox dan WebView
    vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=2)
    vbox.pack_start(hbox, False, False, 0)
    vbox.pack_start(web_view, True, True, 0)

    # Tambahkan vbox ke window
    window.add(vbox)

    # Tampilkan semua widget di window
    window.show_all()

    # Tutup program saat window ditutup
    window.connect("destroy", Gtk.main_quit)

    # Mulai loop GTK
    Gtk.main()

if __name__ == "__main__":
    display_html()
