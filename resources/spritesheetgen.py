import tmxparser
from PIL import Image
import struct
import os
import argparse
import png2pblpng

class TableEntry(object):
  def __init__(self):
    self.tile_name = str("")
    self.tile_local_id = 0
    self.tile_png_offset = 0
    self.tile_png_size = 0

class SpriteTable(object):
  def __init__(self, path):
    self.path = path
    self.version = 1
    self.filesize = 16 # currently 4 entries in the header of 4 bytes each
    self.header = []
    self.table_entries_size = 0
    self.table_entries = []

  def add_header (self):
    header = []
    header.append(struct.pack("<I", self.version))
    header.append(struct.pack("<I", self.filesize))
    header.append(struct.pack("<I", self.table_entries_size)) # Reserved field
    header.append(struct.pack("<I", 0)) # Reserved field
    self.header = ''.join(header)

  def add_table_entries (self, table_entry):
    self.table_entries.append(struct.pack("<20s",table_entry.tile_name))
    self.table_entries.append(struct.pack("<I", table_entry.tile_local_id))
    self.table_entries.append(struct.pack("<I", table_entry.tile_png_offset))
    self.table_entries.append(struct.pack("<I", table_entry.tile_png_size))
    self.table_entries_size += 32 # 20 bytes for name, 12 bytes for other
    self.filesize += 32 + table_entry.tile_png_size

  def pack_png_entries (self):
    return 0

  def write_table (self, output_filename, png_filename):
    self.add_header()
    png_file = open(png_filename, 'rb')
    with open(output_filename, 'wb') as f:
        f.write(self.header)
        f.write(''.join(self.table_entries))
        f.write(png_file.read())
    f.close()

def parse_and_build_spritesheet (tmx_file, args):
  world_map = tmxparser.TileMapParser().parse_decode(tmx_file)
  concat_filename = os.path.splitext(tmx_file)[0] + ".png.dat"
  spritesheet_filename = os.path.splitext(tmx_file)[0] + ".dat"
  open(concat_filename, 'wb').close()
  sprite_table = SpriteTable(tmx_file)
  png_offset = 0
  temp_files = []
  temp_files.append(concat_filename)

  for tileset in world_map.tile_sets:
    tileoffset = tmxparser.TileOffset()
    if tileset.tileoffsets:
      tileoffset = tileset.tileoffsets[0]
    image = tileset.images[0]
    print "Tileset: " + tileset.name + ", x: " + str(tileoffset.x) + ", y: " + str(tileoffset.y) + ", w: " + image.width + ", h: " + image.height
    print "  source: " + image.source

    xspacing = 0
    yspacing = 0
    # Add spacing from tileset
    if tileset.spacing:
      xspacing += tileset.spacing
      yspacing += tileset.spacing

    # Add optional horizontal and vertical spacing
    if tileset.properties.get("xspacing"):
      xspacing = int(tileset.properties.get("xspacing"))
    if tileset.properties.get("yspacing"):
      yspacing = int(tileset.properties.get("yspacing"))

    # Create crop box from base image
    crop_box = [tileoffset.x, tileoffset.y,
                tileoffset.x + ((int(tileset.tilewidth) + xspacing) * (int(image.width))),
                tileoffset.y + ((int(tileset.tileheight) + yspacing) * (int(image.height)))]
    print "  Crop box: " + str(crop_box)

    base_image = Image.open(image.source)
    ##cropped_filename = tileset.name + ".png"
    ##cropped_image = base_image.crop(crop_box).save(cropped_filename)
    #temp_files.append(cropped_filename)
    #temp_files.append(cropped_filename_converted)

    imagenum = 0
    for y in range(0, int(image.height)):
      for x in range(0, int(image.width)):

        crop_box = [tileoffset.x + (((int(tileset.tilewidth) + xspacing) * x)),
                    tileoffset.y + (((int(tileset.tileheight) + yspacing) * y)),
                    tileoffset.x + (((int(tileset.tilewidth) + xspacing) * (x+1)) - xspacing),
                    tileoffset.y + (((int(tileset.tileheight) + yspacing) * (y+1)) - yspacing)]
        cropped_filename = tileset.name + "_" + str(imagenum) + ".png"
        cropped_filename_converted = tileset.name + "_" + str(imagenum) + "-conv.png"
        print "  Tile " + str(imagenum) + ": " + cropped_filename
        temp_files.append(cropped_filename)
        if not args.keep_tempfiles:
          temp_files.append(cropped_filename_converted)
        cropped_image = base_image.crop(crop_box)
        cropped_image.save(cropped_filename)
        png2pblpng.convert_png_to_pebble_png(cropped_filename,cropped_filename_converted)
        imagenum += 1

        table_entry = TableEntry()
        table_entry.tile_name = tileset.name.encode('ascii', 'ignore')
        table_entry.tile_local_id = imagenum
        table_entry.tile_png_offset = png_offset
        table_entry.tile_png_size = os.stat(cropped_filename_converted).st_size
        padding = (16 - (table_entry.tile_png_size % 16))
        png_offset += table_entry.tile_png_size + padding

        sprite_table.add_table_entries(table_entry)

        concat_file = open(concat_filename, 'ab')
        concat_file.write(file(cropped_filename_converted, 'rb').read())
        for x in range (0, padding):
          byte = struct.pack("<c", '\0')
          concat_file.write(byte)
        concat_file.close()


  print "Creating sprite data table: " + spritesheet_filename
  sprite_table.write_table(spritesheet_filename, concat_filename)

  print "Cleaning up temp files"
  for temp_file in temp_files:
    print "  Removing: " + temp_file
    os.remove(temp_file)

parser = argparse.ArgumentParser(description='Create PNG data file for a sprite sheet')
parser.add_argument('-tmx', '--tmx', dest='tmx_file', nargs="*", required=True, help='TMX filepath')
parser.add_argument('-tempfiles', '--tempfiles', dest='keep_tempfiles', action='store_true', default=False, help='Keep temp files')
args = parser.parse_args()

for tmx_file in args.tmx_file:
  parse_and_build_spritesheet(tmx_file, args)

