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

I make this for my own use, and I share this in the hope others find it useful; I'm not able to commit to support, bug fixes, or feature development.

If you have found a bug, first check [the troubleshooting guide](TROUBLESHOOTING.md) guide; if that doesn't resolve the issue or you have a feature request, please check [GitHub issues](https://github.com/fredemmott/StreamDeck-AudioSwitcher/issues) to see if it has already been reported, and [create a new issue](https://github.com/fredemmott/StreamDeck-AudioSwitcher/issues/new) if not.

Support may be available from the community via:
* [GitHub Discussions](https://github.com/fredemmott/StreamDeck-AudioSwitcher/discussions)
* [Discord](https://discord.gg/CWrvKfuff3)

I am not able to respond to 1:1 requests for help via any means, including GitHub, Discord, Twitter, Reddit, or email.

# Thanks

- Thanks to "EreTIk" for finding/documenting the COM interface.
- Thanks to "LordValgor" for the idea of making this plugin.

# License

This project is [MIT-licensed](LICENSE), except for the image files.

The image files are proprietary, and may not be re-used or modified.
