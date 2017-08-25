attribute vec4 aPosition;
attribute vec4 aColour;
attribute vec2 aTexCoord;
uniform mat4 uModel;
uniform mat4 uScale;
uniform mat4 uProjection;
uniform mat4 uView;
varying float distance;
varying vec2 vTextureCoords;
varying vec4 vColour;

void main() {
    gl_Position =  uProjection * uView * uModel * aPosition;
    vColour = aColour;
    vTextureCoords = (uScale * vec4(aTexCoord.xy, 1 , 1 )).xy;
    distance = normalize( gl_Position.xyz).z;
}
