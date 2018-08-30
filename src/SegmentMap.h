#ifndef MAPSCALER_SEGMENT_MAP_H_
#define MAPSCALER_SEGMENT_MAP_H_

#include <string_view>
#include <unordered_map>
#include <vector>

#include "BMPReader.h"
#include <ck2/Color.h>
#include "common.h"


//NAMESPACE_CK2;
using namespace ck2; // until it is actually in the lib


template<typename EntityT, typename CoordT>
struct SegmentMap
{
  using ColorMap = std::unordered_map<BGR, EntityT>;

  SegmentMap(uint width_,
             uint height_,
             const ColorMap& color_map_, // CAUTION: If this is destroyed when we're not: dangling reference!
             std::string_view image_desc_ = "") // optional desc of image type for error messages (e.g., "provinces bitmap" would work well in one case)
  : _M_width(width_)
  , _M_height(height_)
  , _M_rows(height_)
  , _M_color_map(color_map_)
  , _M_img_desc(image_desc_.empty() ? "bitmap" : image_desc_)
  {}

  struct Segment
  {
    EntityT id;
    CoordT end;

    Segment() : id(), end(0) {}
    Segment(EntityT id_, CoordT end_) : id(id_), end(end_) {}
  };

  auto  width()     const noexcept { return _M_width; }
  auto  height()    const noexcept { return _M_height; } // effectively size() were we to try to be STL-like
  auto& color_map() const noexcept { return _M_color_map; }
  auto& color_map()       noexcept { return _M_color_map; }
  auto  desc()      const noexcept { return _M_img_desc; }

  auto& operator[](uint y) const noexcept { return _M_rows[y]; }
  auto& operator[](uint y)       noexcept { return _M_rows[y]; }

  // This must be called in left-right row order in descending y (from bottom row to top). Typically only
  // BMPReader::foreach_segment calls this method, but it is left public for other potential access patterns.
  void add_segment(BGR color, uint start_x, uint end_x, uint y)
  {
    assert(y < _M_height);
    assert(end_x <= _M_width);
    assert(end_x > start_x); // end_x should always be one past the actual final pixel

    if (auto it = _M_color_map.find(color); it != _M_color_map.end())
    {
      _M_rows[y].emplace_back(it->second, end_x);
    }
    else if (end_x - 1 > start_x)
    {
      throw ck2::Error("Stray color RGB({}, {}, {}) in {} at pixels (x, y) => ({}-{}, {})",
                       color.red(), color.green(), color.blue(), _M_img_desc, start_x, end_x - 1, y);
    }
    else
    {
      throw ck2::Error("Stray color RGB({}, {}, {}) in {} at pixel (x, y) => ({}, {})",
                       color.red(), color.green(), color.blue(), _M_img_desc, start_x, y);
    }
  }

private:
  uint _M_width;
  uint _M_height;

  std::vector< std::vector<Segment> > _M_rows; // y-segments == row of segments == fixed y == y is index

  const ColorMap&  _M_color_map;
  std::string_view _M_img_desc; // purely for reporting *slightly* better errors
};


//NAMESPACE_CK2_END;
#endif
