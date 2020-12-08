#version 330

in vec2 out_texture;
in vec3 out_vertex;
in vec3 out_normal;

uniform sampler2D texture_data, normal_data, heightmap_data;
uniform vec3 light_pos;
uniform vec3 eye_point;

out vec4 out_color;

mat3 compute_tbn(vec2 uv){
    vec3 Q1  = dFdx(out_vertex);
    vec3 Q2  = dFdy(out_vertex);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 n  = normalize(out_normal);
    vec3 t  = normalize(Q1*st2.t - Q2*st1.t);

    vec3 b = normalize(-Q1*st2.s + Q2*st1.s);

    mat3 tbn = mat3(t, b, n);

    return tbn;
}

vec3 get_normal_from_map(vec2 uv) {
    vec3 tangent_normal = texture(normal_data, uv).xyz * 2.0 - 1.0;
    mat3 tbn = compute_tbn(uv);
    return normalize(tbn * tangent_normal);
}

vec2 parallax_occlusion_mapping(vec2 texCoords, vec3 viewDir) {
    // number of depth layers
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float heightScale = 0.1;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(texture_data, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(texture_data, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(texture_data, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

// hard shadow: https://stackoverflow.com/a/55091654/3584162
// soft shadow: https://github.com/piellardj/parallax-mapping/blob/master/shaders/parallax.frag (better effect)
float calcShadow(vec2 texCoords, vec3 lightDir) {
    float selfShadowFactor = 1.0f;

    float minLayers = 8;
    float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), lightDir)));
    float heightScale = 0.1f;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(texture_data, currentTexCoords).r;
    float currentLayerDepth = currentDepthMapValue;

    float layerDepth = 1.0 / numLayers;
    vec2 P = lightDir.xy / lightDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    while (currentLayerDepth > 0.0)
    {
        currentTexCoords += deltaTexCoords;
        currentDepthMapValue = texture(texture_data, currentTexCoords).r;
        currentLayerDepth -= layerDepth;

        if(currentDepthMapValue < currentLayerDepth){
            selfShadowFactor -= (currentLayerDepth - currentDepthMapValue) / layerDepth;
        }
    }

    selfShadowFactor = max(0.0, selfShadowFactor);

    return selfShadowFactor;
}

void main(){
    vec3 tan_view_dir = normalize(compute_tbn(out_texture) * (eye_point - out_vertex));
    vec2 distorted_uv = parallax_occlusion_mapping(out_texture, tan_view_dir);

    // if(distortedUv.x > 1.0 || distortedUv.y > 1.0 || distortedUv.x < 0.0 || distortedUv.y < 0.0)
    //     discard;

    vec4 tex_color = texture(texture_data, distorted_uv) * 0.75;

    vec3 N = get_normal_from_map(distorted_uv);
    vec3 L = normalize(light_pos - out_vertex);
    vec3 V = normalize(eye_point - out_vertex);
    vec3 H = normalize(L + V);

    float ka = 0.2, kd = 0.75, ks = 0.55;
    float alpha = 20;

    out_color = tex_color;

    // vec3 colorFrag = texture(texture_data, out_texture).xyz;
    // out_color = vec4(colorFrag, 1.0);
    // out_color = vec4(0.0, 1.0, 0.0, 1.0);

    vec4 ambient = tex_color * ka;
    vec4 diffuse = tex_color * kd;
    vec4 specular = vec4(vec3(1.0, 1.0, 1.0) * ks, 1.0);

    float dist = length(L);
    float attenuation = 1.0 / (dist * dist);
    float dc = max(dot(N, L), 0.0);
    float sc = pow(max(dot(H, N), 0.0), alpha);

    vec3 tan_light_dir = normalize(compute_tbn(out_texture) * (light_pos - out_vertex));
    float shadow = calcShadow(distorted_uv, tan_light_dir);

    out_color += ambient;
    out_color += diffuse * dc * attenuation * shadow;
    out_color += specular * sc * attenuation * shadow;
}



// #version 330 core

// in vec3 out_vertex;
// in vec3 out_normal;
// in vec2 out_texture;

// uniform sampler2D texture_data;
// uniform sampler2D normal_data;

// out vec4 color;

// void main() {
    // vec3 d = out_vertex - vec3(0, 0, 0);

    // vec3 color_frag = texture2D(texture_data, out_texture).rgb;
    // color = vec4(color_frag, 1.0);

    // color = vec4(0.0, 1.0, 0.0, 1.0);
// }