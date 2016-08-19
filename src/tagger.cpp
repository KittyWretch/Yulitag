// --------------------------------------------------------------------------
//
// Yuliana's Image Tagger - A tool for watermarking images that only the
// most idiotic content thief wouldn't be able to circumvent.
//
// Copyright (c) 2016 @KittyWretch
//
// --------------------------------------------------------------------------
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
//
// --------------------------------------------------------------------------

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cassert>
#include <string>

// FIXME: Switch to libpng from stb_image, unless we can guarantee that
// stb_image is safe to load untrusted images with.
extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
}

// Get a specific bit from an entire buffer. Wrapped by buffer size.
bool get_bit(size_t index, unsigned char *bytes, size_t buffer_length)
{
    size_t byte_index = (index / 8) % buffer_length;
    size_t bit_index = index % 8;
    return !!((bytes[byte_index] >> bit_index) & 1);
}

// Set a specific bit in a buffer.
void set_bit(size_t index, unsigned char *bytes, size_t buffer_length, bool bit)
{
    size_t byte_index = index / 8;
    assert(byte_index < buffer_length);
    size_t bit_index = index % 8;
    if(bit) {
        bytes[byte_index] |= 1 << bit_index;
    } else {
        bytes[byte_index] &= ~(1 << bit_index);
    }
}

// Because we deal with data that may turn into illegible garbage and do
// hilarious things to your terminal, we just replace un-renderable
// characters with '.'. Unfortunately, this will also destroy any unicode.
std::string sanitize(const std::string &input)
{
    std::string tmp = input;
    for(size_t i = 0; i < tmp.size(); i++) {
        if(tmp[i] < 0x20 || tmp[i] > 0x7e) {
            tmp[i] = '.';
        }
    }
    return tmp;
}

void quit_with_error(const char *arg0, const std::string &error_message)
{
    std::cerr << "Error: " << error_message << std::endl;
    std::cerr << "Try '" << arg0 << " --help' for help." << std::endl;
    exit(1);
}

void show_help(const char *arg0)
{
    std::cout << "Usage: " << arg0 << "[options] <input file> <output png file> <encoded text>" << std::endl;
    std::cout << "Usage: " << arg0 << "[options] --read <input file>" << std::endl;
    std::cout << R"HELPTEXT(
Yuliana's Image Tagger 1.0

Encoded text is written into the lowest-order bits of the red, green, and
blue channels of the image. This will result in a reduction of image
quality by one bit's worth of information. Lossy compression will probably
obliterate any hidden message, so only PNG output is supported.

Warning: Do not run this tool on images from untrusted sources. It uses
the public domain stb_image library, which is intended for game developers
to use with trusted image sources and may have vulnerabilities to
maliciously malformed image files.

Options:

  --composite <input file>  Adds an overlay image, anchored in the
                            bottom-right corner, for a much more obvious
                            watermark. Not applicable in read-mode.

Send bug reports, complaints, and sexual innuendo to yuliana@cumallover.me
)HELPTEXT" << std::endl;
}

int main(int argc, char *argv[])
{
    bool read_mode = false;
    std::string filename = "";
    std::string output_filename = "";
    std::string encode_string = "";
    std::string composite_filename = "";

    for(int arg = 1; arg < argc; arg++) {
        std::string arg_name = argv[arg];
        if(arg_name == "--read") {
            read_mode = true;
        } else if(arg_name == "--help") {
            show_help(argv[0]);
            return 0;
        } else if(arg_name == "--composite") {
            arg++;
            if(arg >= argc) {
                quit_with_error(argv[0], "Missing parameter for --composite.");
            }
            composite_filename = argv[arg];
        } else if(!filename.size()) {
            filename = arg_name;
        } else if(!output_filename.size()) {
            output_filename = arg_name;
        } else if(!encode_string.size()) {
            encode_string = arg_name;
        } else {
            quit_with_error(argv[0], std::string("Too many arguments starting at: ") + arg_name);
        }
    }

    if(!filename.size()) {
        quit_with_error(argv[0], "Input filename not specified.");
    }

    if(read_mode) {
        if(output_filename.size()) {
            quit_with_error(argv[0], "Output filename specified in read-only mode.");
        }
        if(encode_string.size()) {
            // I don't think this can actually be hit, but I'm leaving it
            // here in case I decide to re-order the command line
            // parameters later.
            quit_with_error(argv[0], "Output string specified in read-only mode.");
        }
        if(composite_filename.size()) {
            quit_with_error(argv[0], "Composite image specified in read-only mode.");
        }
    } else {
        if(!output_filename.size()) {
            quit_with_error(argv[0], "No output filename specified in write mode.");
        }
    }

    int width = 0;
    int height = 0;
    int components = 0;
    unsigned char *img = nullptr;

    img = stbi_load(filename.c_str(), &width, &height, &components, 4);

    if(!img) {
        quit_with_error(argv[0], std::string("Failed to load: ") + filename);
    }

    // Handle compositing first.

    if(composite_filename.size()) {
        unsigned char *composite_img = nullptr;
        int composite_width = 0;
        int composite_height = 0;
        int composite_components = 0;

        composite_img = stbi_load(composite_filename.c_str(), &composite_width, &composite_height, &composite_components, 4);

        if(!composite_img) {
            stbi_image_free(composite_img);
            stbi_image_free(img);
            quit_with_error(argv[0], std::string("Could not load composite image: ") + composite_filename);
        }

        for(int y = 0; y < composite_height; y++) {
            for(int x = 0; x < composite_width; x++) {

                int src_addr = (x + y * composite_width) * composite_components;
                int src_alpha = composite_img[src_addr + 3];

                int dst_x = width - composite_width + x;
                int dst_y = height - composite_height + y;

                int dst_addr = (dst_x + dst_y * width) * components;

                if(dst_x < 0 || dst_y < 0) {
                    // Destination image is smaller on some axis than the
                    // source image. Skip pixels that are off the edge.
                    continue;
                }

                for(int c = 0; c < 3; c++) {
                    int src_with_alpha = src_alpha * composite_img[src_addr + c];
                    int dst_with_alpha = (255 - src_alpha) * img[dst_addr + c];
                    img[dst_addr + c] = (src_with_alpha + dst_with_alpha) / 255;
                }
            }
        }

        stbi_image_free(composite_img);
    }

    // Handle bit encoding.

    size_t encodePtr = 0;
    for(int c = 0; c < components; c++) {
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                int addr = (x + y * width) * components + c;

                if(c <= 3) {

                    if(read_mode) {

                        if(encodePtr / 8 >= encode_string.size()) {
                            encode_string.append(1, (char)0);
                        }

                        set_bit(encodePtr, (unsigned char*)&encode_string[0], encode_string.size(), img[addr] & 1);

                    } else {

                        set_bit(
                            addr * 8, img, width * height * components,
                            get_bit(encodePtr, (unsigned char *)&encode_string[0], encode_string.size()));
                    }

                    encodePtr++;
                }
            }
        }
    }

    if(read_mode) {
        std::cout << sanitize(encode_string) << std::endl;
    }

    int write_success = 1;

    if(!read_mode) {
        write_success = stbi_write_png(
            output_filename.c_str(),
            width, height, components, img,
            components * width);
    }

    stbi_image_free(img);

    if(!write_success) {
        quit_with_error(argv[0], std::string("Failed to write file: ") + output_filename);
    }

    return 0;
}


