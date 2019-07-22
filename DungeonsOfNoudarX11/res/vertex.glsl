#ifdef GL_ES
precision mediump float;
#endif


#if __VERSION__ >= 140
in vec4 aPosition;
in vec4 aColour;
in vec2 aTexCoord;
out float distance;
out vec2 vTextureCoords;
out vec4 vColour;
#else
attribute vec4 aPosition;
attribute vec4 aColour;
attribute vec2 aTexCoord;
varying float distance;
varying vec2 vTextureCoords;
varying vec4 vColour;
#endif

uniform mat4 uView;
uniform mat4 uModel;
uniform mat4 uScale;
uniform mat4 uProjection;

void main() {
    gl_Position =  uProjection * uView * uModel * aPosition;
    vColour = aColour;
    vTextureCoords = (uScale * vec4(aTexCoord.xy, 1 , 1 )).xy;
    distance = normalize( gl_Position.xyz).z;
}
