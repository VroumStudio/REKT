--[[----------------------------------------------------------------------------
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: <VERSION>  Build: <BUILDNUMBER>
  Copyright (c) <COPYRIGHTYEAR> Audiokinetic Inc.
------------------------------------------------------------------------------]]

if not AK then AK = {} end
if not AK.Platforms then AK.Platforms = {} end

AK.Platforms.Windows =
{
	name = "Windows",
	srcdirname = "Win32",
	projdirname = "Win32",
	configurations =
	{
		"Debug",
		"Debug(StaticCRT)",
		"Profile",
		"Profile_EnableAsserts",
		"Profile(StaticCRT)" ,
		"Profile(StaticCRT)_EnableAsserts" ,
		"Release",
		"Release(StaticCRT)",
	},
	platforms = { "Win32", "x64" },
	validActions = { "vs2013", "vs2015", "vs2017" },

	-- API
	---------------------------------
	HasMotion = function()
		return true
	end,

	-- Project factory. Creates "StaticLib" target by default. Static libs (only) are added to the global list of targets.
	-- Other target types supported by premake are "WindowedApp", "ConsoleApp" and "SharedLib".
	-- Upon returning from this method, the current scope is the newly created project.
	CreateProject = function(in_fileName, in_targetName, in_projectLocation, in_suffix, pathPCH, in_targetType)
		verbosef("        Creating project: %s", in_targetName)

		-- Make sure that directory exist
		os.mkdir(AkMakeAbsolute(in_projectLocation))

		-- Create project
		local prj = project(in_targetName)
			if not _AK_BUILD_AUTHORING then
				platforms({"Win32", "x64"})
			end
			location(AkRelativeToCwd(in_projectLocation))
			targetname(in_targetName)
			if in_targetType == nil or in_targetType == "StaticLib" then
				kind("StaticLib")
				-- Add to global table
				_AK_TARGETS[in_targetName] = in_fileName
			else
				kind(in_targetType)
			end
			language("C++")
			uuid(GenerateUuid(in_fileName))
			filename(in_fileName)
			symbols ("on")
			symbolspath "$(OutDir)$(TargetName).pdb"
			flags { "OmitUserFiles", "ForceFiltersFiles" } -- We never want .user files, we always want .filters files.

			-- Common flags.
			characterset "Unicode"
			exceptionhandling "Default"

			-- Precompiled headers.
			if pathPCH ~= nil then
				files
				{
					AkRelativeToCwd(pathPCH) .. "stdafx.cpp",
					AkRelativeToCwd(pathPCH) .. "stdafx.h",
				}
				--pchheader ( AkRelativeToCwd(pathPCH) .. "stdafx.h" )
				pchheader "stdafx.h"
				pchsource ( AkRelativeToCwd(pathPCH) .. "stdafx.cpp" )
				--pchsource "stdafx.cpp"
			end

			-- Standard configuration settings.
			filter ("Debug*")
				defines "_DEBUG"

			filter ("Profile*")
				defines "NDEBUG"
				optimize ("Speed")

			filter ("Release*")
				defines "NDEBUG"
				optimize ("Speed")

			filter {}

			if not _AK_BUILD_AUTHORING then
			-- Note: The AuthoringRelease config is "profile", really. It must not be AK_OPTIMIZED.
			filter "Release*"
				defines "AK_OPTIMIZED"
			end

			-- Add configuration specific options.
			filter "*_fastcall"
				callingconvention "FastCall"

			-- Add architecture specific libdirs.
			filter "platforms:Win32"
				architecture "x86"
				defines "WIN32"
				libdirs{"$(DXSDK_DIR)/lib/x86"}
				vectorextensions "SSE"
			filter "platforms:x64"
				architecture "x86_64"
				defines "WIN64"
				libdirs{"$(DXSDK_DIR)/lib/x64"}

			filter {}

			-- Style sheets.
			local ssext = ".props"

			if in_targetType == "SharedLib" then
				if _AK_BUILD_AUTHORING then
					filter "Debug*"
						vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/Debug" .. GetSuffixFromCurrentAction() .. ssext)
					filter "Profile* or Release*"
						vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/NDebug" .. GetSuffixFromCurrentAction() .. ssext)
				else
					filter "Debug*"
						vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/Debug_StaticCRT" .. in_suffix .. ssext)
					filter "Profile* or Release*"
						vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/NDebug_StaticCRT" .. in_suffix .. ssext)
				end

			else
				filter "*Debug or Debug_fastcall"
					vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/Debug" .. in_suffix .. ssext)
				filter "*Debug(StaticCRT)*"
					vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/Debug_StaticCRT" .. in_suffix .. ssext)
				filter "*Profile or *Profile_EnableAsserts or *Release or Profile_fastcall or Release_fastcall"
					vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/NDebug" .. in_suffix .. ssext)
				filter "*Profile(StaticCRT)* or *Release(StaticCRT)*"
					vs_propsheet(AkRelativeToCwd(_AK_ROOT_DIR) .. "PropertySheets/Win32/NDebug_StaticCRT" .. in_suffix .. ssext)
			end

			DisablePropSheetElements()
			filter {}
				removeelements {
					"TargetExt"
				}

			-- Set the scope back to current project
			project(in_targetName)

		return prj
	end
}
return AK.Platforms.Windows
