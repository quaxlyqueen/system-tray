# A System Tray for i3 and i3blocks
A system tray written in Python for the GUI and C for the backend.

# Basic Project Structure:
    Backend {
        Take the output from WiFi and Bluetooth-related commands;
            The output is saved to `available.network|device` and `connected.network|device`.
        Format a list of strings and save each to a file called `{network|device}s.txt`;
        These strings are then used as the content of each Python GUI button.
    }

    Frontend {
        Honestly, problably just needs to be rewritten.
    }

# Installation:
Universal and cross-platform install and other utility scripts.
`git clone https://github.com/quaxlyqueen/scripts`<br>
`cd scripts`<br>
`sudo ./install`<br><br>

The system tray itself.
`git clone https://github.com/quaxlyqueen/system-tray`<br>
`cd system-tray`<br>
`sudo inst`<br>
