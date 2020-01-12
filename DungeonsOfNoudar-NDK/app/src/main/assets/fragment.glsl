#ifdef GL_ES
precision lowp float;
#endif


#if __VERSION__ >= 140
in vec2 vTextureCoords;
out vec4 fragColor;
#else
varying vec2 vTextureCoords;
#endif

uniform sampler2D sTexture;
uniform vec4 uMod;
uniform vec4 uFade;

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
