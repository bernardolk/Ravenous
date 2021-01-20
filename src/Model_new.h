#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <Mesh.h>
#include <glm/vec3.hpp>

struct MeshData {
	std::vector<Vertex> vertexes;
	std::vector<unsigned int> indices;
	unsigned int faceCount = 0;
};

struct Parse {
	const char* string;
	size_t size;
	u8 hasToken;
	union{
		char* sToken;
		int iToken;
		float fToken;
		char cToken;
		unsigned int uiToken;
	};
};


inline Parse parse_whitespace(Parse toparse) {
	Parse outparse{toparse.string, toparse.size, 0};
	if (toparse.string[0] == ' ') {
		outparse.iToken = 1;
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	outparse.iToken = 0;
	return outparse;
}

inline Parse parse_letter(Parse toparse) {
	Parse outparse{ toparse.string, toparse.size, 0};
	if (isalpha(toparse.string[0])) {
		outparse.cToken = toparse.string[0];
		outparse.string = &(toparse.string[1]);
		outparse.size = toparse.size - 1;
		outparse.hasToken = 1;
		return outparse;
	}
	return outparse;
}

inline Parse parse_int(Parse toparse) {
	Parse outparse{ toparse.string, toparse.size, 0};
	u32 ten_powers[10]{1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000};
	char int_buf[10];
	u16 count = 0;
	u16 rev_count = 0;
	u16 sign = 1;
	if (outparse.string[0] == '-') {
		outparse.string = &(outparse.string[1]);
		outparse.size -= 1;
		sign = -1;
	}
	if (isdigit(outparse.string[0])){
		do {
			int_buf[count++] = outparse.string[0];
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
		} while (isdigit(outparse.string[0]));
		while (count > 0) {
			outparse.iToken += (int_buf[count - 1] - '0') * ten_powers[rev_count];
			rev_count++;
			count--;
		}
		outparse.iToken *= sign;
		outparse.hasToken = 1;
	}
	return outparse;
}

inline Parse parse_uint(Parse toparse) {
	Parse outparse{ toparse.string, toparse.size, 0};
	u32 ten_powers[10]{1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000};
	char int_buf[10];
	u16 count = 0;
	u16 rev_count = 0;
	if (isdigit(outparse.string[0])){
		do {
			int_buf[count++] = outparse.string[0];
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
		} while (isdigit(outparse.string[0]));
		while (count > 0) {
			outparse.uiToken += (int_buf[count - 1] - '0') * ten_powers[rev_count];
			rev_count++;
			count--;
		}
		outparse.hasToken = 1;
	}
	return outparse;
}


inline Parse parse_float(Parse toparse) {
	Parse outparse{ toparse.string, toparse.size, 0 };
	int ten_powers[10]{ 1, 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000, 1000000000 };
	float ten_inverse_powers[10]{ 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f,
		0.000001f, 0.0000001f, 0.00000001f, 0.000000001f, 0.0000000001f};
	char int_buf[10];
	char float_buf[10];
	int count = 0, fcount = 0;
	int rev_count = 0;
	int sign = 1;
	if (outparse.string[0] == '-') {
		outparse.string = &(outparse.string[1]);
		outparse.size -= - 1;
		sign = -1;
	}
	if (isdigit(outparse.string[0])) {
		do{
			int_buf[count++] = outparse.string[0];
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
		} while (isdigit(outparse.string[0])); 

		if(outparse.string[0] == '.'){
			outparse.string = &(outparse.string[1]);
			outparse.size -= 1;
			while (isdigit(outparse.string[0])){
				float_buf[fcount++] = outparse.string[0];
				outparse.string = &(outparse.string[1]);
				outparse.size -= 1;
			}; 
		}
		while (count > 0) {
			outparse.fToken += (int_buf[count - 1] - '0') * ten_powers[rev_count];
			rev_count++;
			count--;
		}
		rev_count = 0;
		while(fcount > 0){
			outparse.fToken += (float_buf[rev_count] - '0') * ten_inverse_powers[rev_count];
			rev_count++;
			fcount--;
		}
		outparse.fToken *= sign;
		outparse.hasToken = 1;
	}
	return outparse;
}


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


// An obj model may be more like a scene, having multiple objects inside of it.
// I don't care whether it is a single object or multiple, each individual thing in the file must be
// constructed as a single entity in the editor. So it can be moved around and have its materials changed.

// Each entity have a model associated with it. The model has a single mesh, and it has material properties as well.
// Every entity has its own mesh data, even if copied, except for billboard-type entities, which all share the same primordial quad mesh.

