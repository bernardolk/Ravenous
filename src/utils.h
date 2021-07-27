void print_vec(vec3 vec, std::string prefix)
{
   std::cout << prefix << ": (" << vec.x << ", " << vec.y << ", " << vec.z << ") \n";
}

void print_vertex_array_position(Vertex* vertex, size_t length, std::string title)
{
   std::cout << title << "\n";
   for(int i = 0; i < length; i++)
   {
      vec3 pos = vertex[i].position;
      std::cout << "[" << i << "] : (" << pos.x << ", " << pos.y << ", " << pos.z << ") \n";
   }
}

void print_vec_every_3rd_frame(vec3 vec, std::string prefix)
{
   if(G_FRAME_INFO.frame_counter_3 == 0)
      print_vec(vec, prefix);
}

void print_every_3rd_frame(std::string thing, std::string prefix)
{
   if(G_FRAME_INFO.frame_counter_3 == 0)
      std::cout << prefix << ": " << thing << "\n";
}

inline
string format_float_tostr(float num, int precision) 
{
	string temp = std::to_string(num);
	return temp.substr(0, temp.find(".") + 3);
}

inline 
bool is_zero(float x)
{
   return abs(x) < 0.0001;
}

float VEC_COMPARE_PRECISION = 0.00001f;

inline
bool is_equal(vec2 vec1, vec2 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   
   return x_diff < VEC_COMPARE_PRECISION && y_diff < VEC_COMPARE_PRECISION;
}

inline
bool is_equal(vec3 vec1, vec3 vec2)
{
   float x_diff = abs(vec1.x - vec2.x);
   float y_diff = abs(vec1.y - vec2.y);
   float z_diff = abs(vec1.z - vec2.z);

   return x_diff < VEC_COMPARE_PRECISION 
      && y_diff < VEC_COMPARE_PRECISION 
      && z_diff < VEC_COMPARE_PRECISION;
}

// compare sign
bool comp_sign(float a, float b)
{
   return a * b >= 0.f;
}

// compare vector length with scalar

bool square_EQ(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z == n * n; 
}

bool square_LT(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z < n * n; 
}

bool square_GT(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z > n * n; 
}

bool square_LE(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z <= n * n; 
}

bool square_GE(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z >= n * n; 
}

inline
float vector_angle(vec2 A, vec2 B)
{
   float dot = glm::dot(A, B);
   float len_A = glm::length(A);
   float len_B = glm::length(B);
   float theta = acos(dot / (len_A * len_B));
   return theta;
}

inline
float vector_cos(vec2 A, vec2 B)
{
   float dot = glm::dot(A, B);
   float len_A = glm::length(A);
   float len_B = glm::length(B);
   float cos = dot / (len_A * len_B);
   return cos;
}