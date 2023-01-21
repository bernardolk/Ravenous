#pragma once

struct Mesh;

enum class RenderMethodEnum
{
	triangles = 0x0004,
};

using StrVec = std::vector<std::string>;

Mesh* load_wavefront_obj_as_mesh(
const std::string& path,
const std::string& filename,
const std::string& name = "",
bool setup_gl_data = true,
RenderMethodEnum render_method = RenderMethodEnum::triangles);

CollisionMesh* load_wavefront_obj_as_collision_mesh(std::string path, std::string filename, std::string name = "");
unsigned int load_texture_from_file(const std::string& filename, const std::string& directory, bool gamma = false);
void attach_extra_data_to_mesh(std::string filename, std::string filepath, Mesh* mesh);
void load_mesh_extra_data(std::string filename, Mesh* mesh);
void write_mesh_extra_data_file(std::string filename, Mesh* mesh);
void load_textures_from_assets_folder();
StrVec get_files_in_folder(std::string directory);
