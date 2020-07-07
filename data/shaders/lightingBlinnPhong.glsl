//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void pointLightBlinnPhong( in    int   i,      // Light number between 0 and NUM_LIGHTS
                           in    vec3  N,      // Normalized normal at v_P
                           in    vec3  E,      // Normalized direction at v_P to the eye
                           in    vec3  S,      // Normalized light spot direction
                           in    vec3  L,      // Unnormalized direction at v_P to the light
                           in    float shadow, // shadow factor
                           inout vec4  Ia,     // Ambient light intensity
                           inout vec4  Id,     // Diffuse light intensity
                           inout vec4  Is)     // Specular light intensity
{
    // Normalized halfvector between the eye and the light vector
    vec3 H = normalize(E + L);

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
    float diffFactor = max(dot(N, L), 0.0);
    float specFactor = 0.0;
    if (diffFactor!=0.0)
    specFactor = pow(max(dot(N, H), 0.0), u_matShin);

    // Calculate spot attenuation
    if (u_lightSpotDeg[i] < 180.0)
    {
        float spotDot;// Cosine of angle between L and spotdir
        float spotAtt;// Spot attenuation
        spotDot = dot(-L, S);
        if (spotDot < u_lightSpotCos[i]) spotAtt = 0.0;
        else spotAtt = max(pow(spotDot, u_lightSpotExp[i]), 0.0);
        att *= spotAtt;
    }

    // Accumulate light intesities
    Ia += att * u_lightAmbi[i];
    Id += att * u_lightDiff[i] * diffFactor * (1.0 - shadow);
    Is += att * u_lightSpec[i] * specFactor * (1.0 - shadow);
}
//-----------------------------------------------------------------------------
