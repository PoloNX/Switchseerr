add_rules("mode.debug", "mode.release")
set_languages("cxx20")

if is_host("windows") then
    add_cxflags("/utf-8")
end

add_repositories("polonx-repo https://github.com/PoloNX/xmake-repo.git")
add_repositories("zeromake-repo https://github.com/zeromake/xrepo.git")

add_requires("libcurl", "nlohmann_json", "fmt", "borealis")
add_requires("xmake-repo@lunasvg", { alias = "lunasvg" })
add_requires("xmake-repo@plutovg", { alias = "plutovg" })


add_defines(
    'BRLS_RESOURCES="resources/"',
    "YG_ENABLE_EVENTS",
    "NOMINMAX"
)

target("Switchseerr")
    set_kind("binary")
    add_files("source/**.cpp")
    add_includedirs("include")

    add_packages("libcurl", "nlohmann_json", "fmt", "borealis", "lunasvg", "plutovg")
    set_rundir("$(projectdir)")

    if is_plat("macosx") then
        add_frameworks("CoreWLAN")
    end

