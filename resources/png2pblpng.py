#!/usr/bin/env python

import png
import itertools

from pebble_image_routines import num_colors_to_bitdepth, \
    pebble_nearest_color_to_pebble_palette, pebble_truncate_color_to_pebble_palette

# color reduction methods
TRUNCATE = "truncate"
NEAREST = "nearest"
COLOR_REDUCTION_CHOICES = [TRUNCATE, NEAREST]
DEFAULT_COLOR_REDUCTION = NEAREST

#public APIs
def convert_png_to_pebble_png(input_filename, output_filename,
                              color_reduction_method=DEFAULT_COLOR_REDUCTION):
    with open(output_filename, 'wb') as output_file:
        input_png = png.Reader(filename=input_filename)

        #open as RGBA 32-bit (allows for simpler parsing cases)
        width, height, pixels, metadata = input_png.asRGBA8()

        palette = []  # rgba32 image palette
        is_grey = True  # does the image only contain greyscale pixels (and only full or opaque)
        has_alpha = False  # does the image contain alpha
        transparent_grey = None  # transparent color matching greyscale value

        # iterators are one shot, so make a copy of just the iterator and not the data
        # to be able to parse the data twice (we do not modify the pixel data itself)
        # once to generate the palette
        # once to output the final pixel data as greyscale or palette indexes
        pixels, pixels2 = itertools.tee(pixels)

        #convert RGBA 32-bit image colors to pebble color table
        for (r, g, b, a) in grouper(itertools.chain.from_iterable(pixels2), 4):
            if color_reduction_method == NEAREST:
                (r, g, b, a) = pebble_nearest_color_to_pebble_palette(r, g, b, a)
            else:
                (r, g, b, a) = pebble_truncate_color_to_pebble_palette(r, g, b, a)

            if (r, g, b, a) not in palette:
                palette.append((r, g, b, a))
                # Check if image contains any transparent pixels
                if (a != 0xFF):
                    has_alpha = True
                # greyscale only if rgb is gray and opaque or fully transparent
                if is_grey and not (((r == g == b) and a == 255) or (r, g, b, a) == (0, 0, 0, 0)):
                    is_grey = False

        # Calculate required bit depth
        if is_grey:
            # for Greyscale, it is the required colors that set the bitdepth
            # so if image contains LightGray or DarkGray it requires bitdepth 2
            if (85, 85, 85, 255) in palette or (170, 170, 170, 255) in palette:
                # if palette contains all 4 greyscale and transparent, bump up bitdepth
                if (len(palette)) >= 5:
                    bitdepth = 4
                else:
                    bitdepth = 2
            else:
                # if palette contains black, white and transparent, bump up bitdepth
                if (len(palette)) >= 3:
                    bitdepth = 2
                else:
                    bitdepth = 1

            # determine the grey value for tRNs transparency
            if has_alpha:
                if bitdepth == 4: 
                    # 4 available shades of grey are occupied
                    transparent_grey = 0xC # bitdepth 4 supported value
                else:
                    greyscale_list = [0, 255, 85, 170]  # in order of bitdepth required
                    for lum in greyscale_list:
                        # find the first unused greyscale value in terms of available bitdepth
                        if (lum, lum, lum, 255) not in palette:
                            # transparent grey value for requested greyscale bitdepth
                            transparent_grey = lum >> (8 - bitdepth)
                            break

        else:
            # get the bitdepth for the number of colors
            bitdepth = num_colors_to_bitdepth(len(palette))

        #convert RGBA 32-bit boxed rows to list for output
        rgba32_list = grouper(itertools.chain.from_iterable(pixels), 4)

        # update data for RGB output format
        if not is_grey and not has_alpha:
            # recreate the palette without an alpha channel to support RGB PNG
            palette = [(p_r, p_g, p_b) for p_r, p_g, p_b, p_a in palette]

        # second pass of pixel data, converts rgba32 pixels to greyscale or palettized output
        image = []
        for (r, g, b, a) in rgba32_list:
            # operating on original pixel values, need to do the same color reduction
            # as when the palette was generated
            if color_reduction_method == NEAREST:
                (r, g, b, a) = pebble_nearest_color_to_pebble_palette(r, g, b, a)
            else:
                (r, g, b, a) = pebble_truncate_color_to_pebble_palette(r, g, b, a)

            if is_grey:
                # convert red channel (as luminosity value) to a greyscale at bitdepth
                # if transparent, output the transparent_grey value for that bitdepth
                if a == 0:
                  image.append(transparent_grey)
                else:
                  image.append(r >> (8 - bitdepth))
            elif has_alpha:
                # append the palette index for output
                image.append(palette.index((r, g, b, a)))
            else:
                # append the palette index for output
                image.append(palette.index((r, g, b)))

        if is_grey:
          # remove the palette for greyscale output with writer
          palette = None

        output_png = png.Writer(width=width, height=height, compression=9, bitdepth=bitdepth,
                                palette=palette, greyscale=is_grey, transparent=transparent_grey)
        output_png.write_array(output_file, image)


def grouper(iterable, n, fillvalue=None):
    from itertools import izip_longest

    args = [iter(iterable)] * n
    return izip_longest(fillvalue=fillvalue, *args)


def main():
    import argparse

    parser = argparse.ArgumentParser(
        description='Convert PNG to 64-color palettized or grayscale PNG')
    parser.add_argument('input_filename', type=str, help='png file to convert')
    parser.add_argument('output_filename', type=str, help='converted file output')
    parser.add_argument('--color_reduction_method', metavar='method', required=False,
                        nargs=1, default=NEAREST, choices=COLOR_REDUCTION_CHOICES,
                        help="Method used to convert colors to Pebble's color palette, "
                             "options are [{}, {}]".format(NEAREST, TRUNCATE))
    args = parser.parse_args()
    convert_png_to_pebble_png(args.input_filename, args.output_filename, args.color_reduction_method)


if __name__ == '__main__':
    main()
