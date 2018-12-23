//#############################################################################
//  File:      TextureOnly.frag
//  Purpose:   GLSL fragment shader for texture mapping only
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture0;            // Color map
uniform float     u_oneOverGamma = 1.0f; // 1.0f / Gamma correction value
varying vec2      v_texCoord;            // Interpol. texture coordinate

void main()
{     
    gl_FragColor = texture2D(u_texture0, v_texCoord);

    // Apply gamma correction
    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(u_oneOverGamma));
}
