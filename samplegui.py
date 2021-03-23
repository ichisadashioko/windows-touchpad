import os
import time
import traceback

import PySimpleGUI as sg

window_layout = [
    [
        sg.Graph(
            size=(1280, 720),
            key='canvas',
            enable_events=True,
        ),
    ],
]

window = sg.Window(
    'kankaku client',
    layout=window_layout,
    no_titlebar=True,
    alpha_channel=0.2,
)

while True:
    event, values = window.read()
    print(time.time(), event, values)

    if event == sg.WIN_CLOSED or event == 'Cancel':
        break

window.close()
