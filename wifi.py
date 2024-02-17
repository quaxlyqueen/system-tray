import customtkinter as ctk
import os

from button import Button
from functools import partial

# default colors
base = '#262940'
accent = '#4D5382'
hover = '#C6CAED'

class Wifi(ctk.CTk):
    networks = []
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

        self.read_networks()
        
        self.create_layout(5)
        self.create_widgets(5)

        self.mainloop()

    def read_networks(self):
        os.system('net-info -l')

        # Print each line in the file
        with open('/tmp/networks.txt', 'r') as f:
            for line in f:

                # If the line is empty, skip it
                if line == '\n':
                    continue

                self.networks.append(line[:-1])
        
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
    def create_layout(self, n):
        for i in range(n):
            self.grid_rowconfigure(i, weight=1)
        self.columnconfigure(0, weight=1)

    # Create the display and button widgets
    def create_widgets(self, n):
        for i in range(n):
            # If this is the first network and it has , then it is active
            if i == 0 and self.networks[i].startswith(''):
                b = Button(self, self.networks[i][:-1] + ' ', 'active', partial(self.connect, i, 1), i, 0, 2, 'nsew')

            else:
                b = Button(self, self.networks[i] + ' ', 'inactive', partial(self.connect, i, 2), i, 0, 2, 'nsew')

            b.grid(row=i, column=0, ipadx=2, ipady=5, columnspan=1, sticky='nsew')
            b.set_font(('Lato', 18))
            b.configure(anchor='w')

    def connect(self, i, active):

        if active == 1:
            os.system('iwctl station wlan0 disconnect')
            print('Disconnecting from ', self.networks[i])
            self.withdraw()
            self.callback()
            return

        # Print each line in the file
        with open('/tmp/available.network', 'r') as f:
            curr = 0

            for line in f:

                # If the line is empty, skip it
                if line == '\n':
                    continue

                if curr == i:
                    os.system('iwctl station wlan0 connect ' + line[:-1])
                    print('Connecting to ', self.networks[i])
                    self.withdraw()
                    self.callback()
                    return
                
                curr += 1
                
        print('No network found')
