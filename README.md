## Description

StreamDeck-AudioSwitch is a C++ plugin for the Elgato StreamDeck for setting the default audio device.

It supports:
- setting input or output device
- setting default device or communication device
- either one-button-per-device, or one button to toggle between two devices

For example, this can be useful to switch between headphones and speakers if they are on different sound cards (e.g. USB speakers or USB headphones).

# Video Demo

[![YouTube Demo Video](https://img.youtube.com/vi/Y5avo5WrwwM/0.jpg)](https://www.youtube.com/watch?v=Y5avo5WrwwM)

# Installation

Download the `com.fredemmott.audiooutputswitch.streamDeckPlugin` file from [the releases page](https://github.com/fredemmott/StreamDeck-AudioOutputSwitcher/releases), and double-click it.

# Notes

This uses undocumented and unsupported Windows APIs. These have apparently worked since Windows 7, but they
might stop working at any time or have unexpected side effects.


# FAQ

## Changing both 'communication' and 'default'

Use a multi-action switch, not a normal multi-action:

![image](https://user-images.githubusercontent.com/360927/206601016-e8785e16-edf9-4c6e-8829-1c54b9acaeaa.png)

## MacOS: "Sound effects" aren't changing, or are changing when I don't want them to

This is a MacOS bug that only Apple can fix.

# Getting Help

Check [the troubleshooting guide](TROUBLESHOOTING.md) guide. I make this for my own use, and share in the hope that others find it useful - I am unable to offer support, or to act on bug reports or feature requests. Do not contact me for help via any means, including GitHub, Discord, Twitter, Reddit, or email. This software is used by many, and I do generally fix it when something changes to break it, but I do not guarantee this, and I'm not able to help with anyone's specific issues.

If 'fuzzy matching' is required - or not functioning properly for you - ask your device manufacturer to fix their device/drivers to not change device IDs; Microsoft requires that these do not change.

# Thanks

- Thanks to "EreTIk" for finding/documenting the COM interface.
- Thanks to "LordValgor" for the idea of making this plugin.

# License

This project is [MIT-licensed](LICENSE), except for the image files.

The image files are proprietary, and may not be re-used or modified.
