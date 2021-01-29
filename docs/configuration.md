# Migration for background images

How the background image is drawn has been changed since version 0.17.
Intuitively, the background image is now drawn above the background color instead of below it.
Technically, the background image is no longer blended with the background color.

Before migrating the settings, the background image should be converted to a translucent one with the transparency level matching the original terminal transparency.
For example, if the original terminal transparency of qtermwidget was 25%, the converted image should have transparency 25%.
The conversion can be done via ImageMagick, GraphicsMagick, GIMP or Krita.
Here is an example command using ImageMagick:

    $ convert original_image.jpg -matte -channel A +level 0,25% +channel translucent_image.png

The converted image can be specified as the terminal background image to achieve a similar effect to previous versions.
Note that you also need to change the terminal transparency to 0% if you do not want to see another window or the desktop below the terminal.
