#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
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

vec2 OffsetParallax(vec2 texCoord, vec3 texDir3D )
{
    float parallaxScale = 0.1;
    float mapHeight     = texture( heightmap_data, texCoord.st ).r;
    //vec2  texCoordOffst = parallaxScale * mapHeight * texDir3D.xy / texDir3D.z;
    vec2  texCoordOffst = -parallaxScale * mapHeight * texDir3D.xy;
    return vec2(texCoord.xy + texCoordOffst.xy);
}

vec2 SteepParallax(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * 0.1; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(heightmap_data, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(heightmap_data, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    return currentTexCoords;
}

vec2 OcclusionParallax(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * 0.1; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentheightmap_dataValue = texture(heightmap_data, currentTexCoords).r;
      
    while(currentLayerDepth < currentheightmap_dataValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get heightmap_data value at current texture coordinates
        currentheightmap_dataValue = texture(heightmap_data, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentheightmap_dataValue - currentLayerDepth;
    float beforeDepth = texture(heightmap_data, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {           
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    if (isNan(viewDir.x) == true) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {

        texCoords = SteepParallax(fs_in.TexCoords, viewDir);       
        if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        } else {
            // // obtain normal from normal map
            // vec3 normal = texture(normal_data, texCoords).rgb;
            // normal = normalize(normal * 2.0 - 1.0);   
        
            
            vec3 color = texture(texture_data, texCoords).rgb;
            
            // vec3 ambient = 0.1 * color;  // ambient
            
            // vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
            // float diff = max(dot(lightDir, normal), 0.2);
            // vec3 diffuse = diff * color;  // diffuse
            
            // vec3 reflectDir = reflect(-lightDir, normal);
            // vec3 halfwayDir = normalize(lightDir + viewDir);  
            // float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
            // vec3 specular = vec3(0.2) * spec; // specular    

            vec3 aa = texture(normal_data, fs_in.TexCoords).rgb;
            FragColor = vec4(color, 1.0);
        }
    }
}