#ifndef MAPSCALER_BMP_READER_H
#define MAPSCALER_BMP_READER_H

#include <cstdio>
#include <memory>

#include <ck2/BMPHeader.h>
#include <ck2/Color.h>
#include <ck2/FileLocation.h>
#include "common.h"
#include "filesystem.h"


//NAMESPACE_CK2;
using namespace ck2; // until it is actually in the lib


struct BMPReader
{
  // Once constructor is complete, the header will have been read, and the BMPReader will be in a state where
  // the raw bitmap data can start being read.
  BMPReader(const fs::path&);

  auto& path()         const noexcept { return _M_path; }
  auto  width()        const noexcept { return _M_width; }
  auto  height()       const noexcept { return _M_height; }
  auto  bpp()          const noexcept { return _M_hdr.n_bpp; }
  auto  file_size()    const noexcept { return _M_hdr.n_file_size; } // TODO: verify truth with stat() in init
  auto  color_count()  const noexcept { return (_M_hdr.n_colors == 0) ? (1 << bpp()) : _M_hdr.n_colors; }

  auto bitmap_size() const noexcept
  {
    return (_M_hdr.n_bitmap_size == 0) ? _M_row_sz * _M_height
                                       : _M_hdr.n_bitmap_size;
  }

  // It's inevitable to need this, sadly:
  void dump_header(FILE* f = stderr) const { _M_hdr.print(f); }

  // Once foreach_segment is called, we will stream the entire bitmap from disk, do any palette color resolution
  // if necessary, and execute the given lambda whenever a contiguous segment of the same color on the same row
  // (same y-coord) completes (i.e., a new color segment started, or the reader hit the end of a row). The
  // callback is supplied with the color (if paletted, then the actual color -- not the palette index), the
  // start & end x-coordinates, and the common y-coordinate.
  //
  // NOTE: BMPs are 99.999% of the time stored in bottom-to-top row order (i.e., image is flipped vertically if
  // you interpret the first row as the top row rather than the bottom row). Given this, we'll guarantee that
  // 100% of the time, the row order emitted will be bottom-to-top (largest y-coords first). Row scan order is
  // totally unaffected (left to right).
  template<typename FuncT>
  void foreach_segment(const FuncT&);

  // TODO: add the raw row reading code (which one would use with continuous-tone images) to a separate class C,
  // wherein BMPReader is *currently* but would become B such that B & C derive from a superclass A which can
  // still handle most of the repetitive error-checking code and such whilst it will be impossible to intermix
  // the raw, foreach_row interface with the foreach_segment interface.
  //
  // template<typename FuncT>
  // void foreach_row(FuncT&);

private:
  uint        _M_width; // BMPHeader's dimensions are in packed struct; we need this well-aligned (and unsigned)
  uint        _M_height; // ^--
  uint        _M_row_sz; // Actual, calculated BMP raw row size with appropriate zero-padding for alignment.
  BMPHeader   _M_hdr;
  fs::path    _M_path;
  unique_fptr _M_file;
};


template<typename FuncT>
void BMPReader::foreach_segment(const FuncT& segment_callback)
{
  /* seek directly to file offset of pixel array. */
  if (fseek(_M_file.get(), _M_hdr.n_bitmap_offset, SEEK_SET) != 0)
    throw FLError(FLoc(_M_path),
                  "Failed to seek to raw bitmap data section (byte offset: 0x{0:08X} / {0}): {1}",
                  _M_hdr.n_bitmap_offset, strerror(errno));

  /* read bitmap image data, row by row, in bottom-to-top raster scan order */

  auto row_buf = std::make_unique<uint8_t[]>(_M_row_sz);

  for (uint row = 0, y = _M_height - 1; row < _M_height; ++row, --y)
  {
    if (errno = 0; fread(row_buf.get(), _M_row_sz, 1, _M_file.get()) < 1)
    {
      if (errno)
      {
        throw FLError(FLoc(_M_path),
                      "Failed to read [bottom-to-top] scanline #{} of bitmap data: {}", row, strerror(errno));
      }
      else
        throw FLError(FLoc(_M_path), "Unexpected EOF while reading [bottom-to-top] scanline #{}", row);
    }

    uint start_x = 0;
    auto p_cur = &row_buf[0];
    BGR cur_color(p_cur);

    for (uint x = 1; x < _M_width; ++x)
    {
      auto color = BGR(p_cur += 3);

      if (cur_color != color)
      {
        segment_callback(cur_color, start_x, x, y);
        cur_color = color;
        start_x = x;
      }
    }

    // Final segment in each row will never be recognized by the main loop above, but since we know that
    // absolutely, we may also unconditionally emit a segment here to complete the row.

    segment_callback(cur_color, start_x, _M_width, y);
  }
}

//NAMESPACE_CK2_END;
#endif
