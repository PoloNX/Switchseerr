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
    add_requires("switch-repo@zlib", {alias = "zlib"})
    add_requires("switch-repo@libcurl", {alias = "libcurl"})
    add_requires("polonx-repo@plutovg", { alias = "plutovg" })
    add_requires("polonx-repo@lunasvg", { alias = "lunasvg" })

    add_requires("deko3d", "liblzma", "lz4", "libexpat", "libzstd", "fmt")
else
    -- Linux installation
    if get_config("install") then
        add_requires("borealis", {config = {resources_dir = "/usr/share/switchseerr"}})
    else
        add_requires("borealis")
    end
    add_requires("libcurl", "fmt")
    add_requires("xmake-repo@lunasvg", { alias = "lunasvg" })
    add_requires("xmake-repo@plutovg", { alias = "plutovg" })
end


add_defines(
    "YG_ENABLE_EVENTS",
    "NOMINMAX"
)

option("install")
    set_default(false)
    set_showmenu(true)

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

    local version = "1.0.0"

    set_version(version)

    if get_config("install") then
        add_defines('BRLS_RESOURCES="/usr/share/switchseerr/resources/"')
    else 
        if is_plat("cross") then
            add_defines('BRLS_RESOURCES="romfs:/"')
        else
            add_defines('BRLS_RESOURCES="./resources/"')    
        end
    end 

    add_configfiles("include/utils/Constants.hpp.in", {prefixdir = "include/utils"})
    add_includedirs("$(builddir)/include")


    if is_plat("cross") then
        set_arch("aarch64")
        add_rules("switch")
        set_toolchains("devkita64")
        set_languages("c++17")
        
        set_values("switch.name", "Switchseerr")
        set_values("switch.author", "PoloNX")
        set_values("switch.version", version)
        set_values("switch.romfs", "resources")
        set_values("switch.icon", "platform/switch/icon.jpg")
        add_packages("borealis", "deko3d", "zlib", "liblzma", "lz4", "libexpat", "libzstd", "lunasvg", "plutovg", "libcurl", "fmt")
    else
        if is_plat("macosx") then 
            add_rules("xcode.application")
            set_values("xcode.bundle_identifier", "com.polonx.switchseerr")
            set_values("xcode.bundle_version", version)
            add_configfiles("platform/macos/Info.plist", {prefixdir = "platform/macos"})
            after_build(function(target)
                local bundle_path = target:targetdir() .. "/Switchseerr.app"
                local resources_path = bundle_path .. "/Contents/Resources"
                local contents_path = bundle_path .. "/Contents"
                os.mkdir(resources_path)
                os.vcp("resources", resources_path)
                os.vcp("platform/macos/AppIcon.icns", resources_path)
                os.vcp("build/platform/macos/Info.plist", path.join(contents_path, "Info.plist"))
            end)
            add_frameworks("CoreWLAN", "SystemConfiguration")
        elseif is_plat("windows") then
            -- Windows specific rules (icon)
            add_configfiles("platform/windows/resources.rc", {prefixdir = "platform/windows"})
            add_files("$(builddir)/platform/windows/resources.rc")
        end

        add_rules("install_resources")
        add_packages("libcurl", "nlohmann_json", "fmt", "borealis", "lunasvg", "plutovg")
    end

    set_rundir("$(projectdir)")

