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

AK.Platforms.macOS =
{
	name = "Mac",
	srcdirname = "Mac",
	projdirname = "Mac",
	configurations =
	{
		"Debug",
		"Profile",
		"Profile_EnableAsserts",
		"Release"
	},
	platforms = { "macOS" },
	validActions = { "xcode4" },
	xcodebuildsettings =
	{
		ARCHS = {"x86_64"},
		VALID_ARCHS = {"x86_64"},
		MACOSX_DEPLOYMENT_TARGET = "10.9",
		DEBUG_INFORMATION_FORMAT = "dwarf",
		ONLY_ACTIVE_ARCH = "NO",
		ENABLE_BITCODE = "NO",
		PRECOMPS_INCLUDE_HEADERS_FROM_BUILT_PRODUCTS_DIR = "YES",
		SCAN_ALL_SOURCE_FILES_FOR_INCLUDES = "NO",
		GCC_WARN_MISSING_PARENTHESES = "NO",
		GCC_WARN_CHECK_SWITCH_STATEMENTS = "NO",
		GCC_ENABLE_SSE3_EXTENSIONS = "YES",
		GCC_ENABLE_SUPPLEMENTAL_SSE3_INSTRUCTIONS = "YES",
		GCC_STRICT_ALIASING = "NO",
		GCC_ENABLE_FIX_AND_CONTINUE = "NO",
		GCC_GENERATE_DEBUGGING_SYMBOLS = "YES",
		GCC_DYNAMIC_NO_PIC = "NO",
		GCC_GENERATE_TEST_COVERAGE_FILES = "NO",
		GCC_INLINES_ARE_PRIVATE_EXTERN = "NO",
		GCC_INSTRUMENT_PROGRAM_FLOW_ARCS = "NO",
		GCC_ENABLE_KERNEL_DEVELOPMENT = "NO",
		GCC_REUSE_STRINGS = "YES",
		GCC_NO_COMMON_BLOCKS = "NO",
		GCC_FAST_MATH = "YES",
		GCC_THREADSAFE_STATICS = "YES",
		GCC_SYMBOLS_PRIVATE_EXTERN = "NO",
		GCC_UNROLL_LOOPS = "NO",
		GCC_CHAR_IS_UNSIGNED_CHAR = "NO",
		GCC_ENABLE_ASM_KEYWORD = "YES",
		GCC_C_LANGUAGE_STANDARD = "c99",
		GCC_CW_ASM_SYNTAX = "YES",
		GCC_INPUT_FILETYPE = "automatic",
		GCC_ENABLE_CPP_EXCEPTIONS = "NO",
		GCC_ENABLE_CPP_RTTI = "NO",
		GCC_LINK_WITH_DYNAMIC_LIBRARIES = "YES",
		GCC_ENABLE_OBJC_EXCEPTIONS = "YES",
		GCC_ENABLE_TRIGRAPHS = "NO",
		GCC_ENABLE_FLOATING_POINT_LIBRARY_CALLS = "NO",
		GCC_INCREASE_PRECOMPILED_HEADER_SHARING = "NO",
		OTHER_CFLAGS = "-Wno-sign-compare",
		GCC_PRECOMPILE_PREFIX_HEADER = "NO",
		GCC_ENABLE_BUILTIN_FUNCTIONS = "YES",
		GCC_ENABLE_PASCAL_STRINGS = "YES",
		GCC_SHORT_ENUMS = "NO",
		GCC_USE_STANDARD_INCLUDE_SEARCHING = "YES",
		GCC_WARN_ABOUT_RETURN_TYPE = "YES",
		GCC_WARN_ABOUT_POINTER_SIGNEDNESS = "YES",
		GCC_WARN_UNUSED_VARIABLE = "NO",
		OTHER_CPLUSPLUSFLAGS = {"$(OTHER_CFLAGS)", "-Wno-write-strings", "-fvisibility-inlines-hidden", "-Wno-invalid-offsetof"},
		CLANG_ENABLE_OBJC_ARC = "YES",
		-- 9.2 additions
		CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = "YES",
		CLANG_WARN_BOOL_CONVERSION = "YES",
		CLANG_WARN_COMMA = "YES",
		CLANG_WARN_CONSTANT_CONVERSION = "YES",
		CLANG_WARN_EMPTY_BODY = "YES",
		CLANG_WARN_ENUM_CONVERSION = "YES",
		CLANG_WARN_INFINITE_RECURSION = "YES",
		CLANG_WARN_INT_CONVERSION = "YES",
		CLANG_WARN_NON_LITERAL_NULL_CONVERSION = "YES",
		CLANG_WARN_OBJC_LITERAL_CONVERSION = "YES",
		CLANG_WARN_RANGE_LOOP_ANALYSIS = "YES",
		CLANG_WARN_STRICT_PROTOTYPES = "YES",
		CLANG_WARN_SUSPICIOUS_MOVE = "YES",
		CLANG_WARN_UNREACHABLE_CODE = "YES",
		CLANG_WARN__DUPLICATE_METHOD_MATCH = "YES",
		ENABLE_STRICT_OBJC_MSGSEND = "YES",
		GCC_WARN_64_TO_32_BIT_CONVERSION = "YES",
		GCC_WARN_UNDECLARED_SELECTOR = "YES",
		GCC_WARN_UNINITIALIZED_AUTOS = "YES",
		GCC_WARN_UNUSED_FUNCTION = "YES"
	},

	-- API
	---------------------------------
	HasMotion = function()
		return false
	end,

	-- Project factory. Creates "StaticLib" target by default. Static libs (only) are added to the global list of targets.
	-- Other target types supported by premake are "WindowedApp", "ConsoleApp" and "SharedLib".
	-- Upon returning from this method, the current scope is the newly created project.
	CreateProject = function(in_fileName, in_targetName, in_projectLocation, in_suffix, pathPCH, in_targetType)
		verbosef("        Creating project: %s", in_targetName)

		-- Make sure that directory exist
		os.mkdir(AkMakeAbsolute(in_projectLocation))

		-- Create project
		local prj = project (in_targetName)

			platforms {"macOS"}
			system(premake.MACOSX)
			systemversion "10.6"
			flags {"HackSysIncludeDirs"} -- We need everything to be sysincludedirs, not includedirs.

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

			xcodebuildsettings ( AK.Platforms.macOS.xcodebuildsettings )
			buildoptions { "-Wno-invalid-offsetof" }

			-- Standard configuration settings.
			filter "*Debug*"
				defines ("_DEBUG")
				symbols ("On")
				xcodebuildsettings { GCC_OPTIMIZATION_LEVEL = 0 }

			filter "Profile*"
				defines ("NDEBUG")
				optimize ("Speed")
				symbols ("On")

			filter "*Release*"
				defines ({"NDEBUG","AK_OPTIMIZED"})
				optimize ("Speed")
				symbols ("On")

			filter "*EnableAsserts"
				defines( "AK_ENABLE_ASSERTS" )

			-- 9.2
			filter "not *Release*"
				xcodebuildsettings {
					ENABLE_TESTABILITY = "YES",
				}

			-- Set the scope back to current project
			project(in_targetName)

		return prj
	end
}
return AK.Platforms.macOS
