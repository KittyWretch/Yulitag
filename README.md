# Yulitag

Simple commandline image watermarking tool.

### What?

It encodes data into the low-order bits of a PNG file. You give it a
string, and it hides the string in the image in a barely-noticable
way. Magic!

This technique is fairly easy to defeat if you know what to look for.
Hopefully whoever pirates your (or my) content is an idiot!

Also, it can alpha-blend another image on top, anchored in the
lower-right corner, for something a little more visible. See the
--composite parameter for details.

### Why?

To keep people from ripping off my shitty art, passing it off as their
own, and getting away with it.

### And you use the UNLICENSE for this?

Code is easy. Drawing dirty pictures is hard. Want credit for the art.
Don't care about the code. Do whatever with it.

### How?

Run like this:

    yulitag input_image.png output_image.png "Some text you want to encode "

It'll spit out an image file called output_image.png, with some
very-hard-to-notice changes. To read the watermark back, you can do this:

    yulitag --read output_image.png

The watermarked data will be spat out to stdout.

For more help see:

    yulitag --help

### Installing

Make sure you have Make and a C++11-capable C compiler installed and
run this:

    make && sudo make install

There are no dependencies besides the standard C++ library and the
already-included stb_image and stb_image_write headers.

### Special thanks

Thanks to Sean Barrett (@nothings) for all the awesome stb_* headers
that make this possible.

### I love and/or hate this!

Complaints go to yuliana@cumallover.me.

