//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _GUIRADARTSCTRL_H_
#define _GUIRADARTSCTRL_H_

#ifndef _GUITSCONTROL_H_
#include "gui/3d/guiTSControl.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

//----------------------------------------------------------------------------
class GuiRadarTSCtrl : public GuiTSCtrl
{
private:
	typedef GuiTSCtrl Parent;

	StringTableEntry  mEllipticBitmap;           // path to the bitmap for the horizontal plane
	GFXTexHandle      mEllipticTextureHandle;    // texture handle for the horizontal plane

	StringTableEntry mPlayerBitmap;
	GFXTexHandle mPlayerTextureHandle;

	StringTableEntry mBotBitmap;
	GFXTexHandle mBotTextureHandle;

	StringTableEntry mVehicleBitmap;
	GFXTexHandle mVehicleTextureHandle;

	S32			mSphereDetail;

	ColorF   mEllipticColor;   // color to use when drawing the elliptic plane
	ColorF   mSphereColor;     // color of the sphere
	ColorF   mBlipColor;       // color for the radar 'blips'
	ColorF   mStemColor;       // color for the stems of the blips
	F32      mRange;           // range of the radar
	F32      mBlipSize;        // base size for the blips

	bool mShowVehicles;
	bool mShowPlayers;
	bool mShowBots;

	char		mTargetShapeName[32]; //This keeps the name of the locked target

public:
	GuiRadarTSCtrl();

	bool processCameraQuery(CameraQuery *query);
	void renderWorld(const RectI &updateRect);

	static void initPersistFields();
	bool onWake();
	void onSleep();

	// gets and sets
	void setEllipticBitmap(const char *name);
	void setBlipBitmap(const char *name);
	void setRange(F32 range) { mRange = range; }
	F32 getRange() { return mRange; }
	void setEllipticColor(ColorF color) { mEllipticColor = color; }
	void setSphereColor(ColorF color) { mSphereColor = color; }
	void setBlipColor(ColorF color) { mBlipColor = color; }
	void setStemColor(ColorF color) { mStemColor = color; }

	void setPlayerBitmap(const char *name);
	void setBotBitmap(const char *name);
	void setVehicleBitmap(const char *name);

	void setTarget(const char *TargetShapeBaseName);

	DECLARE_CONOBJECT(GuiRadarTSCtrl);
};

#endif
