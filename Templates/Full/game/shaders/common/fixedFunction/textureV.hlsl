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

#include "../shaderModel.hlsl"

struct Appdata
{
	float3 position   : POSITION;
<<<<<<< HEAD
	float2 texCoord   : TEXCOORD;
=======
	float4 color      : COLOR;
	float2 texCoord   : TEXCOORD0;
>>>>>>> caf5ad10dbe54848c139926b37e931ec05da0fac
};

struct Conn
{
	float4 hpos             : TORQUE_POSITION;
	float2 texCoord         : TEXCOORD0;
};

uniform float4x4 modelview;

Conn main( Appdata In )
{
	Conn Out;
	Out.hpos = mul(modelview, float4(In.position, 1.0));
	Out.texCoord = In.texCoord;
	return Out;
}