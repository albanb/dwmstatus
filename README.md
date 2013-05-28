DWM Status

A lightweight status bar for dwm.

This is "hardcoded" for my system and was not meant to be flexible.  Flexibility costs memory and processor time.

Note that this was written to work with the status colors patch and use symbols from the stlarch font.  Both the colors and the symbols are int the format strings of the sprintf commands.  However, as they are mostly nonprintable they can show up oddly, or not at all, depending on your editor.

To remove the dependecy on status colors and stlarch font, simply remove these characters from the format strings.

Based on Trillby and Unia dwm status bar.
