import customtkinter as ctk
import re
import os

from wifi import Wifi
from power import Power
from button import Button

# default colors
base = '#262940'
accent = '#4D5382'
hover = '#C6CAED'

class Slider(ctk.CTkSlider):
    def __init__(self, root, command, type, row, column, columnspan, sticky):
        super().__init__(
                root,
                from_ = 0,
                to = 100,
                number_of_steps = 100,
                command = command,
                height = 10,
                border_width = 10,

                fg_color = base,
                progress_color = hover,
                button_color = hover,
                button_hover_color = base,
                border_color = accent,
                bg_color = accent,

            )
        
        self.set_value(type)
        self.grid(row=row, column=column, ipadx=5, ipady=5, columnspan=columnspan, sticky=sticky)

    def set_value(self, type):
        home = os.path.expanduser('~')
        if type == 'volume':
            with open(home + '/.config/xob/volume', 'r') as f:
                value = f.read()
            self.set(int(value))
        elif type == 'brightness':
            with open(home + '/.config/xob/brightness', 'r') as f:
                value = f.read()
            self.set(int(value))

class Tray(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.attributes('-type', 'splash')
        self.attributes('-topmost', True)

        self.set_size()
        self.resizable(False, False)

        self.create_layout()
        self.create_widgets()

        self.mainloop()

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
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)

        # Row 1 & 2: Brightness & Volume Sliders
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)

        # Row 3: Wi-Fi & Bluetooth
        self.grid_rowconfigure(2, weight=1)

        # Row 4: Theme toggle & Power options
        self.grid_rowconfigure(3, weight=1)

        # Row 5: Focus Mode Select
        self.grid_rowconfigure(4, weight=1)

    # Create the display and button widgets
    def create_widgets(self):
        Slider(self, self.volume, 'volume', 0, 0, 2, 'nsew')
        Slider(self, self.brightness, 'brightness', 1, 0, 2, 'nsew')
        Button(self, '   ', 'wifi', self.wifi, 2, 0, 1, 'nsew')
        Button(self, '  󰂯  ', 'bluetooth', self.bluetooth, 2, 1, 1, 'nsew')
        Button(self, '  󰐥  ', 'power', self.power, 3, 0, 1, 'nsew')
        Button(self, '    ', 'theme', self.theme, 3, 1, 1, 'nsew')
        Button(self, 'Focus Mode', 'focus', self.focus_mode, 4, 0, 2, 'nsew')

    def focus_mode(self):
        print('focus mode')
        exit()

    def wifi(self):
        self.withdraw()
        wifi_window = Wifi(self.on_window_close, base, accent, hover)
        wifi_window.mainloop()

    def bluetooth(self):
        home = os.path.expanduser('~')
        file = home + '/.config/status/bluetooth'
        if os.path.exists(file):
            os.remove(file)
            os.system('bluetoothctl power off > /tmp/bluetooth.log')
        else:
            open(file, 'w').close()
            os.system('bluetoothctl power on > /tmp/bluetooth.log')
        exit()

    def power(self):
        self.withdraw()
        power_window = Power(self.on_window_close, base, accent, hover)
        power_window.mainloop()

    def on_window_close(self):
        self.deiconify()

    def theme(self):
        os.system('set-theme')
        exit()

    def volume(self, value):
        value = str(int(value))
        os.system('echo ' + value + ' > ~/.config/xob/volume')
        os.system('pamixer --set-volume ' + value)

    def brightness(self, value):
        value = str(int(value))
        os.system('echo ' + value + ' > ~/.config/xob/brightness')
        os.system('brightnessctl set ' + value + '%')

def theme(theme):
    global base
    global accent
    global hover
    # obtain the user's home directory
    home = os.path.expanduser('~')

    # Read in the colors from the config file
    with open(home + '/.config/' + theme, 'r') as f:
        for line in f:
            if line.startswith('base'):
                base = re.search(r'#[0-9A-Fa-f]{6}', line).group(0)
            elif line.startswith('accent'):
                accent = re.search(r'#[0-9A-Fa-f]{6}', line).group(0)
            elif line.startswith('hover'):
                hover = re.search(r'#[0-9A-Fa-f]{6}', line).group(0)

theme('theme/active.theme')
Tray()
