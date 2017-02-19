precision mediump float;
varying vec2 vTextureCoords;
varying vec4 vColour;
varying float distance;
uniform sampler2D sTexture;
uniform vec4 uMod;
uniform vec4 uFade;
uniform vec4 uFog;

void main() {

	gl_FragColor = texture2D( sTexture, vTextureCoords );
	
	if ( gl_FragColor.a < 0.5 ) {
		discard;
	}
	
	gl_FragColor = gl_FragColor * uFade.a;
	
}
