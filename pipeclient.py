import os
import win32file
import win32pipe
import pywintypes
import time

pipe_name = '\\\\.\\pipe\\kankaku'
print(pipe_name)

quit_flag = False

while not quit_flag:
    try:
        handle = win32file.CreateFile(
            pipe_name,
            win32file.GENERIC_READ,
            0,
            None,
            win32file.OPEN_EXISTING,
            0,
            None,
        )

        # print('SetNamedPipeHandleState')

        # res = win32pipe.SetNamedPipeHandleState(
        #     handle,
        #     win32pipe.PIPE_READMODE_BYTE,
        #     None,
        #     None,
        # )

        quit_flag = True

        while True:
            print('reading data from pipe server')
            resp = win32file.ReadFile(handle, 1024)
            print(resp, end='')

    except pywintypes.error as e:
        if e.args[0] == 2:
            print("no pipe, trying again in a sec")
            time.sleep(1)
        elif e.args[0] == 109:
            print("broken pipe, bye bye")
            quit_flag = True

# with open(pipe_name, mode='rb+', buffering=0) as inpipe:
#     while True:
#         bs = inpipe.read()
#         if len(bs) == 0:
#             break
#         print(bs, end='')
