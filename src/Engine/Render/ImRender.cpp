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
	list = new RImDrawElement[im_buffer_size];
	for (int i = 0; i < im_buffer_size; i++)
	{
		EmptySlot(i);
		list[i].mesh.SetupGLBuffers();
	}
}

void RImDraw::Update(float frame_duration)
{
	for (int i = 0; i < im_buffer_size; i++)
	{
		auto& obj = list[i];
		if (obj.empty)
			continue;

		obj.duration -= frame_duration * 1000.0;
		if (obj.duration <= 0)
			EmptySlot(i);
	}
}

void RImDraw::Render(RCamera* camera)
{
	RShader* im_point_shader = ShaderCatalogue.find("immediate_point")->second;
	RShader* im_mesh_shader = ShaderCatalogue.find("im_mesh")->second;
	RShader* shader = im_point_shader;
	for (int i = 0; i < im_buffer_size; i++)
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
				auto mat_model = GetMatModel(obj.pos, obj.rot, obj.scale);
				shader->SetMatrix4("model", mat_model);
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
void RImDraw::Add(uint _hash, std::vector<RVertex> vertex_vec, GLenum draw_method, RenderOptions opts)
{
	IM_R_FIND_SLOT();

	SetMesh(slot.index, vertex_vec, draw_method, opts);
}


void RImDraw::Add(uint _hash, std::vector<RTriangle> triangles, GLenum draw_method = GL_LINE_LOOP, RenderOptions opts = RenderOptions{})
{
	IM_R_FIND_SLOT();

	std::vector<RVertex> vertex_vec;
	for (int i = 0; i < triangles.size(); i++)
	{
		vertex_vec.push_back(RVertex{triangles[i].a});
		vertex_vec.push_back(RVertex{triangles[i].b});
		vertex_vec.push_back(RVertex{triangles[i].c});
	}

	SetMesh(slot.index, vertex_vec, draw_method, opts);
}

void RImDraw::AddLine(uint _hash, vec3 point_a, vec3 point_b, vec3 color)
{
	AddLine(_hash, point_a, point_b, 1.0, false, color);
}


void RImDraw::AddLine(uint _hash, vec3 point_a, vec3 point_b, float line_width,
                     bool always_on_top, vec3 color, float duration)
{
	IM_R_FIND_SLOT();

	auto vertex_vec = std::vector<RVertex>{RVertex{point_a}, RVertex{point_b}};

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


void RImDraw::AddLineLoop(uint _hash, std::vector<vec3> points, float line_width, bool always_on_top)
{
	IM_R_FIND_SLOT();

	auto vertex_vec = std::vector<RVertex>();
	for (int i = 0; i < points.size(); i++)
		vertex_vec.push_back(RVertex{points[i]});

	RenderOptions opts;
	opts.line_width = line_width;
	opts.always_on_top = always_on_top;
	opts.dont_cull_face = true;

	SetMesh(slot.index, vertex_vec, GL_LINE_LOOP, opts);
}


void RImDraw::AddPoint(uint _hash, vec3 point, float point_size, bool always_on_top, vec3 color, float duration)
{
	IM_R_FIND_SLOT();

	if (duration != 0)
	{
		auto& obj = list[slot.index];
		obj.hash = _hash;
		obj.duration = duration;
	}

	auto vertex_vec = std::vector<RVertex>{RVertex{point}};

	RenderOptions opts;
	opts.point_size = point_size;
	opts.always_on_top = always_on_top;
	opts.color = color;

	SetMesh(slot.index, vertex_vec, GL_POINTS, opts);
}

void RImDraw::AddPoint(uint _hash, vec3 point, vec3 color)
{
	AddPoint(_hash, point, 1.0, false, color);
}


void RImDraw::AddTriangle(uint _hash, RTriangle t, float line_width, bool always_on_top, vec3 color)
{
	IM_R_FIND_SLOT();

	auto vertex_vec = std::vector<RVertex>{RVertex{t.a}, RVertex{t.b}, RVertex{t.c}};
	auto indices = std::vector<uint>{0, 1, 2};

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
void RImDraw::AddMesh(uint _hash, RMesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
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


void RImDraw::AddMesh(uint _hash, RMesh* mesh, vec3 color, float duration)
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


void RImDraw::AddMesh(uint _hash, EEntity* entity, int duration)
{
	AddMesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale, vec3(1.0, 0, 0), duration);
}


void RImDraw::AddMesh(uint _hash, EEntity* entity)
{
	AddMesh(_hash, entity->mesh, entity->position, entity->rotation, entity->scale);
}


void RImDraw::AddMesh(uint _hash, EEntity* entity, vec3 pos)
{
	AddMesh(_hash, entity->mesh, pos, entity->rotation, entity->scale);
}

/* --------------------------- */
/*     > Private functions     */
/* --------------------------- */
void RImDraw::EmptySlot(int i)
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


RImDrawSlot RImDraw::FindElementOrEmptySlot(uint hash)
{
	int slot = -1;
	for (int i = 0; i < im_buffer_size; i++)
	{
		if (slot == -1 && list[i].empty)
			slot = i;
		if (list[i].hash == hash)
			return RImDrawSlot{false, i};
	}

	if (slot == -1)
		print("IM RENDER BUFFER IS FULL");

	return RImDrawSlot{true, slot};
}


void RImDraw::SetMesh(int i, std::vector<RVertex> vertices, GLenum draw_method, RenderOptions opts)
{
	auto& obj = list[i];
	obj.mesh.vertices = vertices;
	obj.mesh.render_method = draw_method;
	obj.render_options = opts;
	obj.empty = false;

	obj.mesh.SendDataToGLBuffer();
}


void RImDraw::SetMesh(int i, RMesh* mesh, RenderOptions opts)
{
	auto& obj = list[i];
	obj.mesh = *mesh;
	obj.render_options = opts;
	obj.is_mesh = true;
	obj.empty = false;
	obj.mesh.SendDataToGLBuffer();
}


void RImDraw::UpdateMesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration)
{
	auto& obj = list[i];
	obj.render_options.color = color;
	obj.duration = duration;
	obj.pos = pos;
	obj.rot = rot;
	obj.scale = scale;
}


void RImDraw::UpdateMesh(int i, vec3 color, int duration)
{
	auto& obj = list[i];
	obj.render_options.color = color;
	obj.duration = duration;
}


//@TODO: Probably redundant code
mat4 RImDraw::GetMatModel(vec3 pos, vec3 rot, vec3 scale)
{
	glm::mat4 model = translate(Mat4Identity, pos);
	model = rotate(model, glm::radians(rot.x), vec3(1.0f, 0.0f, 0.0f));
	model = rotate(model, glm::radians(rot.y), vec3(0.0f, 1.0f, 0.0f));
	model = rotate(model, glm::radians(rot.z), vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);
	return model;
}


void RImDraw::SetIndices(int i, std::vector<uint> indices)
{
	list[i].mesh.indices = indices;
}
