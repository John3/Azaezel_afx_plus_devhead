#ifndef AmbientLightProbe_H
#define AmbientLightProbe_H

#ifndef _SCENEPOLYHEDRALSPACE_H_
#include "scene/scenePolyhedralSpace.h"
#endif

#ifndef _MSILHOUETTEEXTRACTOR_H_
#include "math/mSilhouetteExtractor.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _POSTEFFECTCOMMON_H_
#include "postFx/postEffectCommon.h"
#endif

#include "materials/matTextureTarget.h"
#include "renderInstance/renderBinManager.h"
#include "core/resourceManager.h"

struct CameraQuery;
class CubemapData;

/// A volume in space that blocks visibility.
class AmbientLightProbe : public ScenePolyhedralSpace
{
public:
   typedef ScenePolyhedralSpace Parent;

   bool mOverrideSkyColor;
   ColorF mSkyColor;
   ColorF mGroundColor;
   F32 mIntensity;

   String mCubemapName;
   CubemapData *mCubemap;
   bool mUseCubemap;

protected:
   // We sample from prepass and render to light map buffer.
   NamedTexTarget*         mPrepassTarget;
   NamedTexTarget*         mIndirectLightingTarget;
   NamedTexTarget*         mMatInfoTarget;
   GFXTextureTargetRef     mRenderTarget;

   // Stateblock for shaders
   GFXStateBlockRef        mStateBlock;

   // Propagated Shader
   GFXShaderRef            mProbeShader;
   GFXShaderConstBufferRef mProbeShaderConsts;
   GFXShaderConstHandle    *mEyePosWorldPropSC;
   GFXShaderConstHandle    *mRTParamsPropSC;
   GFXShaderConstHandle    *mVolumeStartPropSC;
   GFXShaderConstHandle    *mVolumeSizePropSC;

   GFXShaderConstHandle    *mInverseViewSC;

   //
   GFXShaderConstHandle    *mSkyColorSC;
   GFXShaderConstHandle    *mGroundColorSC;
   GFXShaderConstHandle    *mIntensitySC;
   GFXShaderConstHandle    *mUseCubemapSC;
   GFXShaderConstHandle    *mCubemapSC;

   void     _initShaders();

   void _handleBinEvent(RenderBinManager *bin, const SceneRenderState* sceneState, bool isBinStart);
   void _updateScreenGeometry(const Frustum &frustum, GFXVertexBufferHandle<PFXVertex> *outVB);

   // World Editor Visualization.
   typedef SilhouetteExtractorPerspective< PolyhedronType > SilhouetteExtractorType;
   bool mTransformDirty;
   Vector< Point3F > mWSPoints;
   SilhouetteExtractorType mSilhouetteExtractor;

   virtual void _renderObject(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat);

public:

   AmbientLightProbe();
   ~AmbientLightProbe();

   // SimObject.
   DECLARE_CONOBJECT(AmbientLightProbe);
   DECLARE_DESCRIPTION("Ambient Light Probe");
   DECLARE_CATEGORY("3D Scene");

   virtual bool onAdd();
   virtual void onRemove();
   void inspectPostApply();

   // Network
   U32  packUpdate(NetConnection *, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *, BitStream *stream);

   // SceneObject.
   virtual void buildSilhouette(const SceneCameraState& cameraState, Vector< Point3F >& outPoints);
   virtual void setTransform(const MatrixF& mat);

   // Static Functions.
   static void consoleInit();
   static void initPersistFields();
};

#endif // AmbientLightProbe_H