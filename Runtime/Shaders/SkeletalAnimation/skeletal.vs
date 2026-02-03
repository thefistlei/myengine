#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;
layout(location = 5) in ivec4 a_BoneIDs; 
layout(location = 6) in vec4 a_Weights;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_FinalBonesMatrices[MAX_BONES];

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(a_BoneIDs[i] == -1) 
            continue;
        if(a_BoneIDs[i] >= MAX_BONES) 
        {
            totalPosition = vec4(a_Position, 1.0f);
            totalNormal = a_Normal;
            break;
        }
        vec4 localPosition = u_FinalBonesMatrices[a_BoneIDs[i]] * vec4(a_Position, 1.0f);
        totalPosition += localPosition * a_Weights[i];
        
        vec3 localNormal = mat3(u_FinalBonesMatrices[a_BoneIDs[i]]) * a_Normal;
        totalNormal += localNormal * a_Weights[i];
    }
    
    mat4 viewModel = u_View * u_Model;
    gl_Position = u_Projection * viewModel * totalPosition;
    
    v_TexCoord = a_TexCoord;
    v_Normal = mat3(transpose(inverse(u_Model))) * totalNormal;
    v_FragPos = vec3(u_Model * totalPosition);
}
