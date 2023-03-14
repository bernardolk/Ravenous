#pragma once

struct E_BaseEntity
{
	TypeID type_id;
	u64 id = 0;
	float position[3]{};
};
