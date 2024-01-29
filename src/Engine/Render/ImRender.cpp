#include "ImRender.h"
#include "Shader.h"
#include "engine/camera/camera.h"
#include "engine/geometry/mesh.h"
#include "engine/geometry/triangle.h"
#include "engine/entities/Entity.h"
#include <glad/glad.h>
#include "engine/geometry/vertex.h"

// ==============================
//	Init        
// ==============================
void RImDraw::Init()
{
	List = new RImDrawElement[ImBufferSize];
	for (int I = 0; I < ImBufferSize; I++)
	{
		EmptySlot(I);
		List[I].Mesh.SetupGLBuffers();
	}
}

// ==============================
//	Update        
// ==============================
void RImDraw::Update(float FrameDuration)
{
	for (int I = 0; I < ImBufferSize; I++)
	{
		auto& Obj = List[I];
		if (Obj.Empty)
			continue;

		Obj.Duration -= (int) (FrameDuration * 1000.0f);
		if (Obj.Duration <= 0)
			EmptySlot(I);
	}
}

// ==============================
//	Render          
// ==============================
void RImDraw::Render(RCamera* Camera)
{
	RShader* ImPointShader = ShaderCatalogue.find("immediate_point")->second;
	RShader* ImMeshShader = ShaderCatalogue.find("im_mesh")->second;
	RShader* Shader = ImPointShader;
	for (int I = 0; I < ImBufferSize; I++)
	{
		auto& Obj = List[I];
		if (Obj.Empty)
			continue;

		// vec3 Color = IsEqual(Obj.RRenderOptions.Color.x, -1) ? vec3(0.9, 0.2, 0.0) : Obj.RRenderOptions.Color;

		if (Obj.IsMesh)
		{
			Shader = ImMeshShader;
			Shader->Use();
			if (!Obj.IsMultplByMatmodel) {
				auto MatModel = GetMatModel(Obj.Position, Obj.Rotation, Obj.Scale);
				Shader->SetMatrix4("model", MatModel);
			}
			else {
				Shader->SetMatrix4("model", Mat4Identity);
			}
		}
		else {
			Shader->Use();
		}

		Shader->SetMatrix4("view", Camera->MatView);
		Shader->SetMatrix4("projection", Camera->MatProjection);
		Shader->SetFloat("opacity", Obj.RRenderOptions.Opacity);
		Shader->SetFloat3("Color", Obj.RRenderOptions.Color);

		RenderMesh(&(List[I].Mesh), Obj.RRenderOptions);
	}
}

void RImDraw::AddEntity(uint Hash, EEntity* Entity, int Duration, RRenderOptions Opts)
{
	AddMesh(Hash, Entity->Mesh, Duration, Opts);
}


// ==============================
//	Add Mesh           
// ==============================
void RImDraw::AddMesh(uint _hash, RMesh* Mesh, int Duration, RRenderOptions Opts)
{
	int Index = FindDrawElement(_hash);
	if (Index != -1) {
		UpdateMeshDuration(Index, Duration);
		return;
	}

	Index = GetNewSlotIndex();
	if (Index == -1) return;
	
	auto& Obj = List[Index];
	Obj.Hash = _hash;
	Obj.Duration = Duration;
	Obj.IsMultplByMatmodel = true;
	Obj.Empty = false;

	SetMesh(Index, Mesh, Opts);
}

// ==============================
//	Add Mesh With Transform           
// ==============================
void RImDraw::AddMeshWithTransform(uint _hash, RMesh* Mesh, vec3 Position, vec3 Rotation, vec3 Scale, int Duration, RRenderOptions Opts)
{
	int Index = FindDrawElement(_hash);
	if (Index != -1) {
		UpdateMeshDuration(Index, Duration);
		return;
	}

	Index = GetNewSlotIndex();
	if (Index == -1) return;

	auto& Obj = List[Index];
	Obj.Hash = _hash;
	Obj.Duration = Duration;
	Obj.Position = Position;
	Obj.Rotation = Rotation;
	Obj.Scale = Scale;
	Obj.Empty = false;

	SetMesh(Index, Mesh, Opts);
}

// ==============================
//	Add Mesh At Position           
// ==============================
void RImDraw::AddMeshAtPosition(uint Hash, EEntity* Entity, vec3 Position, int Duration, RRenderOptions Opts)
{
	AddMeshWithTransform(Hash, Entity->Mesh, Position, Entity->Rotation, Entity->Scale, Duration, Opts);
}

// ==============================
//	Add Collision Mesh   
// ==============================
void RImDraw::AddCollisionMesh(uint _hash, RCollisionMesh* CollisionMesh, int Duration, RRenderOptions Opts)
{
	int Index = FindDrawElement(_hash);
	if (Index != -1) {
		UpdateMeshDuration(Index, Duration);
		return;
	}

	Index = GetNewSlotIndex();
	if (Index == -1) return;
	
	auto& Obj = List[Index];
	Obj.Hash = _hash;
	Obj.Duration = Duration;
	Obj.IsMultplByMatmodel = true;
	Obj.RRenderOptions = Opts;
	Obj.IsMesh = true;
	Obj.Empty = false;

	// Copy data from collision mesh into imdraw mesh inside slot
	Obj.Mesh = RMesh{};
	for (auto& Vec : CollisionMesh->Vertices) {
		RVertex Vertex;
		Vertex.Position = Vec;
		Obj.Mesh.Vertices.push_back(Vertex);
	}
	Obj.Mesh.Indices = CollisionMesh->Indices;

	Obj.Mesh.SetupGLData();
	Obj.Mesh.SendDataToGLBuffer();
}

// ==============================
//	Add Line           
// ==============================
void RImDraw::AddLine(uint _hash, vec3 PointA, vec3 PointB, int Duration, vec3 Color, float LineWidth, bool AlwaysOnTop)
{
	RRenderOptions Opts;
	Opts.LineWidth = LineWidth;
	Opts.AlwaysOnTop = AlwaysOnTop;
	Opts.Color = Color;
	Opts.DontCullFace = true;

	vector<RVertex> Vertices {RVertex{PointA}, RVertex{PointB}};
	AddOrUpdateDrawElement(_hash, Vertices, Duration, Opts, GL_LINES);
}

// ==============================
//	Add Line Loop           
// ==============================
void RImDraw::AddLineLoop(uint _hash, vector<RVertex>& Vertices, int Duration, vec3 Color, RRenderOptions Opts)
{
	AddOrUpdateDrawElement(_hash, Vertices, Duration, Opts, GL_LINE_LOOP);
}

// ==============================
//	Add Point           
// ==============================
void RImDraw::AddPoint(uint _hash, vec3 Point, int Duration, vec3 Color, float PointSize, bool AlwaysOnTop)
{
	RRenderOptions Opts;
	Opts.PointSize = PointSize;
	Opts.AlwaysOnTop = AlwaysOnTop;
	Opts.Color = Color;

	vector<RVertex> Vertices {RVertex{Point}};
	AddOrUpdateDrawElement(_hash, Vertices, Duration, Opts, GL_POINTS);
}

// ==============================
//	Add Quad  
// ==============================
void RImDraw::AddQuad(uint _hash, RQuad Quad, int Duration, vec3 Color, RRenderOptions Opts)
{
	vector<RVertex> Vertices;
	Vertices.emplace_back(Quad.T1.A);
	Vertices.emplace_back(Quad.T1.B);
	Vertices.emplace_back(Quad.T1.C);
	Vertices.emplace_back(Quad.T2.A);
	Vertices.emplace_back(Quad.T2.B);
	Vertices.emplace_back(Quad.T2.C);
	
	AddLineLoop(_hash, Vertices, Duration, Color, Opts);
}


// ==============================
//	Add Vertex List           
// ==============================
void RImDraw::AddVertexList(uint _hash, vector<RVertex>& VertexVec, int Duration, RRenderOptions Opts, GLenum DrawMethod)
{
	AddOrUpdateDrawElement(_hash, VertexVec, Duration, Opts, DrawMethod);
}

// ===========================
//	Private Methods
// ===========================

void RImDraw::EmptySlot(int Index)
{
	auto& Obj = List[Index];
	Obj.Mesh.Indices.clear();
	Obj.Mesh.Vertices.clear();
	Obj.Empty = true;
	Obj.Hash = 0;
	Obj.Duration = 0;
	Obj.IsMesh = false;
	Obj.Scale = vec3(0);
	Obj.Position = vec3(0);
	Obj.Rotation = vec3(0);
	Obj.IsMultplByMatmodel = false;
}

int RImDraw::FindDrawElement(uint Hash)
{
	for (int i = 0; i < ImBufferSize; i++) {
		if (List[i].Hash == Hash) {
			return i;
		}
	}

	return -1;
}

int RImDraw::GetNewSlotIndex()
{
	int FirstEmptyIndex = -1;
	for (int i = 0; i < ImBufferSize; i++)
	{
		if (List[i].Empty) {
			FirstEmptyIndex = i;
			break;
		}
	}

	if (FirstEmptyIndex == -1) {
		Log("ImDraw buffer is full.");
	}
	
	return FirstEmptyIndex;
}

void RImDraw::SetMeshFromVertices(int Index, vector<RVertex>& Vertices, GLenum DrawMethod, RRenderOptions Opts)
{
	auto& Obj = List[Index];
	Obj.Mesh.Vertices = Vertices;
	Obj.Mesh.RenderMethod = DrawMethod;
	Obj.RRenderOptions = Opts;
	Obj.Empty = false;
	Obj.Mesh.SendDataToGLBuffer();
}

void RImDraw::SetMesh(int Index, RMesh* Mesh, RRenderOptions Opts)
{
	auto& Obj = List[Index];
	Obj.Mesh = *Mesh;
	Obj.RRenderOptions = Opts;
	Obj.IsMesh = true;
	Obj.Empty = false;
	Obj.Mesh.SendDataToGLBuffer();
}

void RImDraw::UpdateMeshTransform(int Index, vec3 Position, vec3 Rotation, vec3 Scale, vec3 Color, int Duration)
{
	auto& Obj = List[Index];
	Obj.RRenderOptions.Color = Color;
	Obj.Duration = Duration;
	Obj.Position = Position;
	Obj.Rotation = Rotation;
	Obj.Scale = Scale;
}

void RImDraw::UpdateMeshDuration(int Index, int Duration)
{
	auto& Obj = List[Index];
	Obj.Duration = Duration;
}

void RImDraw::UpdateMeshColor(int Index, vec3 Color)
{
	auto& Obj = List[Index];
	Obj.RRenderOptions.Color = Color;
}

mat4 RImDraw::GetMatModel(vec3 Position, vec3 Rotation, vec3 Scale)
{
	glm::mat4 Model = translate(Mat4Identity, Position);
	Model = rotate(Model, glm::radians(Rotation.x), vec3(1.0f, 0.0f, 0.0f));
	Model = rotate(Model, glm::radians(Rotation.y), vec3(0.0f, 1.0f, 0.0f));
	Model = rotate(Model, glm::radians(Rotation.z), vec3(0.0f, 0.0f, 1.0f));
	Model = scale(Model, Scale);
	return Model;
}

void RImDraw::AddOrUpdateDrawElement(uint _hash, vector<RVertex>& Vertices, int Duration, RRenderOptions Opts, uint DrawMethod)
{
	int Index = FindDrawElement(_hash);
	if (Index != -1) {
		UpdateMeshDuration(Index, Duration);
		return;
	}

	Index = GetNewSlotIndex();
	if (Index == -1) return;

	auto& Obj = List[Index];
	Obj.Hash = _hash;
	Obj.Duration = Duration;
	Obj.Empty = false;

	SetMeshFromVertices(Index, Vertices, DrawMethod, Opts);
}