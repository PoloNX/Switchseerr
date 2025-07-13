add_repositories("polonx-repo https://github.com/PoloNX/xmake-repo.git")
add_repositories("zeromake-repo https://github.com/zeromake/xrepo.git")

add_requires("libcurl", "nlohmann_json", "fmt", "borealis")


set_plat("linux")

add_rules("mode.debug", "mode.release")


add_defines(
    'BRLS_RESOURCES="resources/"',
    "YG_ENABLE_EVENTS"
)

target("Switchseerr")
    set_kind("binary")
    add_files("source/**.cpp")
    add_includedirs("include")
    add_packages("libcurl", "nlohmann_json", "fmt", "borealis")
    set_rundir("$(projectdir)")