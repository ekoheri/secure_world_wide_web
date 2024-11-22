import os
# import time
from datetime import datetime
from config import cfg

def create_log_directory():
    """Cek apakah folder log sudah ada, jika belum buat folder tersebut."""
    if not os.path.exists(cfg.log_directory):
        try:
            os.mkdir(cfg.log_directory)
        except OSError as e:
            print(f"Warning: Failed to create log directory: {e}")

def write_log(format_string, *args):
    """Tulis pesan log ke file log dengan format timestamp."""
    # Format tanggal untuk nama file log
    log_filename = os.path.join(cfg.log_directory, f"{datetime.now():%Y-%m-%d}.log")

    try:
        with open(log_filename, "a") as log_file:
            # Format timestamp
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            # Tulis timestamp dan pesan log ke file
            log_file.write(f"[{timestamp}] {format_string % args}\n")
    except OSError as e:
        print(f"Failed to open log file: {e}")

# Contoh penggunaan
# create_log_directory()
# write_log("Log message: %s", "Program started")
# write_log("Another log message: %d", 42)
