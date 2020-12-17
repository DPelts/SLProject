//#############################################################################
//  File:      SLGLProgramGenerated.cpp
//  Author:    Marcus Hudritsch
//  Date:      December 2020
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <SLApplication.h>
#include <SLAssetManager.h>
#include <SLGLProgramGenerated.h>
#include <SLGLShader.h>
#include <SLCamera.h>
#include <SLLight.h>

using std::string;
using std::to_string;

//-----------------------------------------------------------------------------
string lightingBlinnPhong = R"(

void directLightBlinnPhong(in    int  i,       // Light number between 0 and NUM_LIGHTS
                           in    vec3 N,       // Normalized normal at v_P
                           in    vec3 E,       // Normalized direction at v_P to the eye
                           in    vec3 S,       // Normalized light spot direction
                           in    float shadow, // shadow factor
                           inout vec4 Ia,      // Ambient light intensity
                           inout vec4 Id,      // Diffuse light intensity
                           inout vec4 Is)      // Specular light intensity
{
    // Calculate diffuse & specular factors
    float diffFactor = max(dot(N, S), 0.0);
    float specFactor = 0.0;

    if (diffFactor!=0.0)
    {
        vec3 H = normalize(S + E);// Half vector H between S and E
        specFactor = pow(max(dot(N, H), 0.0), u_matShin);
    }

    // accumulate directional light intesities w/o attenuation
    Ia += u_lightAmbi[i];
    Id += u_lightDiff[i]  * diffFactor * (1.0 - shadow);
    Is += u_lightSpec[i] * specFactor * (1.0 - shadow);
}

void pointLightBlinnPhong( in    int   i,
                           in    vec3  N,
                           in    vec3  E,
                           in    vec3  S,
                           in    vec3  L,
                           in    float shadow,
                           inout vec4  Ia,
                           inout vec4  Id,
                           inout vec4  Is)
{
    // Calculate attenuation over distance & normalize L
    float att = 1.0;
    if (u_lightDoAtt[i])
    {
        vec3 att_dist;
        att_dist.x = 1.0;
        att_dist.z = dot(L, L);// = distance * distance
        att_dist.y = sqrt(att_dist.z);// = distance
        att = 1.0 / dot(att_dist, u_lightAtt[i]);
        L /= att_dist.y;// = normalize(L)
    }
    else
        L = normalize(L);

    // Calculate diffuse & specular factors
    vec3 H = normalize(E + L);              // Blinn's half vector is faster than Phongs reflected vector
    float diffFactor = max(dot(N, L), 0.0); // Lambertian downscale factor for diffuse reflection
    float specFactor = 0.0;
    if (diffFactor!=0.0)    // specular reflection is only possible if surface is lit from front
        specFactor = pow(max(dot(N, H), 0.0), u_matShin); // specular shininess

    // Calculate spot attenuation
    if (u_lightSpotDeg[i] < 180.0)
    {
        float spotDot;// Cosine of angle between L and spotdir
        float spotAtt;// Spot attenuation
        spotDot = dot(-L, S);
        if (spotDot < u_lightSpotCos[i])  // if outside spot cone
            spotAtt = 0.0;
        else
            spotAtt = max(pow(spotDot, u_lightSpotExp[i]), 0.0);
        att *= spotAtt;
    }

    // Accumulate light intesities
    Ia += att * u_lightAmbi[i];
    Id += att * u_lightDiff[i] * diffFactor * (1.0 - shadow);
    Is += att * u_lightSpec[i] * specFactor * (1.0 - shadow);
}
)";
//-----------------------------------------------------------------------------
string lightingCookTorrance = R"(

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(vec3 N, vec3 E, vec3 L, float roughness)
{
    float NdotV = max(dot(N, E), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

void directLightCookTorrance(in    int   i,        // Light index
                             in    vec3  N,        // Normalized normal at v_P_VS
                             in    vec3  E,        // Normalized vector from v_P to the eye
                             in    vec3  S,        // Normalized light spot direction
                             in    vec3  lightDiff,// diffuse light intensity
                             in    vec3  matDiff,  // diffuse material reflection
                             in    float matMetal, // diffuse material reflection
                             in    float matRough, // diffuse material reflection
                             inout vec3  Lo)       // reflected intensity
{
    vec3 H = normalize(E + S);  // Normalized halfvector between eye and light vector

    vec3 radiance = lightDiff;  // Per light radiance without attenuation

    // Init Fresnel reflection at 90 deg. (0 to N)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, matDiff, matMetal);

    // cook-torrance brdf
    float NDF = distributionGGX(N, H, matRough);
    float G   = geometrySmith(N, E, S, matRough);
    vec3  F   = fresnelSchlick(max(dot(H, E), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - matMetal;

    vec3  nominator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, E), 0.0) * max(dot(N, S), 0.0) + 0.001;
    vec3  specular    = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, S), 0.0);

    Lo += (kD*matDiff.rgb/PI + specular) * radiance * NdotL;
}

void pointLightCookTorrance(in    int   i,        // Light index
                            in    vec3  N,        // Normalized normal at v_P_VS
                            in    vec3  E,        // Normalized vector from v_P to the eye
                            in    vec3  L,        // Vector from v_P to the light
                            in    vec3  S,        // Normalized light spot direction
                            in    vec3  lightDiff,// diffuse light intensity
                            in    vec3  matDiff,  // diffuse material reflection
                            in    float matMetal, // diffuse material reflection
                            in    float matRough, // diffuse material reflection
                            inout vec3  Lo)       // reflected intensity
{
    float distance = length(L); // distance to light
    L /= distance;              // normalize light vector
    float att = 1.0 / (distance*distance);  // quadratic light attenuation

    // Calculate spot attenuation
    if (u_lightSpotDeg[i] < 180.0)
    {
        float spotAtt; // Spot attenuation
        float spotDot; // Cosine of angle between L and spotdir
        spotDot = dot(-L, S);
        if (spotDot < u_lightSpotCos[i]) spotAtt = 0.0;
        else spotAtt = max(pow(spotDot, u_lightSpotExp[i]), 0.0);
        att *= spotAtt;
    }

    vec3 radiance = lightDiff * att;        // per light radiance

    // Init Fresnel reflection at 90 deg. (0 to N)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, matDiff, matMetal);

    // cook-torrance brdf
    vec3  H   = normalize(E + L);  // Normalized halfvector between eye and light vector
    float NDF = distributionGGX(N, H, matRough);
    float G   = geometrySmith(N, E, L, matRough);
    vec3  F   = fresnelSchlick(max(dot(H, E), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - matMetal;

    vec3  nominator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, E), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3  specular    = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD*matDiff.rgb/PI + specular) * radiance * NdotL;
}
)";
//-----------------------------------------------------------------------------
string doStereoSeparation = R"(

void doStereoSeparation()
{
    // See SLProjection in SLEnum.h
    if (u_camProjection > 8) // stereoColors
    {
        // Apply color filter but keep alpha
        o_fragColor.rgb = u_camStereoColors * o_fragColor.rgb;
    }
    else if (u_camProjection == 6) // stereoLineByLine
    {
        if (mod(floor(gl_FragCoord.y), 2.0) < 0.5)// even
        {
            if (u_camStereoEye ==-1)
                discard;
        } else // odd
        {
            if (u_camStereoEye == 1)
                discard;
        }
    }
    else if (u_camProjection == 7) // stereoColByCol
    {
        if (mod(floor(gl_FragCoord.x), 2.0) < 0.5)// even
        {
            if (u_camStereoEye ==-1)
                discard;
        } else // odd
        {
            if (u_camStereoEye == 1)
                discard;
        }
    }
    else if (u_camProjection == 8) // stereoCheckerBoard
    {
        bool h = (mod(floor(gl_FragCoord.x), 2.0) < 0.5);
        bool v = (mod(floor(gl_FragCoord.y), 2.0) < 0.5);
        if (h==v)// both even or odd
        {
            if (u_camStereoEye ==-1)
                discard;
        } else // odd
        {
            if (u_camStereoEye == 1)
                discard;
        }
    }
}
)";
//-----------------------------------------------------------------------------
string fogBlend = R"(

vec4 fogBlend(vec3 P_VS, vec4 inColor)
{
    float factor = 0.0f;
    float distance = length(P_VS);

    switch (u_camFogMode)
    {
        case 0:
            factor = (u_camFogEnd - distance) / (u_camFogEnd - u_camFogStart);
            break;
        case 1:
            factor = exp(-u_camFogDensity * distance);
            break;
        default:
            factor = exp(-u_camFogDensity * distance * u_camFogDensity * distance);
            break;
    }

    vec4 outColor = factor * inColor + (1.0 - factor) * u_camFogColor;
    outColor = clamp(outColor, 0.0, 1.0);
    return outColor;
}
)";
//-----------------------------------------------------------------------------
string stringName4 = R"(

)";
//-----------------------------------------------------------------------------
string stringName5 = R"(

)";
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildShaderProgram(SLMaterial* mat,
                                              SLCamera*   cam,
                                              SLVLight*   lights)
{
    assert(mat && "No material pointer passed!");
    assert(cam && "No camera pointer passed!");
    assert(!lights->empty() && "No lights passed!");

    bool matHasTm = !mat->textures().empty();
    bool matHasNm = mat->textures().size() > 1 &&
                    mat->textures()[1]->texType() == TT_normal;
    bool matHasAo = mat->textures().size() > 2 &&
                    mat->textures()[2]->texType() == TT_ambientOcclusion;
    bool lightsHaveSm = lights->at(0)->createsShadows();

    if (mat->lightModel() == LM_BlinnPhong)
    {
        if (matHasTm)
        {
            if (matHasNm && matHasAo && lightsHaveSm)
                buildPerPixBlinnTmNmAoSm(mat, cam, lights);
            else if (matHasNm && matHasAo)
                buildPerPixBlinnTmNmAo(mat, cam, lights);
            else if (matHasNm && lightsHaveSm)
                buildPerPixBlinnTmNmSm(mat, cam, lights);
            else if (matHasNm)
                buildPerPixBlinnTmNm(mat, cam, lights);
            else if (lightsHaveSm)
                buildPerPixBlinnTmSm(mat, cam, lights);
            else
                buildPerPixBlinnTm(mat, cam, lights);
        }
        else
        {
            if (matHasAo && lightsHaveSm)
                buildPerPixBlinnAoSm(mat, cam, lights);
            else if (lightsHaveSm)
                buildPerPixBlinnSm(mat, cam, lights);
            else
                buildPerPixBlinn(mat, cam, lights);
        }
    }
    else
        SL_EXIT_MSG("Only Blinn-Phong supported yet.");
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnTmNmAoSm(SLMaterial* mat,
                                                    SLCamera*   cam,
                                                    SLVLight*   lights)
{
    assert(_shaders.size() > 1 &&
           _shaders[0]->type() == ST_vertex &&
           _shaders[1]->type() == ST_fragment);

    // Assemble vertex shader code
    string vertCode;
    vertCode += "precision highp float;\n";
    vertCode += "#define NUM_LIGHTS " + to_string(lights->size()) + "\n";
    vertCode += R"(

layout (location = 0) in vec4  a_position;  // Vertex position attribute
layout (location = 1) in vec3  a_normal;    // Vertex normal attribute
layout (location = 2) in vec2  a_uv1;       // Vertex tex.coord. 1 for diffuse color
layout (location = 3) in vec2  a_uv2;       // Vertex tex.coord. 2 for AO
layout (location = 5) in vec4  a_tangent;   // Vertex tangent attribute

uniform mat4  u_mvMatrix;   // modelview matrix
uniform mat3  u_nMatrix;    // normal matrix=transpose(inverse(mv))
uniform mat4  u_mvpMatrix;  // = projection * modelView
uniform mat4  u_mMatrix;    // model matrix

uniform vec4  u_lightPosVS[NUM_LIGHTS];     // position of light in view space
uniform vec3  u_lightSpotDir[NUM_LIGHTS];   // spot direction in view space
uniform float u_lightSpotDeg[NUM_LIGHTS];   // spot cutoff angle 1-180 degrees

out     vec3  v_P_VS;                   // Point of illumination in view space (VS)
out     vec3  v_P_WS;                   // Point of illumination in world space (WS)
out     vec3  v_N_VS;                   // Normal at P_VS in view space
out     vec2  v_uv1;                    // Texture coordiante 1 output
out     vec2  v_uv2;                    // Texture coordiante 2 output
out     vec3  v_eyeDirTS;               // Vector to the eye in tangent space
out     vec3  v_lightDirTS[NUM_LIGHTS]; // Vector to the light 0 in tangent space
out     vec3  v_spotDirTS[NUM_LIGHTS];  // Spot direction in tangent space
//-----------------------------------------------------------------------------
void main()
{
    v_uv1 = a_uv1;  // pass diffuse color tex.coord. 1 for interpolation
    v_uv2 = a_uv2;  // pass ambient occlusion tex.coord. 2 for interpolation

    // Building the matrix Eye Space -> Tangent Space
    // See the math behind at: http://www.terathon.com/code/tangent.html
    vec3 n = normalize(u_nMatrix * a_normal);
    vec3 t = normalize(u_nMatrix * a_tangent.xyz);
    vec3 b = cross(n, t) * a_tangent.w; // bitangent w. corrected handedness
    mat3 TBN = mat3(t,b,n);

    v_P_VS = vec3(u_mvMatrix *  a_position); // vertex position in view space
    v_P_WS = vec3(u_mMatrix * a_position);   // vertex position in world space

    // Transform vector to the eye into tangent space
    v_eyeDirTS = -v_P_VS;  // eye vector in view space
    v_eyeDirTS *= TBN;

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        // Transform spotdir into tangent space
        v_spotDirTS[i] = u_lightSpotDir[i];
        v_spotDirTS[i]  *= TBN;

        // Transform vector to the light 0 into tangent space
        vec3 L = u_lightPosVS[i].xyz - v_P_VS;
        v_lightDirTS[i] = L;
        v_lightDirTS[i] *= TBN;
    }

    // pass the vertex w. the fix-function transform
    gl_Position = u_mvpMatrix * a_position;
}
)";

    // Add vertex shader code to the SLGLShader instance
    SLGLShader* vertSh = _shaders[0];
    vertSh->code(SLGLShader::removeComments(vertCode));
    vertSh->name("generatedPerPixBlinnTmNmAoSm.vert");
    vertSh->file(SLApplication::configPath + vertSh->name());

    // Assemble fragment shader code
    string fragCode;
    fragCode += "\nprecision highp float;\n";
    fragCode += "\n#define NUM_LIGHTS " + to_string(lights->size()) + "\n";
    fragCode += R"(

out     vec4        o_fragColor;    // output fragment color

in      vec3        v_P_VS;     // Interpol. point of illum. in view space (VS)
in      vec3        v_P_WS;     // Interpol. point of illum. in world space (WS)
in      vec2        v_uv1;      // Texture coordiante 1 varying for diffuse color
in      vec2        v_uv2;      // Texture coordiante 2 varying for AO
in      vec3        v_eyeDirTS;                 // Vector to the eye in tangent space
in      vec3        v_lightDirTS[NUM_LIGHTS];   // Vector to light 0 in tangent space
in      vec3        v_spotDirTS[NUM_LIGHTS];    // Spot direction in tangent space

uniform bool        u_lightIsOn[NUM_LIGHTS];                // flag if light is on
uniform vec4        u_lightPosVS[NUM_LIGHTS];               // position of light in view space
uniform vec4        u_lightPosWS[NUM_LIGHTS];               // position of light in world space
uniform vec4        u_lightAmbi[NUM_LIGHTS];                // ambient light intensity (Ia)
uniform vec4        u_lightDiff[NUM_LIGHTS];                // diffuse light intensity (Id)
uniform vec4        u_lightSpec[NUM_LIGHTS];                // specular light intensity (Is)
uniform vec3        u_lightSpotDir[NUM_LIGHTS];             // spot direction in view space
uniform float       u_lightSpotDeg[NUM_LIGHTS];             // spot cutoff angle 1-180 degrees
uniform float       u_lightSpotCos[NUM_LIGHTS];             // cosine of spot cutoff angle
uniform float       u_lightSpotExp[NUM_LIGHTS];             // spot exponent
uniform vec3        u_lightAtt[NUM_LIGHTS];                 // attenuation (const,linear,quadr.)
uniform bool        u_lightDoAtt[NUM_LIGHTS];               // flag if att. must be calc.
uniform vec4        u_globalAmbi;                           // Global ambient scene color
uniform float       u_oneOverGamma;                         // 1.0f / Gamma correction value
uniform mat4        u_lightSpace[NUM_LIGHTS * 6];           // projection matrices for lights
uniform bool        u_lightCreatesShadows[NUM_LIGHTS];      // flag if light creates shadows
uniform bool        u_lightDoSmoothShadows[NUM_LIGHTS];     // flag if percentage-closer filtering is enabled
uniform int         u_lightSmoothShadowLevel[NUM_LIGHTS];   // radius of area to sample for PCF
uniform float       u_lightShadowMinBias[NUM_LIGHTS];       // min. shadow bias value at 0° to N
uniform float       u_lightShadowMaxBias[NUM_LIGHTS];       // min. shadow bias value at 90° to N
uniform bool        u_lightUsesCubemap[NUM_LIGHTS];         // flag if light has a cube shadow map

uniform vec4        u_matAmbi;          // ambient color reflection coefficient (ka)
uniform vec4        u_matDiff;          // diffuse color reflection coefficient (kd)
uniform vec4        u_matSpec;          // specular color reflection coefficient (ks)
uniform vec4        u_matEmis;          // emissive color for self-shining materials
uniform float       u_matShin;          // shininess exponent
uniform bool        u_matGetsShadows;   // flag if material receives shadows
uniform sampler2D   u_matTexture0;      // diffuse color map
uniform sampler2D   u_matTexture1;      // normal bump map
uniform sampler2D   u_matTexture2;      // ambient occlusion map

uniform int         u_camProjection;    // type of stereo
uniform int         u_camStereoEye;     // -1=left, 0=center, 1=right
uniform mat3        u_camStereoColors;  // color filter matrix
uniform bool        u_camFogIsOn;       // flag if fog is on
uniform int         u_camFogMode;       // 0=LINEAR, 1=EXP, 2=EXP2
uniform float       u_camFogDensity;    // fog densitiy value
uniform float       u_camFogStart;      // fog start distance
uniform float       u_camFogEnd;        // fog end distance
uniform vec4        u_camFogColor;      // fog color (usually the background)

)";
    addShadowMapDeclaration(lights, fragCode);
    fragCode += lightingBlinnPhong;
    fragCode += fogBlend;
    fragCode += doStereoSeparation;
    addShadowTestCode(lights, fragCode);
    fragCode += R"(

void main()
{
    vec4 Ia = vec4(0.0); // Accumulated ambient light intensity at v_P_VS
    vec4 Id = vec4(0.0); // Accumulated diffuse light intensity at v_P_VS
    vec4 Is = vec4(0.0); // Accumulated specular light intensity at v_P_VS

    // Get normal from normal map, move from [0,1] to [-1, 1] range & normalize
    vec3 N = normalize(texture(u_matTexture1, v_uv1).rgb * 2.0 - 1.0);
    vec3 E = normalize(v_eyeDirTS);   // normalized eye direction

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        if (u_lightIsOn[i])
        {
            if (u_lightPosVS[i].w == 0.0)
            {
                // We use the spot light direction as the light direction vector
                vec3 S = normalize(-v_spotDirTS[i]);

                // Test if the current fragment is in shadow
                float shadow = u_matGetsShadows ? shadowTest(i, N, S) : 0.0;

                directLightBlinnPhong(i, N, E, S, shadow, Ia, Id, Is);
            }
            else
            {
                vec3 S = normalize(v_spotDirTS[i]); // normalized spot direction in TS
                vec3 L = v_lightDirTS[i]; // Vector from v_P to light in TS

                // Test if the current fragment is in shadow
                float shadow = u_matGetsShadows ? shadowTest(i, N, L) : 0.0;

                pointLightBlinnPhong(i, N, E, S, L, shadow, Ia, Id, Is);
            }
        }
    }

    // Get ambient occlusion factor
    float AO = texture(u_matTexture2, v_uv2).r;

    // Sum up all the reflected color components
    o_fragColor =  u_matEmis +
                   u_globalAmbi +
                   Ia * u_matAmbi * AO +
                   Id * u_matDiff;

    // Componentwise multiply w. texture color
    o_fragColor *= texture(u_matTexture0, v_uv1);

    // add finally the specular RGB-part
    vec4 specColor = Is * u_matSpec;
    o_fragColor.rgb += specColor.rgb;

    // Apply gamma correction
    o_fragColor.rgb = pow(o_fragColor.rgb, vec3(u_oneOverGamma));

    // Apply fog by blending over distance
    if (u_camFogIsOn)
    o_fragColor = fogBlend(v_P_VS, o_fragColor);

    // Apply stereo eye separation
    if (u_camProjection > 1)
    doStereoSeparation();
}
)";

    // Add fragment shader code to the SLGLShader instance
    SLGLShader* fragSh = _shaders[1];
    fragSh->code(SLGLShader::removeComments(fragCode));
    fragSh->name("generatedPerPixBlinnTmNmAoSm.frag");
    fragSh->file(SLApplication::configPath + fragSh->name());
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnTmNmAo(SLMaterial* mat,
                                                  SLCamera*   cam,
                                                  SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnTmNmSm(SLMaterial* mat,
                                                  SLCamera*   cam,
                                                  SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnTmNm(SLMaterial* mat,
                                                SLCamera*   cam,
                                                SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnTmSm(SLMaterial* mat,
                                                SLCamera*   cam,
                                                SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnAoSm(SLMaterial* mat,
                                                SLCamera*   cam,
                                                SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnSm(SLMaterial* mat,
                                              SLCamera*   cam,
                                              SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinnTm(SLMaterial* mat,
                                              SLCamera*   cam,
                                              SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::buildPerPixBlinn(SLMaterial* mat,
                                            SLCamera*   cam,
                                            SLVLight*   lights)
{
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::addShadowMapDeclaration(SLVLight* lights,
                                                   string&   fragCode)
{
    for (SLuint i = 0; i < lights->size(); ++i)
    {
        SLLight* light = lights->at(i);
        if (light->createsShadows())
        {
            SLShadowMap* shadowMap = light->shadowMap();
            if (shadowMap->useCubemap())
                fragCode += "uniform samplerCube u_shadowMapCube_";
            else
                fragCode += "uniform sampler2D   u_shadowMap_";
            fragCode += to_string(i) + ";\n";
        }
    }
}
//-----------------------------------------------------------------------------
void SLGLProgramGenerated::addShadowTestCode(SLVLight* lights,
                                             string&   fragCode)
{
    fragCode += R"(

int vectorToFace(vec3 vec) // Vector to process
{
    vec3 absVec = abs(vec);
    if (absVec.x > absVec.y && absVec.x > absVec.z)
        return vec.x > 0.0 ? 0 : 1;
    else if (absVec.y > absVec.x && absVec.y > absVec.z)
        return vec.y > 0.0 ? 2 : 3;
    else
        return vec.z > 0.0 ? 4 : 5;
}

float shadowTest(in int i, in vec3 N, in vec3 lightDir)
{
    if (u_lightCreatesShadows[i])
    {
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

        // calculate bias between min. and max. bias depending on the angle between N and lightDir
        float bias = max(u_lightShadowMaxBias[i] * (1.0 - dot(N, lightDir)), u_lightShadowMinBias[i]);

        // Use percentage-closer filtering (PCF) for softer shadows (if enabled)
        if (!u_lightUsesCubemap[i] && u_lightDoSmoothShadows[i])
        {
            vec2 texelSize;
)";

    for (SLuint i = 0; i < lights->size(); ++i)
    {
        SLLight*     light     = lights->at(i);
        SLShadowMap* shadowMap = light->shadowMap();
        if (!shadowMap->useCubemap())
            fragCode += "            if (i == " +
                        to_string(i) + ") texelSize = 1.0 / vec2(textureSize(u_shadowMap_" +
                        to_string(i) + ", 0));\n";
    }

    fragCode += R"(
            int level = u_lightSmoothShadowLevel[i];

            for (int x = -level; x <= level; ++x)
            {
                for (int y = -level; y <= level; ++y)
                {
)";

    for (SLuint i = 0; i < lights->size(); ++i)
    {
        SLLight*     light     = lights->at(i);
        SLShadowMap* shadowMap = light->shadowMap();
        if (!shadowMap->useCubemap())
            fragCode += "                    if (i == " +
                        to_string(i) + ") closestDepth = texture(u_shadowMap_" +
                        to_string(i) + ", projCoords.xy + vec2(x, y) * texelSize).r;\n";
    }

    fragCode += R"(
                    shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
                }
            }
            shadow /= pow(1.0 + 2.0 * float(level), 2.0);
        }
        else
        {
            if (u_lightUsesCubemap[i])
            {
)";

    for (SLuint i = 0; i < lights->size(); ++i)
    {
        SLLight*     light     = lights->at(i);
        SLShadowMap* shadowMap = light->shadowMap();
        if (shadowMap->useCubemap())
            fragCode += "                if (i == " +
                        to_string(i) + ") closestDepth = texture(u_shadowMapCube_" +
                        to_string(i) + ", lightToFragment).r;\n";
    }

    fragCode += R"(
            }
            else
            {
)";

    for (SLuint i = 0; i < lights->size(); ++i)
    {
        SLLight*     light     = lights->at(i);
        SLShadowMap* shadowMap = light->shadowMap();
        if (!shadowMap->useCubemap())
            fragCode += "                if (i == " +
                        to_string(i) + ") closestDepth = texture(u_shadowMap_" +
                        to_string(i) + ", projCoords.xy).r;\n";
    }

    fragCode += R"(
            }

            // The fragment is in shadow if the light doesn't "see" it
            if (currentDepth > closestDepth + bias)
                shadow = 1.0;
        }

        return shadow;
    }

    return 0.0;
}
)";
}
//-----------------------------------------------------------------------------
string shadowMapUniformName(SLVLight* lights, int lightNum)
{
    if (lights->at(lightNum)->createsShadows())
    {
        SLLight* light = lights->at(lightNum);
        if (light->createsShadows())
        {
            SLShadowMap* shadowMap = light->shadowMap();
            if (shadowMap->useCubemap())
                return "u_shadowMapCube_" + to_string(lightNum);
            else
                return "u_shadowMap_" + to_string(lightNum);
        }
    }
    return "Light creates no shadows!";
}
//-----------------------------------------------------------------------------