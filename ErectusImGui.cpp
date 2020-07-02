#include "ErectusInclude.h"

bool ImGuiContextCreated = false;
bool ImGuiD3D9Initialized = false;
bool ImGuiWin32Initialized = false;
bool ImGuiInitialized = false;

POINT PointerPosition = { 0, 0 };
ImVec2 PointerOrigin = { 0.0f, 0.0f };
bool PointerDrag = false;

DWORD64 GetPtrResult = 0;
DWORD64 GetAddressResult = 0;

bool SwapperSourceToggle = false;
bool SwapperDestinationToggle = false;

bool TransferSourceToggle = false;
bool TransferDestinationToggle = false;

int AlphaCode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int BravoCode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int CharlieCode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

char **FavoritedWeaponsArray = nullptr;

bool DragMenu()
{
	if (!GetCursorPos(&PointerPosition)) return false;

	WindowPosition[0] = PointerPosition.x - int(PointerOrigin.x);
	WindowPosition[1] = PointerPosition.y - int(PointerOrigin.y);

	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	if (WindowPosition[0] < 0)
	{
		WindowPosition[0] = 0;
	}

	if (WindowPosition[1] < 0)
	{
		WindowPosition[1] = 0;
	}

	if (WindowPosition[0] > (ScreenX - WindowSize[0]))
	{
		WindowPosition[0] = (ScreenX - WindowSize[0]);
	}

	if (WindowPosition[1] > (ScreenY - WindowSize[1]))
	{
		WindowPosition[1] = (ScreenY - WindowSize[1]);
	}

	return MoveWindow(WindowHwnd, WindowPosition[0], WindowPosition[1], WindowSize[0], WindowSize[1], FALSE);
}

void ProcessMenu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(float(WindowSize[0]), float(WindowSize[1])));
	ImGui::SetNextWindowCollapsed(false);

	bool AllowDrag = true;
	if (ImGui::Begin("ErectusReborn - Process Menu", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Exit"))
			{
				Close();
			}

			if (ProcessSelected)
			{
				if (ImGui::MenuItem("Overlay Menu"))
				{
					SetOverlayMenu();
				}

				if (ImGui::MenuItem("Overlay"))
				{
					SetOverlayPosition(false, true);
				}
			}
			else
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				ImGui::MenuItem("Overlay Menu");
				ImGui::MenuItem("Overlay");
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}

			ImGui::EndMenuBar();
		}

		ImGui::SetNextItemWidth(float(WindowSize[0]) - 16.0f);
		if (ImGui::BeginCombo("###ProcessList", ProcessList[ProcessIndex]))
		{
			if (!ProcessListUpdated)
			{
				ProcessListUpdated = true;
				if (!UpdateProcessList())
				{
					ResetProcessData(true, 1);
				}
			}

			for (int i = 0; i < ProcessListSize; i++)
			{
				if (ImGui::Selectable(ProcessList[i]))
				{
					ProcessIndex = i;
					if (ProcessIndex)
					{
						ProcessSelected = ProcessValid(ProcessIdList[ProcessIndex]);
						if (!ProcessSelected)
						{
							ResetProcessData(false, 1);
						}
					}
				}
			}

			ImGui::EndCombo();
			AllowDrag = false;
		}
		else
		{
			if (ProcessListUpdated)
			{
				ProcessListUpdated = false;
			}
		}

		ImGui::Separator();
		switch (ProcessErrorId)
		{
		case 0:
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ProcessError);
			break;
		case 1:
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ProcessError);
			break;
		case 2:
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ProcessError);
			break;
		default:
			ImGui::Text(ProcessError);
			break;
		}

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Columns(2);
		ImGui::Separator();
		ImGui::Text("Overlay Menu Keybind");
		ImGui::NextColumn();
		ImGui::Text("CTRL+ENTER");
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("ImGui (D3D9) FPS");
		ImGui::NextColumn();
		ImGui::Text("%.1f", ImGui::GetIO().Framerate);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("PID (Process Id)");
		ImGui::NextColumn();
		ImGui::Text("%lu", Pid);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("HWND (Window)");
		ImGui::NextColumn();
		ImGui::Text("%p", Hwnd);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("Base Address");
		ImGui::NextColumn();
		ImGui::Text("%016llX", Exe);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("HANDLE");
		ImGui::NextColumn();
		ImGui::Text("%p", Handle);
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopItemFlag();

		if (ImGui::IsMouseDragging(0) && AllowDrag)
		{
			if (!PointerDrag)
			{
				PointerOrigin = ImGui::GetMousePos();
				PointerDrag = true;
			}
		}
		else
		{
			if (PointerDrag)
			{
				PointerOrigin = { 0.0f, 0.0f };
				PointerDrag = false;
			}
		}

		if (PointerDrag)
		{
			DragMenu();
		}
	}

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void ButtonToggle(const char *Label, bool *State)
{
	if (*State)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(224.0f, 0.0f))) *State = false;
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(224.0f, 0.0f))) *State = true;
		ImGui::PopStyleColor(3);
	}
}

void LargeButtonToggle(const char *Label, bool *State)
{
	if (*State)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(451.0f, 0.0f))) *State = false;
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(451.0f, 0.0f))) *State = true;
		ImGui::PopStyleColor(3);
	}
}

void SmallButtonToggle(const char *Label, bool *State)
{
	if (*State)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(110.0f, 0.0f))) *State = false;
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(110.0f, 0.0f))) *State = true;
		ImGui::PopStyleColor(3);
	}
}

void OverlayMenu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(float(WindowSize[0]), float(WindowSize[1])));
	ImGui::SetNextWindowCollapsed(false);

	bool AllowDrag = true;
	if (ImGui::Begin("ErectusReborn - Overlay Menu", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Exit"))
			{
				Close();
			}

			if (ImGui::MenuItem("Process Menu"))
			{
				SetProcessMenu();
			}

			if (ImGui::MenuItem("Overlay"))
			{
				if (!SetOverlayPosition(false, true))
				{
					SetProcessMenu();
				}
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::BeginTabBar("###OverlayMenuTabBar", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("ESP###ESPTab"))
			{
				if (ImGui::CollapsingHeader("Player ESP Settings"))
				{
					ButtonToggle("Player ESP Enabled", &PlayerSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###PlayerSettingsEnabledDistance", &PlayerSettings.EnabledDistance, 0, 3000, "Distance: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&PlayerSettings.EnabledDistance, 0, 3000);

					ButtonToggle("Draw Living Players", &PlayerSettings.DrawAlive);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsAliveColor", PlayerSettings.AliveColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.AliveColor);

					ButtonToggle("Draw Downed Players", &PlayerSettings.DrawDowned);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsDownedColor", PlayerSettings.DownedColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.DownedColor);

					ButtonToggle("Draw Dead Players", &PlayerSettings.DrawDead);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsDeadColor", PlayerSettings.DeadColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.DeadColor);

					ButtonToggle("Draw Unknown Players", &PlayerSettings.DrawUnknown);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsUnknownColor", PlayerSettings.UnknownColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.UnknownColor);

					ButtonToggle("Draw Enabled Players", &PlayerSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###PlayerSettingsEnabledAlpha", &PlayerSettings.EnabledAlpha, 0.0f, 1.0f, "Alpha: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&PlayerSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Draw Disabled Players", &PlayerSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###PlayerSettingsDisabledAlpha", &PlayerSettings.DisabledAlpha, 0.0f, 1.0f, "Alpha: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&PlayerSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Draw Named Players", &PlayerSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Draw Unnamed Players", &PlayerSettings.DrawUnnamed);

					ButtonToggle("Show Player Name", &PlayerSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Show Player Distance", &PlayerSettings.ShowDistance);

					ButtonToggle("Show Player Health", &PlayerSettings.ShowHealth);
					ImGui::SameLine(235.0f);
					ButtonToggle("Show Dead Player Health", &PlayerSettings.ShowDeadHealth);

					ButtonToggle("Player Text Shadowed", &PlayerSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Player Text Centered", &PlayerSettings.TextCentered);
				}

				if (ImGui::CollapsingHeader("NPC ESP Settings"))
				{
					ButtonToggle("NPC ESP Enabled", &NpcSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###NpcSettingsEnabledDistance", &NpcSettings.EnabledDistance, 0, 3000, "Distance: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&NpcSettings.EnabledDistance, 0, 3000);

					ButtonToggle("Draw Living NPCs", &NpcSettings.DrawAlive);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsAliveColor", NpcSettings.AliveColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.AliveColor);

					ButtonToggle("Draw Downed NPCs", &NpcSettings.DrawDowned);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsDownedColor", NpcSettings.DownedColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.DownedColor);

					ButtonToggle("Draw Dead NPCs", &NpcSettings.DrawDead);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsDeadColor", NpcSettings.DeadColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.DeadColor);

					ButtonToggle("Draw Unknown NPCs", &NpcSettings.DrawUnknown);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsUnknownColor", NpcSettings.UnknownColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.UnknownColor);

					ButtonToggle("Draw Enabled NPCs", &NpcSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###NpcSettingsEnabledAlpha", &NpcSettings.EnabledAlpha, 0.0f, 1.0f, "Alpha: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&NpcSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Draw Disabled NPCs", &NpcSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###NpcSettingsDisabledAlpha", &NpcSettings.DisabledAlpha, 0.0f, 1.0f, "Alpha: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&NpcSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Draw Named NPCs", &NpcSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Draw Unnamed NPCs", &NpcSettings.DrawUnnamed);

					ButtonToggle("Show NPC Name", &NpcSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Show NPC Distance", &NpcSettings.ShowDistance);

					ButtonToggle("Show NPC Health", &NpcSettings.ShowHealth);
					ImGui::SameLine(235.0f);
					ButtonToggle("Show Dead NPC Health", &NpcSettings.ShowDeadHealth);

					ButtonToggle("NPC Text Shadowed", &NpcSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("NPC Text Centered", &NpcSettings.TextCentered);

					ButtonToggle("Always Draw Living 1* NPCs", &CustomLegendarySettings.OverrideLivingOneStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###LivingOneStarColor", CustomLegendarySettings.LivingOneStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.LivingOneStarColor);

					ButtonToggle("Always Draw Dead 1* NPCs", &CustomLegendarySettings.OverrideDeadOneStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###DeadOneStarColor", CustomLegendarySettings.DeadOneStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.DeadOneStarColor);

					ButtonToggle("Always Draw Living 2* NPCs", &CustomLegendarySettings.OverrideLivingTwoStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###LivingTwoStarColor", CustomLegendarySettings.LivingTwoStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.LivingTwoStarColor);

					ButtonToggle("Always Draw Dead 2* NPCs", &CustomLegendarySettings.OverrideDeadTwoStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###DeadTwoStarColor", CustomLegendarySettings.DeadTwoStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.DeadTwoStarColor);

					ButtonToggle("Always Draw Living 3* NPCs", &CustomLegendarySettings.OverrideLivingThreeStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###LivingThreeStarColor", CustomLegendarySettings.LivingThreeStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.LivingThreeStarColor);

					ButtonToggle("Always Draw Dead 3* NPCs", &CustomLegendarySettings.OverrideDeadThreeStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###DeadThreeStarColor", CustomLegendarySettings.DeadThreeStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.DeadThreeStarColor);

					LargeButtonToggle("Hide NPCs in the Settler Faction", &CustomExtraNPCSettings.HideSettlerFaction);
					LargeButtonToggle("Hide NPCs in the Crater Raider Faction", &CustomExtraNPCSettings.HideCraterRaiderFaction);
					LargeButtonToggle("Hide NPCs in the Diehards Faction", &CustomExtraNPCSettings.HideDieHardFaction);
					LargeButtonToggle("Hide NPCs in the Secret Service Faction", &CustomExtraNPCSettings.HideSecretServiceFaction);

					LargeButtonToggle("NPC Blacklist Enabled", &CustomExtraNPCSettings.UseNPCBlacklist);
					if (ImGui::CollapsingHeader("NPC Blacklist"))
					{
						for (int i = 0; i < 64; i++)
						{
							char NPCBlacklistEnabledText[sizeof("NPC Blacklist: 63")];
							char NPCBlacklistLabelText[sizeof("###NPCBlacklist63")];
							sprintf_s(NPCBlacklistEnabledText, "NPC Blacklist: %d", i);
							sprintf_s(NPCBlacklistLabelText, "###NPCBlacklist%d", i);
							ButtonToggle(NPCBlacklistEnabledText, &CustomExtraNPCSettings.NPCBlacklistEnabled[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomExtraNPCSettings.NPCBlacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(NPCBlacklistLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomExtraNPCSettings.NPCBlacklist[i]);
						}
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Combat###CombatTab"))
			{
				if (ImGui::CollapsingHeader("Weapon Editor"))
				{
					ButtonToggle("No Recoil", &CustomWeaponSettings.NoRecoil);
					ImGui::SameLine(235.0f);
					ButtonToggle("Infinite Ammo", &CustomWeaponSettings.InfiniteAmmo);

					ButtonToggle("No Spread", &CustomWeaponSettings.NoSpread);
					ImGui::SameLine(235.0f);
					ButtonToggle("Instant Reload", &CustomWeaponSettings.InstantReload);

					ButtonToggle("No Sway", &CustomWeaponSettings.NoSway);
					ImGui::SameLine(235.0f);
					ButtonToggle("Automatic Flag###WeaponAutomatic", &CustomWeaponSettings.Automaticflag);

					ButtonToggle("Capacity###WeaponCapacityEnabled", &CustomWeaponSettings.CapacityEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###WeaponCapacity", &CustomWeaponSettings.Capacity, 0, 999, "Capacity: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomWeaponSettings.Capacity, 0, 999);

					ButtonToggle("Speed###WeaponSpeedEnabled", &CustomWeaponSettings.SpeedEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###WeaponSpeed", &CustomWeaponSettings.Speed, 0.0f, 100.0f, "Speed: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomWeaponSettings.Speed, 0.0f, 100.0f);

					ButtonToggle("Reach###WeaponReachEnabled", &CustomWeaponSettings.ReachEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###WeaponReach", &CustomWeaponSettings.Reach, 0.0f, 999.0f, "Reach: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomWeaponSettings.Reach, 0.0f, 999.0f);
				}

				if (ImGui::CollapsingHeader("Targeting Settings"))
				{
					ButtonToggle("Player Targeting (Keybind: T)", &CustomTargetSettings.LockPlayers);
					ImGui::SameLine(235.0f);
					ButtonToggle("NPC Targeting (Keybind: T)", &CustomTargetSettings.LockNPCs);

					ButtonToggle("Damage Redirection (Players)", &CustomTargetSettings.IndirectPlayers);
					ImGui::SameLine(235.0f);
					ButtonToggle("Damage Redirection (NPCs)", &CustomTargetSettings.IndirectNPCs);

					ButtonToggle("Send Damage (Players)", &CustomTargetSettings.DirectPlayers);
					ImGui::SameLine(235.0f);
					ButtonToggle("Send Damage (NPCs)", &CustomTargetSettings.DirectNPCs);

					SmallButtonToggle("Living###TargetLiving", &CustomTargetSettings.TargetLiving);
					ImGui::SameLine(122.0f);
					SmallButtonToggle("Downed###TargetDowned", &CustomTargetSettings.TargetDowned);
					ImGui::SameLine(235.0f);
					SmallButtonToggle("Dead###TargetDead", &CustomTargetSettings.TargetDead);
					ImGui::SameLine(349.0f);
					SmallButtonToggle("Unknown###TargetUnknown", &CustomTargetSettings.TargetUnknown);

					ButtonToggle("Ignore Render Distance###IgnoreRenderDistance", &CustomTargetSettings.IgnoreRenderDistance);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###TargetLockingFOV", &CustomTargetSettings.LockingFOV, 5.0f, 40.0f, "Targeting FOV: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomTargetSettings.LockingFOV, 5.0f, 40.0f);

					ButtonToggle("Ignore Essential NPCs###IgnoreEssentialNPCs", &CustomTargetSettings.IgnoreEssentialNPCs);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###TargetLockingColor", CustomTargetSettings.LockingColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.UnknownColor);

					ButtonToggle("Automatic Retargeting###TargetLockingRetargeting", &CustomTargetSettings.Retargeting);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char TargetLockingCooldownText[sizeof("Cooldown: 120 (1920 ms)")];
					sprintf_s(TargetLockingCooldownText, "Cooldown: %d (%d ms)", CustomTargetSettings.Cooldown, CustomTargetSettings.Cooldown * 16);
					ImGui::SliderInt("###TargetLockingCooldown", &CustomTargetSettings.Cooldown, 0, 120, TargetLockingCooldownText);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomTargetSettings.Cooldown, 0, 120);

					ImGui::SetNextItemWidth(224.0f);
					char SendDamageMinText[sizeof("Send Damage (Min): 60 (960 ms)")];
					sprintf_s(SendDamageMinText, "Send Damage (Min): %d (%d ms)", CustomTargetSettings.SendDamageMin, CustomTargetSettings.SendDamageMin * 16);
					ImGui::SliderInt("###SendDamageMin", &CustomTargetSettings.SendDamageMin, 1, 60, SendDamageMinText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomTargetSettings.SendDamageMax < CustomTargetSettings.SendDamageMin)
						{
							CustomTargetSettings.SendDamageMax = CustomTargetSettings.SendDamageMin;
						}
					}
					ValidateInt(&CustomTargetSettings.SendDamageMin, 1, 60);
					ValidateInt(&CustomTargetSettings.SendDamageMax, 1, 60);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char SendDamageMaxText[sizeof("Send Damage (Max): 60 (960 ms)")];
					sprintf_s(SendDamageMaxText, "Send Damage (Max): %d (%d ms)", CustomTargetSettings.SendDamageMax, CustomTargetSettings.SendDamageMax * 16);
					ImGui::SliderInt("###SendDamageMax", &CustomTargetSettings.SendDamageMax, 1, 60, SendDamageMaxText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomTargetSettings.SendDamageMax < CustomTargetSettings.SendDamageMin)
						{
							CustomTargetSettings.SendDamageMin = CustomTargetSettings.SendDamageMax;
						}
					}
					ValidateInt(&CustomTargetSettings.SendDamageMin, 1, 60);
					ValidateInt(&CustomTargetSettings.SendDamageMax, 1, 60);
					if (CustomTargetSettings.SendDamageMax < CustomTargetSettings.SendDamageMin)
					{
						CustomTargetSettings.SendDamageMax = CustomTargetSettings.SendDamageMin;
					}

					char *FavoritedWeaponsPreview = nullptr;
					if (CustomTargetSettings.FavoriteIndex < 12)
					{
						FavoritedWeaponsPreview = GetFavoritedWeaponText(BYTE(CustomTargetSettings.FavoriteIndex));
						if (FavoritedWeaponsPreview == nullptr)
						{
							FavoritedWeaponsPreview = new char[sizeof("[?] Favorited Item Invalid")];
							sprintf_s(FavoritedWeaponsPreview, sizeof("[?] Favorited Item Invalid"), "[%c] Favorited Item Invalid", GetFavoriteSlot(BYTE(CustomTargetSettings.FavoriteIndex)));
						}
					}
					else
					{
						FavoritedWeaponsPreview = new char[sizeof("[?] No Weapon Selected")];
						sprintf_s(FavoritedWeaponsPreview, sizeof("[?] No Weapon Selected"), "[?] No Weapon Selected");
					}

					ImGui::SetNextItemWidth(451.0f);
					if (ImGui::BeginCombo("###BeginTempCombo", FavoritedWeaponsPreview))
					{
						FavoritedWeaponsArray = GetFavoritedWeapons();
						if (FavoritedWeaponsArray == nullptr)
						{
							FavoritedWeaponsArray = new char*[13];
							FavoritedWeaponsArray[0] = new char[sizeof("[?] No Weapon Selected")];
							sprintf_s(FavoritedWeaponsArray[0], sizeof("[?] No Weapon Selected"), "[?] No Weapon Selected");
							for (int i = 1; i < 13; i++)
							{
								FavoritedWeaponsArray[i] = new char[sizeof("[?] Favorited Item Invalid")];
								sprintf_s(FavoritedWeaponsArray[i], sizeof("[?] Favorited Item Invalid"), "[%c] Favorited Item Invalid", GetFavoriteSlot(BYTE(i - 1)));
							}
						}

						for (int i = 0; i < 13; i++)
						{
							if (ImGui::Selectable(FavoritedWeaponsArray[i]))
							{
								if (i)
								{
									CustomTargetSettings.FavoriteIndex = i - 1;
								}
								else
								{
									CustomTargetSettings.FavoriteIndex = 12;
								}
							}
						}

						ImGui::EndCombo();
						AllowDrag = false;
					}

					if (FavoritedWeaponsPreview != nullptr)
					{
						delete[]FavoritedWeaponsPreview;
						FavoritedWeaponsPreview = nullptr;
					}

					if (FavoritedWeaponsArray != nullptr)
					{
						for (int i = 0; i < 13; i++)
						{
							if (FavoritedWeaponsArray[i] != nullptr)
							{
								delete[]FavoritedWeaponsArray[i];
								FavoritedWeaponsArray[i] = nullptr;
							}
						}

						delete[]FavoritedWeaponsArray;
						FavoritedWeaponsArray = nullptr;
					}

					ValidateInt(&CustomTargetSettings.FavoriteIndex, 0, 12);
				}

				if (ImGui::CollapsingHeader("Melee Settings"))
				{
					LargeButtonToggle("Melee Enabled (Keybind: U)", &CustomMeleeSettings.MeleeEnabled);

					ImGui::SetNextItemWidth(224.0f);
					char MeleeSpeedMinText[sizeof("Melee Speed (Min): 60 (960 ms)")];
					sprintf_s(MeleeSpeedMinText, "Melee Speed (Min): %d (%d ms)", CustomMeleeSettings.MeleeSpeedMin, CustomMeleeSettings.MeleeSpeedMin * 16);
					ImGui::SliderInt("###MeleeSpeedMin", &CustomMeleeSettings.MeleeSpeedMin, 1, 60, MeleeSpeedMinText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomMeleeSettings.MeleeSpeedMax < CustomMeleeSettings.MeleeSpeedMin)
						{
							CustomMeleeSettings.MeleeSpeedMax = CustomMeleeSettings.MeleeSpeedMin;
						}
					}
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMin, 1, 60);
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMax, 1, 60);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char MeleeSpeedMaxText[sizeof("Melee Speed (Max): 60 (960 ms)")];
					sprintf_s(MeleeSpeedMaxText, "Melee Speed (Max): %d (%d ms)", CustomMeleeSettings.MeleeSpeedMax, CustomMeleeSettings.MeleeSpeedMax * 16);
					ImGui::SliderInt("###MeleeSpeedMax", &CustomMeleeSettings.MeleeSpeedMax, 1, 60, MeleeSpeedMaxText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomMeleeSettings.MeleeSpeedMax < CustomMeleeSettings.MeleeSpeedMin)
						{
							CustomMeleeSettings.MeleeSpeedMin = CustomMeleeSettings.MeleeSpeedMax;
						}
					}
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMin, 1, 60);
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMax, 1, 60);
					if (CustomMeleeSettings.MeleeSpeedMax < CustomMeleeSettings.MeleeSpeedMin)
					{
						CustomMeleeSettings.MeleeSpeedMax = CustomMeleeSettings.MeleeSpeedMin;
					}
				}

				if (ImGui::CollapsingHeader("One Position Kill"))
				{
					LargeButtonToggle("OPK Players (Keybind: CTRL+B)", &CustomOpkSettings.PlayersEnabled);
					LargeButtonToggle("OPK NPCs (Keybind: CTRL+N)", &CustomOpkSettings.NpcsEnabled);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Player###PlayerTab"))
			{
				if (ImGui::CollapsingHeader("Local Player Settings"))
				{
					LargeButtonToggle("Position Spoofing (Keybind CTRL+L)##LocalPlayerPositionSpoofingEnabled", &CustomLocalPlayerSettings.PositionSpoofingEnabled);
					ButtonToggle("Draw Position Status###LocalPlayerDrawPositionSpoofingEnabled", &CustomLocalPlayerSettings.DrawPositionSpoofingEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerPositionSpoofingHeight", &CustomLocalPlayerSettings.PositionSpoofingHeight, -524287, 524287, "Spoofed Height: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.PositionSpoofingHeight, -524287, 524287);

					ButtonToggle("Noclip (Keybind CTRL+Y)###NoclipEnabled", &CustomLocalPlayerSettings.NoclipEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###NoclipSpeed", &CustomLocalPlayerSettings.NoclipSpeed, 0.0f, 2.0f, "Speed: %.5f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomLocalPlayerSettings.NoclipSpeed, 0.0f, 2.0f);

					ButtonToggle("Client State", &CustomLocalPlayerSettings.ClientState);
					ImGui::SameLine(235.0f);
					ButtonToggle("Automatic Client State", &CustomLocalPlayerSettings.AutomaticClientState);

					LargeButtonToggle("Freeze Action Points###LocalPlayerFreezeApEnabled", &CustomLocalPlayerSettings.FreezeApEnabled);

					ButtonToggle("Action Points###LocalPlayerAPEnabled", &CustomLocalPlayerSettings.ActionPointsEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerAP", &CustomLocalPlayerSettings.ActionPoints, 0, 99999, "Action Points: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.ActionPoints, 0, 99999);

					ButtonToggle("Strength###LocalPlayerStrengthEnabled", &CustomLocalPlayerSettings.StrengthEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerStrength", &CustomLocalPlayerSettings.Strength, 0, 99999, "Strength: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Strength, 0, 99999);

					ButtonToggle("Perception###LocalPlayerPerceptionEnabled", &CustomLocalPlayerSettings.PerceptionEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerPerception", &CustomLocalPlayerSettings.Perception, 0, 99999, "Perception: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Perception, 0, 99999);

					ButtonToggle("Endurance###LocalPlayerEnduranceEnabled", &CustomLocalPlayerSettings.EnduranceEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerEndurance", &CustomLocalPlayerSettings.Endurance, 0, 99999, "Endurance: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Endurance, 0, 99999);

					ButtonToggle("Charisma###LocalPlayerCharismaEnabled", &CustomLocalPlayerSettings.CharismaEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerCharisma", &CustomLocalPlayerSettings.Charisma, 0, 99999, "Charisma: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Charisma, 0, 99999);

					ButtonToggle("Intelligence###LocalPlayerIntelligenceEnabled", &CustomLocalPlayerSettings.IntelligenceEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerIntelligence", &CustomLocalPlayerSettings.Intelligence, 0, 99999, "Intelligence: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Intelligence, 0, 99999);

					ButtonToggle("Agility###LocalPlayerAgilityEnabled", &CustomLocalPlayerSettings.AgilityEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerAgility", &CustomLocalPlayerSettings.Agility, 0, 99999, "Agility: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Agility, 0, 99999);

					ButtonToggle("Luck###LocalPlayerLuckEnabled", &CustomLocalPlayerSettings.LuckEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerLuck", &CustomLocalPlayerSettings.Luck, 0, 99999, "Luck: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Luck, 0, 99999);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Utility###UtilityTab"))
			{
				if (ImGui::CollapsingHeader("Utility"))
				{
					ButtonToggle("Draw Local Player Data", &CustomUtilitySettings.DebugPlayer);
					ImGui::SameLine(235.0f);
					ButtonToggle("ESP Debug Mode", &CustomUtilitySettings.DebugEsp);

					if (CustomUtilitySettings.PtrFormid)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Get Pointer###GetPointerEnabled", ImVec2(224.0f, 0.0f)))
						{
							GetPtrResult = GetPtr(CustomUtilitySettings.PtrFormid);
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Get Pointer###GetPointerDisabled", ImVec2(224.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					ImGui::SameLine(235.0f);
					char PtrFormidText[sizeof("00000000")];
					sprintf_s(PtrFormidText, "%08lX", CustomUtilitySettings.PtrFormid);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::InputText("###PtrFormidText", PtrFormidText, sizeof(PtrFormidText), ImGuiInputTextFlags_CharsHexadecimal))
					{
						GetPtrResult = 0;
					}
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(PtrFormidText, "%08lX", &CustomUtilitySettings.PtrFormid);

					ImGui::SameLine(318.0f);
					char PtrPointerText[sizeof("0000000000000000")];
					sprintf_s(PtrPointerText, "%016llX", GetPtrResult);
					ImGui::SetNextItemWidth(141.0f);
					ImGui::InputText("###PtrPointerText", PtrPointerText, sizeof(PtrPointerText), ImGuiInputTextFlags_ReadOnly);
					if (ImGui::IsItemActive()) AllowDrag = false;

					if (CustomUtilitySettings.AddressFormid)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Get Address###GetAddressEnabled", ImVec2(224.0f, 0.0f)))
						{
							GetAddressResult = GetAddress(CustomUtilitySettings.AddressFormid);
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Get Address###GetAddressDisabled", ImVec2(224.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					ImGui::SameLine(235.0f);
					char AddressFormidText[sizeof("00000000")];
					sprintf_s(AddressFormidText, "%08lX", CustomUtilitySettings.AddressFormid);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::InputText("###AddressFormidText", AddressFormidText, sizeof(AddressFormidText), ImGuiInputTextFlags_CharsHexadecimal))
					{
						GetAddressResult = 0;
					}
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(AddressFormidText, "%08lX", &CustomUtilitySettings.AddressFormid);

					ImGui::SameLine(318.0f);
					char AddressPointerText[sizeof("0000000000000000")];
					sprintf_s(AddressPointerText, "%016llX", GetAddressResult);
					ImGui::SetNextItemWidth(141.0f);
					ImGui::InputText("###AddressPointerText", AddressPointerText, sizeof(AddressPointerText), ImGuiInputTextFlags_ReadOnly);
					if (ImGui::IsItemActive()) AllowDrag = false;
				}

				if (ImGui::CollapsingHeader("Item Transferring"))
				{
					SmallButtonToggle("Source###TransferSourceFormidToggle", &TransferSourceToggle);

					ImGui::SameLine(122.0f);
					char SourceFormidText[sizeof("00000000")];
					sprintf_s(SourceFormidText, "%08lX", CustomTransferSettings.SourceFormid);
					ImGui::SetNextItemWidth(110.0f);
					ImGui::InputText("###TransferSourceFormidText", SourceFormidText, sizeof(SourceFormidText), ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(SourceFormidText, "%08lX", &CustomTransferSettings.SourceFormid);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
					ImGui::SameLine(235.0f);
					if (ImGui::Button("Get Player###TransferSourceLocalPlayer", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.SourceFormid = GetLocalPlayerFormid();
					}
					ImGui::SameLine(349.0f);
					if (ImGui::Button("Get STASH###TransferSourceSTASH", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.SourceFormid = GetStashFormid();
					}
					ImGui::PopStyleColor(3);

					SmallButtonToggle("Destination###TransferDestinationFormidToggle", &TransferDestinationToggle);
					ImGui::SameLine(122.0f);
					char DestinationFormidText[sizeof("00000000")];
					sprintf_s(DestinationFormidText, "%08lX", CustomTransferSettings.DestinationFormid);
					ImGui::SetNextItemWidth(110.0f);
					ImGui::InputText("###TransferDestinationFormidText", DestinationFormidText, sizeof(DestinationFormidText), ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(DestinationFormidText, "%08lX", &CustomTransferSettings.DestinationFormid);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
					ImGui::SameLine(235.0f);
					if (ImGui::Button("Get Player###TransferDestinationLocalPlayer", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.DestinationFormid = GetLocalPlayerFormid();
					}
					ImGui::SameLine(349.0f);
					if (ImGui::Button("Get STASH###TransferDestinationSTASH", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.DestinationFormid = GetStashFormid();
					}
					ImGui::PopStyleColor(3);

					bool AllowTransfer = false;

					if (TransferSourceToggle && CustomTransferSettings.SourceFormid && TransferDestinationToggle && CustomTransferSettings.DestinationFormid)
					{
						if (CustomTransferSettings.UseWhitelist)
						{
							AllowTransfer = CheckItemTransferList();
						}
						else
						{
							AllowTransfer = true;
						}
					}

					if (AllowTransfer)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Transfer Items###TransferItemsEnabled", ImVec2(451.0f, 0.0f)))
						{
							TransferItems(CustomTransferSettings.SourceFormid, CustomTransferSettings.DestinationFormid);
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Transfer Items###TransferItemsDisabled", ImVec2(451.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					LargeButtonToggle("Use Item Transfer Whitelist", &CustomTransferSettings.UseWhitelist);
					LargeButtonToggle("Use Item Transfer Blacklist", &CustomTransferSettings.UseBlacklist);

					if (ImGui::CollapsingHeader("Item Transfer Whitelist Settings"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Transfer Whitelist Slot: 31")];
							char WhitelistText[sizeof("###ItemTransferWhitelist31")];
							sprintf_s(WhitelistedText, "Transfer Whitelist Slot: %d", i);
							sprintf_s(WhitelistText, "###ItemTransferWhitelist%d", i);
							ButtonToggle(WhitelistedText, &CustomTransferSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomTransferSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomTransferSettings.Whitelist[i]);
						}
					}

					if (ImGui::CollapsingHeader("Item Transfer Blacklist Settings"))
					{
						for (int i = 0; i < 32; i++)
						{
							char BlacklistedText[sizeof("Transfer Blacklist Slot: 31")];
							char BlacklistText[sizeof("###ItemTransferBlacklist31")];
							sprintf_s(BlacklistedText, "Transfer Blacklist Slot: %d", i);
							sprintf_s(BlacklistText, "###ItemTransferBlacklist%d", i);
							ButtonToggle(BlacklistedText, &CustomTransferSettings.Blacklisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomTransferSettings.Blacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(BlacklistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomTransferSettings.Blacklist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Nuke Codes"))
				{
					ButtonToggle("Automatic Nuke Codes", &CustomNukeCodeSettings.AutomaticNukeCodes);
					ImGui::SameLine(235.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
					if (ImGui::Button("Get Nuke Codes", ImVec2(224.0f, 0.0f)))
					{
						GetNukeCode(0x000921AE, AlphaCode);
						GetNukeCode(0x00092213, BravoCode);
						GetNukeCode(0x00092214, CharlieCode);
					}
					ImGui::PopStyleColor(3);

					ButtonToggle("Draw Nuke Code Alpha", &CustomNukeCodeSettings.DrawCodeAlpha);
					ImGui::SameLine(255.0f);
					ImGui::Text("%d %d %d %d %d %d %d %d - Alpha", AlphaCode[0], AlphaCode[1], AlphaCode[2], AlphaCode[3], AlphaCode[4], AlphaCode[5], AlphaCode[6], AlphaCode[7]);

					ButtonToggle("Draw Nuke Code Bravo", &CustomNukeCodeSettings.DrawCodeBravo);
					ImGui::SameLine(255.0f);
					ImGui::Text("%d %d %d %d %d %d %d %d - Bravo", BravoCode[0], BravoCode[1], BravoCode[2], BravoCode[3], BravoCode[4], BravoCode[5], BravoCode[6], BravoCode[7]);

					ButtonToggle("Draw Nuke Code Charlie", &CustomNukeCodeSettings.DrawCodeCharlie);
					ImGui::SameLine(255.0f);
					ImGui::Text("%d %d %d %d %d %d %d %d - Charlie", CharlieCode[0], CharlieCode[1], CharlieCode[2], CharlieCode[3], CharlieCode[4], CharlieCode[5], CharlieCode[6], CharlieCode[7]);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Teleporter###TeleporterTab"))
			{
				for (int i = 0; i < 16; i++)
				{
					char TeleportHeaderText[sizeof("Teleporter Slot: 15")];
					sprintf_s(TeleportHeaderText, "Teleport Slot: %d", i);
					if (ImGui::CollapsingHeader(TeleportHeaderText))
					{
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextX[sizeof("###TeleportDestinationX15")];
						sprintf_s(TeleportDestinationTextX, "###TeleportDestinationX%d", i);
						ImGui::InputFloat(TeleportDestinationTextX, &CustomTeleportSettings.TeleportEntryData[i].Destination[0]);
						if (ImGui::IsItemActive()) AllowDrag = false;
						ImGui::SameLine(122.0f);
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextY[sizeof("###TeleportDestinationY15")];
						sprintf_s(TeleportDestinationTextY, "###TeleportDestinationY%d", i);
						ImGui::InputFloat(TeleportDestinationTextY, &CustomTeleportSettings.TeleportEntryData[i].Destination[1]);
						if (ImGui::IsItemActive()) AllowDrag = false;
						ImGui::SameLine(235.0f);
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextZ[sizeof("###TeleportDestinationZ15")];
						sprintf_s(TeleportDestinationTextZ, "###TeleportDestinationZ%d", i);
						ImGui::InputFloat(TeleportDestinationTextZ, &CustomTeleportSettings.TeleportEntryData[i].Destination[2]);
						if (ImGui::IsItemActive()) AllowDrag = false;
						ImGui::SameLine(349.0f);
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextW[sizeof("###TeleportDestinationW15")];
						sprintf_s(TeleportDestinationTextW, "###TeleportDestinationW%d", i);
						ImGui::InputFloat(TeleportDestinationTextW, &CustomTeleportSettings.TeleportEntryData[i].Destination[3]);
						if (ImGui::IsItemActive()) AllowDrag = false;

						char FormidLabelText[sizeof("###TeleportCellFormid15")];
						sprintf_s(FormidLabelText, "###TeleportCellFormid%d", i);
						char FormidText[sizeof("00000000")];
						sprintf_s(FormidText, "%08lX", CustomTeleportSettings.TeleportEntryData[i].CellFormid);
						ImGui::SetNextItemWidth(110.0f);
						ImGui::InputText(FormidLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
						if (ImGui::IsItemActive()) AllowDrag = false;
						sscanf_s(FormidText, "%08lX", &CustomTeleportSettings.TeleportEntryData[i].CellFormid);
						ImGui::SameLine(122.0f);
						char TeleportDestinationEnabledText[sizeof("Set Position###TeleportDestinationEnabled15")];
						sprintf_s(TeleportDestinationEnabledText, "Set Position###TeleportDestinationEnabled%d", i);
						char TeleportDestinationDisabledText[sizeof("Set Position###TeleportDestinationDisabled15")];
						sprintf_s(TeleportDestinationDisabledText, "Set Position###TeleportDestinationDisabled%d", i);
						if (!CustomTeleportSettings.TeleportEntryData[i].DisableSaving)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
							if (ImGui::Button(TeleportDestinationEnabledText, ImVec2(110.0f, 0.0f))) GetTeleportPosition(i);
							ImGui::PopStyleColor(3);
						}
						else
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
							ImGui::Button(TeleportDestinationDisabledText, ImVec2(110.0f, 0.0f));
							ImGui::PopStyleColor(3);
							ImGui::PopItemFlag();
						}
						ImGui::SameLine(235.0f);
						char DisableSavingText[sizeof("Lock###DisableSaving15")];
						sprintf_s(DisableSavingText, "Lock###DisableSaving%d", i);
						SmallButtonToggle(DisableSavingText, &CustomTeleportSettings.TeleportEntryData[i].DisableSaving);
						ImGui::SameLine(349.0f);
						char TeleportRequestEnabledText[sizeof("Teleport###TeleportRequestEnabled15")];
						sprintf_s(TeleportRequestEnabledText, "Teleport###TeleportRequestEnabled%d", i);
						char TeleportRequestDisabledText[sizeof("Teleport###TeleportRequestDisabled15")];
						sprintf_s(TeleportRequestDisabledText, "Teleport###TeleportRequestDisabled%d", i);
						if (CustomTeleportSettings.TeleportEntryData[i].CellFormid)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
							if (ImGui::Button(TeleportRequestEnabledText, ImVec2(110.0f, 0.0f))) RequestTeleport(i);
							ImGui::PopStyleColor(3);
						}
						else
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
							ImGui::Button(TeleportRequestDisabledText, ImVec2(110.0f, 0.0f));
							ImGui::PopStyleColor(3);
							ImGui::PopItemFlag();
						}
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("BitMsgWriter###BitMsgWriterTab"))
			{
				LargeButtonToggle("Message Sender Enabled", &AllowMessages);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		if (ImGui::GetActiveID() == ImGui::GetWindowScrollbarID(ImGui::GetCurrentWindow(), ImGuiAxis_Y))
		{
			AllowDrag = false;
		}

		if (ImGui::IsMouseDragging(0) && AllowDrag)
		{
			if (!PointerDrag)
			{
				PointerOrigin = ImGui::GetMousePos();
				PointerDrag = true;
			}
		}
		else
		{
			if (PointerDrag)
			{
				PointerOrigin = { 0.0f, 0.0f };
				PointerDrag = false;
			}
		}

		if (PointerDrag)
		{
			DragMenu();
		}
	}

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiInitialize()
{
	ImGui::CreateContext();
	ImGuiContextCreated = true;

	if (!ImGui_ImplWin32_Init(WindowHwnd)) return false;
	else ImGuiWin32Initialized = true;

	if (!ImGui_ImplDX9_Init(D3D9Device)) return false;
	else ImGuiD3D9Initialized = true;

	return true;
}

void ImGuiCleanup()
{
	if (ImGuiD3D9Initialized)
	{
		ImGui_ImplDX9_Shutdown();
		ImGuiD3D9Initialized = false;
	}

	if (ImGuiWin32Initialized)
	{
		ImGui_ImplWin32_Shutdown();
		ImGuiWin32Initialized = false;
	}

	if (ImGuiContextCreated)
	{
		ImGui::DestroyContext();
		ImGuiContextCreated = false;
	}
}
