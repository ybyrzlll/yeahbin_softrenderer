#ifndef MESH_H
#define MESH_H

#include "mMath.h"
#include <vector>
#include <iostream>

class Mesh {
public:
	//Per vertex values
	int numVertices = 0;
	std::vector<Vector3f> vertices;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> texels;
	std::vector<Vector3f> tangents;
	std::vector<Vector3f> biTangents;

	//Per face values
	int numFaces = 0;
	std::vector<Vector3f> fNormals; //Normals for the whole face
	std::vector<Vector3i> vertexIndices;
	std::vector<Vector3i> textureIndices;
	std::vector<Vector3i> normalsIndices;

	void buildFacetNormal() {
		for (int i = 0; i < numFaces; i++) {
			Vector3i indices = vertexIndices[i];
			Vector3f N1 = vertices[indices.data[1]] - vertices[indices.data[0]];
			Vector3f N2 = vertices[indices.data[2]] - vertices[indices.data[0]];
			Vector3f N3 = N1.crossProduct(N2);
			fNormals.push_back(N3.normalized());
		}
	};
};

#endif