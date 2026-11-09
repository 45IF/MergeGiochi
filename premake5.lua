local sdk = os.getenv('PLUGIN_SDK_DIR')
assert(sdk and os.isdir(sdk), 'Impostare PLUGIN_SDK_DIR sul clone plugin-sdk')
workspace 'MergeGiochi'
    location 'build'
    configurations { 'Release' }
    architecture 'x86'
    system 'windows'
    systemversion 'latest'
    language 'C++'
    cppdialect 'C++latest'
    staticruntime 'On'
    characterset 'MBCS'
    optimize 'Speed'
    defines { 'GTASA', 'PLUGIN_SGV_10US', 'RW', '_CRT_SECURE_NO_WARNINGS', '_CRT_NON_CONFORMING_SWPRINTFS', '_DX9_SDK_INSTALLED' }
    includedirs { sdk .. '/plugin_sa', sdk .. '/plugin_sa/game_sa', sdk .. '/plugin_sa/game_sa/enums', sdk .. '/plugin_sa/game_sa/rw', sdk .. '/shared', sdk .. '/shared/game', sdk .. '/shared/dxsdk', sdk .. '/safetyhook' }
    targetdir 'build/bin'
    objdir 'build/obj/%{prj.name}'
project 'PluginSDK_SA'
    kind 'StaticLib'
    files { sdk .. '/plugin_sa/**.cpp', sdk .. '/shared/**.cpp', sdk .. '/hooking/**.cpp', sdk .. '/safetyhook/**.cpp', sdk .. '/safetyhook/**.c' }
project 'ProvaASI'
    kind 'SharedLib'
    targetextension '.asi'
    files { 'src/ProvaASI.cpp' }
    links { 'PluginSDK_SA' }
