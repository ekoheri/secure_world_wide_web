// sudo apt-get install libgtk-3-dev libwebkit2gtk-4.0-dev
// gcc -o browser_gui browser_gui.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0)

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fungsi callback untuk memuat URL saat tombol "Enter" ditekan
void on_activate_entry(
    GtkEntry *entry, 
    WebKitWebView *web_view, 
    GtkWidget *spinner) {
    
    const char *url = gtk_entry_get_text(entry); 
    webkit_web_view_load_uri(web_view, url);
}

// Fungsi untuk menampilkan dan menyembunyikan spinner
void on_load_changed(
    WebKitWebView *web_view, 
    WebKitLoadEvent load_event, 
    gpointer user_data) {
    
    GtkWidget *spinner = GTK_WIDGET(user_data);
    
    if (load_event == WEBKIT_LOAD_STARTED) {
        // Tampilkan spinner saat halaman mulai dimuat
        gtk_spinner_start(GTK_SPINNER(spinner));
        gtk_widget_show(spinner);
    } else if (load_event == WEBKIT_LOAD_FINISHED) {
        // Sembunyikan spinner setelah halaman website selesai dimuat
        gtk_spinner_stop(GTK_SPINNER(spinner));
        gtk_widget_hide(spinner);
    }
}

// Fungsi untuk menampilkan halaman website
void display_html() {
    // Inisialisasi GTK
    gtk_init(NULL, NULL);

    g_object_set(
        gtk_settings_get_default(), 
        "gtk-overlay-scrolling", 
        FALSE, NULL);

    // Buat window utama
    GtkWidget *window = 
        gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // Dapatkan ukuran layar
    GdkMonitor *monitor = gdk_display_get_primary_monitor(
        gdk_display_get_default()
    );
    // Dapatkan geometri monitor
    GdkRectangle monitor_geometry;
    gdk_monitor_get_geometry(monitor, &monitor_geometry);

    // Dapatkan ukuran monitor dalam piksel
    int screen_width = monitor_geometry.width;
    int screen_height = monitor_geometry.height;

    gtk_window_set_default_size(
        GTK_WINDOW(window), 
        screen_width, 
        screen_height);
    gtk_window_set_title(GTK_WINDOW(window), "My Browser");

    // Buat widget WebKit untuk menampilkan halaman web
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(
        webkit_web_view_new()
    );

    // Buat entry (textbox) untuk input URL
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(entry), 
        "Isi URL dan tekan Enter");

    // Buat spinner untuk loading
    GtkWidget *spinner = gtk_spinner_new();
    gtk_widget_set_size_request(spinner, 20, 20); 
    gtk_widget_hide(spinner); 

    // Hubungkan sinyal "activate" (tekan Enter) 
    // pada gtk_entry
    g_signal_connect(
        entry, 
        "activate", 
        G_CALLBACK(on_activate_entry), 
        web_view);

    // Hubungkan sinyal untuk spinner
    g_signal_connect(
        web_view, 
        "load-changed", 
        G_CALLBACK(on_load_changed), 
        spinner);

    // Buat container horizontal untuk menempatkan entry dan spinner
    GtkWidget *hbox = 
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_pack_start(
        GTK_BOX(hbox), entry, TRUE, TRUE, 0); 
    gtk_box_pack_start(
        GTK_BOX(hbox), spinner, 
        FALSE, FALSE, 0); 

    // Buat container vertical untuk mengatur 
    // posisi hbox dan WebView
    GtkWidget *vbox = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start(
        GTK_BOX(vbox), hbox, FALSE, FALSE, 0); 
    gtk_box_pack_start(
        GTK_BOX(vbox), GTK_WIDGET(web_view), 
        TRUE, TRUE, 0);

    // Tambahkan vbox ke window
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Tampilkan semua widget di window
    gtk_widget_show_all(window);

    // Tutup program saat window ditutup
    g_signal_connect(
        window, 
        "destroy", 
        G_CALLBACK(gtk_main_quit), 
        NULL);

    // Mulai loop GTK
    gtk_main();
}

int main(int argc, char *argv[]) {
    display_html();
    return 0;
}

