#pragma once

#include "menu/items/ID3DMenuItem.hpp"
#include "menu/items/D3DMenuBoolItem.hpp"
#include "menu/items/D3DMenuSubFolderItem.hpp"
#include "menu/items/D3DMenuFlagItem.hpp"
#include "menu/items/D3DMenuIntItem.hpp"
#include "menu/items/D3DMenuFloatItem.hpp"
#include <functional>

int font_height = 15;
int menu_height = 5;
int rpad = 100;

std::vector<std::shared_ptr<ID3DMenuItem>> menu_items;

void draw_menu_item(std::shared_ptr<ID3DMenuItem> item, const float padding)
{
	auto color = D3DCOLOR_XRGB(255, 255, 255);
	if (item->is_selected())
		color = D3DCOLOR_XRGB(60, 140, 255);

	auto name = item->get_name();

	if (item->is_subfolder())
	{
		const auto folder = std::static_pointer_cast<D3DMenuSubFolderItem>(item);

		if (folder->is_opened())
			name = "[ – ] " + item->get_name();
		else
			name = "[ + ] " + item->get_name();
	}

	//std::string fstr = name + " " + item->get_value_text();
	std::string value = item->get_value_text();

	//overlay->draw_string({ 5 + padding, 3.f + static_cast<float>(menu_height += font_height) }, FONT_LEFT, color, "%s %s", name.c_str(), item->get_value_text().c_str());
	//DrawString(fstr.c_str(), (int)(5 + padding), (int)(3.f + static_cast<float>(menu_height += font_height)), 255, 255, 255, pFont);
	/*if (item->is_selected()) 
	{
		DrawString(fstr.c_str(), (int)(5 + padding), (int)(3.f + static_cast<float>(menu_height += font_height)), 255, 0, 0, pFont);
	}
	else 
	{
		DrawString(fstr.c_str(), (int)(5 + padding), (int)(3.f + static_cast<float>(menu_height += font_height)), 168, 168, 168, pFont);
	}*/

	int he = (int)(3.f + static_cast<float>(menu_height += font_height));
	if (item->is_selected())
	{
		DrawString(name.c_str(), (int)(5 + padding), he, 255, 0, 0, pFont);
		DrawString(value.c_str(), (int)(5 + padding) + rpad, he, 255, 0, 0, pFont);
	}
	else
	{	
		DrawString(name.c_str(), (int)(5 + padding), he, 168, 168, 168, pFont);
		DrawString(value.c_str(), (int)(5 + padding) + rpad, he, 168, 168, 168, pFont);
	}

	if (item->is_subfolder() && std::static_pointer_cast<D3DMenuSubFolderItem>(item)->is_opened())
	{
		static auto length = 22;

		for (const auto& sub : std::static_pointer_cast<D3DMenuSubFolderItem>(item)->get_sub_items())
		{
			draw_menu_item(sub, padding + length + 6.f);
		}
	}
}

std::vector<std::shared_ptr<ID3DMenuItem>> get_all_visible_items()
{
	std::vector<std::shared_ptr<ID3DMenuItem>> ret;

	std::function<void(const std::vector<std::shared_ptr<ID3DMenuItem>> & list)> add_folder_items;
	add_folder_items = [&](const std::vector<std::shared_ptr<ID3DMenuItem>>& list)
	{
		for (const auto& item : list)
		{
			ret.emplace_back(item);

			if (item->is_subfolder())
			{
				auto folder = std::static_pointer_cast<D3DMenuSubFolderItem>(item);

				if (folder->is_opened())
					add_folder_items(folder->get_sub_items());
			}
		}
	};

	add_folder_items(menu_items);

	return ret;
}

std::shared_ptr<ID3DMenuItem> get_current_selected()
{
	auto items = get_all_visible_items();
	for (const auto& item : items)
	{
		if (item->is_selected()) return item;
	}

	return nullptr;
}

void handle_left()
{
	auto current = get_current_selected();
	if (!current) return;
	current->handle_left();
}

void handle_right()
{
	auto current = get_current_selected();
	if (!current) return;
	current->handle_right();
}

void handle_up()
{
	auto items = get_all_visible_items();

	auto current = get_current_selected();
	auto index = std::find(items.begin(), items.end(), current) - items.begin();

	if (index == 0)
		return;

	index--;

	auto indexed = items[index];
	current->is_selected() = false;
	indexed->is_selected() = true;
}

void handle_down()
{
	auto items = get_all_visible_items();

	auto current = get_current_selected();
	auto index = std::find(items.begin(), items.end(), current) - items.begin();

	if (index >= static_cast<int>(items.size() - 1))
		return;

	index++;

	auto indexed = items[index];
	current->is_selected() = false;
	indexed->is_selected() = true;
}

void init_menu() 
{	
	s_showmenu = false;
	
	auto esp = std::make_shared<D3DMenuSubFolderItem>(xorstr_("Wallhack"));
	esp->add_sub_item(std::make_shared<D3DMenuBoolItem>(xorstr_("Box"), s_box, true));
	esp->add_sub_item(std::make_shared<D3DMenuBoolItem>(xorstr_("Name"), s_name, true));
	esp->add_sub_item(std::make_shared<D3DMenuBoolItem>(xorstr_("Health"), s_health, true));
	esp->add_sub_item(std::make_shared<D3DMenuBoolItem>(xorstr_("Distance"), s_distance, true));

	auto misc = std::make_shared<D3DMenuSubFolderItem>(xorstr_("Misc"));
	misc->add_sub_item(std::make_shared<D3DMenuBoolItem>(xorstr_("FPS limiter"), s_fpslimiter, true));

	menu_items.emplace_back(esp);
	menu_items.emplace_back(misc);

	menu_items.at(0)->is_selected() = true;
}

void render_menu(IDirect3DDevice9* device)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		s_showmenu = !s_showmenu;
	
	if (!s_showmenu)
		return;
	
	//DrawGUIBox(0, 0, 150, static_cast<int>(menu_height + font_height * 1.5f + 3), 41, 43, 54, 255, 25, 26, 32, 255);
	FillRGBC(0, 0, 175, static_cast<int>(menu_height + font_height * 1.5f + 3), D3DCOLOR_XRGB(0, 0, 0));
	
	menu_height = -static_cast<int>(font_height * 0.75f);

	if (GetAsyncKeyState(VK_LEFT) & 1)
		handle_left();
	else if (GetAsyncKeyState(VK_RIGHT) & 1)
		handle_right();
	else if (GetAsyncKeyState(VK_UP) & 1)
		handle_up();
	else if (GetAsyncKeyState(VK_DOWN) & 1)
		handle_down();

	for (const auto& item : menu_items)
	{
		draw_menu_item(item, 0);
	}

	//overlay->draw_gradient_box_outline({ 0, 0 }, 150, static_cast<int>(menu_height + font_height * 1.5f + 3), D3DCOLOR_XRGB(41, 43, 54), D3DCOLOR_XRGB(25, 26, 32), true);
	//DrawGUIBox(0, 0, 150, static_cast<int>(menu_height + font_height * 1.5f + 3), 41, 43, 54, 255, 25, 26, 32, 255);
}