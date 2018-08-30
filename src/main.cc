#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BMPReader.h"
#include "SegmentMap.h"
#include "Tracer.h"
#include <ck2/AdjacenciesFile.h>
#include <ck2/BMPHeader.h>
#include <ck2/Color.h>
#include <ck2/DefaultMap.h>
#include <ck2/DefinitionsTable.h>
#include <ck2/FileLocation.h>
#include <ck2/Point.h>
#include <ck2/ProvEdgeSet.h>
#include <ck2/ProvMap.h>
#include <ck2/VFS.h>
#include "common.h"
#include "filesystem.h"

constexpr char const* VERSION = "0.0.2";

constexpr char const* GAME_PATH = "C:/Program Files (x86)/Steam/steamapps/common/Crusader Kings II";
constexpr char const* MOD_PATH = "C:/git/SWMH-BETA/SWMH";
constexpr char const* TEST_MOD_PATH = "C:/git/zmod/edgeTest";
constexpr char const* PROVBMP_TEST_OUTPUT_PATH = "C:/git/MapScaler/tmp/provinces.bmp";


using namespace std;
using ck2::prov_id_t;
using ck2::BGR;


static const pair<BGR, prov_id_t> ImpassableColorMap = {{0,0,0}, numeric_limits<prov_id_t>::max()};
static const pair<BGR, prov_id_t> OceanColorMap = {{255,255,255}, numeric_limits<prov_id_t>::max() - 1};


int main()
{
  typedef unsigned int uint;

  try
  {
    ck2::VFS vfs{ fs::path(GAME_PATH) };
    vfs.push_mod_path( fs::path(MOD_PATH) );
    //vfs.push_mod_path( fs::path(TEST_MOD_PATH) );

    ck2::DefaultMap dm(vfs);
    ck2::DefinitionsTable def_tbl(vfs, dm);
    ck2::AdjacenciesFile adj_file(vfs, dm);
    BMPReader bmp( vfs["map" / dm.province_map_path()] );

    // TODO: check for any failures to insert into color2id_map due to a duplicated color key
    std::unordered_map<BGR, prov_id_t> color2id_map;

    for (const auto& row : def_tbl)
      color2id_map.emplace(BGR(row.color.blue(), row.color.green(), row.color.red()), row.id);

    color2id_map.emplace(ImpassableColorMap);
    color2id_map.emplace(OceanColorMap);

    SegmentMap<prov_id_t, uint16_t> seg_map(bmp.width(), bmp.height(), color2id_map, "provinces bitmap");
    bmp.foreach_segment([&](BGR c, uint x1, uint x2, uint y) { seg_map.add_segment(c, x1, x2, y); });

    // Prepare output provinces.bmp (no actual scaling yet) ... //

    ck2::BMPHeader prov_bmp_hdr;
    memset(&prov_bmp_hdr, 0, sizeof(prov_bmp_hdr));

    uint n_rows = bmp.height(), n_cols = bmp.width();
    auto row_sz = ((n_cols * 24 + 31) / 32) * 4;
    auto bmp_sz = row_sz * n_rows;
    auto file_sz = static_cast<uint>(sizeof(prov_bmp_hdr) + bmp_sz);

    prov_bmp_hdr.magic = ck2::BMPHeader::MAGIC;
    prov_bmp_hdr.n_file_size = file_sz;
    prov_bmp_hdr.n_bitmap_offset = sizeof(prov_bmp_hdr);
    prov_bmp_hdr.n_header_size = 40;
    prov_bmp_hdr.n_width = n_cols;
    prov_bmp_hdr.n_height = n_rows;
    prov_bmp_hdr.n_planes = 1;
    prov_bmp_hdr.n_bpp = 24;
    prov_bmp_hdr.n_bitmap_size = bmp_sz;

    const auto ferr = ck2::FLErrorStaticFactory(ck2::FLoc(PROVBMP_TEST_OUTPUT_PATH));

    unique_fptr ufp( std::fopen(PROVBMP_TEST_OUTPUT_PATH, "wb"), std::fclose );

    if (!ufp)
      throw ferr("Failed to open file for writing: {}", strerror(errno));

    if (errno = 0; fwrite(&prov_bmp_hdr, sizeof(prov_bmp_hdr), 1, ufp.get()) < 1)
      throw ferr("Failed to write bitmap header: {}", strerror(errno));

    auto row_buf = std::make_unique<uint8_t[]>(row_sz);

    for (int y = (signed)n_rows - 1; y >= 0; --y)
    {
      const auto& seg_row = seg_map[y];
      assert( !seg_row.empty() );

      uint start_x = 0;
      auto p_out = &row_buf[0];

      // BLIT BLIT BLIT LIKE THE MADMAN THAT YOU ALWAYS WANTED TO BE!
      for (auto seg : seg_row)
      {
        BGR color;

        if (seg.id == OceanColorMap.second)
          color = OceanColorMap.first;
        else if (seg.id == ImpassableColorMap.second)
          color = ImpassableColorMap.first;
        else
        {
          auto c = def_tbl[seg.id].color; // currently these are in RGB rather than BGR
          color = BGR(c.blue(), c.green(), c.red());
        }

        uint x = start_x;

        for (; x < seg.end; ++x, p_out += 3)
        {
          p_out[0] = color.blue();
          p_out[1] = color.green();
          p_out[2] = color.red();
        }

        start_x = x;
      }

      if (errno = 0; fwrite(row_buf.get(), row_sz, 1, ufp.get()) < 1)
        throw ferr("Failed to write row of bitmap data: {}", strerror(errno));
    }

    if (auto f = ufp.release(); fclose(f) != 0)
      throw ferr("Failed to complete writing file: {}", strerror(errno));
  }
  catch (std::exception& e) {
    fmt::print(stderr, "Fatal error:\n{}\n", e.what());
    return 255;
  }

  return 0;
}
