#include <ctime>

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

inline
string format_float_tostr(float num, int precision = 3)   
{
	string temp = std::to_string(num);
	return temp.substr(0, temp.find(".") + precision);
}

inline
string fmt_tostr(float num, int precision)   
{
	return format_float_tostr(num, precision);
}

inline
string to_string(vec3 vec)
{
   return "(" + to_string(vec.x) + ", " + to_string(vec.y) + ", " + to_string(vec.z) + ")";
}

inline
string to_string(vec2 vec)
{
   return "(" + to_string(vec.x) + ", " + to_string(vec.y) + ")";
}


enum FloatTolerance {
   FloatTolerance_3_001       = 0,
   FloatTolerance_4_0001      = 1,
   FloatTolerance_5_00001     = 2,
   FloatTolerance_6_000001    = 3,
};

inline 
bool are_equal_floats(float x, float y, FloatTolerance tolerance = FloatTolerance_5_00001)
{
   /* checks if two floats are equal within a certain tolerance level */
   float tolerances[] = {0.001f, 0.0001f, 0.00001f, 0.000001f};
   return abs(x - y) < tolerances[tolerance];
}

// SIGN COMPARISON
inline
bool comp_sign(float a, float b)
{
   return a * b >= 0.f;
}

inline
float sign(float a)
{
   return a < 0 ? -1 : 1;
}

// VECTOR LENGTH COMPARISON
inline
bool square_EQ(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z == n * n; 
}

inline
bool square_LT(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z < n * n; 
}

inline
bool square_GT(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z > n * n; 
}

inline
bool square_LE(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z <= n * n; 
}

inline
bool square_GE(vec3 v, float n)
{
   return v.x * v.x + v.y * v.y + v.z * v.z >= n * n; 
}

inline
float abs(vec3 v)
{
   return glm::length(v);
}

// VECTOR ANGLE

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
float vector_angle_signed(vec2 A, vec2 B)
{
   return atan2( A.x*B.y - A.y*B.x, A.x*B.x + A.y*B.y );
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

// VECTOR DIMENSION CONVERSION
inline
vec3 to_xz(vec3 vector)
{
   return vec3(vector.x, 0, vector.z);
}

inline
vec3 to_xy(vec3 vector)
{
   return vec3(vector.x, vector.y, 0);
}

inline
vec3 to_zy(vec3 vector)
{
   return vec3(0, vector.y, vector.z);
}

inline
vec2 to2d_xz(vec3 vector)
{
   return vec2(vector.x, vector.z);
}

inline
vec2 to2d_xy(vec3 vector)
{
   return vec2(vector.x, vector.y);
}

inline
vec2 to2d_zy(vec3 vector)
{
   return vec2(vector.z, vector.y);
}

inline
vec3 to3d_xz(vec2 vector)
{
   return vec3(vector.x, 0, vector.y);
}

// VECTOR OPERATIONS
inline
vec3 nrmlz(vec3 vec)
{
   return glm::normalize(vec);
}

inline
vec2 nrmlz(vec2 vec)
{
   return glm::normalize(vec);
}

inline
vec3 cross(vec3 A, vec3 B)
{
   return glm::cross(A,B);
}

inline
vec3 project_vec_into_ref(vec3 vec, vec3 ref)
{
   auto proj = dot(vec, ref) / (abs(ref) * abs(ref)) * ref;
   return proj;
}


// OTHER

inline
int get_random_int(int min, int max)
{
   static bool first = true;
   if (first) 
   {  
      std::srand( std::time(NULL) ); //seeding for the first time only!
      first = false;
   }
   return min + rand() % (( max + 1 ) - min);
}

inline float get_random_float(int min, int max)
{
   return get_random_int(min * 1000, max * 1000) / 1000.f;
}


// colors
inline
vec3 get_random_color()
{
   return vec3(
      get_random_float(0, 1),
      get_random_float(0, 1),
      get_random_float(0, 1)
   );
}


// macros

#define For(x) for (int i = 0; i < x; i ++)
#define for_less(x) for (int i = 0; i < x; i ++)
#define for_less_equal(x) for (int i = 0; i <+ x; i ++)
