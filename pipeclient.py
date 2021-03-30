import os
import io
import sys
import time
import traceback
import threading
import ctypes

# pywin32 modules
import win32file
import win32pipe
import pywintypes

# graphics modules
import cairo

# fmt: off
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
# fmt: on

GENERIC_READ = ctypes.c_ulong(0x80000000)
OPEN_EXISTING = ctypes.c_int(3)

global_last_update_clock = None
global_last_update_clock_lock = threading.Lock()
global_np_rgb_image = None


class KankakuWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title='kankaku')

        self.drawingarea = Gtk.DrawingArea()
        self.drawingarea.connect('draw', self.render_strokes)
        self.add(self.drawingarea)
        self.strokes = []
        self.current_stroke = None
        self.line_width = 12
        self.line_cap = cairo.LINE_CAP_ROUND

    def render_strokes(self, da, ctx):
        print(time.perf_counter(), 'render_strokes')

        for stroke in self.strokes:
            num_contacts = len(stroke)
            if num_contacts == 0:
                continue
            elif num_contacts == 1:
                # TODO
                pass
            else:
                ctx.set_source_rgba(0.0, 1.0, 0.0, 1.0)
                ctx.set_line_width(self.line_width)
                ctx.set_line_cap(self.line_cap)

                is_first_point = True
                for contact_info in stroke:
                    x = contact_info['x']
                    y = contact_info['y']

                    if is_first_point:
                        ctx.move_to(x, y)
                        is_first_point = False
                    else:
                        ctx.line_to(x, y)

                ctx.stroke()


class PipeClientThread(threading.Thread):
    def __init__(
        self,
        kankaku_window: KankakuWindow,
    ):
        super().__init__()

        self.tracking_id = None
        self.last_contacts = {}
        self.kankaku_window = kankaku_window

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
            self.kankaku_window.current_stroke = [contact_info]
            self.kankaku_window.strokes.append(self.kankaku_window.current_stroke)

    def on_touch_move(self, contact_info: dict):
        if self.tracking_id is not None:
            if self.tracking_id == contact_info['id']:
                last_contact = self.kankaku_window.current_stroke[-1]

                if (last_contact['x'] == contact_info['x']) and (last_contact['y'] == contact_info['y']):
                    pass
                else:
                    self.kankaku_window.current_stroke.append(contact_info)

    def on_touch_up(self, contact_info: dict):
        if self.tracking_id is not None:
            if self.tracking_id == contact_info['id']:
                self.tracking_id = None
                last_contact = self.kankaku_window.current_stroke[-1]

                if (last_contact['x'] == contact_info['x']) and (last_contact['y'] == contact_info['y']):
                    pass
                else:
                    self.kankaku_window.current_stroke.append(contact_info)

                self.kankaku_window.current_stroke = None

    def run(self):
        global global_last_update_clock, global_last_update_clock

        pipe_name = '\\\\.\\pipe\\kankaku'
        pipe_name_bytes = pipe_name.encode('ASCII')
        pipe_name_ptr = ctypes.create_string_buffer(pipe_name_bytes)
        print(pipe_name)

        READ_BUFFER_SIZE = 1024

        try:
            ctypes.windll.kernel32.CreateFileA(
                pipe_name_ptr,  # LPCSTR lpFileName
                GENERIC_READ,  # DWORD dwDesiredAccess,
                ctypes.c_ulong(0),  # DWORD dwShareMode
                None,  # LPSECURITY_ATTRIBUTES lpSecurityAttributes
                OPEN_EXISTING,  # DWORD dwCreationDisposition
                ctypes.c_ulong(0),  # DWORD dwFlagsAndAttributes
                None,  # HANDLE hTemplateFile
            )

            handle = win32file.CreateFile(
                pipe_name,
                win32file.GENERIC_READ,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None,
            )

            print('reading data from pipe server')

            resp = win32file.ReadFile(handle, READ_BUFFER_SIZE)
            print(resp)

            res_bs = resp[1]

            if len(res_bs) < 4:
                print('not enough data for device width and height dimension')
                # TODO wait for more data
                return

            device_width = int.from_bytes(res_bs[0:2], byteorder='little', signed=False)
            device_height = int.from_bytes(res_bs[2:4], byteorder='little', signed=False)

            # TODO dynamically resize the rendered canvas while keeping the aspect ratio
            print(device_width, device_height)

            # TODO resize Gtk window

            # TODO allow user to change stroke width

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

                if self.update_canvas:
                    # create a delay thread for refreshing the UI
                    self.update_canvas = False
                    global_last_update_clock_lock.acquire()

                    global_last_update_clock = time.perf_counter()
                    # print(global_last_update_clock, 'started')

                    global_last_update_clock_lock.release()

                resp = win32file.ReadFile(handle, READ_BUFFER_SIZE)
                # print(resp)
                remain_bs = remain_bs + resp[1]
        except pywintypes.error as win32error:
            # TODO broken pipe / server closed connection
            print(type(win32error))
            print(win32error)


# def cv2_show_image_thread_fn():
#     global global_np_rgb_image
#     while True:
#         if global_np_rgb_image is not None:
#             cv2.imshow('frame', global_np_rgb_image)
#             # print(np.max(global_np_rgb_image))
#             k = cv2.waitKey(100)
#             k = k & 0xff

#             if k == ord('q') or k == 27:
#                 break
#         else:
#             time.sleep(1)

#     cv2.destroyAllWindows()


kankaku_window = KankakuWindow()

# threading.Thread(target=cv2_show_image_thread_fn).start()

pipe_client_thread = PipeClientThread(kankaku_window=kankaku_window)
pipe_client_thread.start()

kankaku_window.connect('destroy', Gtk.main_quit)
kankaku_window.show_all()
Gtk.main()

pipe_client_thread.join()
