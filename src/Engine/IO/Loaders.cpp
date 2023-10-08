// @TODO: abstract platform layer away
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "engine/geometry/vertex.h"
#include "engine/core/logging.h"
#include <glm/gtx/quaternion.hpp>
#include "engine/geometry/mesh.h"
#include <engine/collision/CollisionMesh.h>
#include <glad/glad.h>
#include <engine/serialization/parsing/parser.h>
#include "engine/io/loaders.h"

#include <iomanip>

#include "engine/rvn.h"
#include "engine/io/display.h"
#include "engine/render/Shader.h"

void LoadTexturesFromAssetsFolder()
{
	auto Filenames = GetFilesINFolder(Paths::Textures);
	if (Filenames.size() > 0)
	{
		for (const auto& TextureFilename : Filenames)
		{
			unsigned int TextureID = LoadTextureFromFile(TextureFilename, Paths::Textures);

			if (TextureID == 0)
			{
				print("Texture '%s' could not be loaded.", TextureFilename.c_str());
				assert(false);
			}

			auto Index = TextureFilename.find('.');
			string TextureName = TextureFilename.substr(0, Index);

			string TextureType = "texture_diffuse";
			if (TextureName.find("normal") != string::npos)
			{
				TextureType = "texture_normal";
			}

			TextureCatalogue.insert({TextureName, RTexture{TextureID, TextureType, TextureFilename, TextureName}});
		}
	}
}

RMesh* LoadWavefrontObjAsMesh(const string& Path, const string& Filename, const string& Name, bool SetupGlData, RenderMethodEnum RenderMethod)
{
	/* Loads a model from the provided path and filename and add it to the Geometry_Catalogue with provided name */

	// @todo: Currently, we are loading each vertex using the .obj "f line", but that creates 3 vertex per faces.
	//    For faces that share an exact vertex (like in the case of a box where each face is a quad, sharing one
	//    triangle). This means we must use glDrawArrays instead of glDrawElements. One option is to code a custom
	//    exporter in blender to get just unique vertex data + indices, but maybe for now this is enough.

	const auto FullPath = Path + Filename + ".obj";
	auto P = Parser{FullPath};

	// @TODO: use a memory pool
	auto Mesh = new RMesh();

	vector<vec3> VPos;
	vector<vec2> VTexels;
	vector<vec3> VNormals;
	int FacesCount = 0;

	// Parses file
	while (P.NextLine())
	{
		P.ParseToken();
		const auto Attr = GetParsed<string>(P);

		if (!P.HasToken())
			continue;

		if (Attr == "m")
		{
			// ?
		}

		// vertex coordinates
		else if (Attr == "v")
		{
			P.ParseVec3();
			VPos.push_back(GetParsed<glm::vec3>(P));
		}

		// texture coordinates
		else if (Attr == "vt")
		{
			P.ParseVec2();
			VTexels.push_back(GetParsed<glm::vec2>(P));
		}

		else if (Attr == "vn")
		{
			P.ParseVec3();
			VNormals.push_back(GetParsed<glm::vec3>(P));
		}

		// construct faces
		else if (Attr == "f")
		{
			/* We are dealing now with the format: 'f 1/1/1 2/2/1 3/3/1 4/4/1' which follows the template 'vi/vt/vn' for each vertex.
			   We can then use the anticlockwise winding order to use the 123 341 index convention to get triangles from each f line. 
			   If faces are triangulated, then we will have only 3 vertices per face instead of 4 like in the example above.
			   Note about indices (mesh->indices): They refer not to the index 'vi' but to the index of the vertex in the mesh->vertices array.
			*/

			int NumberOfVertexesInFace = 0;

			// iterate over face's vertices. We expect either faces with 3 or 4 vertices only.
			while (true)
			{
				RVertex V;
				P.ParseAllWhitespace();

				// parses vertex index
				{
					P.ParseUint();
					if (!P.HasToken())
						break;

					uint Index = GetParsed<uint>(P) - 1;
					V.Position = VPos[Index];
				}

				P.ParseSymbol();
				if (P.HasToken() || GetParsed<char>(P) == '/')
				{
					// parses texel index
					P.ParseUint();
					if (P.HasToken() && !VTexels.empty())
					{
						uint Index = GetParsed<uint>(P) - 1;
						V.TexCoords = VTexels[Index];
					}

					// parses normal index
					P.ParseSymbol();
					if (P.HasToken() || GetParsed<char>(P) == '/')
					{
						P.ParseUint();
						if (P.HasToken() && !VNormals.empty())
						{
							uint Index = GetParsed<uint>(P) - 1;
							V.Normal = VNormals[Index];
						}
					}
				}

				// adds vertex to mesh, finally
				Mesh->Vertices.push_back(V);

				NumberOfVertexesInFace++;
			}


			// Creates the faces respecting winding order: Assumes that each face has unique vertices.
			// Index vector reads like: T0_v0, T0_v1, T0_v2, T1_v0, T1_v1, T1_v2, T2_v0 ... where T is triangle and v is vertex.
			// face's triangle #1
			int FirstVertexIndexInFace = Mesh->Vertices.size() - NumberOfVertexesInFace;
			Mesh->Indices.push_back(FirstVertexIndexInFace + 0);
			Mesh->Indices.push_back(FirstVertexIndexInFace + 1);
			Mesh->Indices.push_back(FirstVertexIndexInFace + 2);

			if (NumberOfVertexesInFace == 4)
			{
				// face's triangle #2
				Mesh->Indices.push_back(FirstVertexIndexInFace + 2);
				Mesh->Indices.push_back(FirstVertexIndexInFace + 3);
				Mesh->Indices.push_back(FirstVertexIndexInFace + 0);

				FacesCount += 2;
			}

			else if (NumberOfVertexesInFace > 4)
				fatal_error("mesh file %s.obj contain at least one face with unsupported ammount of vertices. Please triangulate or quadfy faces.", Filename.c_str())

			else if (NumberOfVertexesInFace < 3)
				fatal_error("mesh file %s.obj contain at least one face with 2 or less vertices. Please review the geometry.", Filename.c_str())

			else
				FacesCount++;
		}
	}

	Mesh->FacesCount = FacesCount;

	// load/computes tangents and bitangents
	if (!VTexels.empty())
		AttachExtraDataToMesh(Filename, Path, Mesh);

	// setup gl data
	if (SetupGlData)
		Mesh->SetupGLData();

	// sets render method for mesh
	Mesh->RenderMethod = static_cast<uint>(RenderMethod);


	// sets texture name and adds to catalogue
	string CatalogueName = !Name.empty() ? Name : Filename;
	Mesh->Name = CatalogueName;
	GeometryCatalogue.insert({CatalogueName, Mesh});

	return Mesh;
}

RCollisionMesh* LoadWavefrontObjAsCollisionMesh(string Path, string Filename, string Name)
{
	/* Loads a model from the provided path and filename and add it to the Collision_Geometry_Catalogue with provided name */

	const auto FullPath = Path + Filename + ".obj";
	Parser P{FullPath};

	// @TODO: Use a memory pool
	auto* CMesh = new RCollisionMesh;

	// Parses file
	while (P.NextLine())
	{
		P.ParseToken();
		const auto Attr = GetParsed<string>(P);

		if (!P.HasToken())
			continue;

		// vertex coordinates
		if (Attr == "v")
		{
			P.ParseVec3();
			CMesh->Vertices.push_back(GetParsed<glm::vec3>(P));
		}

		// construct faces
		else if (Attr == "f")
		{
			int NumberOfVertexesInFace = 0;
			uint IndexBuffer[4];
			// iterate over face's vertices
			while (true)
			{
				P.ParseAllWhitespace();

				// parses vertex index
				{
					P.ParseUint();
					if (!P.HasToken())
						break;

					// corrects from 1-first-element convention (from .obj file) to 0-first.
					const uint Index = GetParsed<uint>(P) - 1;
					// CMesh.index.push_back(index);
					IndexBuffer[NumberOfVertexesInFace] = Index;
				}

				// discard remaining face's vertex info
				P.ParseSymbol();
				P.ParseUint();
				P.ParseSymbol();
				P.ParseUint();

				NumberOfVertexesInFace++;
			}

			// Creates the faces respecting winding order: Assumes that each face has unique vertices.
			// Index vector reads like: T0_v0, T0_v1, T0_v2, T1_v0, T1_v1, T1_v2, T2_v0 ... where T is triangle and v is vertex position.
			// face's triangle #1
			CMesh->Indices.push_back(IndexBuffer[0]);
			CMesh->Indices.push_back(IndexBuffer[1]);
			CMesh->Indices.push_back(IndexBuffer[2]);

			if (NumberOfVertexesInFace == 4)
			{
				// face's triangle #2
				CMesh->Indices.push_back(IndexBuffer[2]);
				CMesh->Indices.push_back(IndexBuffer[3]);
				CMesh->Indices.push_back(IndexBuffer[0]);
			}
		}
	}

	// adds to catalogue
	const string CatalogueName = !Name.empty() ? Name : Filename;
	CollisionGeometryCatalogue.insert({CatalogueName, CMesh});

	return CMesh;
}

unsigned int LoadTextureFromFile(const string& Filename, const string& Directory, bool Gamma)
{
	// returns the gl_texture ID

	string Path;
	if (Path.substr(0, Path.length() - 2) == "/")
		Path = Directory + Filename;
	else
		Path = Directory + "/" + Filename;

	int Width, Height, NrComponents;
	unsigned char* Data = stbi_load(Path.c_str(), &Width, &Height, &NrComponents, 0);
	if (!Data)
	{
		print("Texture failed to load at path '%s'", Path.c_str())
		stbi_image_free(Data);
		return 0;
	}

	// sets color channel format
	GLenum Format;
	switch (NrComponents)
	{
		case 1:
			Format = GL_RED;
			break;
		case 3:
			Format = GL_RGB;
			break;
		case 4:
			Format = GL_RGBA;
			break;
		default: ;
	}

	unsigned int TextureId;
	glGenTextures(1, &TextureId);
	glBindTexture(GL_TEXTURE_2D, TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(Data);
	return TextureId;
}


StrVec GetFilesINFolder(string Directory)
{
	StrVec Filenames;
	string PathToFiles = Directory + "\\*";
	WIN32_FIND_DATA Files;
	HANDLE FindFilesHandle = FindFirstFile(PathToFiles.c_str(), &Files);

	if (FindFilesHandle == INVALID_HANDLE_VALUE)
	{
		print("Error: Invalid directory '%s' for finding files.", Directory.c_str());
		return Filenames;
	}

	do
	{
		int A = strcmp(Files.cFileName, ".");
		int B = strcmp(Files.cFileName, "..");
		if (!(A == 0 || B == 0))
			Filenames.push_back(Files.cFileName);
	} while (FindNextFile(FindFilesHandle, &Files));

	FindClose(FindFilesHandle);

	return Filenames;
}


void WriteMeshExtraDataFile(string Filename, RMesh* Mesh)
{
	const auto ExtraDataPath = Paths::Models + "extra_data/" + Filename + ".objplus";
	std::ofstream Writer{ExtraDataPath};

	if (!Writer.is_open())
		fatal_error("couldn't write mesh extra data.");

	Writer << std::fixed << std::setprecision(4);

	// write tangents
	For(Mesh->Vertices.size())
	{
		Writer << "vtan"
		<< " " << Mesh->Vertices[i].Tangent.x
		<< " " << Mesh->Vertices[i].Tangent.y
		<< " " << Mesh->Vertices[i].Tangent.z << "\n";
	}

	// write bitangents
	For(Mesh->Vertices.size())
	{
		Writer << "vbitan"
		<< " " << Mesh->Vertices[i].Bitangent.x
		<< " " << Mesh->Vertices[i].Bitangent.y
		<< " " << Mesh->Vertices[i].Bitangent.z << "\n";
	}

	Writer.close();

	Log(LOG_INFO, "Wrote mesh extra data for " + Filename + " mesh.");
}


void LoadMeshExtraData(string Filename, RMesh* Mesh)
{
	const auto ExtraDataPath = Paths::Models + "extra_data/" + Filename + ".objplus";
	Parser P{ExtraDataPath};

	uint VTanIndex = 0;
	uint VBitanIndex = 0;
	while (P.NextLine())
	{
		P.ParseToken();

		if (!P.HasToken())
			continue;

		auto Attr = GetParsed<string>(P);

		if (Attr == "vtan")
		{
			P.ParseVec3();
			Mesh->Vertices[VTanIndex++].Tangent = GetParsed<glm::vec3>(P);
		}

		else if (Attr == "vbitan")
		{
			P.ParseVec3();
			Mesh->Vertices[VBitanIndex++].Bitangent = GetParsed<glm::vec3>(P);
		}
	}
}


void AttachExtraDataToMesh(string Filename, string Filepath, RMesh* Mesh)
{
	/* Attach tangents and bitangents data to the mesh from a precomputation based on mesh vertices.
	   If the extra mesh data file is outdated from mesh file or inexistent, compute data and write to it.
	   If exists and is up to date, loads extra data from it.
	*/

	string ExtraDataPath = Paths::Models + "extra_data/" + Filename + ".objplus";
	string MeshPath = Filepath + Filename + ".obj";

	bool ComputeExtraData = false;

	//@todo: platform dependency
	WIN32_FIND_DATA FindDataExtraData;
	HANDLE FindHandle = FindFirstFileA(ExtraDataPath.c_str(), &FindDataExtraData);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		WIN32_FIND_DATA FindDataMesh;
		HANDLE FindHandleMesh = FindFirstFileA(MeshPath.c_str(), &FindDataMesh);
		if (FindHandleMesh == INVALID_HANDLE_VALUE)
			fatal_error("Unexpected: couldn't find file handle for mesh obj while checking for extra mesh data.")

		if (CompareFileTime(&FindDataMesh.ftLastWriteTime, &FindDataExtraData.ftLastWriteTime) == 1)
			ComputeExtraData = true;

		FindClose(FindHandleMesh);
	}
	else
		ComputeExtraData = true;


	if (ComputeExtraData)
	{
		Mesh->ComputeTangentsAndBitangents();
		WriteMeshExtraDataFile(Filename, Mesh);
	}
	else
	{
		LoadMeshExtraData(Filename, Mesh);
	}

	FindClose(FindHandle);
}


void LoadShaders()
{
	/* Parses shader program info from programs file, assembles each shader program and stores them into the
	   shaders catalogue. */

	Parser P{Paths::Shaders + "programs.csv"};

	// discard header
	P.NextLine();

	while (P.NextLine())
	{
		bool Error = false, MissingComma = false, HasGeometryShader = false;

		P.ParseToken();
		if (!P.HasToken())
			Error = true;
		const auto ShaderName = GetParsed<string>(P);

		P.ParseAllWhitespace();
		P.ParseSymbol();
		if (!P.HasToken())
			MissingComma = true;

		P.ParseAllWhitespace();
		P.ParseToken();
		if (!P.HasToken())
			Error = true;
		const auto VertexShaderName = GetParsed<string>(P);

		P.ParseAllWhitespace();
		P.ParseSymbol();
		if (!P.HasToken())
			MissingComma = true;

		P.ParseAllWhitespace();
		P.ParseToken();
		if (P.HasToken())
			HasGeometryShader = true;
		const auto GeometryShaderName = GetParsed<string>(P);

		P.ParseAllWhitespace();
		P.ParseSymbol();
		if (!P.HasToken())
			MissingComma = true;

		P.ParseAllWhitespace();
		P.ParseToken();
		if (!P.HasToken())
			Error = true;
		const auto FragmentShaderName = GetParsed<string>(P);

		// load shaders code and mounts program from parsed shader attributes
		RShader* Shader = HasGeometryShader ?
		CreateShaderProgram(ShaderName, VertexShaderName, GeometryShaderName, FragmentShaderName) :
		CreateShaderProgram(ShaderName, VertexShaderName, FragmentShaderName);

		ShaderCatalogue.insert({Shader->Name, Shader});

		if (Error)
			fatal_error("Error in shader programs file definition. Couldn't parse line %i.", P.LineCount);

		if (MissingComma)
			fatal_error("Error in shader programs file definition. There is a missing comma in line %i.", P.LineCount);
	}

	// setup for text shader
	auto TextShader = ShaderCatalogue.find("text")->second;
	TextShader->Use();
	TextShader->SetMatrix4("projection", glm::ortho(0.0f, GlobalDisplayState::ViewportWidth, 0.0f, GlobalDisplayState::ViewportHeight));
}
