/* ------------------
      SIMPLEX
------------------ */
struct Simplex {
		vec3 points[4];
		u32 p_size;

		Simplex()
      {
         points[0] = vec3(0);
         points[1] = vec3(0);
         points[2] = vec3(0);
         points[3] = vec3(0);
         p_size = 0;
      }

		Simplex& operator=(std::initializer_list<vec3> list) {
			for (auto v = list.begin(); v != list.end(); v++) {
				points[std::distance(list.begin(), v)] = *v;
			}
			p_size = list.size();

			return *this;
		}

		void push_front(vec3 point);
		vec3& operator[](u32 i);
		u32 size() const;
};