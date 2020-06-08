//#############################################################################
//  File:      PerPixBlinnShadowMapping.frag
//  Purpose:   GLSL per pixel lighting without texturing (and Shadow mapping)
//             Parts of this shader are based on the tutorial on
//             https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
//             by Joey de Vries.
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifdef GL_ES
precision mediump float;
#endif

varying vec3        v_P_VS;                   //!< Interpol. point of illum. in view space (VS)
varying vec3        v_P_WS;                   //!< Interpol. point of illum. in world space (WS)
varying vec3        v_N_VS;                   //!< Interpol. normal at v_P_VS in view space
varying vec2        v_texCoord;               //!< interpol. texture coordinate

uniform int         u_numLightsUsed;          //!< NO. of lights used light arrays
uniform bool        u_lightIsOn[8];           //!< flag if light is on
uniform vec4        u_lightPosWS[8];          //!< position of light in world space
uniform vec4        u_lightPosVS[8];          //!< position of light in view space
uniform vec4        u_lightAmbient[8];        //!< ambient light intensity (Ia)
uniform vec4        u_lightDiffuse[8];        //!< diffuse light intensity (Id)
uniform vec4        u_lightSpecular[8];       //!< specular light intensity (Is)
uniform vec3        u_lightSpotDirVS[8];      //!< spot direction in view space
uniform float       u_lightSpotCutoff[8];     //!< spot cutoff angle 1-180 degrees
uniform float       u_lightSpotCosCut[8];     //!< cosine of spot cutoff angle
uniform float       u_lightSpotExp[8];        //!< spot exponent
uniform vec3        u_lightAtt[8];            //!< attenuation (const,linear,quadr.)
uniform bool        u_lightDoAtt[8];          //!< flag if att. must be calc.
uniform mat4        u_lightSpace[8 * 6];      //!< projection matrices for lights
uniform bool        u_lightCreatesShadows[8]; //!< flag if light creates shadows
uniform bool        u_lightDoesPCF[8];        //!< flag if percentage-closer filtering is enabled
uniform bool        u_lightUsesCubemap[8];    //!< flag if light has a cube shadow map
uniform bool        u_receivesShadows;        //!< flag if material receives shadows
uniform float       u_shadowBias;             //!< Bias to use to prevent shadow acne
uniform vec4        u_globalAmbient;          //!< Global ambient scene color

uniform vec4        u_matAmbient;             //!< ambient color reflection coefficient (ka)
uniform vec4        u_matDiffuse;             //!< diffuse color reflection coefficient (kd)
uniform vec4        u_matSpecular;            //!< specular color reflection coefficient (ks)
uniform vec4        u_matEmissive;            //!< emissive color for selfshining materials
uniform float       u_matShininess;           //!< shininess exponent

uniform float       u_oneOverGamma;           //!< 1.0f / Gamma correction value

uniform int         u_projection;             //!< type of stereo
uniform int         u_stereoEye;              //!< -1=left, 0=center, 1=right
uniform mat3        u_stereoColorFilter;      //!< color filter matrix

uniform sampler2D   u_shadowMap[8];           //!< shadow maps of the lights
uniform samplerCube u_shadowMapCube[8];       //!< cube maps for point lights

//-----------------------------------------------------------------------------
int vectorToFace(vec3 vec) // Vector to process
{
    vec3 absVec = abs(vec);

    if (absVec.x > absVec.y && absVec.x > absVec.z)
        return vec.x > 0 ? 0 : 1;

    else if (absVec.y > absVec.x && absVec.y > absVec.z)
        return vec.y > 0 ? 2 : 3;

    else
        return vec.z > 0 ? 4 : 5;
}
//-----------------------------------------------------------------------------
float shadowTest(in int i) // Light number
{
    if (u_lightCreatesShadows[i]) {

        // Calculate position in light space
        mat4 lightSpace;
        vec3 lightToFragment = v_P_WS - u_lightPosWS[i].xyz;

        if (u_lightUsesCubemap[i])
            lightSpace = u_lightSpace[i * 6 + vectorToFace(lightToFragment)];
        else
            lightSpace = u_lightSpace[i * 6];

        vec4 lightSpacePosition = lightSpace * vec4(v_P_WS, 1.0);

        // Normalize lightSpacePosition
        vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
        
        // Convert to texture coordinates
        projCoords = projCoords * 0.5 + 0.5;

        float currentDepth = projCoords.z;

        // Look up depth from shadow map
        float shadow = 0.0;
        float closestDepth;

        // Use percentage-closer filtering (PCF) for softer shadows (if enabled)
        if (!u_lightUsesCubemap[i] && u_lightDoesPCF[i]) {
            vec2 texelSize = 1.0 / textureSize(u_shadowMap[i], 0);

            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    closestDepth = texture(u_shadowMap[i], projCoords.xy + vec2(x, y) * texelSize).r;
                    shadow += currentDepth - u_shadowBias > closestDepth ? 1.0 : 0.0;
                }
            }
            shadow /= 9.0;

        } else {
            if (u_lightUsesCubemap[i])
                closestDepth = texture(u_shadowMapCube[i], lightToFragment).r;
            else
                closestDepth = texture(u_shadowMap[i], projCoords.xy).r;

            // The fragment is in shadow if the light doesn't "see" it
            if (currentDepth > closestDepth + u_shadowBias)
                shadow = 1.0;
        }

        return shadow;
    }

    return 0.0;
}
//-----------------------------------------------------------------------------
void DirectLight(in    int  i,   // Light number
                 in    vec3 N,   // Normalized normal at P_VS
                 in    vec3 E,   // Normalized vector from P_VS to eye in VS
                 inout vec4 Ia,  // Ambient light intesity
                 inout vec4 Id,  // Diffuse light intesity
                 inout vec4 Is)  // Specular light intesity
{
    // We use the spot light direction as the light direction vector
    vec3 L = normalize(-u_lightSpotDirVS[i].xyz);

    // Half vector H between L and E
    vec3 H = normalize(L+E);

    // Calculate diffuse & specular factors
    float diffFactor = max(dot(N,L), 0.0);
    float specFactor = 0.0;
    if (diffFactor!=0.0)
        specFactor = pow(max(dot(N,H), 0.0), u_matShininess);

    // Accumulate directional light intesities w/o attenuation
    Ia += u_lightAmbient[i];

    // Test if the current fragment is in shadow
    float shadow = u_receivesShadows ? shadowTest(i) : 0.0;
    
    // The higher the value of the variable shadow, the less light reaches the fragment
    Id += u_lightDiffuse[i] * diffFactor * (1.0 - shadow);
    Is += u_lightSpecular[i] * specFactor * (1.0 - shadow);
}
//-----------------------------------------------------------------------------
void PointLight (in    int  i,      // Light number
                 in    vec3 P_VS,   // Point of illumination in VS
                 in    vec3 N,      // Normalized normal at v_P_VS
                 in    vec3 E,      // Normalized vector from v_P_VS to view in VS
                 inout vec4 Ia,     // Ambient light intensity
                 inout vec4 Id,     // Diffuse light intensity
                 inout vec4 Is)     // Specular light intensity
{
    // Vector from v_P_VS to the light in VS
    vec3 L = u_lightPosVS[i].xyz - v_P_VS;

    // Calculate attenuation over distance & normalize L
    float att = 1.0;
    if (u_lightDoAtt[i])
    {   vec3 att_dist;
        att_dist.x = 1.0;
        att_dist.z = dot(L,L);         // = distance * distance
        att_dist.y = sqrt(att_dist.z); // = distance
        att = 1.0 / dot(att_dist, u_lightAtt[i]);
        L /= att_dist.y;               // = normalize(L)
    } else L = normalize(L);

    // Normalized halfvector between the eye and the light vector
    vec3 H = normalize(E + L);

    // Calculate diffuse & specular factors
    float diffFactor = max(dot(N,L), 0.0);
    float specFactor = 0.0;
    if (diffFactor!=0.0)
        specFactor = pow(max(dot(N,H), 0.0), u_matShininess);

    // Calculate spot attenuation
    if (u_lightSpotCutoff[i] < 180.0)
    {   float spotDot; // Cosine of angle between L and spotdir
        float spotAtt; // Spot attenuation
        spotDot = dot(-L, u_lightSpotDirVS[i]);
        if (spotDot < u_lightSpotCosCut[i]) spotAtt = 0.0;
        else spotAtt = max(pow(spotDot, u_lightSpotExp[i]), 0.0);
        att *= spotAtt;
    }

    // Accumulate light intesities
    Ia += att * u_lightAmbient[i];

    // Test if the current fragment is in shadow
    float shadow = u_receivesShadows ? shadowTest(i) : 0.0;

    // The higher the value of the variable shadow, the less light reaches the fragment
    Id += att * u_lightDiffuse[i] * diffFactor * (1.0 - shadow);
    Is += att * u_lightSpecular[i] * specFactor * (1.0 - shadow);
}
//-----------------------------------------------------------------------------
void main()
{
    vec4 Ia, Id, Is;        // Accumulated light intensities at v_P_VS

    Ia = vec4(0.0);         // Ambient light intesity
    Id = vec4(0.0);         // Diffuse light intesity
    Is = vec4(0.0);         // Specular light intesity

    vec3 N = normalize(v_N_VS);  // A varying normal has not anymore unit length
    vec3 E = normalize(-v_P_VS); // Vector from p to the eye

    /* Some GPU manufacturers do not allow uniforms in for loops
    for (int i=0; i<8; i++)
    {   if (i < u_numLightsUsed && u_lightIsOn[i])
        {
            if (u_lightPosVS[i].w == 0.0)
                DirectLight(i, N, E, Ia, Id, Is);
            else
                PointLight(i, v_P_VS, N, E, Ia, Id, Is);
        }
    }*/

    if (u_lightIsOn[0]) {if (u_lightPosVS[0].w == 0.0) DirectLight(0, N, E, Ia, Id, Is); else PointLight(0, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[1]) {if (u_lightPosVS[1].w == 0.0) DirectLight(1, N, E, Ia, Id, Is); else PointLight(1, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[2]) {if (u_lightPosVS[2].w == 0.0) DirectLight(2, N, E, Ia, Id, Is); else PointLight(2, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[3]) {if (u_lightPosVS[3].w == 0.0) DirectLight(3, N, E, Ia, Id, Is); else PointLight(3, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[4]) {if (u_lightPosVS[4].w == 0.0) DirectLight(4, N, E, Ia, Id, Is); else PointLight(4, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[5]) {if (u_lightPosVS[5].w == 0.0) DirectLight(5, N, E, Ia, Id, Is); else PointLight(5, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[6]) {if (u_lightPosVS[6].w == 0.0) DirectLight(6, N, E, Ia, Id, Is); else PointLight(6, v_P_VS, N, E, Ia, Id, Is);}
    if (u_lightIsOn[7]) {if (u_lightPosVS[7].w == 0.0) DirectLight(7, N, E, Ia, Id, Is); else PointLight(7, v_P_VS, N, E, Ia, Id, Is);}


    // Sum up all the reflected color components
    gl_FragColor =  u_globalAmbient +
                    u_matEmissive +
                    Ia * u_matAmbient +
                    Id * u_matDiffuse +
                    Is * u_matSpecular;

    // For correct alpha blending overwrite alpha component
    gl_FragColor.a = u_matDiffuse.a;

    // Apply gamma correction
    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(u_oneOverGamma));

    // Apply stereo eye separation
    if (u_projection > 1)
    {   if (u_projection > 7) // stereoColor??
        {   // Apply color filter but keep alpha
            gl_FragColor.rgb = u_stereoColorFilter * gl_FragColor.rgb;
        }
        else if (u_projection == 5) // stereoLineByLine
        {   if (mod(floor(gl_FragCoord.y), 2.0) < 0.5) // even
            {  if (u_stereoEye ==-1) discard;
            } else // odd
            {  if (u_stereoEye == 1) discard;
            }
        }
        else if (u_projection == 6) // stereoColByCol
        {   if (mod(floor(gl_FragCoord.x), 2.0) < 0.5) // even
            {  if (u_stereoEye ==-1) discard;
            } else // odd
            {  if (u_stereoEye == 1) discard;
            }
        }
        else if (u_projection == 7) // stereoCheckerBoard
        {   bool h = (mod(floor(gl_FragCoord.x), 2.0) < 0.5);
            bool v = (mod(floor(gl_FragCoord.y), 2.0) < 0.5);
            if (h==v) // both even or odd
            {  if (u_stereoEye ==-1) discard;
            } else // odd
            {  if (u_stereoEye == 1) discard;
            }
        }
    }
}
//-----------------------------------------------------------------------------
