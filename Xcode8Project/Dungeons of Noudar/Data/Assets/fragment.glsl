#ifdef GL_ES
precision highp float;
#endif




#if __VERSION__ >= 140
in vec4 aPosition;
in vec4 aColour;
in vec2 aTexCoord;
in mat4 uModel;
in mat4 uProjection;
in mat4 uView;
out float distance;
out vec2 vTextureCoords;
out vec4 vColour;


in vec2 vTextureCoords;
in vec4 vColour;
in float distance;
in sampler2D sTexture;
in vec4 uMod;
in vec4 uFade;
in vec4 uFog;
out vec4 fragColor;
#else
varying vec2 vTextureCoords;
varying vec4 vColour;
varying float distance;
uniform sampler2D sTexture;
uniform vec4 uMod;
uniform vec4 uFade;
uniform vec4 uFog;
#endif


void main() {
	
#if __VERSION__ >= 140
	float fade = 0.75 + ((1.0 - distance) / 4.0);
	fragColor = texture2D( sTexture, vTextureCoords ) * uMod * vec4( fade, fade, fade, 1.0 );
	
	if ( fragColor.a < 0.5 ) {
		discard;
	}
	
	if ( uFade.a >= 0.1 ) {
		fragColor = gl_FragColor * vec4( uFade.xyz, 1.0 );
	}
#else
	float fade = 0.75 + ((1.0 - distance) / 4.0);
	gl_FragColor = texture2D( sTexture, vTextureCoords ) * uMod * vec4( fade, fade, fade, 1.0 );
	
	if ( gl_FragColor.a < 0.5 ) {
		discard;
	}
	
	if ( uFade.a >= 0.1 ) {
		gl_FragColor = gl_FragColor * vec4( uFade.xyz, 1.0 );
	}
#endif
}
