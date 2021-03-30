# https://python-gtk-3-tutorial.readthedocs.io/en/latest/introduction.html#simple-example
# 2.2 Extended Example
# For something a little more useful, here's the PyGObject version of the classic "Hello World" program.

import gi

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

class MyWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title='Hello World')

        self.button = Gtk.Button(label='Click Here')
        self.button.connect('clicked', self.on_button_clicked)
        self.add(self.button)

    def on_button_clicked(self, widget):
        print('Hello World')

win = MyWindow()
win.connect('destroy', Gtk.main_quit)
win.show_all()
Gtk.main()
