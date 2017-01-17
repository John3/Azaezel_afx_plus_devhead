//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "gui/controls/guiBitmapCtrl.h"
#include "console/consoleTypes.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/vehicles/vehicle.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"

//-----------------------------------------------------------------------------
/// A Speedometer control.
/// This gui displays the speed of the current Vehicle based
/// control object. This control only works if a server
/// connection exists and its control object is a vehicle. If
/// either of these requirements is false, the control is not rendered.
class GuiSpeedometerHud : public GuiBitmapCtrl
{
   typedef GuiBitmapCtrl Parent;

   F32   mSpeed;        ///< Current speed
   F32   mMaxSpeed;     ///< Max speed at max need pos
   F32   mMaxAngle;     ///< Max pos of needle
   F32   mMinAngle;     ///< Min pos of needle
   Point2F mCenter;     ///< Center of needle rotation
   ColorF mColor;       ///< Needle Color
   F32   mNeedleLength;
   F32   mNeedleWidth;
   F32   mTailLength;
   F32 mExplicitSpeed;

   GFXStateBlockRef mBlendSB;

   StringTableEntry  mNeedleBitmap;
   GFXTexHandle      mNeedleTextureHandle;

public:
   GuiSpeedometerHud();
   bool onWake();

   void setNeedleBitmap(const char *name);
   void draw2DSquare(const Point2F &screenPoint, F32 length, F32 width, GFXTextureObject* texture, F32 spinAngle = 0.0f);

   void onRender( Point2I, const RectI &);
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiSpeedometerHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Displays the speed of the current Vehicle-based control object." );
};

bool GuiSpeedometerHud::onWake()
{
   if (!Parent::onWake())
      return false;
   setActive(true);

   setNeedleBitmap(mNeedleBitmap);
   return true;
}

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiSpeedometerHud );

ConsoleDocClass( GuiSpeedometerHud,
   "@brief Displays the speed of the current Vehicle based control object.\n\n"

   "This control only works if a server connection exists, and its control "
   "object is a Vehicle derived class. If either of these requirements is false, "
   "the control is not rendered.<br>"

   "The control renders the speedometer needle as a colored quad, rotated to "
   "indicate the Vehicle speed as determined by the <i>minAngle</i>, "
   "<i>maxAngle</i>, and <i>maxSpeed</i> properties. This control is normally "
   "placed on top of a GuiBitmapCtrl representing the speedometer dial.\n\n"

   "@tsexample\n"
   "new GuiSpeedometerHud()\n"
   "{\n"
   "   maxSpeed = \"100\";\n"
   "   minAngle = \"215\";\n"
   "   maxAngle = \"0\";\n"
   "   color = \"1 0.3 0.3 1\";\n"
   "   center = \"130 123\";\n"
   "   length = \"100\";\n"
   "   width = \"2\";\n"
   "   tail = \"0\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiContainers"
);

GuiSpeedometerHud::GuiSpeedometerHud()
{
   mSpeed = 0;
   mMaxSpeed = 100;
   mMaxAngle = 0;
   mMinAngle = 90;
   mCenter.set(0,0);
   mNeedleWidth = 3;
   mNeedleLength = 10;
   mTailLength = 5;
   mColor.set(1,0,0,1);

   mNeedleBitmap = StringTable->insert("art/gui/playgui/needle.png");
   mNeedleTextureHandle = NULL;

   mExplicitSpeed = -1;
}

void GuiSpeedometerHud::initPersistFields()
{
   addGroup("Needle");

   addField("maxSpeed", TypeF32, Offset( mMaxSpeed, GuiSpeedometerHud ),
      "Maximum Vehicle speed (in Torque units per second) to represent on the "
      "speedo (Vehicle speeds greater than this are clamped to maxSpeed)." );

   addField("minAngle", TypeF32, Offset( mMinAngle, GuiSpeedometerHud ),
      "Angle (in radians) of the needle when the Vehicle speed is 0. An angle "
      "of 0 points right, 90 points up etc)." );

   addField("maxAngle", TypeF32, Offset( mMaxAngle, GuiSpeedometerHud ),
      "Angle (in radians) of the needle when the Vehicle speed is >= maxSpeed. "
      "An angle of 0 points right, 90 points up etc)." );

   addField("color", TypeColorF, Offset( mColor, GuiSpeedometerHud ),
      "Color of the needle" );

   addField("center", TypePoint2F, Offset( mCenter, GuiSpeedometerHud ),
      "Center of the needle, offset from the GuiSpeedometerHud control top "
      "left corner" );

   addField("length", TypeF32, Offset( mNeedleLength, GuiSpeedometerHud ),
      "Length of the needle from center to end" );

   addField("width", TypeF32, Offset( mNeedleWidth, GuiSpeedometerHud ),
      "Width of the needle" );

   addField("tail", TypeF32, Offset( mTailLength, GuiSpeedometerHud ),
      "Length of the needle from center to tail" );

   addField("NeedleBitmap", TypeFilename, Offset(mNeedleBitmap, GuiSpeedometerHud));

   addField("ExplicitSpeed", TypeF32, Offset(mExplicitSpeed, GuiSpeedometerHud),
      "forced speed value");
   endGroup("Needle");

   Parent::initPersistFields();
}

void GuiSpeedometerHud::draw2DSquare(const Point2F &screenPoint, F32 length, F32 width, GFXTextureObject* texture, F32 spinAngle)
{
   if (texture == NULL)
      return;

   width *= 0.5;

   Point3F offset(screenPoint.x, screenPoint.y, 0.0);

   GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, 4, GFXBufferTypeVolatile);
   verts.lock();

   verts[0].point.set(-width, -length, 0.0f);
   verts[1].point.set(-width, length, 0.0f);
   verts[2].point.set(width, -length, 0.0f);
   verts[3].point.set(width, length, 0.0f);
   
   GFXVertexColor mBitmapModulation;
   mBitmapModulation.set(0xFF, 0xFF, 0xFF, 0xFF);
   verts[0].color = verts[1].color = verts[2].color = verts[3].color = mBitmapModulation;

   if (spinAngle == 0.0f)
   {
      for (S32 i = 0; i < 4; i++)
         verts[i].point += offset;

      verts[0].texCoord.set(0, 0);
      verts[1].texCoord.set(1, 0);
      verts[2].texCoord.set(0, 1);
      verts[3].texCoord.set(1, 1);
   }
   else
   {
      MatrixF rotMatrix(EulerF(0.0, 0.0, spinAngle));

      for (S32 i = 0; i < 4; i++)
      {
         rotMatrix.mulP(verts[i].point);
         verts[i].point += offset;
      }

      verts[0].texCoord.set(0, 0);
      verts[1].texCoord.set(1, 0);
      verts[2].texCoord.set(0, 1);
      verts[3].texCoord.set(1, 1);
   }

   verts.unlock();
   GFX->setVertexBuffer(verts);

   GFXStateBlockDesc rectFill;
   rectFill.setCullMode(GFXCullNone);
   rectFill.setZReadWrite(false);
   rectFill.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   GFXStateBlockRef mRectFillSB = GFX->createStateBlock(rectFill);   
   GFX->setStateBlock(mRectFillSB);
   
   GFX->setTexture(0, texture);
   GFX->setupGenericShaders(GFXDevice::GSModColorTexture);
   GFX->drawPrimitive(GFXTriangleStrip, 0, 2);
}

//---------------------------------------------------------------------------
void GuiSpeedometerHud::setNeedleBitmap(const char *name)
{
   mNeedleBitmap = StringTable->insert(name);

   if (*mNeedleBitmap)
      mNeedleTextureHandle = GFXTexHandle(mNeedleBitmap, &GFXDefaultStaticDiffuseProfile, "Adescription");
   else
      // Reset handles if UI object is hidden
      mNeedleTextureHandle = NULL;

   setUpdate();
}

//------------------------------------------------------------------------
// script interface
ConsoleMethod(GuiSpeedometerHud, setNeedleBitmap, void, 3, 3, "(string filename)"
   "Set the bitmap for the needle.")
{
   object->setNeedleBitmap(argv[2]);
}


//-----------------------------------------------------------------------------
/**
   Gui onRender method.
   Renders a health bar with filled background and border.
*/
void GuiSpeedometerHud::onRender(Point2I offset, const RectI &updateRect)
{
   if (mExplicitSpeed == -1)
   {
      // Must have a connection and player control object
      GameConnection* conn = GameConnection::getConnectionToServer();
      if (!conn)
         return;
      ShapeBase* control = dynamic_cast<ShapeBase*>(conn->getControlObject());
      if (!control)
         return;
      // Use the vehicle's velocity as its speed...
      mSpeed = control->getVelocity().len();
   }
   else
      mSpeed = mExplicitSpeed;

   Parent::onRender(offset,updateRect);

   if (mSpeed > mMaxSpeed)
      mSpeed = mMaxSpeed;

   // Render the needle
   GFX->pushWorldMatrix();
   Point2F center = mCenter;
   if (mIsZero(center.x) && mIsZero(center.y))
   {
      center.x = getExtent().x / 2.0f;
      center.y = getExtent().y / 2.0f;
   }
   center.x += offset.x;
   center.y += offset.y;
   MatrixF newMat(1);

   newMat.setPosition(Point3F(getLeft() + center.x, getTop() + center.y, 0.0f));

   F32 rotation = mMinAngle + (mMaxAngle - mMinAngle) * (mSpeed / mMaxSpeed);
   AngAxisF newRot(Point3F(0.0f,0.0f,-1.0f), rotation);

   newRot.setMatrix(&newMat);

   if (mBlendSB.isNull())
   {
      GFXStateBlockDesc desc;
      desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
      desc.samplersDefined = true;
      desc.samplers[0].textureColorOp = GFXTOPDisable;
      mBlendSB = GFX->createStateBlock(desc);
   }

   draw2DSquare(center, mNeedleLength, mNeedleWidth, mNeedleTextureHandle, mDegToRad(rotation));

   GFX->popWorldMatrix();
}

