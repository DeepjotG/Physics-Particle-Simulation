#include <Particle.h>



void Particle::subdivisionIscosphere() {
        std::vector<float> tmpVertices;
        std::vector<unsigned int> tmpIndices;
        std::vector<float> newVs((subdivision + 1) * (subdivision + 2) / 2 * 3);
        const float* v1, * v2, * v3;          // ptr to original vertices of a triangle
        float newV1[3], newV2[3], newV3[3]; // new vertex positions
        unsigned int index = 0;
        int i, j, k;
        float a;                            // lerp alpha
        unsigned int i1, i2;                // indices

        // copy prev arrays
        tmpVertices = vertices;
        tmpIndices = indices;
        vertices.clear();
        indices.clear();

        // iterate each triangle of icosahedron
        for (i = 0; i < tmpIndices.size(); i += 3)
        {
            // get 3 vertices of a triangle of icosahedron
            v1 = &tmpVertices[tmpIndices[i] * 3];
            v2 = &tmpVertices[tmpIndices[i + 1] * 3];
            v3 = &tmpVertices[tmpIndices[i + 2] * 3];

            // add top vertex (x,y,z)
            newVs.clear();
            newVs.insert(newVs.end(), v1, v1 + 3);

            // find new vertices by subdividing edges
            for (j = 1; j <= subdivision; ++j)
            {
                a = (float)j / subdivision;    // lerp alpha

                // find 2 end vertices on the edges of the current row
                //          v1           //
                //         / \           // if N = 3,
                //        *---*          // lerp alpha = 1 / N
                //       / \ / \         //
                // newV1*---*---* newV2  // lerp alpha = 2 / N
                //     / \newV3/ \       //
                //    v2--*---*---v3     //
                interpolateVertex(v1, v2, a, radius, newV1);
                interpolateVertex(v1, v3, a, radius, newV2);
                for (k = 0; k <= j; ++k)
                {
                    if (k == 0)      // new vertex on the left edge, newV1
                        newVs.insert(newVs.end(), newV1, newV1 + 3);
                    else if (k == j) // new vertex on the right edge, newV2
                        newVs.insert(newVs.end(), newV2, newV2 + 3);
                    else            // new vertices between newV1 and newV2
                    {
                        a = (float)k / j;
                        interpolateVertex(newV1, newV2, a, radius, newV3);
                        newVs.insert(newVs.end(), newV3, newV3 + 3);
                    }
                }
            }

            // compute sub-triangles from new vertices
            for (j = 1; j <= subdivision; ++j)
            {
                for (k = 0; k < j; ++k)
                {
                    // indices
                    i1 = (j - 1) * j / 2 + k; // index from prev row
                    i2 = j * (j + 1) / 2 + k; // index from curr row

                    v1 = &newVs[i1 * 3];
                    v2 = &newVs[i2 * 3];
                    v3 = &newVs[(i2 + 1) * 3];
                    addVertices(vertices, v1, v2, v3);

                    // add indices
                    addIndices(indices, index, index + 1, index + 2);

                    index += 3; // next index

                    // if K is not the last, add adjacent triangle
                    if (k < (j - 1))
                    {
                        i2 = i1 + 1; // next of the prev row
                        v2 = &newVs[i2 * 3];
                        addVertices(vertices, v1, v3, v2);
                        addIndices(indices, index, index + 1, index + 2);
                        index += 3;
                    }
                }
            }
        }
	}


void Particle::interpolateVertex(const float* v1, const float* v2, float alpha, float radius, float* newV) {
        newV[0] = (1 - alpha) * v1[0] + alpha * v2[0];
        newV[1] = (1 - alpha) * v1[1] + alpha * v2[1];
        newV[2] = (1 - alpha) * v1[2] + alpha * v2[2];

        float length = sqrt(newV[0] * newV[0] + newV[1] * newV[1] + newV[2] * newV[2]);

        newV[0] = (newV[0] / length) * radius;
        newV[1] = (newV[1] / length) * radius;
        newV[2] = (newV[2] / length) * radius;

    }

void Particle::addIndices(vector<unsigned int>& Indices, unsigned int i1, unsigned int i2, unsigned int i3)
    {
        Indices.push_back(i1);
        Indices.push_back(i2);
        Indices.push_back(i3);
    };


void Particle::addVertices(vector<float>& vertices, const float* v1, const float* v2, const float* v3) {
        vertices.insert(vertices.end(), v1, v1 + 3);
        vertices.insert(vertices.end(), v2, v2 + 3);
        vertices.insert(vertices.end(), v3, v3 + 3);
    }



vector<float> Particle::createTriangleVertices() {
    const float PI = 3.1415926f;
    const float H_ANGLE = PI / 180 * 72;    // 72 degree = 360 / 5
    const float V_ANGLE = atanf(1.0f / 2);  // elevation = 26.565 degree

    std::vector<float> vertices(12 * 3);    // array of 12 vertices (x,y,z)
    int i1, i2;                             // indices
    float z, xy;                            // coords
    float hAngle1 = -PI / 2 - H_ANGLE / 2;  // start from -126 deg at 1st row
    float hAngle2 = -PI / 2;                // start from -90 deg at 2nd row

    // the first top vertex at (0, 0, r)
    vertices[0] = 0;
    vertices[1] = 0;
    vertices[2] = radius;

    // compute 10 vertices at 1st and 2nd rows
    for (int i = 1; i <= 5; ++i)
    {
        i1 = i * 3;         // index for 1st row
        i2 = (i + 5) * 3;   // index for 2nd row

        z = radius * sinf(V_ANGLE);            // elevaton
        xy = radius * cosf(V_ANGLE);            // length on XY plane

        vertices[i1] = xy * cosf(hAngle1);      // x
        vertices[i2] = xy * cosf(hAngle2);
        vertices[i1 + 1] = xy * sinf(hAngle1);  // y
        vertices[i2 + 1] = xy * sinf(hAngle2);
        vertices[i1 + 2] = z;                   // z
        vertices[i2 + 2] = -z;

        // next horizontal angles
        hAngle1 += H_ANGLE;
        hAngle2 += H_ANGLE;
    }

    // the last bottom vertex at (0, 0, -r)
    i1 = 11 * 3;
    vertices[i1] = 0;
    vertices[i1 + 1] = 0;
    vertices[i1 + 2] = -radius;

    return vertices;
}

