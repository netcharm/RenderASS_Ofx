# openfx plugin of ASS/SSA (Advanced Substation Alpha/Substation Alpha)

 this is a openfx plugin of ASS/SSA (Advanced Substation Alpha/Substation Alpha), 
 it based libass/freetype

## development env.

1. i7-7700hq/8G/GTX1060-6G
1. visual studio express 2015 for windows desktop
1. windows 10 v1709 (maybe 8.1 up)

## requirements

### libraries

1. libxml2-2.9.7
1. libiconv-1.15
1. freetype-2.6.1
1. fontconfig-2.11.93
1. fribidi-0.19.7
1. harfbuzz-1.3.4
1. libass-0.14.0

### test software

1. MAGIX MovieStudio Platinum 13 (maybe latesd version)

### feature

1. support as VideoFX add to video track
1. ass only support utf-8

### to-do

1. as a generator
1. override style with default style setting
1. fix more bugs

### bugs

1. maybe not **render out**/**crashed host** as a fx in on generator.
1. sometime will crashed the host when loading ass file with cjk contents
1. sometime will not render subtitles content when loading ass file with cjk filename
