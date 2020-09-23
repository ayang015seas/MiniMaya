#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform mat4 u_Bind[100];
uniform mat4 u_Joint[100];

in ivec2 vs_Influence;
in vec2 vs_Weight;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

out vec3 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

void main()
{
    mat4 test = u_Joint[0];
    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
//    fs_Col = vec4(vs_Weight, 0.f, 0.f);
//    fs_Col = test[0];

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
    int ind1 = vs_Influence.x;
    int ind2 = vs_Influence.y;

    mat4 firstBindMatrix = u_Bind[ind1];
    mat4 secondBindMatrix = u_Bind[ind2];

    mat4 firstOverallTransformation = u_Joint[ind1];
    mat4 secondOverallTransformation = u_Joint[ind2];

    float weight1 = vs_Weight.x;
    float weight2 = vs_Weight.y;

    firstBindMatrix = firstOverallTransformation * firstBindMatrix * weight1;
    secondBindMatrix = secondOverallTransformation * secondBindMatrix * weight2;
            // model matrix. This is necessary to ensure the normals remain
    // int ind1 = vs_Influence[0];                                                        // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.


    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    vec4 newPos = firstBindMatrix * modelposition + secondBindMatrix * modelposition;

    //newPos += weight1 * modelposition * firstLocal * firstBindMatrix;
    //newPos += weight2 * modelposition * secondLocal * secondBindMatrix;
/*
    newPos += modelposition * firstBindMatrix;
    newPos += secondBindMatrix * modelposition;
    newPos.x = weight1;
    newPos.y = weight2;

    fs_Pos = modelposition.xyz;*/
    if (ind1 == 0 && ind2 == 0) {
        fs_Pos = modelposition.xyz;
    } else {
        fs_Pos = newPos.xyz;
    }
    // mat4 bindMat1 = uBind[vs_Influence[0]];
    if (ind1 == 0 && ind2 == 0) {
        gl_Position = u_ViewProj * modelposition;
    } else {
        gl_Position = u_ViewProj * newPos;// gl_Position is a built-in variable of OpenGL which is
    }
                                             // used to render the final positions of the geometry's vertices
}
