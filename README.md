# MapScaler for CK2
*A tool to comprehensively resize any map for [Crusaders Kings II](https://en.wikipedia.org/wiki/Crusader_Kings_II) with impressive quality.*

## Comprehensive
Resizing (typically, upscaling — i.e., increasing the resolution) a CKII map is no small undertaking. Several types of images and other data files must be adjusted in very different ways, all of which must be coherent. Additionally, extremely tedious tasks such as adjusting the map's `positions.txt` for the new resolution are also handled automatically.

## High-Quality
As almost all of those numerous different types of images are symbolically meaningful down to each pixel (i.e., we're not dealing with continuous-tone photographs here!), traditional image resizing algorithms are not an option. Furthermore, the semantics derived from that symbolic representation can be leveraged to achieve map scaling with a higher degree of quality and vastly reduced artifacting / aliasing relative to those traditional algorithms.

## Project Status
MapScaler is currently still in a pre-release phase with the base game (vanilla), HIP/SWMH, and the Game of Thrones maps being used for development & testing. It will be widely available with a GUI once it's released, and it will likely support upscaling EU4 ([Europa Universalis IV](https://en.wikipedia.org/wiki/Europa_Universalis_IV)) maps as well shortly thereafter.

Its first use shall be to scale [HIP](http://hip.zijistark.com)'s beautiful custom map (SWMH) to a considerably higher resolution without sacrificing the incredible amount of hours already invested in adjusting every pixel of that map both for accuracy and aesthetic and additionally automating improvement of that map's properties to take advantage of its increased area in-game.

## Author
MapScaler is brought to you by Matthew Hall (i.e., **zijistark**), leader of [HIP](http://hip.zijistark.com) (the [Historical Immersion Project](http://hip.zijistark.com)), which is one of the most prominent overhaul mods for CKII.

*Copyright (C) 2018 Matthew D. Hall*