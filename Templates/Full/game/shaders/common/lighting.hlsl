//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
// Portions Copyright Zefiros
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

#include "./torque.hlsl"

#ifndef TORQUE_SHADERGEN

// These are the uniforms used by most lighting shaders.

uniform float4 inLightPos[3];
uniform float4 inLightInvRadiusSq;
uniform float4 inLightColor[4];

#ifndef TORQUE_BL_NOSPOTLIGHT
   uniform float4 inLightSpotDir[3];
   uniform float4 inLightSpotAngle;
   uniform float4 inLightSpotFalloff;
#endif

uniform float4 ambient;
#define ambientCameraFactor 0.3
uniform float specularPower;
uniform float4 specularColor;

#endif // !TORQUE_SHADERGEN


void compute4Lights( float3 wsView, 
                     float3 wsPosition, 
                     float3 wsNormal,
                     float4 shadowMask,

                     #ifdef TORQUE_SHADERGEN
                     
                        float4 inLightPos[3],
                        float4 inLightInvRadiusSq,
                        float4 inLightColor[4],
                        float4 inLightSpotDir[3],
                        float4 inLightSpotAngle,
                        float4 inLightSpotFalloff,
                        float specularPower,
                        float4 specularColor,

                     #endif // TORQUE_SHADERGEN
                     
                     out float4 outDiffuse,
                     out float4 outSpecular )
{
   // NOTE: The light positions and spotlight directions
   // are stored in SoA order, so inLightPos[0] is the
   // x coord for all 4 lights... inLightPos[1] is y... etc.
   //
   // This is the key to fully utilizing the vector units and
   // saving a huge amount of instructions.
   //
   // For example this change saved more than 10 instructions 
   // over a simple for loop for each light.
   
   int i;

   float4 lightVectors[3];
   for ( i = 0; i < 3; i++ )
      lightVectors[i] = wsPosition[i] - inLightPos[i];

   float4 squareDists = 0;
   for ( i = 0; i < 3; i++ )
      squareDists += lightVectors[i] * lightVectors[i];

   // Accumulate the dot product between the light 
   // vector and the normal.
   //
   // The normal is negated because it faces away from
   // the surface and the light faces towards the
   // surface... this keeps us from needing to flip
   // the light vector direction which complicates
   // the spot light calculations.
   //
   // We normalize the result a little later.
   //
   float4 nDotL = 0;
   for ( i = 0; i < 3; i++ )
      nDotL += lightVectors[i] * -wsNormal[i];

   float4 rDotL = 0;
   #ifndef TORQUE_BL_NOSPECULAR

      // We're using the Phong specular reflection model
      // here where traditionally Torque has used Blinn-Phong
      // which has proven to be more accurate to real materials.
      //
      // We do so because its cheaper as do not need to 
      // calculate the half angle for all 4 lights.
      //   
      // Advanced Lighting still uses Blinn-Phong, but the
      // specular reconstruction it does looks fairly similar
      // to this.
      //
      float3 R = reflect( wsView, -wsNormal );

      for ( i = 0; i < 3; i++ )
         rDotL += lightVectors[i] * R[i];

   #endif
 
   // Normalize the dots.
   //
   // Notice we're using the half type here to get a
   // much faster sqrt via the rsq_pp instruction at 
   // the loss of some precision.
   //
   // Unless we have some extremely large point lights
   // i don't believe the precision loss will matter.
   //
   half4 correction = (half4)rsqrt( squareDists );
   nDotL = saturate( nDotL * correction );
   rDotL = clamp( rDotL * correction, 0.00001, 1.0 );

   // First calculate a simple point light linear 
   // attenuation factor.
   //
   // If this is a directional light the inverse
   // radius should be greater than the distance
   // causing the attenuation to have no affect.
   //
   float4 atten = saturate( 1.0 - ( squareDists * inLightInvRadiusSq ) );

   #ifndef TORQUE_BL_NOSPOTLIGHT

      // The spotlight attenuation factor.  This is really
      // fast for what it does... 6 instructions for 4 spots.

      float4 spotAtten = 0;
      for ( i = 0; i < 3; i++ )
         spotAtten += lightVectors[i] * inLightSpotDir[i];

      float4 cosAngle = ( spotAtten * correction ) - inLightSpotAngle;
      atten *= saturate( cosAngle * inLightSpotFalloff );

   #endif

   // Finally apply the shadow masking on the attenuation.
   atten *= shadowMask;

   // Get the final light intensity.
   float4 intensity = nDotL * atten;

   // Combine the light colors for output.
   outDiffuse = 0;
   for ( i = 0; i < 4; i++ )
      outDiffuse += intensity[i] * inLightColor[i];

   // Output the specular power.
   float4 specularIntensity = pow( rDotL, specularPower.xxxx ) * atten;
   
   // Apply the per-light specular attenuation.
   float4 specular = float4(0,0,0,1);
   for ( i = 0; i < 4; i++ )
      specular += float4( inLightColor[i].rgb * inLightColor[i].a * specularIntensity[i], 1 );

   // Add the final specular intensity values together
   // using a single dot product operation then get the
   // final specular lighting color.
   outSpecular = specularColor * specular;
}

float3 F_schlick( in float3 f0, in float3 f90, in float u )
{
	//
    //  F( v, h ) =  F0 + ( 1.0 - F0 ) *  pow( 1.0f - HdotV,  5.0f )
    //
    //
    //  where 
    //
    //  F0 = BaseColor * nDotL
    //
    //  Dielectric materials always have a range of 0.02 < F0 < 0.05 , use a stock value of 0.04 ( roughly plastics )
    //

	return f0 + ( f90 - f0 ) * pow( 1.0f - u ,  5.0f );
}

float Fr_DisneyDiffuse ( float NdotV , float NdotL , float LdotH , float linearRoughness )
{
	float energyBias = lerp (0 , 0.5 , linearRoughness );
	float energyFactor = lerp (1.0 , 1.0 / 1.51 , linearRoughness );
	float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness ;
	float3 f0 = float3 ( 1.0f , 1.0f , 1.0f );
	float lightScatter = F_schlick( f0 , fd90 , NdotL ).r;
	float viewScatter = F_schlick(f0 , fd90 , NdotV ).r;

	return lightScatter * viewScatter * energyFactor ;
}

float SmithGGX( float NdotL, float NdotV, float alpha )
{
    //
    // G( L, V, h ) = G( L ) G( V )
    //
    //                    nDotL
    // G( L ) = _________________________
    //             nDotL ( 1 - k ) + k
    //
    //         
    //                     NdotV
    // G( V ) = _________________________
    //             NdotV ( 1 - k ) + k
    //
    //
    //               pow( ( Roughness + 1 ), 2)
    //  , Where  k = __________________________     ( unreal 4 )
    //                          8
    //
	
	float alphaSqr = alpha * alpha;

	//float GGX_V = NdotL * sqrt ( ( - NdotV * alphaSqr + NdotV ) * NdotV + alphaSqr );
	//float GGX_L = NdotV * sqrt ( ( - NdotL * alphaSqr + NdotL ) * NdotL + alphaSqr );
	
	float GGX_V = NdotL + sqrt ( ( - NdotV * alphaSqr + NdotV ) * NdotV + alphaSqr );
	float GGX_L = NdotV + sqrt ( ( - NdotL * alphaSqr + NdotL ) * NdotL + alphaSqr );
	
	return rcp( GGX_V + GGX_L ); 
	//return 0.5f / ( GGX_V + GGX_L ); 
}

float D_GGX( float NdotH , float alpha )
{
    //
    // or GGX ( disney / unreal 4 )
    //
    //  alpha = pow( roughness, 2 );
    //
    //                                    pow( alpha, 2 )
    //  D( h ) = ________________________________________________________________      
    //           PI pow( pow( NdotH , 2 ) ( pow( alpha, 2 ) - 1 ) + 1 ), 2 )
    //

	float alphaSqr = alpha*alpha;
	float f = ( NdotH * alphaSqr - NdotH ) * NdotH + 1;
	return alphaSqr / ( M_PI_F * (f * f) );
}

float4 EvalBDRF( float3 baseColor, float3 lightColor, float3 toLight, float3 position, float3 normal,  float roughness, float metallic )
{
	//
    //  Microfacet Specular Cook-Torrance
    //
    //                D( h ) F( v, h ) G( l, v, h )
    //    f( l, v ) = ____________________________
    //                 4 ( dot( n, l ) dot( n, v )
    //                 
    //

	float3 L = normalize( toLight );
	float3 V = normalize( -position );
	float3 H = normalize( L + V );
	float3 N = normal;
	
	float NdotV = abs( dot( N, V ) ) + 1e-5f;
	float NdotH = saturate( dot( N, H ) );
	float NdotL = saturate( dot( N, L ) );
	float LdotH = saturate( dot( L, H ) );
	
	float VdotH = saturate( dot( V, H ) );

	if ( NdotL == 0 ) 
		return float4( 0.0f, 0.0f, 0.0f, 0.0f ); 
	
	float alpha = roughness;
	float visLinAlpha = alpha * alpha;
	
	float3 f0 = baseColor;
	float  metal = metallic;
	
	float3 F_conductor= F_schlick( f0, float3( 1.0, 1.0, 1.0 ), VdotH );
	float3 F_dielec   = F_schlick( float3( 0.04, 0.04, 0.04 ), float3( 1.0, 1.0, 1.0 ), VdotH );
	float  Vis        = SmithGGX( NdotL, NdotV, visLinAlpha );
	float  D          = D_GGX( NdotH, visLinAlpha );
	
	float3 Fr_dielec    = D * F_dielec * Vis; 
	float3 Fr_conductor = D * F_conductor * Vis; 
	
	float FR = Fr_DisneyDiffuse( NdotV , NdotL , LdotH , visLinAlpha ) / M_PI_F ;
	float3 Fd = FR;
    float3 specular = ( 1.0f - metal ) * Fr_dielec + metal * Fr_conductor;
	float3 diffuse  = ( 1.0f - metal ) * Fd * f0;
	
    float3 ret = ( diffuse + specular + lightColor) * NdotL;
	
	return float4(ret,FR);
}
