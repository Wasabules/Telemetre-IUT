Import("env")

# please keep $SOURCE variable, it will be replaced with a path to firmware

# In-line command with arguments

# Python callback
def on_upload(source, target, env):
    print(source, target)
    firmware_path = str(source[0])
    # do something
    env.Execute("C:/WinAVR-20100110/bin/avrdude.exe $UPLOAD_FLAGS -U flash:w:"+ firmware_path +":i")

env.Replace(UPLOADCMD=on_upload)