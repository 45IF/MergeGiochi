#!/bin/bash
# Compilazione di MergeGiochi.asi con MinGW-w64 (i686) su Linux, usata da Claude l'8/11/2026.
# Alternativa a MSVC: stessa sorgente plugin-sdk (DK22Pac). Uso: PSDK=/percorso/plugin-sdk ./compila_mingw.sh
set -e
P=${PSDK:?imposta PSDK}; SRC=$(dirname "$0")/../src; OUT=${OUT:-build-mingw}; mkdir -p $OUT/obj
INC=$(find $P/plugin_sa/game_sa -type d | sed 's|^|-I|' | tr '\n' ' ')
FL="-c -O2 -std=c++23 -fpermissive -w -m32 -DRW -DGTASA -DPLUGIN_SGV_10US -D_USE_MATH_DEFINES -DGTAGAME_NAME=\"San Andreas\" -DGTAGAME_ABBR=\"SA\" -DGTAGAME_ABBRLOW=\"sa\" -DGTAGAME_PROTAGONISTNAME=\"CJ\" -DGTAGAME_CITYNAME=\"San Andreas\" -I$P/shared -I$P/shared/game -I$P/plugin_sa $INC -I$P/injector/include -I$P/modutils -I$P/safetyhook/include"
CXX=i686-w64-mingw32-g++-posix
LIB="shared/Color shared/DynAddress shared/GameVersion shared/Other shared/Patch shared/Pattern shared/PluginBase shared/StringUtils shared/common_sdk shared/plugin shared/game/CRGBA shared/game/CVector shared/game/CompressedVector shared/game/CompressedVector2D"
for c in CPed CPad CWorld CCamera CHud CStreaming CClothes CModelInfo CPedIntelligence CTimer CPlaceable CEntity CMatrix common CTaskManager CAESound CAEWeaponAudioEntity CBaseModelInfo CRect CText RenderWare; do
  LIB="$LIB $(cd $P && find plugin_sa/game_sa -name "$c.cpp" | head -1 | sed 's/\.cpp$//')"; done
for f in $LIB; do eval $CXX $FL $P/$f.cpp -o $OUT/obj/$(echo $f | tr '/' '_').o; done
for f in $(find $SRC -name '*.cpp' ! -name 'ProvaASI.cpp'); do eval $CXX $FL $f -o $OUT/obj/mg_$(basename $f).o; done
$CXX -shared -static -static-libgcc -static-libstdc++ -s -o $OUT/MergeGiochi.asi $OUT/obj/*.o -Wl,--enable-stdcall-fixup
echo "Creato $OUT/MergeGiochi.asi"
