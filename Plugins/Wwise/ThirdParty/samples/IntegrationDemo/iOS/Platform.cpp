/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2018.1.4  Build: 6590
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

// Platform.cpp
/// \file 
/// Contains definitions for functions declared in Platform.h

#include "stdafx.h"
#include "Platform.h"
#include "UniversalInput.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>

class RenderingEngine;
RenderingEngine*	g_renderingEngine = NULL;
UGBtnState			g_btnState = 0;
UGStickState		g_sticksState[2];
char				g_szBasePath[AK_IOS_MAX_BANK_PATH];
char				g_szDocumentsPath[AK_IOS_MAX_DOCUMENTS_PATH];
unsigned int g_windowWidth;
unsigned int g_windowHeight;

UInt32 g_uSamplesPerFrame = 512;