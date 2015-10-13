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

#include "terrain.hlsl"
#include "../shaderModel.hlsl"

struct ConnectData
{
   float4 hpos : TORQUE_POSITION;
   float2 layerCoord : TEXCOORD0;
   float2 texCoord : TEXCOORD1;
};

TORQUE_UNIFORM_SAMPLER2D(layerTex, 0);
TORQUE_UNIFORM_SAMPLER2D(textureMap, 1);

float4 main(   ConnectData IN,
               uniform float texId,
               uniform float layerSize) : TORQUE_TARGET0
{
   float4 layerSample = round( TORQUE_TEX2D( layerTex, IN.layerCoord ) * 255.0f );

   float blend = calcBlend( texId, IN.layerCoord, layerSize, layerSample );

   clip( blend - 0.0001 );

   return float4( TORQUE_TEX2D(textureMap, IN.texCoord).rgb, blend);
}
