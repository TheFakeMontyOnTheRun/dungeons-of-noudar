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
#else
attribute vec4 aPosition;
attribute vec4 aColour;
attribute vec2 aTexCoord;
uniform mat4 uModel;
uniform mat4 uProjection;
uniform mat4 uView;
varying float distance;
varying vec2 vTextureCoords;
varying vec4 vColour;
#endif



void main() {
    gl_Position =  uProjection * uView * uModel * aPosition;
    vColour = aColour;
    vTextureCoords = aTexCoord;
    distance = normalize( gl_Position.xyz).z;
}
