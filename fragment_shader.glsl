#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 frag_coords;
    vec2 tex_coords;
    vec3 tangent_light_pos;
    vec3 tangent_view_pos;
    vec3 tangent_frag_pos;
} fs_in;

uniform sampler2D texture_data;
uniform sampler2D normal_data;
uniform sampler2D heightmap_data;

bool isNan(float val) {
    return (val <= 0.0 || 0.0 <= val) ? false : true;
}

void debug_float(float val) {
    if (val > 0) {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);  // right: green
    } else {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // wrong: red
    }
}

void debug_bool(bool val) {
    if (val == true) {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);  // right: green
    } else {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // wrong: red
    }
}

vec2 OcclusionParallax(vec2 tex_coords, vec3 view_dir) { 
    
    const float min_l = 8;
    const float max_l = 32;
    float num_layer = mix(max_l, min_l, abs(dot(vec3(0.0, 0.0, 1.0), view_dir)));  

    float depth_unit = 1.0 / num_layer;
    float curr_layer_depth = 0.0;
    vec2 P = view_dir.xy / view_dir.z * 0.01; 
    vec2 delta = P / num_layer;

    vec2  curr_tex_coords = tex_coords;
    float curr_heightmap_value = 1.0 - texture(heightmap_data, curr_tex_coords).r;
      
    while(curr_layer_depth < curr_heightmap_value) {
        curr_tex_coords -= delta;
        curr_heightmap_value = 1.0 - texture(heightmap_data, curr_tex_coords).r;  
        curr_layer_depth += depth_unit;  
    }
    
    vec2 prevtex_coords = curr_tex_coords + delta;
    float after  = curr_heightmap_value - curr_layer_depth;
    float before = 1.0 - texture(heightmap_data, prevtex_coords).r - curr_layer_depth + depth_unit;
 
    float weight = after / (after - before);
    vec2 finaltex_coords = prevtex_coords * weight + curr_tex_coords * (1.0 - weight);

    return finaltex_coords;
}

void main() {           
    vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_frag_pos);
    vec2 final_tex_coords = fs_in.tex_coords;

    if (isNan(view_dir.x) == true) {
        discard;
    } else {
        final_tex_coords = OcclusionParallax(fs_in.tex_coords, view_dir);       
        if(final_tex_coords.x > 1.0 || final_tex_coords.y > 1.0 || final_tex_coords.x < 0.0 || final_tex_coords.y < 0.0) {
            discard;
            
        } else {
            vec3 normal = texture(normal_data, final_tex_coords).rgb;
            normal = normalize(normal * 2.0 - 1.0);   
            vec3 color = texture(texture_data, final_tex_coords).rgb;
            
            vec3 ambient = 0.1 * color;
            vec3 light_dir = normalize(fs_in.tangent_light_pos - fs_in.tangent_frag_pos);

            float diff = max(dot(light_dir, normal), 0.0);
            vec3 diffuse = diff * color;

            vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_frag_pos);
            vec3 reflectDir = reflect(-light_dir, normal);
            vec3 halfwayDir = normalize(light_dir + view_dir);  
            vec3 specular = vec3(0.2) * pow(max(dot(normal, halfwayDir), 0.0), 32.0);

            FragColor = vec4(ambient + diffuse + specular, 1.0);
        }
    }
}