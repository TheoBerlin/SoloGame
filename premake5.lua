function array_to_string(array)
	local resultingString = ""
	for _, value in ipairs(array) do
		resultingString = resultingString .. value .. " "
	end

	return string.format("[ %s ]", resultingString)
end

function get_vk_sdk_path()
	local sdkPathVars = {"VK_SDK_PATH", "VULKAN_SDK"}
	for _, sdkPathVar in ipairs(sdkPathVars) do
		sdkPath = os.getenv(sdkPathVar)
		if sdkPath ~= nil then
			return sdkPath
		end
	end

	print(string.format("No environment variables for path to Vulkan SDK are set: %s", array_to_string(sdkPathVars)))
	return ""
end

function get_fmod_dll_path()
	local driveLetters = {"C", "D"}
	local potentialPaths = {
		":/FMOD Studio API Windows/api/core/lib/x64/fmodL.dll",
		":/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64/fmodL.dll"
	}

	for _, path in ipairs(potentialPaths) do
		for _, driveLetter in ipairs(driveLetters) do
			local fullPath = driveLetter .. path
			if os.isfile(fullPath) then
				return "\"" .. fullPath .. "\""
			end
		end
	end

	print("fmodL.dll was not found, ensure that FMOD engine is installed or modify premake5.lua")
end

VK_SDK_PATH		= get_vk_sdk_path()
FMOD_DLL_PATH	= get_fmod_dll_path()

workspace "GameProject"
	startproject "GameProject"
	architecture "x64"
	warnings "extra"
	flags { "MultiProcessorCompile" }

	platforms {
		"x64"
    }

	configurations {
		"Debug",
		"Release",
		"Production",
	}

	filter "configurations:Debug"
		symbols "on"
		runtime "Debug"
		defines {
			"_DEBUG",
			"TOUCAN_CONFIG_DEBUG",
		}
	filter "configurations:Release"
		symbols "on"
		runtime "Release"
		optimize "Full"
		defines {
			"NDEBUG",
			"TOUCAN_CONFIG_RELEASE",
		}
	filter "configurations:Production"
		symbols "off"
		runtime "Release"
		optimize "Full"
		defines {
			"NDEBUG",
			"TOUCAN_CONFIG_PRODUCTION",
		}
	filter {}

	-- Compiler option
	filter "action:vs*"
		defines {
			"_CRT_SECURE_NO_WARNINGS",
		}
	filter { "action:vs*", "configurations:Debug" }
		defines {
			"_CRTDBG_MAP_ALLOC",
        }
    filter {}

	filter "system:windows"
		defines {
			"TOUCAN_PLATFORM_WINDOWS",
		}
	filter {}

	project "GameProject"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++latest"
		systemversion "latest"

        -- Targets
        outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{cfg.platform}"
		targetdir ("build/bin/" .. outputdir .. "/GameProject")
        objdir ("build/obj/" .. outputdir .. "/GameProject")

		includedirs {
            "src",
            "vendor",
            ""
        }

		sysincludedirs {
            -- FMOD
            "C:/FMOD Studio API Windows/api/core/inc",
            "D:/FMOD Studio API Windows/api/core/inc",
            "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc",
            "D:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc",

            -- Vulkan
            VK_SDK_PATH .. "/Include",
        }

		-- Files
		files {
            "src/**.hpp",
            "src/**.cpp",

            "vendor/DirectXTK/**.hpp",
            "vendor/DirectXTK/**.cpp"
        }

        pchheader "Engine/EnginePCH.hpp"
        pchsource "src/Engine/EnginePCH.cpp"

        forceincludes {
			"Engine/EnginePCH.hpp"
		}

        links {
            "vulkan-1",
            "fmodL_vc.lib",
        }

        libdirs {
            "libs",

			VK_SDK_PATH .. "/Lib",

			"vendor/assimp/libs",
			"vendor/freetype/libs",
			"vendor/GLFW/libs",

            -- FMOD
            "C:/FMOD Studio API Windows/api/core/lib/x64",
            "D:/FMOD Studio API Windows/api/core/lib/x64",
            "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64",
            "D:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64"
        }

        filter { "system:windows" }
            links {
				"d3d11.lib",
				"runtimeobject.lib",
                "D3DCompiler.lib"
			}
			postbuildcommands {
				("{COPY} " .. FMOD_DLL_PATH .. " \"build/bin/" .. outputdir .. "/GameProject/\""),
			}
		filter { "configurations:Debug" }
			links {
				-- Assimp
				"/debug/assimp-vc142-mtd.lib",
				"/debug/IrrXMLd.lib",
				"/debug/zlibstaticd.lib",

				"/debug/freetype-d.lib",
				"/debug/glfw3.lib"
			}
			postbuildcommands {
				("{COPY} vendor/assimp/bin/debug \"build/bin/" .. outputdir .. "/GameProject/\""),
				("{COPY} vendor/freetype/bin/debug \"build/bin/" .. outputdir .. "/GameProject/\""),
				("{COPY} vendor/GLFW/bin \"build/bin/" .. outputdir .. "/GameProject/\"")
			}
		filter { "configurations:Release or Production" }
			links {
				-- Assimp
				"/release/assimp-vc142-mt.lib",
				"/release/IrrXML.lib",
				"/release/zlibstatic.lib",

				"/release/freetype.lib",
				"/release/glfw3.lib"
			}
			postbuildcommands {
				("{COPY} vendor/assimp/bin/release \"build/bin/" .. outputdir .. "/GameProject/\""),
				("{COPY} vendor/freetype/bin/release \"build/bin/" .. outputdir .. "/GameProject/\""),
				("{COPY} vendor/GLFW/bin \"build/bin/" .. outputdir .. "/GameProject/\"")
			}
        filter {}
