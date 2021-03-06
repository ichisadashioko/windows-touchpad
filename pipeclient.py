import os
import win32file
import win32pipe
import pywintypes
import time

pipe_name = '\\\\.\\pipe\\kankaku'
print(pipe_name)

READ_BUFFER_SIZE = 1024

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

    # TODO this thing block the call and never return
    # print('SetNamedPipeHandleState')

    # res = win32pipe.SetNamedPipeHandleState(
    #     handle,
    #     win32pipe.PIPE_READMODE_BYTE,
    #     None,
    #     None,
    # )

    print('reading data from pipe server')

    while True:
        try:
            resp = win32file.ReadFile(handle, READ_BUFFER_SIZE)
            print(resp)

            res_bs = resp[1]
            device_width = (res_bs[1] << 8) + res_bs[0]
            device_height = (res_bs[3] << 8) + res_bs[2]

            print(device_width, device_height)
        except pywintypes.error as win32error:
            # TODO broken pipe / server closed connection
            print(type(win32error))
            print(win32error)
            break


except pywintypes.error as e:
    print(e)

# with open(pipe_name, mode='rb+', buffering=0) as inpipe:
#     while True:
#         bs = inpipe.read()
#         if len(bs) == 0:
#             break
#         print(bs, end='')
