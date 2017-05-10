//-----------------------------------------------------------------------------
// Torque 3D
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

$WetnessPostFx::splashRandom = 1;
$WetnessPostFx::splashAlpha = 1;

function WetnessPostFx::setShaderConsts( %this )
{
   if($WetnessPostFx::splashAlpha <= 0)
   {
      %rand = getRandom(0, 1);
      if(%rand == true)
         $WetnessPostFx::splashAlpha = 1;
   }else{
      $WetnessPostFx::splashAlpha -= 0.1;
   }
   
   %rand = getRandom(0, 3);
   if($WetnessPostFx::splashRandom == %rand)
   {
      if($WetnessPostFx::splashRandom == 3)
         $WetnessPostFx::splashRandom--;
      else
         $WetnessPostFx::splashRandom++;
   }else{
      $WetnessPostFx::splashRandom = %rand;
   }
   
   if (isObject(%this-->rainsplash))
   {
      %this-->rainsplash.setShaderConst( "$splashRandom", $WetnessPostFx::splashRandom );
      %this-->rainsplash.setShaderConst( "$splashAlpha", $WetnessPostFx::splashAlpha );
   }
}

//-----------------------------------------------------------------------------
// GFXStateBlockData / ShaderData
//-----------------------------------------------------------------------------

singleton GFXStateBlockData( WetnessStateBlock : PFX_DefaultStateBlock )
{   
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampLinear;
   samplerStates[2] = SamplerWrapLinear;
   samplerStates[3] = SamplerWrapLinear;
   
   zDefined = true; //Set to true if the depth state is not all defaults.
   zEnable = true; //Enables z-buffer reads. The default is true.
   zWriteEnable = true; //Enables z-buffer writes. The default is true.
   
   blendDefined = true;
   blendEnable = true; 
   blendSrc = GFXBlendOne;
   blendDest = GFXBlendOne;
};

singleton GFXStateBlockData( WetnessLayerStateBlock : PFX_DefaultStateBlock )
{   
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampPoint;  // $reflectbuff
   samplerStates[2] = SamplerClampPoint;  // $backbuff
   samplerStates[3] = SamplerWrapLinear;  // $texture
   
   cullDefined = true;
   cullMode = "GFXCullNone";
   
   zDefined = true; //Set to true if the depth state is not all defaults.
   zEnable = true; //Enables z-buffer reads. The default is true.
   zWriteEnable = true; //Enables z-buffer writes. The default is true.
   
   blendDefined = true;
   blendEnable = true; 
   blendSrc = GFXBlendOne;
   blendDest = GFXBlendOne;
};

singleton ShaderData( WetnessShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/wetness/wet_surfP.hlsl";  
   
   samplerNames[0] = "deferredTex";
   samplerNames[1] = "directLightingBuffer";
   samplerNames[2] = "$wetMap";
             
   pixVersion = 3.0;
};

singleton ShaderData( WetnessRefractShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/wetness/wet_lightrefractV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/wetness/wet_lightrefractP.hlsl";  
   
   samplerNames[0] = "deferredTex";
   samplerNames[1] = "directLightingBuffer";
   samplerNames[2] = "$wetMap";
   samplerNames[3] = "$backbuffer";
             
   pixVersion = 3.0;
};

singleton ShaderData( WetnessRainFallShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/wetness/wet_rainfallP.hlsl";  
   
   samplerNames[0] = "deferredTex";
   samplerNames[1] = "directLightingBuffer";
   samplerNames[2] = "$rainfall";
             
   pixVersion = 3.0;
};


singleton ShaderData( WetnessRainSplashShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/wetness/wet_splashP.hlsl";  
   
   samplerNames[0] = "deferredTex";
   samplerNames[1] = "directLightingBuffer";
   samplerNames[2] = "$splashNormal";
             
   pixVersion = 3.0;
};

singleton ShaderData( cloneShader )
{   
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/cloneP.hlsl"; 
   
   samplerNames[0] = "inputTex";
             
   pixVersion = 3.0;
};

//-----------------------------------------------------------------------------
// PostEffects
//-----------------------------------------------------------------------------
singleton PostEffect( DryPostFX )
{ 
   isEnabled = false;
   
   renderTime = "PFXBeforeBin";
   renderBin = "ProbeBin";
   target = "#matres";
   renderPriority = 9999;
   
   shader = cloneShader;   

   texture[0] = "#matinfo";
};

singleton PostEffect( WetnessPostFX )
{     
   isEnabled = false;
     
   renderTime = "PFXBeforeBin";
   renderBin = "ProbeBin";
   target = "#matres";
   renderPriority = 9999;
   
   shader = WetnessShader;
   stateBlock = WetnessStateBlock;
   
   texture[0] = "#deferred";
   texture[1] = "#directLighting";
   texture[2] = "./wetMap.png";
   texture[3] = "#color";
   
   singleton PostEffect()
   {
      internalName = "refract";
      renderTime = "PFXBeforeBin";
      renderBin = "FinalBin";
      
      shader = WetnessRefractShader;
      stateBlock = WetnessStateBlock;
      
      texture[0] = "#deferred";
      texture[1] = "#directLighting";
      texture[2] = "./wetMap.png";
      texture[3] = "$backbuffer";
   };
   
   singleton PostEffect()
   {
      internalName = "rainfall";
      renderTime = "PFXBeforeBin";
      renderBin = "FinalBin";
      
      shader = WetnessRainFallShader;
      stateBlock = WetnessStateBlock;
      
      texture[0] = "#deferred";
      texture[1] = "#directLighting";
      texture[2] = "./rainfall.png";
   };
   
   singleton PostEffect()
   {
      internalName = "rainsplash";
      renderTime = "PFXBeforeBin";
      renderBin = "FinalBin";
      
      shader = WetnessRainSplashShader;
      stateBlock = WetnessStateBlock;
      
      texture[0] = "#deferred";
      texture[1] = "#directLighting";
      texture[2] = "./splashNormal.png";  
   };
};