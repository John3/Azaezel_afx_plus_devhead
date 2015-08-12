#include "laser.h"
#include "core/stream/fileStream.h"

LaserData::LaserData() 
{
    ProjectileData::ProjectileData();
    beamStartRadius = 0.0;
    beamEndRadius = 0.0;
    interval = 0;
    textureName = NULL;          // texture filename
    mScrollSpeed = 0.05f;
}

bool LaserData::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;
      
   return true;
}

void LaserData::initPersistFields() 
{
    Parent::initPersistFields();
    addNamedField(beamStartRadius, TypeF32, LaserData);
    addNamedField(beamEndRadius, TypeF32, LaserData);
    addField( "textureName", TYPEID< StringTableEntry >(), Offset(textureName, LaserData), "Texture file to use for this particle." );
    addField( "scrollSpeed", TypeF32, Offset(mScrollSpeed, LaserData));
} 

void LaserData::packData(BitStream* stream)
{
    Parent::packData(stream);
    stream->writeFloat(beamStartRadius/20.0, 8);
    stream->writeFloat(beamEndRadius/20.0, 8);
    if (stream->writeFlag(textureName && textureName[0]))
        stream->writeString(textureName);
    stream->write(mScrollSpeed);
}

void LaserData::unpackData(BitStream* stream)
{
    Parent::unpackData(stream);
    beamStartRadius = stream->readFloat(8) * 20;
    beamEndRadius = stream->readFloat(8) * 20;
    textureName = (stream->readFlag()) ? stream->readSTString() : 0;
    stream->read(&mScrollSpeed);
}

Laser::Laser()
{
    mLastTime = 0;
    mIndex = 0;
    mRange = 0;
    textureHandle = NULL;        // loaded texture handle
    mUVOffset = 0;
}

Laser::~Laser()
{
} 

void Laser::initPersistFields()
{
    Parent::initPersistFields();
    addField("range", TypeF32, Offset(mRange, Laser));
	addField("lifetime", TypeS32, Offset(lifetime, Laser), 
      "@brief Amount of time, in milliseconds, before the projectile is removed from the simulation.\n\n"
      "Used with fadeDelay to determine the transparency of the projectile at a given time. "
      "A projectile may exist up to a maximum of 131040ms (or 4095 ticks) as defined by Projectile::MaxLivingTicks in the source code."
      "@see fadeDelay");
}

U32 Laser::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
    U32 retMask = Parent::packUpdate(con, mask, stream);
    if (stream->writeFlag(mask & GameBase::InitialUpdateMask))
    {
        // Initial update
        stream->write(mRange);
    }
	stream->write(lifetime);
    return retMask;
}

void Laser::unpackUpdate(NetConnection* con, BitStream* stream)
{
    Parent::unpackUpdate(con, stream);
    if (stream->readFlag())
    {
        // initial update
        stream->read(&mRange);
    }
   stream->read(&lifetime);
}

void Laser::loadDml()
{
    // Here we attempt to load the particle's texture if specified. An undefined
    // texture is *not* an error since the emitter may provide one.
    if (mDataBlock->textureName && mDataBlock->textureName[0])
    {
        textureHandle = GFXTexHandle(mDataBlock->textureName, &GFXDefaultStaticDiffuseProfile, avar("%s() - textureHandle (line %d)", __FUNCTION__, __LINE__));
    }
}

bool Laser::onAdd()
{
    if(!Parent::onAdd())
        return false;
    if (!isServerObject())
    {
        if(mDataBlock->textureName[0])
            loadDml();
    }
    return true;
}

bool Laser::onNewDataBlock(GameBaseData* dptr, bool reload)
{
    mDataBlock = dynamic_cast<LaserData*>(dptr);
    if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
        return false;
    return true;
}

void Laser::processTick(const Move* move)  
{
    mCurrTick++;
	if (isServerObject() && mCurrTick >= lifetime)
    {
        deleteObject();
        return; 
    }
	if(mSourceObject.isValid()) 
    { 
        Point3F newPosition;
        Point3F mMuzzlePosition;

        MatrixF muzzleTrans;
        mSourceObject->getMuzzleTransform(mSourceObjectSlot, &muzzleTrans);
        muzzleTrans.getColumn(3, &mMuzzlePosition);  
        if (mMuzzlePosition == mSourceObject->getPosition()) mSourceObject->getWorldBox().getCenter( &mMuzzlePosition );
        Point3F mMuzzleVector;  
        mSourceObject->getMuzzleVector(mSourceObjectSlot, &mMuzzleVector); 
        newPosition = mMuzzleVector * mRange;  
        newPosition += mMuzzlePosition;  
      
        mSourceObject->disableCollision();  
      
        RayInfo rInfo;  
        if (getContainer()->castRay( mMuzzlePosition, newPosition, csmDynamicCollisionMask | csmStaticCollisionMask, &rInfo) == true)
        {
            // Next order of business: do we explode on this hit?  
            if (mCurrTick > mDataBlock->armingDelay)
            {
				MatrixF xform(true);  
				xform.setColumn(3, rInfo.point);  
				setTransform(xform); 
				newPosition    = rInfo.point; 
				mSourceObject->enableCollision(); 
            if (mDamageCycle == 0) onCollision(rInfo.point, rInfo.normal, rInfo.object);
				mDamageCycle++;
				if (mDamageCycle > mDataBlock->interval) mDamageCycle = 0;
				mSourceObject->disableCollision();
			}
        }
        mSourceObject->enableCollision();  
      
        if(isClientObject())  
        {
            emitParticles(mCurrPosition, newPosition, Point3F(1, 1, 1), TickMs * 4);
            updateSound();  
        }  
      
        mCurrDeltaBase = newPosition;  
        mCurrBackDelta = mCurrPosition - newPosition;  
        mCurrPosition = newPosition;  
      
        MatrixF xform(true);  
        xform.setColumn(3, mCurrPosition);  
        setTransform(xform);
    }
}

void Laser::prepRenderImage( SceneRenderState *state ) 
{
    ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
    ri->renderDelegate.bind( this, &Laser::render );
    ri->type = RenderPassManager::RIT_ObjectTranslucent;
    ri->translucentSort = true;
    ri->defaultKey = 1;
    state->getRenderPass()->addInst(ri);
} 

void Laser::prepModelView(SceneRenderState* state)  
{  
    MatrixF rotMatrix( true );  
    Point3F targetVector;  
      
    targetVector = getPosition() - state->getCameraPosition();  
    targetVector.normalize();  
      
    MatrixF explOrient = MathUtils::createOrientFromDir( targetVector );  
    explOrient.mul( rotMatrix );  
    explOrient.setPosition( getPosition() );  
      
    explOrient.scale( mDataBlock->scale );  
    GFX->setWorldMatrix( explOrient );
}

void Laser::render( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *mi )  
{  
    MatrixF projection = GFX->getProjectionMatrix();  
    RectI viewport = GFX->getViewport();  
      
    GFX->pushWorldMatrix();  
    prepModelView( state ); 

    if(mProjectileShape)  
    {
        AssertFatal(mProjectileShape != NULL,  
        "Laser::renderObject: Error, projectile shape should always be present in renderObject");  
        mProjectileShape->getCurrentDetail();  
        mProjectileShape->animate();  
      
        Point3F cameraOffset;  
        mObjToWorld.getColumn(3,&cameraOffset);  
        cameraOffset -= state->getCameraPosition();  
        // Set up our TS render state.
        TSRenderState rdata;
        rdata.setSceneState( state );
        mProjectileShape->render( rdata );  
    }  
    GFX->popWorldMatrix();  
      
    GFX->setViewport(viewport);  
    GFX->setProjectionMatrix(projection);  
      
    S32 thisTime = Sim::getCurrentTime();  
    S32 timeDelta = thisTime - mLastTime;  
    if (timeDelta > mDataBlock->interval)  
    {
        mIndex++;  
        if (mIndex >= mNumTextures)
            mIndex = 0;
    }
    
    mUVOffset -= mDataBlock->mScrollSpeed;
    if (mUVOffset < -1.0) mUVOffset = 0;
      
    Point3F mMuzzlePosition;  
    Point3F mMuzzleVector;  
      
    if (bool(mSourceObject))  
    {
        MatrixF muzzleTrans;  
        mSourceObject->getRenderMuzzleTransform(mSourceObjectSlot, &muzzleTrans);  
        muzzleTrans.getColumn(3, &mMuzzlePosition);
		if (mMuzzlePosition == mSourceObject->getPosition()) mSourceObject->getRenderWorldBox().getCenter( &mMuzzlePosition );
        mSourceObject->getMuzzleVector(mSourceObjectSlot, &mMuzzleVector);
    }
    // Sang -> 'billboarding' calculations  
    Point3F clientView = state->getCameraPosition();  
    Point3F clientVec = clientView - mMuzzlePosition;  
    clientVec.normalize();  
      
    Point3F crossA, crossB;  
      
    mCross(clientVec, mMuzzleVector, &crossA);  
    mCross(mMuzzleVector, clientVec, &crossB);  
    crossA.normalize();  
    crossB.normalize();  
      
    Point3F sPt1, sPt2, ePt1, ePt2;  
      
    sPt1 = (crossA * mDataBlock->beamStartRadius) + mMuzzlePosition;  
    sPt2 = (crossB * mDataBlock->beamStartRadius) + mMuzzlePosition;  
    ePt1 = (crossA * mDataBlock->beamEndRadius) + mCurrPosition;  
    ePt2 = (crossB * mDataBlock->beamEndRadius) + mCurrPosition;

    Point3F distvec = mMuzzlePosition - mCurrPosition;

    // set up the graphics  
    if (mBlendInvSrcAlphaSB.isNull())
    {
       GFXStateBlockDesc desc;
       desc.zEnable = false;
       desc.ffLighting = false;
       desc.setCullMode(GFXCullNone);
       desc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
       desc.samplers[0].textureColorOp = GFXTOPModulate;
       desc.samplers[1].textureColorOp = GFXTOPDisable;
       desc.samplersDefined = true;
       mBlendInvSrcAlphaSB = GFX->createStateBlock(desc);
    }
    GFX->setStateBlock(mBlendInvSrcAlphaSB);
      
    // Select the objects' texture.  
    GFX->setTexture(0, textureHandle);  
      
    // Set Colour/Alpha.  
    PrimBuild::color4f(1,1,1,1);  
      
    PrimBuild::begin(GFXTriangleStrip, 4);
    PrimBuild::texCoord2f(0, mUVOffset);  
    PrimBuild::vertex3f(sPt1.x, sPt1.y, sPt1.z);
    PrimBuild::texCoord2f(0, mUVOffset+distvec.len());
    PrimBuild::vertex3f(ePt1.x, ePt1.y, ePt1.z);  
    PrimBuild::texCoord2f(1, mUVOffset);
    PrimBuild::vertex3f(sPt2.x, sPt2.y, sPt2.z);  
    PrimBuild::texCoord2f(1, mUVOffset+distvec.len());
    PrimBuild::vertex3f(ePt2.x, ePt2.y, ePt2.z);  
    PrimBuild::end();
}  