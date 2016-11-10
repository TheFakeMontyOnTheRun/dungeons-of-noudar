#version 140

in vec2 vTextureCoords;
in vec4 vColour;
in float distance;

uniform vec4 uMod;
uniform vec4 uFade;
uniform vec4 uFog;

uniform sampler2D sTexture;

out vec4 fragColor;

void main() {
	float fade = 0.75 + ((1.0 - distance) / 4.0);
	fragColor = texture( sTexture, vTextureCoords ) * uMod * vec4( fade, fade, fade, 1.0 );
	
	if ( fragColor.a < 0.5 ) {
		discard;
	}
	
	if ( uFade.a >= 0.1 ) {
		fragColor = fragColor * vec4( uFade.xyz, 1.0 );
	}

}
