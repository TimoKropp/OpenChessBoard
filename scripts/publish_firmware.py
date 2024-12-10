import os
import shutil
import json

print("release .bin file")
# Paths
build_path = ".pio/build/arduino_nano_esp32/firmware.bin"
release_folder = "release"
version_file = os.path.join(release_folder, "version.json")


# Read version from version.json
with open(version_file, "r") as f:
    version_data = json.load(f)

# Extract the latest version
latest_version = version_data.get("latest_version", "unknown_version")

# Destination filename
dest_filename = f"firmware_v{latest_version}.bin"
dest_path = os.path.join(release_folder, dest_filename)

# Copy and rename the file
if os.path.exists(build_path):
    shutil.copy(build_path, dest_path)
    print(f"Copied and renamed to: {dest_path}")
else:
    print(f"Build file not found: {build_path}")
