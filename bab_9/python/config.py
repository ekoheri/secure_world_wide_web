import os

class Config:
    def __init__(self):
        self.server_name = ""
        self.server_port = 0
        self.document_root = ""
        self.default_page = ""
        self.request_buffer_size = 0

# Global config object
cfg = Config()

def trim(s):
    """
    Removes leading and trailing whitespace from a string.
    """
    return s.strip()

def load_config(filename):
    """
    Loads cfg from a file.
    """
    global cfg
    if not os.path.isfile(filename):
        print(f"Error: Config file '{filename}' not found.")
        return

    with open(filename, "r") as file:
        section = ""  # Store current section (if needed)
        for line in file:
            # Remove trailing newline and leading/trailing whitespace
            line = line.strip()

            # Skip comments and empty lines
            if not line or line.startswith("#"):
                continue

            # Handle sections like [Performance]
            if line.startswith("[") and line.endswith("]"):
                section = line[1:-1].strip()
                continue

            # Split the line into key and value
            if "=" in line:
                key, value = map(trim, line.split("=", 1))

                # Match the keys and set corresponding config attributes
                if key == "server_name":
                    cfg.server_name = value
                elif key == "server_port":
                    cfg.server_port = int(value)
                elif key == "document_root":
                    cfg.document_root = value
                elif key == "default_page":
                    cfg.default_page = value
                elif key == "request_buffer_size":
                    cfg.request_buffer_size = int(value)
#End def load_config
