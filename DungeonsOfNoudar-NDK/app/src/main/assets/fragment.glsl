#ifdef GL_ES
precision mediump float;
#endif


#if __VERSION__ >= 140
in vec2 vTextureCoords;
in vec4 vColour;
in float distance;
out vec4 fragColor;
#else
varying vec2 vTextureCoords;
varying vec4 vColour;
varying float distance;
#endif

uniform sampler2D sTexture;
uniform vec4 uMod;
uniform vec4 uFade;
uniform vec4 uFog;

void main() {
    #if __VERSION__ >= 140
    fragColor = texture2D( sTexture, vTextureCoords );

    if ( fragColor.a < 0.5 ) {
        discard;
    }
    fragColor = fragColor * uFade.a * uMod;

    #else
    gl_FragColor = texture2D( sTexture, vTextureCoords );

    if ( gl_FragColor.a < 0.5 ) {
        discard;
    }
    gl_FragColor = gl_FragColor * uFade.a * uMod;
    #endif
}
