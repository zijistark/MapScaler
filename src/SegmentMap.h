#ifndef MAPSCALER_SEGMENT_MAP_H_
#define MAPSCALER_SEGMENT_MAP_H_

#include <vector>

#include "BMPReader.h"
#include <ck2/Color.h>
#include "common.h"


//NAMESPACE_CK2;
using namespace ck2; // until it is actually in the lib


template<typename EntityT, typename CoordT>
struct SegmentMap
{
  SegmentMap(uint width_, uint height_)
  : _M_width(width_)
  , _M_height(height_)
  , _M_rows(height_) {}

  struct Segment
  {
    EntityT id;
    CoordT end;

    Segment() : id(), end(0) {}
    Segment(EntityT id_, CoordT end_) : id(id_), end(end_) {}
  };

  auto width()  const noexcept { return _M_width; }
  auto height() const noexcept { return _M_height; } // effectively size() were we to try to be STL-like

  auto& operator[](uint y) const noexcept { return _M_rows[y]; }
  auto& operator[](uint y)       noexcept { return _M_rows[y]; }

private:
  uint _M_width;
  uint _M_height;
  std::vector< std::vector<Segment> > _M_rows; // y-segments == row of segments == fixed y == y is outer index
};


//NAMESPACE_CK2_END;
#endif
