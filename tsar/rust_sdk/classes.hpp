#pragma once
#include <cstdint>
#include <mutex>
#include "vector.hpp"

extern std::mutex entity_mutex;

template <typename t>
struct mono_array
{
	char pad_0[0x20];
	t array[];
};

struct mono_string
{
	char pad_0[ 0x10 ];
	std::uint32_t size;
	wchar_t buffer;
};

struct item_container
{
	char pad_0[ 0x30 ];
	void* contents;
};

struct player_inventory
{
	char pad_0[ 0x20 ];
	item_container* container_main;
	item_container* container_belt;
	item_container* container_wear;
};

struct player_input
{
	char pad_0[ 0x3c ];
	geo::vec3_t body_angles;
};

struct player_model
{
	char pad_0[ 0x1E8 ];
	bool is_local_player;
};

/*struct player_model
{
	char pad_0[0x1d4]; // 468
	bool is_visible; // 1
	char pad_1[0xc]; // 12
	bool is_local_player; // 481
};*/

struct unity_transform
{
	char pad_0[ 0x10 ];
	void* transform;
};

struct model_transforms
{
	char pad_0[ 0x190 ];
	unity_transform* neck;
	unity_transform* head;
};

struct model
{
	char pad_0[ 0x28 ];
	unity_transform* head_bone_transform;
	char pad_1[ 0x10 ];
	model_transforms* transforms;
};

struct base_collision
{
	char pad_0[ 0x20 ];
	model* model;
};

struct base_player
{
	char pad_0[ 0xb0 ];
	model* model;
	char pad_1[ 0x13c ];
	float health;
	char pad_2[ 0x298 ];
	player_inventory* inventory;
	char pad_3[ 0x10 ];
	player_input* input;
	char pad_4[ 0x8 ];
	base_collision* collision;
	char pad_5[ 0x28 ];
	mono_string* display_name;
	char pad_6[ 0x30 ];
	player_model* player_model;
	char pad_7[ 0x41 ];
	bool sleeping;
	char pad_8[ 0x3e ];
	std::uint32_t team;
};

struct base_camera
{
	char pad_0[ 0x2e4 ];
	geo::mat4x4_t view_matrix;
};

struct unk2
{
	char pad_0[ 0x28 ];
	base_player* player;
};

struct game_object
{
	char pad_0[ 0x18 ];
	unk2* unk;
};

struct mono_object
{
	char pad_1[ 0x30 ];
	game_object* object;
	char pad_2[ 0x1c ];
	std::uint16_t tag;
	char pad_3[ 0xa ];
	mono_string* name;
};

struct unk1
{
	char pad_0[ 0x8 ];
	unk1* next;
	mono_object* object;
};

struct buffer_list
{
	char pad_0[ 0x10 ];
	std::uint32_t size;
	void* buffer;
};

struct dictionary
{
	char pad_0[ 0x20 ];
	buffer_list* keys;
	buffer_list* values;
};

struct entity_realm
{
	char pad_0[ 0x10 ];
	dictionary* list;
};
