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

#include "../../ShaderModelAutoGen.hlsl"
#include "../postFx.hlsl"

TORQUE_UNIFORM_SAMPLER2D(backBuffer,0);
uniform float radiusX;
uniform float radiusY;

float4 main(PFXVertToPix IN) : TORQUE_TARGET0
{
   float4 base = TORQUE_TEX2D(backBuffer, IN.uv0);   
	  
	float4 sepia = base;
   if (fmod(distance(IN.uv0.y,0.5)*1024,4) < 1)
      sepia.rgb = float3(0,0,0);
	
   float dist = distance(IN.uv0, float2(0.5,0.5));
   float4 shift = float4(1.0,1.0,1.0,1.0);
   shift.rgb *= smoothstep(radiusX, radiusY, dist);
   float4 result = base;
   result.r = result.b;
   result.g = result.b;
   result = lerp( sepia, result, saturate((abs(shift.r)-0.1)));
   base = lerp(result,base,shift);
   return base;
}