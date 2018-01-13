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

1. support as VideoFX filter add to video track
1. support as VideoFX filter add to static image clip
1. support as Generator, but need setting duration length manual
1. support clip frame position only, but can set position offset in filter setup dialog
1. ass only support utf-8 encoding
1. add a simple Tawawa Blue filter :P

### to-do

1. as a generator
1. override style with default style setting
1. fix more bugs

### bugs

1. spacing changing is no effect
1. margins changing maybe with bugs
1. override style with default style has much bugs, such can't live view when changing font name, 
   font size no effect, color only alpha part effecive, and may crashing host app.
1. maybe not **render out**/**crashed host** as a fx with generator.
1. sometime will crashed the host when loading ass file with cjk contents
1. sometime will not render subtitles content when loading ass file with cjk filename

### download

#### source

1. https://github.com/netcharm/renderass_ofx
1. https://bitbucket.org/netcharm/renderass_ofx

#### binary

1. https://bitbucket.org/netcharm/renderass_ofx/downloads/
