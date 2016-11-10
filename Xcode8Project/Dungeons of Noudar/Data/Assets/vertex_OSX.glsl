#version 140

in vec4 aPosition;
in vec4 aColour;
in vec2 aTexCoord;
uniform mat4 uModel;
uniform mat4 uProjection;
uniform mat4 uView;
out float distance;
out vec2 vTextureCoords;
out vec4 vColour;

void main() {
    gl_Position =  uProjection * uView * uModel * aPosition;
    vColour = aColour;
    vTextureCoords = aTexCoord;
    distance = normalize( gl_Position.xyz).z;
}
