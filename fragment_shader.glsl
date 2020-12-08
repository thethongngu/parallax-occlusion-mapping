#version 400

in vec2 out_texture;
in vec3 out_vertex;
in vec3 out_normal;

uniform sampler2D texture_data, normal_data, heightmap_data;
uniform vec3 light_pos;
uniform vec3 eye_point;

out vec4 out_color;

vec4 CalculateNormal( in vec2 texCoords )
{
    vec2 texOffs = 1.0 / textureSize( heightmap_data, 0 ).xy;
    vec2 scale   = 1.0 / texOffs;

    float hx[9];
    hx[0] = texture( heightmap_data, texCoords.st + texOffs * vec2(-1.0, -1.0) ).r;
    hx[1] = texture( heightmap_data, texCoords.st + texOffs * vec2( 0.0, -1.0) ).r;
    hx[2] = texture( heightmap_data, texCoords.st + texOffs * vec2( 1.0, -1.0) ).r;
    hx[3] = texture( heightmap_data, texCoords.st + texOffs * vec2(-1.0,  0.0) ).r;
    hx[4] = texture( heightmap_data, texCoords.st ).r;
    hx[5] = texture( heightmap_data, texCoords.st + texOffs * vec2( 1.0, 0.0) ).r;
    hx[6] = texture( heightmap_data, texCoords.st + texOffs * vec2(-1.0, 1.0) ).r;
    hx[7] = texture( heightmap_data, texCoords.st + texOffs * vec2( 0.0, 1.0) ).r;
    hx[8] = texture( heightmap_data, texCoords.st + texOffs * vec2( 1.0, 1.0) ).r;
    vec2  deltaH = vec2(hx[0]-hx[2] + 2.0*(hx[3]-hx[5]) + hx[6]-hx[8], hx[0]-hx[6] + 2.0*(hx[1]-hx[7]) + hx[2]-hx[8]);
    float h_mid  = hx[4];

    return vec4( normalize( vec3( deltaH * scale, 1.0 ) ), h_mid );
}
float CalculateHeight( in vec2 texCoords )
{
    float height = texture( heightmap_data, texCoords ).x;
    return clamp( height, 0.0, 1.0 );
}

vec3 NoParallax( in vec3 texDir3D, in vec2 texCoord )
{
    return vec3(texCoord.xy, 0.0);
}

void main(){
    float u_ambient = 0.2;
    float u_specular = 0.55;
    float u_shininess = 0.0;
    float u_diffuse = 0.75;

    vec3  objPosEs     = out_vertex;
    vec3  objNormalEs  = out_normal;
    vec2  texCoords    = out_texture.st;
    vec3  normalEs     = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs );

    // (co-)tangent space
    vec3  N            = normalize( objNormalEs );
    vec3  dp1          = dFdx( objPosEs );
    vec3  dp2          = dFdy( objPosEs );
    vec2  duv1         = dFdx( texCoords );
    vec2  duv2         = dFdy( texCoords );
    vec3  dp2perp      = cross(dp2, N); 
    vec3  dp1perp      = cross(N, dp1);
    vec3  T            = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3  B            = dp2perp * duv1.y + dp1perp * duv2.y;   
    float invmax       = inversesqrt(max(dot(T, T), dot(B, B)));
    mat3  tbnMat       = mat3(T * invmax, B * invmax, N * 1.0);

    // parallax mapping
    vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs );
    vec3  newTexCoords = abs(1.0) < 0.001 ? vec3(texCoords.st, 0.0) : NoParallax( texDir3D, texCoords.st );
    texCoords.st       = newTexCoords.xy;
    vec4  normalVec    = CalculateNormal( texCoords ); 
    tbnMat[2].xyz     *= (gl_FrontFacing ? 1.0 : -1.0) * N / 1.0;
    vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );

    // texture color
    vec3 color = texture( texture_data, texCoords.st ).rgb;

    // ambient part
    vec3 lightCol = u_ambient * color;

    // diffuse part
    vec3  normalV = normalize( nvMappedEs );
    vec3  lightV  = normalize( -light_pos );
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color;

    // specular part
    vec3  eyeV      = normalize( -objPosEs );
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color;

    out_color = vec4( lightCol.rgb, 1.0 );
}