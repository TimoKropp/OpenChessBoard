import os
from shutil import copyfile

def after_build(source, target, env):
    # Define the output directory (release folder)
    release_dir = os.path.join(env['PROJECT_DIR'], 'release')

    # Create the release directory if it doesn't exist
    if not os.path.exists(release_dir):
        os.makedirs(release_dir)

    # Debug: Print all target files
    print("Target files:")
    for t in target:
        print(f"  {t.get_abspath()}")  # Ensure we're printing the absolute path

    # Look for the firmware.bin file in the target list
    for t in target:
        firmware_path = t.get_abspath()  # Get the absolute path of the target file
        if firmware_path.endswith("firmware.bin"):
            dest_path = os.path.join(release_dir, os.path.basename(firmware_path))
            
            # Copy firmware.bin to the release directory
            try:
                copyfile(firmware_path, dest_path)
                print(f"Firmware copied to: {dest_path}")
            except Exception as e:
                print(f"Error copying firmware: {e}")
            return  # Exit after copying the file

    print("Error: firmware.bin not found in targets.")

# Hook into the build process
Import("env")
env.AddPostAction("buildprog", after_build)
