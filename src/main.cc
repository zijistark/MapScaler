
#include <ck2/AdjacenciesFile.h>
#include <ck2/BMPHeader.h>
#include <ck2/DefaultMap.h>
#include <ck2/DefinitionsTable.h>
#include <ck2/FileLocation.h>
#include <ck2/ProvEdgeSet.h>
#include <ck2/ProvMap.h>
#include <ck2/VFS.h>
#include "common.h"
#include "filesystem.h"

constexpr char const* VERSION = "0.0.1";

constexpr char const* GAME_PATH = "C:/Program Files (x86)/Steam/steamapps/common/Crusader Kings II";
constexpr char const* MOD_PATH = "C:/git/SWMH-BETA/SWMH";
constexpr char const* TEST_MOD_PATH = "C:/git/zmod/edgeTest";
constexpr char const* PROVBMP_TEST_OUTPUT_PATH = "C:/git/MapScaler/tmp/provinces.bmp";


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
    ck2::ProvMap pm(vfs, dm, def_tbl);
    ck2::ProvEdgeSet edge_set(pm);

    // prepare output provinces.bmp (no actual scaling yet)
    ck2::BMPHeader prov_bmp_hdr;
    memset(&prov_bmp_hdr, 0, sizeof(prov_bmp_hdr));

    auto n_rows = pm.height(), n_cols = pm.width();
    auto row_sz = ((n_cols * 24 + 31) / 32) * 4;
    auto bmp_sz = row_sz * n_rows;
    auto file_sz = static_cast<uint>(sizeof(prov_bmp_hdr)) + bmp_sz;

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

    unique_file_ptr ufp( std::fopen(PROVBMP_TEST_OUTPUT_PATH, "wb"), std::fclose );
    FILE* f = ufp.get();

    if (f == nullptr)
     throw ferr("Failed to open file for writing: {}", strerror(errno));

    if (errno = 0; fwrite(&prov_bmp_hdr, sizeof(prov_bmp_hdr), 1, f) < 1)
     throw ferr("Failed to write bitmap header: {}", strerror(errno));

    // TODO: output entire bitmap from edge_set input:

    // 1. iterate over all edges in the edge set

    // 2. check H-segment endpoints for x=0. when found, add to array for the first column, so that we know when
    // and how to switch color upon row completion/change.

    // 3. otherwise skip H-segments. for every V-segment, skip the "lowest" (largest y) point in it, which
    // should always be a knot control point, and the rest of the segment's implied points' x-values to an
    // array of vectors indexed by their y-values.

    // 4. sort each y-vector (none should ever be completely empty on any well-formed map... but allow for it)
    // by ascending x-value.

    // 5. sort the single x-vector (for x = 0 horizontal edges pulled out of step (2)) by ascending y. then
    // precompute the start color for each row by attaching it to the type which encapsulates the y-vectors. the
    // x-vector can be discarded at that point.

    // 6. blit each row w/ essentially a precomputed RLE-color plan. write to file. profit!

    if (errno = 0, ufp.release(); fclose(f) != 0)
     throw ferr("Failed to complete writing file: {}", strerror(errno));

    // sort(edges.begin(), edges.end(), [](const unique_ptr<ProvEdge>& p1, const unique_ptr<ProvEdge>& p2) {

    // });

    // fmt::print("traced edges: {}\n", edge_set.size());

    // for (auto&& pe : edge_set)
    // {
    //   auto& e = *pe;
    //   fmt::print("  edge between #{} and #{}:\n", e.relation().front(), e.relation().back());

    //   auto prev_knot = e.front();

    //   for (auto it = std::next(e.cbegin()); it != e.cend(); ++it)
    //   {
    //     auto dir = (it->x == prev_knot.x) ? "vertical" : "horizontal";
    //     fmt::print("    {} segment: {} --> {}\n", dir, prev_knot, *it);
    //     prev_knot = *it;
    //   }
    // }
  }
  catch (std::exception& e) {
    fmt::print(stderr, "Fatal Error:\n{}\n", e.what());
    return 255;
  }

  return 0;
}
