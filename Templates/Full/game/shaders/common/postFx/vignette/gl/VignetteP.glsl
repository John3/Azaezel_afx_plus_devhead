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

#include "../../../gl/hlslCompat.glsl"
#include "shadergen:/autogenConditioners.h"

uniform sampler2D backBuffer;
uniform float radiusX;
uniform float radiusY;

in vec2 uv0;

out vec4 OUT_col;

void main()
{
   vec4 base = texture(backBuffer, uv0);
   
	vec4 sepia = base;
   if (mod(distance(uv0.y,0.5)*1024,4) < 1)
      sepia.rgb = vec3(0,0,0);
	
   float dist = distance(uv0, vec2(0.5,0.5));
   vec4 shift = vec4(1.0,1.0,1.0,1.0);
   shift.rgb *= smoothstep(radiusX, radiusY, dist);
   vec4 result = base;
   result.r = result.b;
   result.g = result.b;
   result = mix( sepia, result, clamp((abs(shift.r)-0.1)*4,0.0,1.0));
   base = mix(result,base,shift);
   
   OUT_col = base;
}