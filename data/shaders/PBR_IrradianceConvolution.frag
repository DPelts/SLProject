//#############################################################################
//  File:      PBR_IrradianceConvolution.frag
//  Purpose:   GLSL fragment program to generate an irradiance map by
//             convoluting of an environment map.
//  Author:    Carlos Arauz
//  Date:      April 2018
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

precision highp float;

//-----------------------------------------------------------------------------
in      vec3        v_P_VS;         // sample direction

uniform samplerCube u_matTexture0;  // environment cube map texture

const   float       PI = 3.14159265359;

out     vec4        o_fragColor;    // output fragment color
//-----------------------------------------------------------------------------
void main()
{        
    vec3 N = normalize(v_P_VS);       // a varying normal has not anymore a unit length

    vec3 irradiance = vec3(0.0);    
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up         = cross(N, right);
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
  
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));

            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(u_matTexture0, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
  
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    o_fragColor = vec4(irradiance, 1.0);
}
//-----------------------------------------------------------------------------
