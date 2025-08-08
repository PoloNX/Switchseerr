add_rules("mode.debug", "mode.release")
set_languages("cxx20")

includes("toolchain/*.lua")

if is_host("windows") then
    add_cxflags("/utf-8")
end

add_repositories("polonx-repo https://github.com/PoloNX/xmake-repo.git")
add_repositories("zeromake-repo https://github.com/zeromake/xrepo.git")


if is_plat("cross") then
    add_repositories("switch-repo https://github.com/PoloNX/switch-repo.git")
    add_requires("switch-repo@borealis", {alias = "borealis"})
    add_requires("deko3d", "libcurl", "switch-repo@zlib", {alias = "zlib"}, "liblzma", "lz4", "libexpat", "libzstd", "lunasvg", "plutovg", "fmt")
else
    add_requires("libcurl", "fmt", "borealis")
    add_requires("xmake-repo@lunasvg", { alias = "lunasvg" })
    add_requires("xmake-repo@plutovg", { alias = "plutovg" })
end


add_defines(
    'BRLS_RESOURCES="resources/"',
    "YG_ENABLE_EVENTS",
    "NOMINMAX"
)

target("Switchseerr")
    set_kind("binary")
    add_files("source/***.cpp")
    add_includedirs("include")
    set_version("1.0.0")

    add_configfiles("include/utils/Constants.hpp.in", {prefixdir = "include/utils"})
    add_includedirs("$(builddir)/include")


    if is_plat("cross") then
        set_arch("aarch64")
        add_rules("switch")
        set_toolchains("devkita64")
        set_languages("c++17")
        
        set_values("switch.name", "Switchseerr")
        set_values("switch.author", "PoloNX")
        set_values("switch.version", "$(version)")
        set_values("switch.romfs", "resources")
        add_packages("borealis", "deko3d", "zlib", "liblzma", "lz4", "libexpat", "libzstd", "lunasvg", "plutovg", "libcurl", "fmt")
    else
        add_packages("libcurl", "nlohmann_json", "fmt", "borealis", "lunasvg", "plutovg")
    end

    set_rundir("$(projectdir)")

    if is_plat("macosx") then
        add_frameworks("CoreWLAN")
    end

