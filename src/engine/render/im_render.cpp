#include "im_render.h"
#include "shader.h"
#include "engine/camera/camera.h"
#include "engine/geometry/mesh.h"
#include "engine/geometry/triangle.h"
#include "engine/entities/entity.h"
#include <glad/glad.h>
#include "engine/geometry/vertex.h""

void ImDraw::Init()
{
	list = new ImDrawElement[IM_BUFFER_SIZE];
	for (int i = 0; i < IM_BUFFER_SIZE; i++)
	{
		EmptySlot(i);
		list[i].mesh.SetupGLBuffers();
	}
}

void ImDraw::Update(float frame_duration)
{
	for (int i = 0; i < IM_BUFFER_SIZE; i++)
	{
		auto& obj = list[i];
		if (obj.empty)
			continue;

		obj.duration -= frame_duration * 1000.0;
		if (obj.duration <= 0)
			EmptySlot(i);
	}
}

void ImDraw::Render(Camera* camera)
{
	Shader* im_point_shader = ShaderCatalogue.find("immediate_point")->second;
	Shader* im_mesh_shader = ShaderCatalogue.find("im_mesh")->second;
	Shader* shader = im_point_shader;
	for (int i = 0; i < IM_BUFFER_SIZE; i++)
	{
		auto& obj = list[i];
		if (obj.empty)
			continue;

		vec3 color = obj.render_options.color.x == -1 ? vec3(0.9, 0.2, 0.0) : obj.render_options.color;

		if (obj.is_mesh)
		{
			shader = im_mesh_shader;
			shader->Use();
			if (!obj.is_multpl_by_matmodel)
			{
				auto matModel = GetMatModel(obj.pos, obj.rot, obj.scale);
				shader->SetMatrix4("model", matModel);
			}
			else
			{
				shader->SetMatrix4("model", Mat4Identity);
			}
		}
		else
		{
			shader = im_point_shader;
			shader->Use();
		}

		shader->SetMatrix4("view", camera->mat_view);
		shader->SetMatrix4("projection", camera->mat_projection);
		shader->SetFloat("opacity", obj.render_options.opacity);
		shader->SetFloat3("color", obj.render_options.color);

		RenderMesh(&(list[i].mesh), obj.render_options);
	}
}


/* --------------------------- */
/*      > Add primitives       */
/* --------------------------- */
void ImDraw::Add(size_t _hash, std::vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts)
{
	IM_R_FIND_SLOT();

	SetMesh(slot.index, vertex_vec, draw_method, opts);
}


void ImDraw::Add(size_t _hash, std::vector<Triangle> triangles, GLenum draw_method = GL_LINE_LOOP, RenderOptions opts = RenderOptions{})
{
	IM_R_FIND_SLOT();

	std::vector<Vertex> vertex_vec;
	for (int i = 0; i < triangles.size(); i++)
	{
		vertex_vec.push_back(Vertex{triangles[i].a});
		vertex_vec.push_back(Vertex{triangles[i].b});
		vertex_vec.push_back(Vertex{triangles[i].c});
	}

	SetMesh(slot.index, vertex_vec, draw_method, opts);
}

void ImDraw::AddLine(size_t _hash, vec3 pointA, vec3 pointB, vec3 color)
{
	AddLine(_hash, pointA, pointB, 1.0, false, color);
}


void ImDraw::AddLine(size_t _hash, vec3 pointA, vec3 pointB, float line_width,
                     bool always_on_top, vec3 color, float duration)
{
	IM_R_FIND_SLOT();

	auto vertex_vec = std::vector<Vertex>{Vertex{pointA}, Vertex{pointB}};

	RenderOptions opts;
	opts.line_width = line_width;
	opts.always_on_top = always_on_top;
	opts.color = color;
	opts.dont_cull_face = true;

	if (duration != 0)
	{
		auto& obj = list[slot.index];
		obj.hash = _hash;
		obj.duration = duration;
	}

	SetMesh(slot.index, vertex_vec, GL_LINES, opts);
}


void ImDraw::AddLineLoop(size_t _hash, std::vector<vec3> points, float line_width, bool always_on_top)
{
	IM_R_FIND_SLOT();

	auto vertex_vec = std::vector<Vertex>();
	for (int i = 0; i < points.size(); i++)
		vertex_vec.push_back(Vertex{points[i]});

	RenderOptions opts;
	opts.line_width = line_width;
	opts.always_on_top = always_on_top;
	opts.dont_cull_face = true;

	SetMesh(slot.index, vertex_vec, GL_LINE_LOOP, opts);
}


void ImDraw::AddPoint(size_t _hash, vec3 point, float point_size, bool always_on_top, vec3 color, float duration)
{
	IM_R_FIND_SLOT();

	if (duration != 0)
	{
		auto& obj = list[slot.index];
		obj.hash = _hash;
		obj.duration = duration;
	}

	auto vertex_vec = std::vector<Vertex>{Vertex{point}};

	RenderOptions opts;
	opts.point_size = point_size;
	opts.always_on_top = always_on_top;
	opts.color = color;

	SetMesh(slot.index, vertex_vec, GL_POINTS, opts);
}

void ImDraw::AddPoint(size_t _hash, vec3 point, vec3 color)
{
	AddPoint(_hash, point, 1.0, false, color);
}


void ImDraw::AddTriangle(size_t _hash, Triangle t, float line_width, bool always_on_top, vec3 color)
{
	IM_R_FIND_SLOT();

	auto vertex_vec = std::vector<Vertex>{Vertex{t.a}, Vertex{t.b}, Vertex{t.c}};
	auto indices = std::vector<u32>{0, 1, 2};

	RenderOptions opts;
	opts.line_width = line_width;
	opts.always_on_top = always_on_top;
	opts.color = color;

	SetMesh(slot.index, vertex_vec, GL_TRIANGLES, opts);
	SetIndices(slot.index, indices);
}


/* --------------------------- */
/*        > Add Mesh           */
/* --------------------------- */
void ImDraw::AddMesh(size_t _hash, Mesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
{
	IM_R_FIND_SLOT();

	if (!slot.empty)
	{
		UpdateMesh(slot.index, pos, rot, scale, color, duration);
		return;
	}

	RenderOptions opts;
	opts.color = color;
	opts.wireframe = true;

	auto& obj = list[slot.index];
	obj.hash = _hash;
	obj.duration = duration;
	obj.pos = pos;
	obj.rot = rot;
	obj.scale = scale;

	SetMesh(slot.index, mesh, opts);
}


void ImDraw::AddMesh(size_t _hash, Mesh* mesh, vec3 color, float duration)
{
	IM_R_FIND_SLOT();

	if (!slot.empty)
	{
		UpdateMesh(slot.index, color, duration);
		return;
	}

	RenderOptions opts;
	opts.color = color;
	opts.wireframe = true;

	auto& obj = list[slot.index];
	obj.hash = _hash;
	obj.duration = duration;
	obj.is_multpl_by_matmodel = true;

	SetMesh(slot.index, mesh, opts);
}


void ImDraw::AddMesh(size_t _hash, Entity* entity, int duration)
{
	AddMesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale, vec3(1.0, 0, 0), duration);
}


void ImDraw::AddMesh(size_t _hash, Entity* entity)
{
	AddMesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale);
}


void ImDraw::AddMesh(size_t _hash, Entity* entity, vec3 pos)
{
	AddMesh(_hash, entity->mesh, pos, entity->rotation, entity->scale);
}

/* --------------------------- */
/*     > Private functions     */
/* --------------------------- */
void ImDraw::EmptySlot(int i)
{
	auto& obj = list[i];
	obj.mesh.indices.clear();
	obj.mesh.vertices.clear();
	obj.empty = true;
	obj.hash = 0;
	obj.duration = 0;
	obj.is_mesh = false;
	obj.scale = vec3(0);
	obj.pos = vec3(0);
	obj.rot = vec3(0);
	obj.is_multpl_by_matmodel = false;
}


ImDrawSlot ImDraw::FindElementOrEmptySlot(size_t hash)
{
	int slot = -1;
	for (int i = 0; i < IM_BUFFER_SIZE; i++)
	{
		if (slot == -1 && list[i].empty)
			slot = i;
		if (list[i].hash == hash)
			return ImDrawSlot{false, i};
	}

	if (slot == -1)
		std::cout << "IM RENDER BUFFER IS FULL\n";

	return ImDrawSlot{true, slot};
}


void ImDraw::SetMesh(int i, std::vector<Vertex> vertices, GLenum draw_method, RenderOptions opts)
{
	auto& obj = list[i];
	obj.mesh.vertices = vertices;
	obj.mesh.render_method = draw_method;
	obj.render_options = opts;
	obj.empty = false;

	obj.mesh.SendDataToGLBuffer();
}


void ImDraw::SetMesh(int i, Mesh* mesh, RenderOptions opts)
{
	auto& obj = list[i];
	obj.mesh = *mesh;
	obj.render_options = opts;
	obj.is_mesh = true;
	obj.empty = false;
	obj.mesh.SendDataToGLBuffer();
}


void ImDraw::UpdateMesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
{
	auto& obj = list[i];
	obj.render_options.color = color;
	obj.duration = duration;
	obj.pos = pos;
	obj.rot = rot;
	obj.scale = scale;
}


void ImDraw::UpdateMesh(int i, vec3 color, int duration)
{
	auto& obj = list[i];
	obj.render_options.color = color;
	obj.duration = duration;
}


//@TODO: Probably redundant code
mat4 ImDraw::GetMatModel(vec3 pos, vec3 rot, vec3 scale)
{
	glm::mat4 model = translate(Mat4Identity, pos);
	model = rotate(model, glm::radians(rot.x), vec3(1.0f, 0.0f, 0.0f));
	model = rotate(model, glm::radians(rot.y), vec3(0.0f, 1.0f, 0.0f));
	model = rotate(model, glm::radians(rot.z), vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);
	return model;
}


void ImDraw::SetIndices(int i, std::vector<u32> indices)
{
	list[i].mesh.indices = indices;
}
