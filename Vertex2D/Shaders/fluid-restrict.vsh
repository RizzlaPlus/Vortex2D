precision highp float;

attribute mediump vec2 a_Position;
attribute mediump vec2 a_TexCoords;

uniform mediump mat4 u_Projection;
uniform vec2 h;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordx;
varying mediump vec2 v_texCoordy;
varying mediump vec2 v_texCoordxy;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    v_texCoord = a_TexCoords;

    mediump vec2 dx = vec2(1.0/h.x, 0.);
    mediump vec2 dy = vec2(0., 1.0/h.y);

    v_texCoordx = v_texCoord + dx;
    v_texCoordy = v_texCoord + dy;
    v_texCoordxy = v_texCoord + dx + dy;
}