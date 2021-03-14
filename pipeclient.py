import os
import io
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

global_last_update_clock = None
global_last_update_clock_lock = threading.Lock()
global_tk_photo_ref = None
global_np_image = None
global_np_rgb_image = None


class UpdateCanvasThread(threading.Thread):
    def __init__(
        self,
        clock,
        delay: float,
        cairo_surface: cairo.ImageSurface,
        tk_root: tk.Tk,
        tk_canvas: tk.Canvas,
    ):
        super().__init__()
        self.clock = clock
        self.delay = delay
        self.cairo_surface = cairo_surface
        self.tk_root = tk_root
        self.tk_canvas = tk_canvas

    def run(self):
        global global_last_update_clock, global_last_update_clock, global_tk_photo_ref, global_np_image, global_np_rgb_image

        time.sleep(self.delay)

        global_last_update_clock_lock.acquire()
        if self.clock == global_last_update_clock:
            # TODO update canvas
            # print(self.clock, 'excuted')
            with io.BytesIO() as fileobj:
                self.cairo_surface.write_to_png(fileobj)
                pil_image = PIL.Image.open(fileobj)
                global_tk_photo_ref = PIL.ImageTk.PhotoImage(image=pil_image)
                self.tk_canvas.delete(tk.ALL)
                self.tk_canvas.create_image(
                    0, 0,
                    anchor='nw',
                    image=global_tk_photo_ref,
                )

                global_np_image = np.array(pil_image)
                # print(type(pil_image), type(global_np_image), global_np_image.dtype, global_np_image.shape)
                global_np_rgb_image = global_np_image[:, :, :3]
                # print(type(global_np_rgb_image))
                # print(global_np_rgb_image.shape, global_np_rgb_image.dtype)

            # TODO update tkinter window
            # self.tk_root.update()
            pass

        global_last_update_clock_lock.release()


class PipeClientThread(threading.Thread):
    def __init__(
        self,
        tk_root,
        tk_canvas,
    ):
        super().__init__()

        self.tk_root = tk_root
        self.tk_canvas = tk_canvas

        self.tracking_id = None
        self.last_contacts = {}
        self.current_stroke = None
        self.strokes = []
        self.update_canvas = False
        self.cairo_surface: cairo.ImageSurface = None
        self.cairo_ctx: cairo.Context = None

    def translate_and_trigger_events(self, contact_info: dict):
        # translate to touch events
        contact_id = contact_info['id']
        is_contact_on_surface = contact_info['onSurface']

        if contact_id in self.last_contacts:
            if is_contact_on_surface:
                # touch move event
                contact_info['type'] = 'move'
                self.on_touch_move(contact_info)
                self.last_contacts[contact_id] = contact_info
            else:
                # touch up event
                contact_info['type'] = 'up'
                self.on_touch_up(contact_info)
                del self.last_contacts[contact_id]
        else:
            if is_contact_on_surface:
                # touch down event
                contact_info['type'] = 'down'
                self.on_touch_down(contact_info)
                self.last_contacts[contact_id] = contact_info
            else:
                # broken state
                contact_info['type'] = 'broken'

    def on_touch_down(self, contact_info: dict):
        if self.tracking_id is None:
            self.tracking_id = contact_info['id']
            self.current_stroke = [contact_info]

            self.strokes.append(self.current_stroke)

            # TODO update canvas
            self.update_canvas = True

            self.cairo_ctx.set_source_rgba(1.0, 1.0, 1.0, 1.0)
            self.cairo_ctx.set_line_width(12)
            self.cairo_ctx.set_line_cap(cairo.LINE_CAP_ROUND)
            self.cairo_ctx.move_to(contact_info['x'], contact_info['y'])
            self.cairo_ctx.stroke()

    def on_touch_move(self, contact_info: dict):
        if self.tracking_id is not None:
            if self.tracking_id == contact_info['id']:
                last_contact = self.current_stroke[-1]

                if (last_contact['x'] == contact_info['x']) and (last_contact['y'] == contact_info['y']):
                    pass
                else:
                    self.current_stroke.append(contact_info)

                    # TODO update canvas
                    self.update_canvas = True

                    self.cairo_ctx.set_source_rgba(1.0, 1.0, 1.0, 1.0)
                    self.cairo_ctx.set_line_width(12)
                    self.cairo_ctx.set_line_cap(cairo.LINE_CAP_ROUND)
                    self.cairo_ctx.move_to(last_contact['x'], last_contact['y'])
                    self.cairo_ctx.line_to(contact_info['x'], contact_info['y'])
                    self.cairo_ctx.stroke()

    def on_touch_up(self, contact_info: dict):
        if self.tracking_id is not None:
            if self.tracking_id == contact_info['id']:
                self.tracking_id = None
                last_contact = self.current_stroke[-1]

                if (last_contact['x'] == contact_info['x']) and (last_contact['y'] == contact_info['y']):
                    pass
                else:
                    self.current_stroke.append(contact_info)

                    # TODO update canvas
                    self.update_canvas = True

                    self.cairo_ctx.set_source_rgba(1.0, 1.0, 1.0, 1.0)
                    self.cairo_ctx.set_line_width(12)
                    self.cairo_ctx.set_line_cap(cairo.LINE_CAP_ROUND)
                    self.cairo_ctx.move_to(last_contact['x'], last_contact['y'])
                    self.cairo_ctx.line_to(contact_info['x'], contact_info['y'])
                    self.cairo_ctx.stroke()

                self.current_stroke = None

    def run(self):
        global global_last_update_clock, global_last_update_clock

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
            self.cairo_surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, device_width, device_height)

            self.cairo_ctx = cairo.Context(self.cairo_surface)
            self.cairo_ctx.set_source_rgba(1.0, 1.0, 1.0, 1.0)
            self.cairo_ctx.set_line_width(12)
            self.cairo_ctx.set_line_cap(cairo.LINE_CAP_ROUND)

            # TODO allow user to change stroke width

            # resize the tkinter Canvas
            self.tk_canvas.configure(
                width=device_width,
                height=device_height,
            )

            remain_bs: bytes = res_bs[4:]

            while True:
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

                    self.translate_and_trigger_events(contact_info)
                    # print(contact_info)

                    remain_bs = remain_bs[6:]

                    # if tracking_id is not None:
                    #     if contact_id == tracking_id:

                    # need_update_canvas = True

                if self.update_canvas:
                    # TODO create a delay thread for refreshing the UI
                    self.update_canvas = False
                    global_last_update_clock_lock.acquire()

                    global_last_update_clock = time.perf_counter()
                    # print(global_last_update_clock, 'started')
                    UpdateCanvasThread(
                        clock=global_last_update_clock,
                        delay=0.0625,
                        cairo_surface=self.cairo_surface,
                        tk_root=self.tk_root,
                        tk_canvas=self.tk_canvas,
                    ).start()

                    global_last_update_clock_lock.release()

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
cairo_surface = None

tk_root = tk.Tk()
tk_root.wm_title('kankaku client')
tk_root.wm_attributes('-alpha', 0.5)

tk_canvas = tk.Canvas(tk_root, width=640, height=480)
tk_canvas.grid()


def cv2_show_image_thread_fn():
    global global_np_rgb_image
    while True:
        if global_np_rgb_image is not None:
            cv2.imshow('frame', global_np_rgb_image)
            print(np.max(global_np_rgb_image))
            k = cv2.waitKey(100)
            k = k & 0xff

            if k == ord('q') or k == 27:
                break
        else:
            time.sleep(1)

    cv2.destroyAllWindows()


threading.Thread(target=cv2_show_image_thread_fn).start()

pipe_client_thread = PipeClientThread(
    tk_root=tk_root,
    tk_canvas=tk_canvas,
)

pipe_client_thread.start()

tk_root.mainloop()
