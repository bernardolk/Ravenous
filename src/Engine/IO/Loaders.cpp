#include "engine/io/loaders.h"

// @TODO: abstract platform layer away
#include <windows.h>
#include <iomanip>
#include <glad/glad.h>
#include <fstream>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "Engine/Geometry/Vertex.h"
#include "Engine/Geometry/Mesh.h"
#include "Engine/Collision/CollisionMesh.h"
#include "Engine/Serialization/Parsing/Parser.h"
#include "Engine/Rvn.h"
#include "Engine/IO/Display.h"
#include "Engine/Render/Shader.h"

void LoadModels()
{
	auto Filenames = GetFilesInFolder(Paths::Models);
	if (!Filenames.empty())
	{
		for (const auto& ModelFilename : Filenames)
		{
			// check if filename is not a folder (we want only .obj files here)
			auto ExtensionTest = ModelFilename.substr(ModelFilename.length() - 3);
			if (ExtensionTest != "obj")
				continue;
			
			auto ModelName = ModelFilename.substr(0, ModelFilename.length() - 4);

			if (DoesFileExist(Paths::MeshExports + ModelName + ".rmesh")) {
				ImportMeshBinary(ModelName);
			}
			else {
				auto* Mesh = LoadWavefrontObjAsMesh(ModelName);
				ExportMeshBinary(Mesh);
			}
		}
	}
	
	//TEXT
	{
		RGLData TextGlData;
		glGenVertexArrays(1, &TextGlData.VAO);
		glGenBuffers(1, &TextGlData.VBO);
		glBindVertexArray(TextGlData.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, TextGlData.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		auto* TextMesh = new RMesh();
		TextMesh->Name = "text";
		TextMesh->GLData = TextGlData;
		GeometryCatalogue.insert({TextMesh->Name, TextMesh});
	}

	// SLOPE
	// with Z coming at the screen, X to the right, slope starts at x=0 high and goes low on x=1
	std::vector<RVertex> slope_vertex_vec = {
	// bottom
	RVertex{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.5f, 0.5f)}, //0
	RVertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 0.5f)}, //1
	RVertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(1.0f, 1.0f)}, //2
	RVertex{vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f), vec2(0.5f, 1.0f)}, //3
	// right   
	RVertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 0.5f)}, //4
	RVertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec2(0.5f, 0.5f)}, //5
	RVertex{vec3(0.0f, 1.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec2(1.0f, 1.0f)}, //6
	RVertex{vec3(0.0f, 1.0f, 1.0f), vec3(0.5f, 0.5f, 0.0f), vec2(0.5f, 1.0f)}, //7
	// front       
	RVertex{vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f)}, //8
	RVertex{vec3(1.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.5f, 0.0f)}, //9
	RVertex{vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.5f)}, //10
	// back
	RVertex{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.0f)}, //11
	RVertex{vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.0f, 0.5f)}, //12
	RVertex{vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec2(0.5f, 0.0f)}, //13
	// left
	RVertex{vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)}, //14
	RVertex{vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.5f, 0.0f)}, //15
	RVertex{vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.0f, 0.5f)}, //16
	RVertex{vec3(0.0f, 1.0f, 1.0f), vec3(-1.0f, 0.0f, 0.0f), vec2(0.5f, 0.5f)}, //17
	};

	std::vector<uint> slope_vertex_indices =
	{
	0, 1, 2, 2, 3, 0,       // bottom face
	8, 9, 10,               // front
	11, 12, 13,             // back
	14, 15, 16, 17, 16, 15, // left face
	4, 5, 6, 6, 7, 4        // right face (slope)
	};

	auto* SlopeMesh = new RMesh();
	SlopeMesh->Name = "slope";
	SlopeMesh->Vertices = slope_vertex_vec;
	SlopeMesh->Indices = slope_vertex_indices;
	SlopeMesh->RenderMethod = GL_TRIANGLES;
	SlopeMesh->SetupGLData();
	GeometryCatalogue.insert({SlopeMesh->Name, SlopeMesh});
}


void LoadTexturesFromAssetsFolder()
{
	auto Filenames = GetFilesInFolder(Paths::Textures);
	if (Filenames.size() > 0)
	{
		for (const auto& TextureFilename : Filenames)
		{
			// check if filename is not a folder
			auto ExtensionTest = TextureFilename.substr(TextureFilename.length() - 4);
			if (ExtensionTest != ".jpg" && ExtensionTest != ".png" && ExtensionTest != ".bmp" && ExtensionTest != "jpeg")
				continue;
			
			unsigned int TextureID = LoadTextureFromFile(TextureFilename, Paths::Textures);

			if (TextureID == 0)
			{
				Log("Texture '%s' could not be loaded.", TextureFilename.c_str())
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

RMesh* LoadWavefrontObjAsMesh(const string& Filename)
{
	// Loads a model from the provided path and filename and add it to the Geometry_Catalogue with provided name
	const auto FullPath = Paths::Models + Filename + ".obj";
	auto P = Parser{FullPath};

	Log("Loading and parsing Wavefront OBJ ('%s')", Filename.c_str())

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

		if (!P.HasToken()) continue;

		// vertex coordinates
		if (Attr == "v") {
			P.ParseVec3();
			VPos.push_back(GetParsed<glm::vec3>(P));
		}
		// texture coordinates
		else if (Attr == "vt") {
			P.ParseVec2();
			VTexels.push_back(GetParsed<glm::vec2>(P));
		}
		else if (Attr == "vn") {
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

			else if (NumberOfVertexesInFace > 4) {
				FatalError("mesh file %s.obj contain at least one face with unsupported ammount of vertices. Please triangulate or quadfy faces.", Filename.c_str())
			}
			else if (NumberOfVertexesInFace < 3) {
				FatalError("mesh file %s.obj contain at least one face with 2 or less vertices. Please review the geometry.", Filename.c_str())
			}
			else {
				FacesCount++;
			}
		}
	}

	Mesh->FacesCount = FacesCount;

	// load/computes tangents and bitangents
	if (!VTexels.empty()) {
		AttachExtraDataToMesh(Filename, Mesh);
	}

	Mesh->SetupGLData();
	Mesh->Name = Filename;
	GeometryCatalogue.insert({Filename, Mesh});
	return Mesh;
}

RCollisionMesh* LoadWavefrontObjAsCollisionMesh(const string& Filename)
{
	// Loads a model from the provided path and filename and add it to the Collision_Geometry_Catalogue with provided name 
	const auto FullPath = Paths::CollisionMeshes + Filename + ".obj";
	Parser P{FullPath};

	// @TODO: Use a memory pool
	auto* CMesh = new RCollisionMesh;
	CMesh->Name = Filename;
	
	// Parses file
	while (P.NextLine())
	{
		P.ParseToken();
		const auto Attr = GetParsed<string>(P);

		if (!P.HasToken())
			continue;

		// vertex coordinates
		if (Attr == "v") {
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

			if (NumberOfVertexesInFace == 4) {
				// face's triangle #2
				CMesh->Indices.push_back(IndexBuffer[2]);
				CMesh->Indices.push_back(IndexBuffer[3]);
				CMesh->Indices.push_back(IndexBuffer[0]);
			}
		}
	}

	// adds to catalogue
	CollisionGeometryCatalogue.insert({Filename, CMesh});

	return CMesh;
}

unsigned int LoadTextureFromFile(const string& Filename, const string& Directory)
{
	string Path =  Directory + Filename;

	int Width, Height, NrComponents;
	unsigned char* Data = stbi_load(Path.c_str(), &Width, &Height, &NrComponents, 0);
	if (!Data) {
		Log("Texture failed to load at path '%s'", Path.c_str())
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
		default:
			Format = GL_RGB;
			Log("Texture '%s' has invalid Color Channel Format.", Filename.c_str())
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

vector<string> GetFilesInFolder(const string& Directory)
{
	vector<string> Filenames;
	string PathToFiles = Directory + "\\*";
	WIN32_FIND_DATA Files;
	HANDLE FindFilesHandle = FindFirstFile(PathToFiles.c_str(), &Files);

	if (FindFilesHandle == INVALID_HANDLE_VALUE) {
		Log("Error: Invalid directory '%s' for finding files.", Directory.c_str());
		return Filenames;
	}

	do {
		int A = strcmp(Files.cFileName, ".");
		int B = strcmp(Files.cFileName, "..");
		if (!(A == 0 || B == 0)) {
			Filenames.push_back(Files.cFileName);
		}
	} while (FindNextFile(FindFilesHandle, &Files));

	FindClose(FindFilesHandle);

	return Filenames;
}

bool DoesFileExist(const string& Filepath)
{
	WIN32_FIND_DATA Files;
	HANDLE FindFilesHandle = FindFirstFile(Filepath.c_str(), &Files);
	return FindFilesHandle != INVALID_HANDLE_VALUE;
}

void WriteMeshExtraDataFile(string Filename, RMesh* Mesh)
{
	const auto ExtraDataPath = Paths::Models + "extra_data/" + Filename + ".objplus";
	std::ofstream Writer{ExtraDataPath};

	if (!Writer.is_open()) {
		FatalError("couldn't write mesh extra data.")
	}

	Writer << std::fixed << std::setprecision(4);

	// write tangents
	for(auto& Vertex : Mesh->Vertices)
	{
		Writer << "vtan"
		<< " " << Vertex.Tangent.x
		<< " " << Vertex.Tangent.y
		<< " " << Vertex.Tangent.z << "\n";
	}

	// write bitangents
	for(auto& Vertex : Mesh->Vertices)
	{
		Writer << "vbitan"
		<< " " << Vertex.Bitangent.x
		<< " " << Vertex.Bitangent.y
		<< " " << Vertex.Bitangent.z << "\n";
	}

	Writer.close();

	Log("Wrote mesh extra data for '%s' mesh.", Filename.c_str());
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

		if (Attr == "vtan") {
			P.ParseVec3();
			Mesh->Vertices[VTanIndex++].Tangent = GetParsed<glm::vec3>(P);
		}

		else if (Attr == "vbitan") {
			P.ParseVec3();
			Mesh->Vertices[VBitanIndex++].Bitangent = GetParsed<glm::vec3>(P);
		}
	}
}

void AttachExtraDataToMesh(string Filename, RMesh* Mesh)
{
	/* Attach tangents and bitangents data to the mesh from a precomputation based on mesh vertices.
	   If the extra mesh data file is outdated from mesh file or inexistent, compute data and write to it.
	   If exists and is up to date, loads extra data from it.
	*/

	string ExtraDataPath = Paths::Models + "extra_data/" + Filename + ".objplus";
	string MeshPath = Paths::Models + Filename + ".obj";

	bool ComputeExtraData = false;

	//@todo: platform dependency
	WIN32_FIND_DATA FindDataExtraData;
	HANDLE FindHandle = FindFirstFileA(ExtraDataPath.c_str(), &FindDataExtraData);
	if (FindHandle != INVALID_HANDLE_VALUE)
	{
		WIN32_FIND_DATA FindDataMesh;
		HANDLE FindHandleMesh = FindFirstFileA(MeshPath.c_str(), &FindDataMesh);
		if (FindHandleMesh == INVALID_HANDLE_VALUE) {
			FatalError("Unexpected: couldn't find file handle for mesh obj while checking for extra mesh data.")
		}
		if (CompareFileTime(&FindDataMesh.ftLastWriteTime, &FindDataExtraData.ftLastWriteTime) == 1) {
			ComputeExtraData = true;
		}

		FindClose(FindHandleMesh);
	}
	else {
		ComputeExtraData = true;
	}


	if (ComputeExtraData) {
		Mesh->ComputeTangentsAndBitangents();
		WriteMeshExtraDataFile(Filename, Mesh);
	}
	else {
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
		if (!P.HasToken()) Error = true;
		const auto ShaderName = GetParsed<string>(P);

		P.ParseAllWhitespace(); P.ParseSymbol();
		if (!P.HasToken()) MissingComma = true;

		P.ParseAllWhitespace(); P.ParseToken();
		if (!P.HasToken()) Error = true;
		const auto VertexShaderName = GetParsed<string>(P);

		P.ParseAllWhitespace(); P.ParseSymbol();
		if (!P.HasToken())
			MissingComma = true;

		P.ParseAllWhitespace(); P.ParseToken();
		if (P.HasToken())
			HasGeometryShader = true;
		const auto GeometryShaderName = GetParsed<string>(P);

		P.ParseAllWhitespace(); P.ParseSymbol();
		if (!P.HasToken()) MissingComma = true;

		P.ParseAllWhitespace(); P.ParseToken();
		if (!P.HasToken()) Error = true;
		const auto FragmentShaderName = GetParsed<string>(P);

		// load shaders code and mounts program from parsed shader attributes
		RShader* Shader = HasGeometryShader ?
		CreateShaderProgram(ShaderName, VertexShaderName, GeometryShaderName, FragmentShaderName) :
		CreateShaderProgram(ShaderName, VertexShaderName, FragmentShaderName);

		ShaderCatalogue.insert({Shader->Name, Shader});

		if (Error) FatalError("Error in shader programs file definition. Couldn't parse line %i.", P.LineCount)
		if (MissingComma) FatalError("Error in shader programs file definition. There is a missing comma in line %i.", P.LineCount)
	}

	// setup for text shader
	auto TextShader = ShaderCatalogue.find("text")->second;
	TextShader->Use();
	TextShader->SetMatrix4("projection", glm::ortho(0.0f, GlobalDisplayState::ViewportWidth, 0.0f, GlobalDisplayState::ViewportHeight));
}

void ExportWavefrontCollisionMesh(RCollisionMesh* CollisionMesh)
{
	std::ofstream Writer(Paths::CollisionMeshes + CollisionMesh->Name + ".obj");
	if (!Writer.is_open()) {
		Log("Saving config file failed.\n")
	}

	Writer << "# Generated Collision Mesh: " << CollisionMesh->Name << "\n";
	Writer << std::fixed << std::setprecision(6);

	for (vec3 Vertex : CollisionMesh->Vertices) {
		Writer << "v " << Vertex.x << " " << Vertex.y << " " << Vertex.z << "\n";
	}

	for (int i = 0; i < CollisionMesh->Indices.size() / 3; i++)
	{
		// .obj indices are 1-indexed, so we add 1 to export
		uint I1 = CollisionMesh->Indices[i * 3 + 0] + 1;
		uint I2 = CollisionMesh->Indices[i * 3 + 1] + 1;
		uint I3 = CollisionMesh->Indices[i * 3 + 2] + 1;

		// export zeros in texel and normal index positions to signal pretty sure that this is _not_ a regular .obj file
		Writer << "f " << I1 << "/0/0 " << I2 << "/0/0 " << I3 << "/0/0\n"; 
	}
	
	Writer.close();
}

void ExportMeshBinary(RMesh* Mesh)
{
	const string ExportFilepath = Paths::MeshExports + Mesh->Name + ".rmesh";

	const uint VerticesCount = Mesh->Vertices.size();
	const uint IndicesCount = Mesh->Indices.size();
	const uint FacesCount = Mesh->FacesCount;

	FILE* File;
	int ErrnoOpen = fopen_s(&File, ExportFilepath.c_str(), "wb");
	if (!File  || ErrnoOpen != 0) {
		Log("Couldn't open binary mesh data for '%s'. Error code: %i", Mesh->Name.c_str(), ErrnoOpen)
	}

	Log("Exporting binary mesh data (%s)", Mesh->Name.c_str())
	
	// Write file format header
	{
		constexpr uint HeaderSize = 12;
		char HeaderBuffer[HeaderSize];
		auto* p = (uint*) &HeaderBuffer;
		*p = VerticesCount; p++;
		*p = IndicesCount; p++;
		*p = FacesCount; p++;

		uint64 BytesWritten = fwrite(&HeaderBuffer, 1, HeaderSize, File);
		if (BytesWritten != HeaderSize) {
			Log("Wrote different number of bytes to disk while exporting mesh binary data (Header).") DEBUG_BREAK
		}
	}

	// Write Vertices
	{
		uint ItemsWritten = fwrite(Mesh->Vertices.data(), sizeof(RVertex), VerticesCount, File);
		if (ItemsWritten != VerticesCount) {
			Log("Wrote different number of items to disk while exporting mesh binary data (Vertices).") DEBUG_BREAK
		}
	}

	// Write Indices
	{
		uint64 ItemsWritten = fwrite(Mesh->Indices.data(), sizeof(uint), IndicesCount, File);
		if (ItemsWritten != IndicesCount) {
			Log("Wrote different number of items to disk while exporting mesh binary data (Indices).") DEBUG_BREAK
		}
	}

	int ErrnoClose = fclose(File);
	if (ErrnoOpen != 0) {
		Log("Error closing filestream while exporting binary mesh data for mesh '%s'. Error code: %i", Mesh->Name.c_str(), ErrnoClose) DEBUG_BREAK
	}
}

void ImportMeshBinary(const string& ModelName)
{
	const string Filename = ModelName + ".rmesh";
	const string ImportFilepath = Paths::MeshExports + Filename;
	
	FILE* File;
	int ErrnoOpen = fopen_s(&File, ImportFilepath.c_str(), "rb");
	if (!File  || ErrnoOpen != 0) {
		Log("Couldn't open binary mesh data for '%s'. Error code: %i", Filename.c_str(), ErrnoOpen)
	}

	auto* Mesh = new RMesh;
	uint VertexCount = 0;
	uint IndicesCount = 0;
	// Reserve space for vertexes
	{
		if (uint ItemsRead = fread(&VertexCount, sizeof(uint), 1, File); ItemsRead != 1) {
			Log("Error: Read different number of items to disk while importing mesh binary data (Vertices).") DEBUG_BREAK
		}
		if (VertexCount <= 0) { Log("ImportMeshBinary: Invalid VertexCount Reading mesh '%s' binary data.", Filename.c_str()); delete Mesh; return; }
		Mesh->Vertices.resize(VertexCount);
	}
	// Reserve space for indices
	{
		if (uint ItemsRead = fread(&IndicesCount, sizeof(uint), 1, File); ItemsRead != 1) {
			Log("Error: Read different number of items to disk while importing mesh binary data (Indices).") DEBUG_BREAK
		}
		if (VertexCount <= 0) { Log("ImportMeshBinary: Invalid VertexCount Reading mesh '%s' binary data.", Filename.c_str()); delete Mesh; return; }
		Mesh->Indices.resize(IndicesCount);
	}
	// Read faces count
	{
		uint FacesCount = 0;
		if (uint ItemsRead = fread(&FacesCount, sizeof(uint), 1, File); ItemsRead != 1) {
			Log("Error: Read different number of items to disk while importing mesh binary data (Indices).") DEBUG_BREAK
		}
		Mesh->FacesCount = FacesCount;
	}
	// Read vertices data
	{
		if (uint ItemsRead = fread(Mesh->Vertices.data(), sizeof(RVertex), VertexCount, File); ItemsRead != VertexCount) {
			Log("Error: Read different number of items to disk while importing mesh binary data (Vertices).") DEBUG_BREAK
		}
	}
	// Read indices data
	{
		if (uint ItemsRead = fread(Mesh->Indices.data(), sizeof(uint), IndicesCount, File); ItemsRead != IndicesCount) {
			Log("Error: Read different number of items to disk while importing mesh binary data (Vertices).") DEBUG_BREAK
		}
	}

	Mesh->Name = ModelName;
	Mesh->SetupGLData();
	GeometryCatalogue.insert({ModelName, Mesh});
}