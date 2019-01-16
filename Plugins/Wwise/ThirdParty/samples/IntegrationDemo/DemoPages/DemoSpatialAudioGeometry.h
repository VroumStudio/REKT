/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2018.1.4  Build: 6807
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/
// Demo3dBus.h
/// \file
/// Contains the declaration for the Demo3dBus class.

#pragma once

#include "Page.h"
#include "MovableChip.h"
#include "DemoSpatialAudio.h"

class DemoSpatialAudioGeometry : public Page
{
public:

	/// Demo3dBus class constructor.
	DemoSpatialAudioGeometry(
		Menu& in_pParentMenu	///< - Pointer to the Menu that the page belongs to
		);

	/// Override of the Parent's Init() method.
	virtual bool Init();

	/// Override of the Parent's Release() method.
	virtual void Release();

	/// Override of the Parent's Update() method.
	virtual bool Update();

	/// Override of the Parent's Draw() method.
	virtual void Draw();

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	bool Init_SpatialAudioDemo();
	void Init_Geometry();

	void UpdateGameObjPos(MovableChip* in_pChip, AkGameObjectID in_GameObjectId);

	// Helpers

	float PixelsToAKPos_X(float in_X);
	float PixelsToAKPos_Y(float in_y);
	float PixelsToAKLen_X(float in_X);
	float PixelsToAKLen_Y(float in_y);
	int AKPosToPixels_X(float in_X);
	int AKPosToPixels_Y(float in_y);

	// Members
	
	// UI controls/widgets
	MovableChip *m_pEmitterChip;
	MovableChip *m_pListenerChip;
#define k_numGeometryPlanes (2)
	Line * m_geometry[k_numGeometryPlanes];
	Line ** m_aLines;
	int m_numLines;


	float m_fGameObjectX;
	float m_fGameObjectZ;
	float m_diffraction;
	float m_fWidth;
	float m_fHeight;
	float m_room0_width;
	float m_room0_height;
	
	bool m_bMoved;
};
