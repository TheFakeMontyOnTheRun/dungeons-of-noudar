#ifdef GL_ES
precision mediump float;
#endif


#if __VERSION__ >= 140
in vec4 aPosition;
in vec2 aTexCoord;
out vec2 vTextureCoords;
#else
attribute vec4 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTextureCoords;
#endif


uniform mat2 uScale;
uniform mat4 uProjectionView;

void main() {
	gl_Position =  uProjectionView * aPosition;
	vTextureCoords = (uScale * aTexCoord );
}
