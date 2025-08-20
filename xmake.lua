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
    add_defines('BRLS_RESOURCES="romfs:/"')
else
    add_requires("libcurl", "fmt", "borealis")
    add_requires("xmake-repo@lunasvg", { alias = "lunasvg" })
    add_requires("xmake-repo@plutovg", { alias = "plutovg" })
    add_defines('BRLS_RESOURCES="resources/"')
end


add_defines(
    "YG_ENABLE_EVENTS",
    "NOMINMAX"
)

rule("install_resources")
    local resourcesInstalled = false 
    after_install(function(target)
        if (not resourcesInstalled) then
            if is_plat("macosx") then 
                os.vcp("resources", path.join(target:installdir(), "Contents/Resources"))
            else
                os.vcp("resources", path.join(target:installdir(), "bin"))
            end
            resourcesInstalled = true
        end
    end)

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
        set_values("switch.version", "1.0.0")
        set_values("switch.romfs", "resources")
        add_packages("borealis", "deko3d", "zlib", "liblzma", "lz4", "libexpat", "libzstd", "lunasvg", "plutovg", "libcurl", "fmt")
    else
        if is_plat("macosx") then 
            add_rules("xcode.application")
            set_values("xcode.bundle_identifier", "com.polonx.switchseerr")
            set_values("xcode.bundle_version", "1.0.0")
            add_files("resources/Info.plist")
            after_build(function(target)
                local bundle_path = target:targetdir() .. "/Switchseerr.app"
                local resources_path = bundle_path .. "/Contents/Resources"
                local contents_path = bundle_path .. "/Contents"
                os.mkdir(resources_path)
                os.vcp("resources", resources_path)
                os.vcp("resources/img/AppIcon.icns", resources_path)
                os.vcp("resources/Info.plist", path.join(contents_path, "Info.plist"))
            end)
        elseif is_plat("windows") then
            -- Windows specific rules (icon)
            add_files("source/resources.rc")
        end

        add_rules("install_resources")
        add_packages("libcurl", "nlohmann_json", "fmt", "borealis", "lunasvg", "plutovg")
    end

    set_rundir("$(projectdir)")

    if is_plat("macosx") then
        add_frameworks("CoreWLAN")
    end

