import os
import sys
import time
import traceback
import threading

# pywin32 modules
import win32file
import win32pipe
import pywintypes

# graphics modules
import tkinter as tk
import numpy as np
import PIL
import PIL.Image
import PIL.ImageTk
import cv2
import cairo


class PipeClientThread(threading.Thread):
    def __init__(
        self,
        tk_root,
        tk_canvas,
    ):

        self.tk_root = tk_root
        self.tk_canvas = tk_canvas
        super().__init__()

    def run(self):
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

            # TODO dynamically resize the rendered canvas while keeping the aspect ratio
            print(device_width, device_height)

            # create a canvas with those dimensions
            cairo_canvas = cairo.ImageSurface(cairo.FORMAT_ARGB32, device_width, device_height)

            cairo_ctx = cairo.Context(cairo_canvas)
            cairo_ctx.set_source_rgb(1, 1, 1)

            # TODO allow user to change stroke width
            stroke_width = 12  # pixels
            cairo_ctx.set_line_width(stroke_width)
            cairo_ctx.set_line_cap(cairo.LINE_CAP_ROUND)

            # resize the tkinter Canvas
            self.tk_canvas.configure(
                width=device_width,
                height=device_height,
            )

            tracking_id = None
            last_contacts_log = {}
            current_stroke = None
            strokes = []

            remain_bs: bytes = res_bs[4:]

            while True:
                need_update_canvas = False

                while len(remain_bs) >= 6:
                    contact_bs = remain_bs[:6]
                    # print(contact_bs)
                    contact_id = contact_bs[0]
                    is_contact_on_surface = contact_bs[1]
                    contact_x = int.from_bytes(contact_bs[2:4], byteorder='little', signed=False)
                    contact_y = int.from_bytes(contact_bs[4:6], byteorder='little', signed=False)

                    contact_info = {
                        'id': contact_id,
                        'onSurface': is_contact_on_surface,
                        'x': contact_x,
                        'y': contact_y,
                    }

                    # TODO translate to touch events
                    if contact_id in last_contacts_log:
                        if is_contact_on_surface:
                            # touch move event
                            contact_info['type'] = 'move'
                            last_contacts_log[contact_id] = contact_info
                            pass
                        else:
                            # touch up event
                            contact_info['type'] = 'up'
                            del last_contacts_log[contact_id]
                            pass
                    else:
                        if is_contact_on_surface:
                            # touch down event
                            contact_info['type'] = 'down'
                            last_contacts_log[contact_id] = contact_info
                            pass
                        else:
                            # broken state
                            pass

                    print(contact_info)

                    remain_bs = remain_bs[6:]

                    # if tracking_id is not None:
                    #     if contact_id == tracking_id:

                    # need_update_canvas = True

                if need_update_canvas:
                    # TODO create a timeout thread for refreshing the UI
                    need_update_canvas = False

                resp = win32file.ReadFile(handle, READ_BUFFER_SIZE)
                # print(resp)
                remain_bs = remain_bs + resp[1]
        except pywintypes.error as win32error:
            # TODO broken pipe / server closed connection
            print(type(win32error))
            print(win32error)

        # TODO handling application life cycle
        # self.tk_root.destroy()


np_canvas = None
pil_canvas = None
tk_canvas = None
cairo_canvas = None

tk_window = tk.Tk()
tk_window.wm_title('kankaku client')

tk_canvas = tk.Canvas(tk_window, width=640, height=480)
tk_canvas.grid()


def refresh_canvas():
    if np_canvas is None:
        return

    pil_canvas = PIL.Image.fromarray(np_canvas)
    tk_canvas = PIL.ImageTk.PhotoImage(image=pil_canvas)


# def update_canvas():


pipe_client_thread = PipeClientThread(
    tk_root=tk_window,
    tk_canvas=tk_canvas,
)
pipe_client_thread.start()

tk_window.mainloop()
