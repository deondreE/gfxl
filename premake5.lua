workspace "glfx"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "Pglang"

project "GLFX"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    files { "src/**.h", "src/**.cpp" }

    includedirs { "src" }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"
        defines { "DEBUG" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }