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

    resp = win32file.ReadFile(handle, READ_BUFFER_SIZE)
    print(resp)

    res_bs = resp[1]

    if len(res_bs) < 4:
        print('not enough data for device width and height dimension')
        # TODO wait for more data

    device_width = int.from_bytes(res_bs[0:2], byteorder='little', signed=False)
    device_height = int.from_bytes(res_bs[2:4], byteorder='little', signed=False)

    print(device_width, device_height)

    remain_bs: bytes = res_bs[4:]

    while True:
        while len(remain_bs) >= 6:
            contact_bs = remain_bs[:6]
            print(contact_bs)
            contact_id = contact_bs[0]
            is_contact_on_surface = contact_bs[1]
            contact_x = int.from_bytes(contact_bs[2:4], byteorder='little', signed=False)
            contact_y = int.from_bytes(contact_bs[4:6], byteorder='little', signed=False)
            print(f'{{id: {contact_id}, onSurface: {is_contact_on_surface}, x: {contact_x}, y: {contact_y} }}')

            remain_bs = remain_bs[6:]

        resp = win32file.ReadFile(handle, READ_BUFFER_SIZE)
        print(resp)
        remain_bs = remain_bs + resp[1]

    # TODO parse remaining data
except pywintypes.error as win32error:
    # TODO broken pipe / server closed connection
    print(type(win32error))
    print(win32error)
