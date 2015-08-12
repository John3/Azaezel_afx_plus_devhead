#ifndef _LASER_H_
#define _LASER_H_

#ifndef _PROJECTILE_H_  
    #include "T3D/projectile.h"  
    #endif  
      
    #include "console/consoleTypes.h"  
    #include "core/stream/bitStream.h"  
    #include "scene/sceneManager.h"
    #include "T3D/shapeBase.h"  
    #include "ts/tsShapeInstance.h"  
    #include "gfx/primBuilder.h"  
    #include "gfx/gfxDevice.h"  
    #include "gfx/gfxTransformSaver.h"  
    #include "math/mRandom.h"  
    #include "math/mathIO.h"  
    #include "math/mathUtils.h"  
    #include "renderInstance/renderPassManager.h"  
      
    class ExplosionData;  
    class ShapeBase;  
    class TSShapeInstance;  
    class TSThread;  
      
    class LaserData : public ProjectileData  
    {  
        typedef ProjectileData Parent;  
            
    public:  
      
        F32 beamStartRadius;  
        F32 beamEndRadius;
        F32 mScrollSpeed;
        StringTableEntry  textureName;  
      
        LaserData(); 

        void packData(BitStream*);  
        void unpackData(BitStream*);
        bool preload(bool server, String &errorStr);
      
        static void initPersistFields();  
        DECLARE_CONOBJECT(LaserData);  
    };      
      
    class Laser : public Projectile  
    {  
        typedef Projectile Parent;  
        LaserData* mDataBlock;  
      
    protected:  
        GFXTexHandle      textureHandle;  
        MaterialList  mMaterialList;  
        S32           mNumTextures;  
        S32           mIndex;  
        S32           mLastTime;
        F32           mRange;
        F32           mUVOffset;
		S32			  lifetime;
        void loadDml();  
      
        bool onAdd();  
        bool onNewDataBlock(GameBaseData*, bool reload );  
        void processTick(const Move* move);  
        U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);  
        void unpackUpdate(NetConnection *conn,           BitStream *stream);  
        void prepModelView(SceneRenderState* state);  
      
        // Rendering  
        void prepRenderImage( SceneRenderState *state );
        void render( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *mi );  
      
        GFXStateBlockRef  mBlendInvSrcAlphaSB;
    public:  
      
        static void initPersistFields(); 
        Laser();  
        ~Laser();  
      
        DECLARE_CONOBJECT(Laser);  
    };  
      
    IMPLEMENT_CO_DATABLOCK_V1(LaserData);  
    IMPLEMENT_CO_NETOBJECT_V1(Laser);
    #endif _LASER_H_