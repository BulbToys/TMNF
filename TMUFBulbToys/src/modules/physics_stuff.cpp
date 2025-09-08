#include "../../core/bulbtoys.h"
#include "../TMNF.h"

#include <queue>

namespace physics_stuff
{
	// a list entry for the functions that have two vector arguments
	struct Entry2
	{
		uintptr_t object = 0;
		TMNF::GmVec3 first { 0 };
		TMNF::GmVec3 second { 0 };
	};

	// a list entry for the functions that have one vector argument
	struct Entry
	{
		uintptr_t object = 0;
		TMNF::GmVec3 vec { 0 };
	};

	// each panel keeps track of its own stored parameters from each function, and thus has to "listen" for the hooks
	// the hooks will iterate through this list of panels, and for each panel, update its respective stored parameters
	std::vector<void*> listeners;

	// declaring and defining a bulbtoys imgui panel that we will later send off to the main window
	struct PhysicsPanel : IPanel
	{
		// each panel has two lists of stored parameters for each function that we're hooking
		// the first one is updated by our hooks over time, depending on our interval (per frame or over X seconds)
		// the second one is displayed in the main window, depending on our interval (per frame or over X seconds)
		std::vector<Entry2> dyna_addforce2_list;
		std::vector<Entry2> dyna_addforce2_displayed;

		std::vector<Entry> dyna_addforce_list;
		std::vector<Entry> dyna_addforce_displayed;

		std::vector<Entry2> item_addforce2_list;
		std::vector<Entry2> item_addforce2_displayed;

		std::vector<Entry> item_addforce_list;
		std::vector<Entry> item_addforce_displayed;

		// if we choose to update every X seconds, this stopwatch will check elapsed time
		Stopwatch timer;

		// since these settings are stored inside the panel (not globally, in the module namespace), and each main window has its own panel
		// you can basically open as many main windows as you'd like, and have different settings for each panel
		int radio_interval = 0;

		float update_interval = 1.0f;

		char dyna_address[9] { 0 };
		char item_address[9] { 0 };

		PhysicsPanel() : IPanel()
		{
			// as soon as this panel is created, start our timer and add to the listeners
			timer.Start();
			listeners.push_back(this);
		}

		~PhysicsPanel()
		{
			// this panel is getting destroyed, remove from listeners
			listeners.erase(std::remove(listeners.begin(), listeners.end(), this), listeners.end());
		}

		virtual bool Draw() override final
		{
			// if radio_interval is 0, we update the info every frame
			bool should_update = true;
			if (radio_interval == 1)
			{
				// if radio_interval is 1, we update the info only once enough time has passed
				// NOTE: since we're effectively only updating this info within the Draw() function of the panel, which gets called every frame, it might be slightly inaccurate
				should_update = false;

				// has enough time passed for us to update?
				if (((float)timer.Elapsed() / 1'000'000) > update_interval)
				{
					timer.Reset();
					timer.Start();
					should_update = true;
				}
			}

			// if it's time for us to update the display, copy accumulated info over to displayed info, and clear accumulated info for our next interval
			if (should_update)
			{
				dyna_addforce2_displayed = dyna_addforce2_list;
				dyna_addforce2_list.clear();

				dyna_addforce_displayed = dyna_addforce_list;
				dyna_addforce_list.clear();

				item_addforce2_displayed = item_addforce2_list;
				item_addforce2_list.clear();
				
				item_addforce_displayed = item_addforce_list;
				item_addforce_list.clear();
			}

			// add a fancy little menu in the main window, for our settings
			if (ImGui::BulbToys_Menu("Physics Stuff"))
			{
				// radio button selection as to whether you want to update the display every frame or every X seconds
				ImGui::Text("Show new results:");
				ImGui::RadioButton("Every frame", &radio_interval, 0);
				ImGui::RadioButton("Every X seconds:", &radio_interval, 1);

				// if the textbox has been updated
				// (don't let the 'if' fool you, this textbox is shown all the time, it just returns false when it hasn't been updated this frame :3)
				if (ImGui::InputFloat("##UpdateInterval", &update_interval))
				{
					// clamp value to a minimum of 0.001 seconds
					if (update_interval < 0.001f)
					{
						update_interval = 0.001f;
					}
				}

				ImGui::Separator();

				// only display results for the object with the given address
				ImGui::Text("Filter CHmsDyna by address:");
				ImGui::InputText("##CHmsDyna_Filter", dyna_address, IM_ARRAYSIZE(dyna_address), ImGuiInputTextFlags_CharsHexadecimal);

				// convert from (hexadecimal) text to address
				uintptr_t dyna_filter = 0;
				sscanf_s(dyna_address, "%IX", &dyna_filter);

				ImGui::Text("Filter CHmsItem by address:");
				ImGui::InputText("##CHmsDyna_Filter", item_address, IM_ARRAYSIZE(item_address), ImGuiInputTextFlags_CharsHexadecimal);

				uintptr_t item_filter = 0;
				sscanf_s(item_address, "%IX", &item_filter);
			}

			// add a menu and name it after the function we're displaying
			if (ImGui::BulbToys_Menu("CHmsDyna::AddForce(GmVec3 const &, GmVec3 const &)"))
			{
				auto size = dyna_addforce2_displayed.size();

				for (int i = 0; i < size; i++)
				{
					// for each element (Entry2 or Entry) in our list of displayed parameters, display the values
					auto entry = dyna_addforce2_displayed.at(i);

					ImGui::BulbToys_AddyLabel(entry.object, "%d. HmsDyna", i + 1);
					ImGui::Text("%d. Vectors: (%.02f, %.02f, %.02f) and (%.02f, %.02f, %.02f)", i + 1, entry.first.x, entry.first.y, entry.first.z, entry.second.x, entry.second.y, entry.second.z);

					if (i < size - 1)
					{
						ImGui::Separator();
					}
				}
			}

			if (ImGui::BulbToys_Menu("CHmsDyna::AddForce(GmVec3 const &)"))
			{
				auto size = dyna_addforce_displayed.size();

				for (int i = 0; i < size; i++)
				{
					auto entry = dyna_addforce_displayed.at(i);

					ImGui::BulbToys_AddyLabel(entry.object, "%d. HmsDyna", i + 1);
					ImGui::Text("%d. Vector: (%.02f, %.02f, %.02f)", i + 1, entry.vec.x, entry.vec.y, entry.vec.z);

					if (i < size - 1)
					{
						ImGui::Separator();
					}
				}
			}

			if (ImGui::BulbToys_Menu("CHmsItem::AddForce(GmVec3 const &, GmVec3 const &)"))
			{
				auto size = item_addforce2_displayed.size();

				for (int i = 0; i < size; i++)
				{
					auto entry = item_addforce2_displayed.at(i);

					ImGui::BulbToys_AddyLabel(entry.object, "%d. HmsItem", i + 1);
					ImGui::Text("%d. Vectors: (%.02f, %.02f, %.02f) and (%.02f, %.02f, %.02f)", i + 1, entry.first.x, entry.first.y, entry.first.z, entry.second.x, entry.second.y, entry.second.z);

					if (i < size - 1)
					{
						ImGui::Separator();
					}
				}

			}

			if (ImGui::BulbToys_Menu("CHmsItem::AddForce(GmVec3 const &)"))
			{
				auto size = item_addforce_displayed.size();

				for (int i = 0; i < size; i++)
				{
					auto entry = item_addforce_displayed.at(i);

					ImGui::BulbToys_AddyLabel(entry.object, "%d. HmsItem", i + 1);
					ImGui::Text("%d. Vector: (%.02f, %.02f, %.02f)", i + 1, entry.vec.x, entry.vec.y, entry.vec.z);

					if (i < size - 1)
					{
						ImGui::Separator();
					}
				}
			}

			// rendering was successful, return true
			// you'd practically never need to return false here, but there can be weird one-off situations where that is necessary
			// hopefully you won't run into them lol
			return true;
		}
	};

	// this bulbtoys function gets called when bulbtoys wants to draw something in the main window or the overlay
	IPanel* Panel(Module::DrawType dt)
	{
		// create a new physics panel for every main window we open (F9)
		if (dt == Module::DrawType::MainWindow)
		{
			return new PhysicsPanel();
		}

		// we don't care about other drawtypes (the overlay)
		return nullptr;
	}

	// the HOOK macro takes an address, return type, calling convention, name and arguments (if any) and does 2 things:
	// 1. declares our hook function, using the same return type, calling convention, name (suffixed with an underscore) and arguments (if any)
	// 2. declares and defines the original function at the specified address, using the same return type, calling convention, name (suffixed with an underscore) and arguments (if any)

	// we can't declare __thiscall functions, so we use __fastcall instead, as it is nearly identical
	// __fastcall has an additional argument stored in the EDX register, but we don't use it nor do we want to touch it
	// so we just add it to the parameters and let it be (and pass it to the original function when necessary)

	// void __thiscall CHmsDyna::AddForce(GmVec3 const &,GmVec3 const &)
	HOOK(0x533A70, void, __fastcall, CHmsDyna_AddForce2, uintptr_t hms_dyna, void* edx, TMNF::GmVec3* first, TMNF::GmVec3* second);

	// void __thiscall CHmsDyna::AddForce(GmVec3 const &)
	HOOK(0x533B80, void, __fastcall, CHmsDyna_AddForce, uintptr_t hms_dyna, void* edx, TMNF::GmVec3* vec);

	// void __thiscall CHmsItem::AddForce(GmVec3 const &,GmVec3 const &)
	HOOK(0x53CF50, void, __fastcall, CHmsItem_AddForce2, uintptr_t hms_item, void* edx, TMNF::GmVec3* first, TMNF::GmVec3* second);

	// void __thiscall CHmsItem::AddForce(GmVec3 const &)
	HOOK(0x53CEB0, void, __fastcall, CHmsItem_AddForce, uintptr_t hms_item, void* edx, TMNF::GmVec3* vec);

	// this bulbtoys function gets called when the dll is attached to the game
	void Init()
	{
		// hook functions in declared order
		CREATE_HOOK(CHmsDyna_AddForce2);
		CREATE_HOOK(CHmsDyna_AddForce);
		CREATE_HOOK(CHmsItem_AddForce2);
		CREATE_HOOK(CHmsItem_AddForce);
	}

	// this bulbtoys function gets called when the dll is about to be detached from the game
	void End()
	{
		// unhook functions in reverse order
		Hooks::Destroy(0x53CE70);
		Hooks::Destroy(0x53CD80);
		Hooks::Destroy(0x533A50);
		Hooks::Destroy(0x533940);
	}

	// when the game tries to call "void __thiscall CHmsDyna::AddForce(GmVec3 const &,GmVec3 const &)", it will land here first
	void __fastcall CHmsDyna_AddForce2_(uintptr_t hms_dyna, void* edx, TMNF::GmVec3* first, TMNF::GmVec3* second)
	{
		// store the parameters for all listener panels
		for (auto& panel : listeners)
		{
			((PhysicsPanel*)panel)->dyna_addforce2_list.push_back({ hms_dyna, *first, *second });
		}

		// then call the original game function, so the game can perform the physics calculations
		// if you'd want to create a so-called "post hook", ie. doing something after the function you're hooking is done, you'd put this line first, and then everything else
		CHmsDyna_AddForce2(hms_dyna, edx, first, second);
	}

	void __fastcall CHmsDyna_AddForce_(uintptr_t hms_dyna, void* edx, TMNF::GmVec3* vec)
	{
		for (auto& panel : listeners)
		{
			((PhysicsPanel*)panel)->dyna_addforce_list.push_back({ hms_dyna, *vec });
		}

		CHmsDyna_AddForce(hms_dyna, edx, vec);
	}

	void __fastcall CHmsItem_AddForce2_(uintptr_t hms_item, void* edx, TMNF::GmVec3* first, TMNF::GmVec3* second)
	{
		for (auto& panel : listeners)
		{
			((PhysicsPanel*)panel)->item_addforce2_list.push_back({ hms_item, *first, *second });
		}

		CHmsItem_AddForce2(hms_item, edx, first, second);
	}

	void __fastcall CHmsItem_AddForce_(uintptr_t hms_item, void* edx, TMNF::GmVec3* vec)
	{
		for (auto& panel : listeners)
		{
			((PhysicsPanel*)panel)->item_addforce_list.push_back({ hms_item, *vec });
		}

		CHmsItem_AddForce(hms_item, edx, vec);
	}
}

// as physics_stuff is a module, we need to tell bulbtoys we have a Panel(), Init() and End()
MODULE(physics_stuff);