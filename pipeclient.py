import os
import time
import traceback

import win32file
import win32pipe
import pywintypes

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

            if len(res_bs) < 4:
                print('not enough data for device width and height dimension')
                # TODO wait for more data

            device_width = int.from_bytes(res_bs[0:2], byteorder='little', signed=False)
            device_height = int.from_bytes(res_bs[2:4], byteorder='little', signed=False)

            print(device_width, device_height)
            # TODO parse remaining data
        except pywintypes.error as win32error:
            # TODO broken pipe / server closed connection
            print(type(win32error))
            print(win32error)
            break


except pywintypes.error as e:
    print(e)
