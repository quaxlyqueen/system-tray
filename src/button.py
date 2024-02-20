import os
import customtkinter as ctk

# default colors
base = '#262940'
accent = '#4D5382'
hover = '#C6CAED'

# Button component
class Button(ctk.CTkButton):
    def __init__(self, root, text, type, command, row, column, columnspan, sticky):
        super().__init__(
                root,
                text = text,
                fg_color = base,
                text_color = hover,
                font=('Lato', 35),
                corner_radius = 20,
                border_width = 10,
                bg_color = accent,
                border_color = accent,
                command = command
            )

        self.grid(row=row, column=column, ipadx=10, ipady=10, columnspan=columnspan, sticky=sticky)
        self.set_value(type)

    # Set the font of the button
    def set_font(self, font):
        self.configure(font=font)

    # Set the color and hover colors of the button. Allow for 'active' and 'inactive' buttons to show status,
    # and ~/.config/status/<type> to show the status of the button
    def set_value(self, type):
        home = os.path.expanduser('~')
        exists = os.path.exists(home + '/.config/status/' + type)
        if exists or type == 'active':
            self.bind('<Enter>', lambda e: self.configure(fg_color=hover))
            self.bind('<Enter>', lambda e: self.configure(text_color=base))
            self.bind('<Leave>', lambda e: self.configure(fg_color=base))
            self.bind('<Leave>', lambda e: self.configure(text_color=hover))
        else:
            self.configure(fg_color = hover)
            self.configure(text_color = base)
            self.bind('<Enter>', lambda e: self.configure(fg_color=base))
            self.bind('<Enter>', lambda e: self.configure(text_color=hover))
            self.bind('<Leave>', lambda e: self.configure(fg_color=hover))
            self.bind('<Leave>', lambda e: self.configure(text_color=base))
