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
if not AK.Platform then
	newoption {
	   trigger = "akplatform",
	   value = "VALUE",
	   description = "Choose one platform to generate",
	   allowed = {
		  { "Mac", "Mac" },
		  { "Linux", "Linux" },
		  { "iOS", "iOS" },
		  { "tvOS", "tvOS" },
		  { "Windows", "Windows" },
		  { "Android", "Android" },
		  { "XboxOne", "XboxOne" },
		  { "UWP", "UWP" },
		  { "PS4", "PS4" },
		  { "QNX", "QNX" },
		  { "Emscripten", "Emscripten" },
		  { "NX", "NX" },
		  { "Lumin", "Lumin" },
	   }
	}

	-- These are not capitalized
	local platformRename = {
		aix = "AIX",
		bsd = "BSD",
		ios = "iOS",
		macosx = "macOS", -- Note: discrepancy in name here!
		ps4 = "PS4",
		xboxone = "XboxOne",
		nx = "NX",
		tvos = "tvOS",
		uwp = "UWP",
		qnx = "QNX",
	}

	local platformFileName = os.target()
	if platformRename[platformFileName] ~= nil then
		platformFileName = platformRename[platformFileName]
	else
		platformFileName = platformFileName:sub(1,1):upper()..platformFileName:sub(2):lower() -- Capitalize
	end
	AK.Platform = require ("Platforms." .. platformFileName)
end
