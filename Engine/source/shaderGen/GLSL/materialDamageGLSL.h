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
#ifndef _SHADERGEN_GLSL_DAMAGEFEATUREGLSL_H_
#define _SHADERGEN_GLSL_DAMAGEFEATUREGLSL_H_

#ifndef _SHADERGEN_GLSL_SHADERFEATUREGLSL_H_
#include "shaderGen/GLSL/shaderFeatureGLSL.h"
#endif

/// Damage texture - Albedo
class AlbedoDamageFeatGLSL : public ShaderFeatureGLSL
{
public:
   virtual void processVert(Vector<ShaderComponent*> &componentList,
      const MaterialFeatureData &fd);

   virtual void processPix(Vector<ShaderComponent*> &componentList,
      const MaterialFeatureData &fd);
   
   virtual U32 getOutputTargets(const MaterialFeatureData &fd) const;
   virtual Material::BlendOp getBlendOp(){ return Material::LerpAlpha; }

   virtual Resources getResources(const MaterialFeatureData &fd);

   // Sets textures and texture flags for current pass
   virtual void setTexData(Material::StageData &stageDat,
      const MaterialFeatureData &fd,
      RenderPassData &passData,
      U32 &texIndex);

   virtual String getName()
   {
      return "Damage Albedo Texture";
   }
};

/// Damage texture - Composite
class CompositeDamageFeatGLSL : public ShaderFeatureGLSL
{
public:
   virtual void processVert(Vector<ShaderComponent*> &componentList,
      const MaterialFeatureData &fd);

   virtual void processPix(Vector<ShaderComponent*> &componentList,
      const MaterialFeatureData &fd);

   virtual Material::BlendOp getBlendOp(){ return Material::LerpAlpha; }

   virtual Resources getResources(const MaterialFeatureData &fd);

   // Sets textures and texture flags for current pass
   virtual void setTexData(Material::StageData &stageDat,
      const MaterialFeatureData &fd,
      RenderPassData &passData,
      U32 &texIndex);

   virtual String getName()
   {
      return "Damage Composite Texture";
   }
};

#endif