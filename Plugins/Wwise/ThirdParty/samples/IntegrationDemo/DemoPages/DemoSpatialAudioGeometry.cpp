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

// DemoSpatialAudioGeometry.cpp
/// \file
/// Defines all methods declared in DemoSpatialAudioGeometry.h

#include "stdafx.h"

#include <math.h>
#include <float.h>
#include "Menu.h"
#include "MovableChip.h"
#include "DemoSpatialAudioGeometry.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

// Our game object ID.  Completely arbitrary.
const AkGameObjectID EMITTER = 100;
const AkGameObjectID LISTENER = 103;

#define REPEAT_TIME 20

/////////////////////////////////////////////////////////////////////
// DemoSpatialAudio Public Methods
/////////////////////////////////////////////////////////////////////

DemoSpatialAudioGeometry::DemoSpatialAudioGeometry(Menu& in_ParentMenu)
	: Page( in_ParentMenu, "Geometry")
	, m_pEmitterChip( NULL )
	, m_pListenerChip(NULL)
	, m_aLines(NULL)
	, m_numLines(0)
	, m_fGameObjectX( 0 )
	, m_fGameObjectZ( 0 )
	, m_diffraction( 0 )
	, m_fWidth( 0.0f )
	, m_fHeight(0.0f)
	, m_room0_width(0.0f)
	, m_room0_height(0.0f)
	, m_bMoved(false)
{
	for (int i = 0; i < k_numGeometryPlanes; i++)
		m_geometry[i] = NULL;
}

bool DemoSpatialAudioGeometry::Init_SpatialAudioDemo()
{
	// Register the emitter game object
	AK::SoundEngine::RegisterGameObj(EMITTER, "Emitter E");
	
	// Let's register a different listener than the default one (registered in IntegrationDemo.cpp) just for this demo, because we are going to move it around.
	AK::SoundEngine::RegisterGameObj(LISTENER, "Listener L");
	
	// Associate our emitters with our listener. This will set up direct path from EMITTER -> LISTENER
	// The listener object will also be emitting so we need to set LISTENER -> LISTENER too.
	static const int kNumLstnrsForEm = 1;
	static const AkGameObjectID aLstnrsForEmitter[kNumLstnrsForEm] = { LISTENER };
	AK::SoundEngine::SetListeners(EMITTER, aLstnrsForEmitter, kNumLstnrsForEm);
	AK::SoundEngine::SetListeners(LISTENER, aLstnrsForEmitter, kNumLstnrsForEm);

	// Register these two objects as spatial audio emitters.
	AkEmitterSettings em;
	em.diffractionMaxEdges = 3;				/// Max number of edges for calculating diffraction.
	em.diffractionMaxPathLength = 200.f;	/// Heuristic for diffraction path search. Ideally it should be greater than (all) sounds' Max Distance (set in positioning Attenuation, in the Wwise project).
	// Otherwise, occluded objects become visible between distances AkEmitterSettings::diffractionMaxPathLength and Max Distance.
	AK::SpatialAudio::RegisterEmitter(EMITTER, em);
	AK::SpatialAudio::RegisterEmitter(LISTENER, AkEmitterSettings());

	// And register our listener as the one and only spatial audio listener.
	AK::SpatialAudio::RegisterListener(LISTENER);

	m_szHelp =
		"*** Please connect Wwise and open the 3D Game Object Viewer (F12 layout).\n"
		"This demo showcases the Wwise Spatial Audio Geometry API, usable for direct (dry) path diffraction, and for using with the Wwise Reflect plug-in,\n" 
		"although the latter is not used in the Integration Demo Wwise project.\n"
		"In this demo we have 2 walls, and\n"
		"* 'Emitter/[E]' - the position of an emitter,\n"
		"* 'Listener/[L]' - the position of the listener (namely the main character or camera in a game).\n"
		"The diffraction path(s) are displayed, with the resulting diffraction amount in the lower left corner.\n"
		"Spatial Audio is set up so that diffraction controls both project-wide obstruction and the Diffraction built-in parameter, although only the former is used in the project.\n"
		"Refer to Spatial Audio Geometry in the SDK documentation for more details.\n"
		"* <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the listener game object.\n"
		"* <<UG_BUTTON5>> + <<UG_RIGHT_STICK>> + <<DIRECTIONAL_TYPE>> - Move the emitter game object.\n"
		"* <<UG_BUTTON1>> - Open/close portals in sequence.";

	return true;
}

bool DemoSpatialAudioGeometry::Init()
{
	if (!Init_SpatialAudioDemo())
		return false;
	
	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Bus3d_Demo.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Bus3d_Demo.bnk" );
		return false;
	}

	return Page::Init();
}

void DemoSpatialAudioGeometry::Release()
{
	AK::SoundEngine::StopAll();
	AK::SpatialAudio::RemoveGeometry(0);
	AK::SpatialAudio::RemoveGeometry(1);
	AK::SoundEngine::UnregisterGameObj(EMITTER);
	AK::SoundEngine::UnregisterGameObj(LISTENER);
	AK::SoundEngine::UnloadBank( "Bus3d_Demo.bnk", NULL );

	Page::Release();
}

#define POSITION_RANGE (200.0f)

float DemoSpatialAudioGeometry::PixelsToAKPos_X(float in_X)
{
	return ((in_X / m_fWidth) - 0.5f) * POSITION_RANGE;
}

float DemoSpatialAudioGeometry::PixelsToAKPos_Y(float in_y)
{
	return -((in_y / m_fHeight) - 0.5f) * POSITION_RANGE;
}

float DemoSpatialAudioGeometry::PixelsToAKLen_X(float in_X)
{
	return (in_X / m_fWidth) * POSITION_RANGE;
}

float DemoSpatialAudioGeometry::PixelsToAKLen_Y(float in_y)
{
	return (in_y / m_fHeight) * POSITION_RANGE;
}

int DemoSpatialAudioGeometry::AKPosToPixels_X(float in_X)
{
	return (int)(((in_X / POSITION_RANGE) + 0.5f) * m_fWidth);
}

int DemoSpatialAudioGeometry::AKPosToPixels_Y(float in_y)
{
	return (int)(((-in_y / POSITION_RANGE) + 0.5f) * m_fHeight);
}

void DemoSpatialAudioGeometry::UpdateGameObjPos(MovableChip* in_pChip, AkGameObjectID in_GameObjectId)
{
	if (in_pChip)
	{
		float x, y;
		in_pChip->GetPos(x, y);

		// Converting X-Y UI into X-Z world plan.
		AkVector position;
		m_fGameObjectX = position.X = PixelsToAKPos_X(x);
		position.Y = 0;
		m_fGameObjectZ = position.Z = PixelsToAKPos_Y(y);
		AkVector orientationFront;
		orientationFront.Z = 1;
		orientationFront.Y = orientationFront.X = 0;
		AkVector orientationTop;
		orientationTop.X = orientationTop.Z = 0;
		orientationTop.Y = 1;

		AkSoundPosition soundPos;
		soundPos.Set(position, orientationFront, orientationTop);
		AK::SpatialAudio::SetPosition(in_GameObjectId, soundPos);
	}
	
}

bool DemoSpatialAudioGeometry::Update()
{
	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Skip this input device if it's not connected
		if ( ! it->IsConnected() )
			continue;

		if ((*it).IsButtonDown(UG_BUTTON5))
		{
			m_pEmitterChip->Update(*it);
			m_bMoved = true;
		}
		else
		{
			float xprev, yprev;
			m_pListenerChip->GetPos(xprev, yprev);
			m_pListenerChip->Update(*it);
			float x, y; 
			m_pListenerChip->GetPos(x, y);

			m_bMoved = true;
		}
	}

	if ( m_bMoved )
	{
		// Update emitters, set their room.
		UpdateGameObjPos(m_pEmitterChip, EMITTER);
		UpdateGameObjPos(m_pListenerChip, LISTENER);

		//
		// Query propagation for UI display.
		//
		
		for (int i = 0; i < m_numLines; i++)
		{
			if (m_aLines[i])
				delete m_aLines[i];
		}
		delete[] m_aLines;
		m_aLines = NULL;
		m_numLines = 0;

		AkVector emitterPos, listenerPos;
		AkDiffractionPathInfo paths[8];
		AkUInt32 uNumPaths = 8;	// 8 arbitrary
		AK::SpatialAudio::QueryDiffractionPaths(
			EMITTER,	///< The ID of the game object that the client wishes to query.
			listenerPos,	///< Returns the position of the listener game object that is associated with the game object \c in_gameObjectID.
			emitterPos,		///< Returns the position of the emitter game object \c in_gameObjectID.
			paths,			///< Pointer to an array of \c AkPropagationPathInfo's which will be filled after returning.
			uNumPaths		///< The number of slots in \c out_aPaths, after returning the number of valid elements written.
			);

		// Count lines to draw.
		for (AkUInt32 path = 0; path < uNumPaths; path++)
		{
			int numNodes = paths[path].nodeCount;
			int numSegments = numNodes + 1;
			m_numLines += numSegments;
		}

		m_diffraction = 0;

		float diffraction = FLT_MAX;

		if (m_numLines > 0)
		{
			m_aLines = new Line*[m_numLines];

			int line = 0;
			for (AkUInt32 path = 0; path < uNumPaths; path++)
			{
				int numNodes = paths[path].nodeCount;

				if (numNodes > 0)
				{
					// Listener to node 0.
					m_aLines[line] = new Line(*this);
					m_aLines[line]->SetPoints(AKPosToPixels_X(listenerPos.X), AKPosToPixels_Y(listenerPos.Z), AKPosToPixels_X(paths[path].nodes[0].X), AKPosToPixels_Y(paths[path].nodes[0].Z));
					++line;

					// node i-1 to node i
					for (int i = 1; i < numNodes; i++)
					{
						m_aLines[line] = new Line(*this);
						m_aLines[line]->SetPoints(AKPosToPixels_X(paths[path].nodes[i - 1].X), AKPosToPixels_Y(paths[path].nodes[i - 1].Z), AKPosToPixels_X(paths[path].nodes[i].X), AKPosToPixels_Y(paths[path].nodes[i].Z));
						++line;
					}
					
					// Last node to emitter.
					m_aLines[line] = new Line(*this);
					m_aLines[line]->SetPoints(AKPosToPixels_X(paths[path].nodes[numNodes - 1].X), AKPosToPixels_Y(paths[path].nodes[numNodes - 1].Z), AKPosToPixels_X(emitterPos.X), AKPosToPixels_Y(emitterPos.Z));
					++line;
				}
				else
				{
					// A path with no node: completely obstructed. Draw a line from emitter to listener, with style "Selected" (Draw() checks m_diffraction)
					m_aLines[line] = new Line(*this);
					m_aLines[line]->SetPoints(AKPosToPixels_X(emitterPos.X), AKPosToPixels_Y(emitterPos.Z), AKPosToPixels_X(listenerPos.X), AKPosToPixels_Y(listenerPos.Z));
					++line;
				}

				// Display the minimum diffraction angle of the (possibly) two propagation paths. Since we only have Room (i.e. game object) scoped RTPCs in Wwise at the moment, this is the actual value that will be pushed
				// to our built-in game parameter "Diffraction".
				diffraction = AkMin(diffraction, paths[path].diffraction);
			
			}
			m_diffraction = diffraction;
		}
		else
		{
			// No path: we must have direct line of sight. Draw a line from emitter to listener.
			m_numLines = 1;
			m_aLines = new Line*[1];
			m_aLines[0] = new Line(*this);
			m_aLines[0]->SetPoints(AKPosToPixels_X(emitterPos.X), AKPosToPixels_Y(emitterPos.Z), AKPosToPixels_X(listenerPos.X), AKPosToPixels_Y(listenerPos.Z));

			m_diffraction = 0;
		}
	}

	// Reset m_bMoved.
	m_bMoved = false;

	return Page::Update();
}

void DemoSpatialAudioGeometry::Draw()
{
	Page::Draw();

	if (m_pEmitterChip)
		m_pEmitterChip->Draw();
	if (m_pListenerChip)
		m_pListenerChip->Draw();

	for (int i = 0; i < k_numGeometryPlanes; i++)
	{
		if (m_geometry[i])
			m_geometry[i]->Draw(DrawStyle_Selected);
	}

	for (int i = 0; i < m_numLines; i++)
	{
		if (m_aLines[i])
			m_aLines[i]->Draw((m_diffraction < 1.f) ? DrawStyle_Control : DrawStyle_Selected);
	}

	char strBuf[128];
	if (m_numLines)
		snprintf(strBuf, 128, "X: %.2f\nZ: %.2f\nDiffraction: %.2f", m_fGameObjectX, m_fGameObjectZ, m_diffraction);

	static int s_nOffset = 4 * GetLineHeight( DrawStyle_Text );

	DrawTextOnScreen( strBuf, 5, m_pParentMenu->GetHeight() - s_nOffset, DrawStyle_Text );
	
	// Display instructions at the bottom of the page
	int iInstructionsY = m_pParentMenu->GetHeight() - 3 * GetLineHeight( DrawStyle_Text );
	DrawTextOnScreen( "(Press <<UG_BUTTON2>> To Go Back...)", m_pParentMenu->GetWidth() / 4, iInstructionsY, DrawStyle_Text );
}

bool DemoSpatialAudioGeometry::OnPointerEvent(PointerEventType in_eType, int in_x, int in_y)
{
	if ( in_eType == PointerEventType_Moved )
	{
		bool bMoveListener = true;
		for (UniversalInput::Iterator it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++)
		{
			if ((*it).IsButtonDown(UG_BUTTON5))
			{
				bMoveListener = false;
				break;
			}
		}

		if (bMoveListener)
			m_pListenerChip->SetPos((float)in_x, (float)in_y);
		else
			m_pEmitterChip->SetPos((float)in_x, (float)in_y);

		m_bMoved = true;	// Force update of game objects and GUI of sound propagation.
	}

	return Page::OnPointerEvent( in_eType, in_x, in_y );
}

void DemoSpatialAudioGeometry::Init_Geometry()
{
	float room0_width_px = m_room0_width = m_fWidth / 2.f;
	float room0_height_px = m_room0_height = m_fHeight / 2.f - 60;
	if (room0_height_px < 0)
		room0_height_px = 0;

	// 2 disjoint meshes: an horizontal wall (on the bottom left) and a vertical wall (on the right).
	// For diffraction, it is important to join meshes to avoid sound leaking through cracks.
	// Here in our 2D geometry, we do not assign triangles to the top and bottom planes (through screen), and utilize
	// AkGeometryParams::EnableDiffractionOnBoundaryEdges = false so that edges on top and bottom will not allow sound to pass 
	// through the ceiling or floor. Likewise, we do not assign triangles on the left of the horizontal wall. 
	//       
	//        |
	// ----   |

	// Vertical wall.
	//

	// Draw Line.
	m_geometry[0] = new Line(*this);
	int centerx = AKPosToPixels_X(0);
	int centery = AKPosToPixels_Y(0);
	m_geometry[0]->SetPoints(centerx, centery, centerx, centery - (int)room0_height_px);
	
	// Register to SpatialAudio
	float room0_height = PixelsToAKLen_Y(room0_height_px);
	AkGeometryParams geom;
	geom.NumVertices = 8;
	// In the coordinate system we have chosen, x is right-left, z is top-bottom and y is through screen.
	/*
	AkVertex vertices[] = {
		{ 0, -30.f, 0 },
		{ 0, 30.f, 0 },
		{ 0, 30.f, room0_height },
		{ 0, -30.f, room0_height },
		{ -1.f, -30.f, room0_height },
		{ -1.f, 30.f, room0_height },
		{ -1.f, 30.f, 0 },
		{ -1.f, -30.f, 0 }
	};
	*/
	// Init one by one for old compilers.
	AkVertex vertices[8];
	AkPlacementNew(vertices + 0) AkVertex(0, -30.f, 0);
	AkPlacementNew(vertices + 1) AkVertex(0, 30.f, 0);
	AkPlacementNew(vertices + 2) AkVertex(0, 30.f, room0_height);
	AkPlacementNew(vertices + 3) AkVertex(0, -30.f, room0_height);
	AkPlacementNew(vertices + 4) AkVertex(-1.f, -30.f, room0_height);
	AkPlacementNew(vertices + 5) AkVertex(-1.f, 30.f, room0_height);
	AkPlacementNew(vertices + 6) AkVertex(-1.f, 30.f, 0);
	AkPlacementNew(vertices + 7) AkVertex(-1.f, -30.f, 0);
	geom.Vertices = vertices;
	
	// Assign surfaces. Direct path diffraction is not affected by the acoustic properties of surfaces, because in this model, the edges don't absorb any energy.
	// In this demo, the only purpose of using acoustic surfaces with textures defined in Wwise is for showing colors in the 3D Game Object Viewer.
	geom.NumSurfaces = 2; 
	AkAcousticSurface surfaces[2];
	AkPlacementNew(&surfaces[0]) AkAcousticSurface();
	surfaces[0].strName = "Outside";
	surfaces[0].textureID = AK::SoundEngine::GetIDFromString("Brick");
	AkPlacementNew(&surfaces[1]) AkAcousticSurface();
	surfaces[1].strName = "Inside";
	surfaces[1].textureID = AK::SoundEngine::GetIDFromString("Drywall");
	geom.Surfaces = surfaces;
	
	geom.NumTriangles = 8;
	/*
	AkTriangle tri[] = {
		{ 0, 1, 2, 0 },
		{ 0, 2, 3, 0 },
		{ 2, 3, 4, 0 },
		{ 2, 4, 5, 0 },
		{ 4, 5, 6, 1 },
		{ 4, 6, 7, 1 },
		{ 7, 0, 6, 0 },
		{ 6, 0, 1, 0 }
	};
	*/
	// Init one by one for old compilers.
	AkTriangle tri[8];
	AkPlacementNew(tri + 0) AkTriangle(0, 1, 2, 0);
	AkPlacementNew(tri + 1) AkTriangle(0, 2, 3, 0);
	AkPlacementNew(tri + 2) AkTriangle(2, 3, 4, 0);
	AkPlacementNew(tri + 3) AkTriangle(2, 4, 5, 0);
	AkPlacementNew(tri + 4) AkTriangle(4, 5, 6, 1);
	AkPlacementNew(tri + 5) AkTriangle(4, 6, 7, 1);
	AkPlacementNew(tri + 6) AkTriangle(7, 0, 6, 0);
	AkPlacementNew(tri + 7) AkTriangle(6, 0, 1, 0);
	geom.Triangles = tri;

	geom.EnableDiffraction = true;
	// We did not add triangles on the top and bottom (through screen) of our "wall", and set EnableDiffractionOnBoundaryEdges to false,
	// so that sound does not diffract via the top and bottom edges.
	geom.EnableDiffractionOnBoundaryEdges = false;	
	AK::SpatialAudio::SetGeometry(0, geom);

	// Horizontal wall
	//

	// Draw Line.
	m_geometry[1] = new Line(*this);
	int horizontal_wall_right_px = centerx - 60;
	m_geometry[1]->SetPoints(horizontal_wall_right_px, centery, centerx - (int)room0_width_px, centery);

	float horizontal_wall_right = -PixelsToAKLen_X(60.f);
	float horizontal_wall_left = -PixelsToAKLen_X(room0_width_px);

	geom.NumVertices = 8;
	// In the coordinate system we have chosen, x is right-left, z is top-bottom and y is through screen.
	/*
	AkVertex vertices2[] = {
		{ horizontal_wall_right, -30.f, 0 },
		{ horizontal_wall_right, 30.f, 0 },
		{ horizontal_wall_right, 30.f, -1.f },
		{ horizontal_wall_right, -30.f, -1.f },
		{ horizontal_wall_left, -30.f, -1.f },
		{ horizontal_wall_left, 30.f, -1.f },
		{ horizontal_wall_left, 30.f, 0 },
		{ horizontal_wall_left, -30.f, 0 }
	};
	*/
	// Init one by one for old compilers.
	AkVertex vertices2[8];
	AkPlacementNew(vertices2 + 0) AkVertex(horizontal_wall_right, -30.f, 0);
	AkPlacementNew(vertices2 + 1) AkVertex(horizontal_wall_right, 30.f, 0);
	AkPlacementNew(vertices2 + 2) AkVertex(horizontal_wall_right, 30.f, -1.f);
	AkPlacementNew(vertices2 + 3) AkVertex(horizontal_wall_right, -30.f, -1.f);
	AkPlacementNew(vertices2 + 4) AkVertex(horizontal_wall_left, -30.f, -1.f);
	AkPlacementNew(vertices2 + 5) AkVertex(horizontal_wall_left, 30.f, -1.f);
	AkPlacementNew(vertices2 + 6) AkVertex(horizontal_wall_left, 30.f, 0);
	AkPlacementNew(vertices2 + 7) AkVertex(horizontal_wall_left, -30.f, 0);
	geom.Vertices = vertices2;

	// Use same two surfaces as above (0 is outside)

	// Recall that we do not assign triangles on the left of the horizontal wall.
	geom.NumTriangles = 6;
	/*
	AkTriangle tri2[] = {
		{ 0, 1, 2, 0 },
		{ 0, 2, 3, 0 },
		{ 2, 3, 4, 0 },
		{ 2, 4, 5, 0 },
		{ 7, 0, 6, 1 },
		{ 6, 0, 1, 1 }
	};
	*/
	// Init one by one for old compilers.
	AkTriangle tri2[6];
	AkPlacementNew(tri2 + 0) AkTriangle(0, 1, 2, 0);
	AkPlacementNew(tri2 + 1) AkTriangle(0, 2, 3, 0);
	AkPlacementNew(tri2 + 2) AkTriangle(2, 3, 4, 0);
	AkPlacementNew(tri2 + 3) AkTriangle(2, 4, 5, 0);
	AkPlacementNew(tri2 + 4) AkTriangle(7, 0, 6, 1);
	AkPlacementNew(tri2 + 5) AkTriangle(6, 0, 1, 1);
	geom.Triangles = tri2;

	geom.EnableDiffraction = true;
	// We did not add triangles on the top and bottom (through screen) of our "wall", and set EnableDiffractionOnBoundaryEdges to false,
	// so that sound does not diffract via the top and bottom edges.
	geom.EnableDiffractionOnBoundaryEdges = false;
	AK::SpatialAudio::SetGeometry(1, geom);
}

void DemoSpatialAudioGeometry::InitControls()
{
	float fMargin = (float)m_pEmitterChip->GetRightBottomMargin();
	float x = (m_pParentMenu->GetWidth() - fMargin) * 1.f / 2.0f;
	float y = (m_pParentMenu->GetHeight() - fMargin) * 1.f / 2.0f;

	m_pEmitterChip = new MovableChip(*this);
	m_pEmitterChip->SetLabel( "<E>" );
	m_pEmitterChip->UseJoystick(UG_STICKRIGHT);
	m_pEmitterChip->SetPos(x - 100, y - 100);
	m_pEmitterChip->SetNonLinear();
	m_pEmitterChip->SetMaxSpeed(3.f);

	m_pListenerChip = new MovableChip(*this);
	m_pListenerChip->SetLabel("<L>");
	m_pListenerChip->UseJoystick(UG_STICKRIGHT);
	m_pListenerChip->SetNonLinear();
	m_pListenerChip->SetPos(x + 30, y + 30);
	m_pListenerChip->SetMaxSpeed(3.f);
	m_pListenerChip->Update(*(m_pParentMenu->Input()->Begin()));

	m_fWidth = (float)m_pParentMenu->GetWidth() - fMargin;
	m_fHeight = (float)m_pParentMenu->GetHeight() - fMargin;

	/*switch (m_eScenario)
	{
	case Scenario_Portals:
		InitControls_Portals();
		*/
	Init_Geometry();
	AK::SoundEngine::PostEvent("Play_Room_Emitter", EMITTER);
	/*
	break;
	}
	*/

	// Update movable chips the first time.
	UniversalInput::Iterator it;
	for (it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++)
	{
		// Skip this input device if it's not connected
		if (!it->IsConnected())
			continue;

		m_pEmitterChip->Update(*it);
		m_pListenerChip->Update(*it);
		UpdateGameObjPos(m_pEmitterChip, EMITTER);
		UpdateGameObjPos(m_pListenerChip, LISTENER);
	}
}
