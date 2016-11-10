precision highp float;

varying vec2 vTextureCoords;
varying vec4 vColour;
varying float distance;
uniform sampler2D sTexture;
uniform vec4 uMod;
uniform vec4 uFade;
uniform vec4 uFog;

void main() {
	
	float fade = 0.75 + ((1.0 - distance) / 4.0);
	gl_FragColor = texture2D( sTexture, vTextureCoords ) * uMod * vec4( fade, fade, fade, 1.0 );
	
	if ( gl_FragColor.a < 0.5 ) {
		discard;
	}
	
	if ( uFade.a >= 0.1 ) {
		gl_FragColor = gl_FragColor * vec4( uFade.xyz, 1.0 );
	}
}
