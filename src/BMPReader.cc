#include "BMPReader.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include <ck2/Color.h>
#include <ck2/FileLocation.h>
#include "filesystem.h"


//NAMESPACE_CK2;
using namespace ck2;


BMPReader::BMPReader(const fs::path& path)
: _M_width(0)
, _M_height(0)
, _M_row_sz(0)
, _M_path(path)
, _M_file( std::fopen(path.string().c_str(), "rb"), std::fclose )
{
  const auto ferr = FLErrorStaticFactory(FLoc(path));

  if (!_M_file)
    throw ferr("Failed to open file: {}", strerror(errno));

  if (errno = 0; fread(&_M_hdr, sizeof(_M_hdr), 1, _M_file.get()) < 1)
  {
    if (errno)
      throw ferr("Failed to read bitmap file header: {}", strerror(errno));
    else
      throw ferr("Unexpected EOF while reading bitmap file header (file corruption)");
  }

  if (_M_hdr.magic != BMPHeader::MAGIC)
    throw ferr("Unsupported bitmap file type (magic=0x{:04X} but want magic=0x{:04X})",
               _M_hdr.magic, BMPHeader::MAGIC);

  if (_M_hdr.n_header_size < 40)
    throw ferr("Format unsupported: DIB header size is {} bytes but need at least 40", _M_hdr.n_header_size);

  if (_M_hdr.n_width <= 0)
    throw ferr("Format unsupported: Expected positive image width, found {}", _M_hdr.n_width);

  if (_M_hdr.n_height <= 0)
    throw ferr("Format unsupported: Expected positive image height, found {}", _M_hdr.n_height);

  if (_M_hdr.n_width == 1 || _M_hdr.n_height == 1)
    throw ferr("Image dimensions ({}x{}) insufficient to support a map", _M_hdr.n_width, _M_hdr.n_height);

  if (_M_hdr.n_planes != 1)
    throw ferr("Format unsupported: Should only be 1 image plane, found {}", _M_hdr.n_planes);

  // TODO: When moving to indexed color support, this check will be dependent upon our "essence" type (see 'Essence' Design Pattern).
  if (_M_hdr.n_bpp != 24)
    throw ferr("Format unsupported: Need 24bpp color but found {}", _M_hdr.n_bpp);

  if (_M_hdr.compression_type != 0)
    throw ferr("Format unsupported: Found unsupported compression type #{}", _M_hdr.compression_type);

  // TODO: When moving to indexed color support, this check will be dependent upon our "essence" type (see 'Essence' Design Pattern).
  if (_M_hdr.n_colors != 0)
    throw ferr("Format unsupported: Image shouldn't be paletted, but {} colors were specified",
               _M_hdr.n_colors);

  assert( _M_hdr.n_important_colors == 0 );

  _M_width = _M_hdr.n_width;
  _M_height = _M_hdr.n_height;

  // calculate row size with 32-bit alignment padding and consequent raw bitmap size
  _M_row_sz = 4 * ((width() * bpp() + 31) / 32);
  auto bitmap_sz = _M_row_sz * _M_height;

  if (_M_hdr.n_bitmap_size != 0 && _M_hdr.n_bitmap_size != bitmap_sz)
    throw ferr("File corruption: Raw bitmap data section should be {} bytes but {} were specified",
               bitmap_sz, _M_hdr.n_bitmap_size);

  // TODO: load image palette if it has one
}


//NAMESPACE_CK2_END;
