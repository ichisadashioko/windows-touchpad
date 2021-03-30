import os
import io
import sys
import time
import traceback
import threading

# graphics modules
import tkinter as tk
import numpy as np
import PIL
import PIL.Image
import PIL.ImageTk
import cv2
import cairo

tk_root = tk.Tk()
tk_canvas = tk.Canvas(
    master=tk_root,
    width=640,
    height=480,
)


def load_and_set_image(*args):
    print(args)
    pil_image = PIL.Image.open('asahi.jpg')
    img_width, img_height = pil_image.size[:2]

    tk_canvas.configure(
        width=img_width,
        height=img_height,
    )

    tk_photo = PIL.ImageTk.PhotoImage(image=pil_image)
    tk_canvas.create_image(
        0, 0,
        anchor='nw',
        image=tk_photo,
    )

    tk_canvas.image = tk_photo


tk_canvas.bind(
    '<Button-1>',
    load_and_set_image,
)

tk_canvas.grid()

tk_root.mainloop()
