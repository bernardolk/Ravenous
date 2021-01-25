#include <parser.h>
#include <Mesh.h>

MeshData import_wavefront_obj(string path) {
	ifstream reader(path);
	std::string line;

	MeshData mdata;

	while (getline(reader, line)) {
		//istringstream iss(line);
		const char* cline = line.c_str();
		size_t size = line.size();

		Parse p{ cline, size };
		p = parse_letter(p);
		if (p.hasToken && p.cToken == 'm') {

		}
		if (p.hasToken && p.cToken == 'v') {
		Vertex vert;
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_float(p);
		  vert.Position.x = p.fToken;
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_float(p);
		  vert.Position.y = p.fToken;

		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_float(p);
		  vert.Position.z = p.fToken;
		
		  mdata.vertexes.push_back(vert);
		}

		if (p.hasToken && p.cToken == 'f') {
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mdata.indices.push_back(p.uiToken - 1);
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mdata.indices.push_back(p.uiToken - 1);
		  
		  do {
		    p = parse_whitespace(p);
		  } while (p.hasToken);
		  p = parse_uint(p);
		  mdata.indices.push_back(p.uiToken - 1);
		}
		mdata.faceCount ++;
	}

	return mdata;
}
