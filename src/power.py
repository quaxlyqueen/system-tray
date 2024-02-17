import customtkinter as ctk
import os

from button import Button
from functools import partial

# default colors
base = '#262940'
accent = '#4D5382'
hover = '#C6CAED'

class Power(ctk.CTk):
    def __init__(self, callback, base, accent, hover):
        super().__init__()
        base = base
        accent = accent
        hover = hover

        self.callback = callback  # Store the callback function
        self.attributes('-type', 'splash')
        self.attributes('-topmost', True)

        self.set_size()
        self.resizable(False, False)

        self.create_layout()
        self.create_widgets()

        self.mainloop()

    # Overlay on top of the Tray.
    def set_size(self):
        self.overrideredirect(True)
        screen_width = self.winfo_screenwidth()
        screen_height = self.winfo_screenheight()

        # Set the window size
        window_width = int(screen_width * 0.125)
        window_height = int(screen_height * 0.25)

        x_cordinate = screen_width - window_width
        y_cordinate = 45
        self.geometry("{}x{}+{}+{}".format(window_width, window_height, x_cordinate, y_cordinate))

    # Create a grid layout
    def create_layout(self):
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=1)
        self.columnconfigure(0, weight=1)

    # Create the display and button widgets
    def create_widgets(self):
        power_options = ['Shutdown', 'Restart', 'Logout']
        power_functions = [self.shutdown, self.restart, self.logout]

        for i in range(len(power_options)):
            b = Button(self, power_options[i], 'inactive', power_functions[i], i, 0, 2, 'nsew')
            b.grid(row=i, column=0, ipadx=2, ipady=5, columnspan=1, sticky='nsew')
            b.set_font(('Lato', 18))
            b.configure(anchor='w')

    def shutdown(self):
        home = os.path.expanduser('~')
        path = home + '/.config/status/shutdown'
        if not os.path.exists(path):
            with open(path, 'w'):
                pass
            os.system('shutdown -h +1')
        else:
            os.remove(path)
            os.system('shutdown -c')
            return

    def restart(self):
        home = os.path.expanduser('~')
        path = home + '/.config/status/restart'
        if not os.path.exists(path):
            with open(path, 'w'):
                pass
            os.system('shutdown -r +1')
        else:
            os.remove(path)
            os.system('shutdown -c')
            return

    def logout(self):
        os.system('i3-msg exit')
