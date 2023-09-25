#include "ImRender.h"
#include "Shader.h"
#include "engine/camera/camera.h"
#include "engine/geometry/mesh.h"
#include "engine/geometry/triangle.h"
#include "engine/entities/Entity.h"
#include <glad/glad.h>
#include "engine/geometry/vertex.h"

void RImDraw::Init()
{
	List = new RImDrawElement[ImBufferSize];
	for (int I = 0; I < ImBufferSize; I++)
	{
		EmptySlot(I);
		List[I].Mesh.SetupGLBuffers();
	}
}

void RImDraw::Update(float FrameDuration)
{
	for (int I = 0; I < ImBufferSize; I++)
	{
		auto& Obj = List[I];
		if (Obj.Empty)
			continue;

		Obj.Duration -= FrameDuration * 1000.0;
		if (Obj.Duration <= 0)
			EmptySlot(I);
	}
}

void RImDraw::Render(RCamera* Camera)
{
	RShader* ImPointShader = ShaderCatalogue.find("immediate_Point")->second;
	RShader* ImMeshShader = ShaderCatalogue.find("im_mesh")->second;
	RShader* Shader = ImPointShader;
	for (int I = 0; I < ImBufferSize; I++)
	{
		auto& Obj = List[I];
		if (Obj.Empty)
			continue;

		vec3 Color = Obj.RenderOptions.Color.x == -1 ? vec3(0.9, 0.2, 0.0) : Obj.RenderOptions.Color;

		if (Obj.IsMesh)
		{
			Shader = ImMeshShader;
			Shader->Use();
			if (!Obj.IsMultplByMatmodel)
			{
				auto MatModel = GetMatModel(Obj.Position, Obj.Rotation, Obj.Scale);
				Shader->SetMatrix4("model", MatModel);
			}
			else
			{
				Shader->SetMatrix4("model", Mat4Identity);
			}
		}
		else
		{
			Shader = ImPointShader;
			Shader->Use();
		}

		Shader->SetMatrix4("view", Camera->MatView);
		Shader->SetMatrix4("projection", Camera->MatProjection);
		Shader->SetFloat("opacity", Obj.RenderOptions.Opacity);
		Shader->SetFloat3("Color", Obj.RenderOptions.Color);

		RenderMesh(&(List[I].Mesh), Obj.RenderOptions);
	}
}


/* --------------------------- */
/*      > Add primitives       */
/* --------------------------- */
void RImDraw::Add(uint _hash, std::vector<RVertex> VertexVec, GLenum DrawMethod, RenderOptions Opts)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	SetMesh(Slot.Index, VertexVec, DrawMethod, Opts);
}


void RImDraw::Add(uint _hash, std::vector<RTriangle> Triangles, GLenum DrawMethod = GL_LINE_LOOP, RenderOptions Opts = RenderOptions{})
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	std::vector<RVertex> VertexVec;
	for (int i = 0; i < Triangles.size(); i++)
	{
		VertexVec.push_back(RVertex{Triangles[i].A});
		VertexVec.push_back(RVertex{Triangles[i].B});
		VertexVec.push_back(RVertex{Triangles[i].C});
	}

	SetMesh(Slot.Index, VertexVec, DrawMethod, Opts);
}

void RImDraw::AddLine(uint Hash, vec3 PointA, vec3 PointB, vec3 Color)
{
	AddLine(Hash, PointA, PointB, 1.0, false, Color);
}


void RImDraw::AddLine(uint _hash, vec3 PointA, vec3 PointB, float LineWidth, bool AlwaysOnTop, vec3 Color, float Duration)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	auto VertexVec = std::vector<RVertex>{RVertex{PointA}, RVertex{PointB}};

	RenderOptions Opts;
	Opts.LineWidth = LineWidth;
	Opts.AlwaysOnTop = AlwaysOnTop;
	Opts.Color = Color;
	Opts.DontCullFace = true;

	if (Duration != 0)
	{
		auto& Obj = List[Slot.Index];
		Obj.Hash = _hash;
		Obj.Duration = Duration;
	}

	SetMesh(Slot.Index, VertexVec, GL_LINES, Opts);
}


void RImDraw::AddLineLoop(uint _hash, vector<vec3> Points, float LineWidth, bool AlwaysOnTop)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	auto VertexVec = std::vector<RVertex>();
	for (int I = 0; I < Points.size(); I++)
		VertexVec.push_back(RVertex{Points[I]});

	RenderOptions Opts;
	Opts.LineWidth = LineWidth;
	Opts.AlwaysOnTop = AlwaysOnTop;
	Opts.DontCullFace = true;

	SetMesh(Slot.Index, VertexVec, GL_LINE_LOOP, Opts);
}


void RImDraw::AddPoint(uint _hash, vec3 Point, float PointSize, bool AlwaysOnTop, vec3 Color, float Duration)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	if (Duration != 0)
	{
		auto& Obj = List[Slot.Index];
		Obj.Hash = _hash;
		Obj.Duration = Duration;
	}

	auto VertexVec = std::vector<RVertex>{RVertex{Point}};

	RenderOptions Opts;
	Opts.PointSize = PointSize;
	Opts.AlwaysOnTop = AlwaysOnTop;
	Opts.Color = Color;

	SetMesh(Slot.Index, VertexVec, GL_POINTS, Opts);
}

void RImDraw::AddPoint(uint Hash, vec3 Point, vec3 Color)
{
	AddPoint(Hash, Point, 1.0, false, Color);
}


void RImDraw::AddTriangle(uint _hash, RTriangle Triangle, float LineWidth, bool AlwaysOnTop, vec3 Color)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	auto VertexVec = std::vector<RVertex>{RVertex{Triangle.A}, RVertex{Triangle.B}, RVertex{Triangle.C}};
	auto Indices = std::vector<uint>{0, 1, 2};

	RenderOptions Opts;
	Opts.LineWidth = LineWidth;
	Opts.AlwaysOnTop = AlwaysOnTop;
	Opts.Color = Color;

	SetMesh(Slot.Index, VertexVec, GL_TRIANGLES, Opts);
	SetIndices(Slot.Index, Indices);
}


/* --------------------------- */
/*        > Add Mesh           */
/* --------------------------- */
void RImDraw::AddMesh(uint _hash, RMesh* Mesh, vec3 Position, vec3 Rotation, vec3 Scale, vec3 Color, int Duration)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	if (!Slot.Empty)
	{
		UpdateMesh(Slot.Index, Position, Rotation, Scale, Color, Duration);
		return;
	}

	RenderOptions Opts;
	Opts.Color = Color;
	Opts.Wireframe = true;

	auto& Obj = List[Slot.Index];
	Obj.Hash = _hash;
	Obj.Duration = Duration;
	Obj.Position = Position;
	Obj.Rotation = Rotation;
	Obj.Scale = Scale;

	SetMesh(Slot.Index, Mesh, Opts);
}


void RImDraw::AddMesh(uint _hash, RMesh* Mesh, vec3 Color, float Duration)
{
	RImDrawSlot Slot = IM_R_FIND_SLOT();

	if (!Slot.Empty)
	{
		UpdateMesh(Slot.Index, Color, Duration);
		return;
	}

	RenderOptions Opts;
	Opts.Color = Color;
	Opts.Wireframe = true;

	auto& Obj = List[Slot.Index];
	Obj.Hash = _hash;
	Obj.Duration = Duration;
	Obj.IsMultplByMatmodel = true;

	SetMesh(Slot.Index, Mesh, Opts);
}


void RImDraw::AddMesh(uint Hash, EEntity* Entity, int Duration)
{
	AddMesh(Hash, Entity->Mesh, Entity->Position, Entity->Rotation, Entity->Scale, vec3(1.0, 0, 0), Duration);
}


void RImDraw::AddMesh(uint Hash, EEntity* Entity)
{
	AddMesh(Hash, Entity->Mesh, Entity->Position, Entity->Rotation, Entity->Scale);
}


void RImDraw::AddMesh(uint Hash, EEntity* Entity, vec3 Position)
{
	AddMesh(Hash, Entity->Mesh, Position, Entity->Rotation, Entity->Scale);
}

/* --------------------------- */
/*     > Private functions     */
/* --------------------------- */
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


RImDrawSlot RImDraw::FindElementOrEmptySlot(uint Hash)
{
	int Slot = -1;
	for (int I = 0; I < ImBufferSize; I++)
	{
		if (Slot == -1 && List[I].Empty)
			Slot = I;
		if (List[I].Hash == Hash)
			return RImDrawSlot{false, I};
	}

	if (Slot == -1)
		print("IM RENDER BUFFER IS FULL");

	return RImDrawSlot{true, Slot};
}


void RImDraw::SetMesh(int Index, vector<RVertex> Vertices, GLenum DrawMethod, RenderOptions Opts)
{
	auto& Obj = List[Index];
	Obj.Mesh.Vertices = Vertices;
	Obj.Mesh.RenderMethod = DrawMethod;
	Obj.RenderOptions = Opts;
	Obj.Empty = false;

	Obj.Mesh.SendDataToGLBuffer();
}


void RImDraw::SetMesh(int Index, RMesh* Mesh, RenderOptions Opts)
{
	auto& Obj = List[Index];
	Obj.Mesh = *Mesh;
	Obj.RenderOptions = Opts;
	Obj.IsMesh = true;
	Obj.Empty = false;
	Obj.Mesh.SendDataToGLBuffer();
}


void RImDraw::UpdateMesh(int Index, vec3 Position, vec3 Rotation, vec3 Scale, vec3 Color, int Duration)
{
	auto& Obj = List[Index];
	Obj.RenderOptions.Color = Color;
	Obj.Duration = Duration;
	Obj.Position = Position;
	Obj.Rotation = Rotation;
	Obj.Scale = Scale;
}


void RImDraw::UpdateMesh(int Index, vec3 Color, int Duration)
{
	auto& Obj = List[Index];
	Obj.RenderOptions.Color = Color;
	Obj.Duration = Duration;
}


//@TODO: Probably redundant code
mat4 RImDraw::GetMatModel(vec3 Position, vec3 Rotation, vec3 Scale)
{
	glm::mat4 Model = translate(Mat4Identity, Position);
	Model = rotate(Model, glm::radians(Rotation.x), vec3(1.0f, 0.0f, 0.0f));
	Model = rotate(Model, glm::radians(Rotation.y), vec3(0.0f, 1.0f, 0.0f));
	Model = rotate(Model, glm::radians(Rotation.z), vec3(0.0f, 0.0f, 1.0f));
	Model = glm::scale(Model, Scale);
	return Model;
}


void RImDraw::SetIndices(int Index, std::vector<uint> Indices)
{
	List[Index].Mesh.Indices = Indices;
}
