# Troubleshooting

## No property page, or does not work

1. 32-bit Windows is not supported.
2. If you can see the property page, open the device list; if your device is listed multiple times, you usually want the first one, but try every entry.
3. Try fully quitting the StreamDeck software and re-opening it. To fully quit, right click on the system tray icon, and select "Quit Stream Deck".
4. Check your anti-virus or other anti-malware history to make sure it has not modified it.
5. Try removing the action/icon and re-adding it.
6. Try uninstalling and re-installing the plugin. If this fixes the issue, check your anti-virus/anti-malware software again.

## Devices are listed multiple times in the properties page

The plugin will remember the exact Windows/MacOS device that it was configured with, and this will always appear in the list. The same physical device may sometimes be considered a different device by Windows/MacOS, e.g. if plugged into a different USB port. In this case, the list will show both the original Windows/MacOS device (even if not currently present), and all currently present devices, which may appear twice.

This is expected even if fuzzy matching is enabled; if fuzzy matching is enabled, the plugin will first attempt to match the original device exactly, and only resort to fuzzy matching if that fails.

## The plugin doesn't work when I plug/unplug the device or reboot

Try enabling fuzzy matching - this will match by name instead. On Windows, this will still require USB sound cards to
be plugged in the same port they were originally plugged into.
