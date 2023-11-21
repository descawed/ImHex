#include "content/views/view_tools.hpp"
#include <imgui_internal.h>

#include <hex/api/content_registry.hpp>

namespace hex::plugin::builtin {

    ViewTools::ViewTools() : View::Window("hex.builtin.view.tools.name") {
        this->m_dragStartIterator = ContentRegistry::Tools::impl::getEntries().end();
    }

    void ViewTools::drawContent() {
        auto &tools = ContentRegistry::Tools::impl::getEntries();

        // Draw all tools
        for (auto iter = tools.begin(); iter != tools.end(); ++iter) {
            auto &[name, function, detached] = *iter;

            // If the tool has been detached from the main window, don't draw it here anymore
            if (detached) continue;

            // Draw the tool
            if (ImGui::CollapsingHeader(Lang(name))) {
                function();
                ImGui::NewLine();
            } else {
                // Handle dragging the tool out of the main window

                // If the user clicks on the header, start dragging the tool remember the iterator
                if (ImGui::IsMouseClicked(0) && ImGui::IsItemActivated() && this->m_dragStartIterator == tools.end())
                    this->m_dragStartIterator = iter;

                // If the user released the mouse button, stop dragging the tool
                if (!ImGui::IsMouseDown(0))
                    this->m_dragStartIterator = tools.end();

                // Detach the tool if the user dragged it out of the main window
                if (!ImGui::IsItemHovered() && this->m_dragStartIterator == iter) {
                    detached = true;
                }

            }
        }
    }

    void ViewTools::drawAlwaysVisibleContent() {
        // Make sure the tool windows never get drawn over the welcome screen
        if (!ImHexApi::Provider::isValid())
            return;

        auto &tools = ContentRegistry::Tools::impl::getEntries();

        for (auto iter = tools.begin(); iter != tools.end(); ++iter) {
            auto &[name, function, detached] = *iter;

            // If the tool is still attached to the main window, don't draw it here
            if (!detached) continue;

            // Create a new window for the tool
            if (ImGui::Begin(View::toWindowName(name).c_str(), &detached, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
                if (!ImGui::IsWindowDocked())
                    ImGui::SetWindowSize(scaled(ImVec2(600, 0)));

                ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

                // Draw the tool
                function();

                // Handle the first frame after the tool has been detached
                if (ImGui::IsWindowAppearing() && this->m_dragStartIterator == iter) {
                    this->m_dragStartIterator = tools.end();

                    // Attach the newly created window to the cursor, so it gets dragged around
                    GImGui->MovingWindow = ImGui::GetCurrentWindow();
                    GImGui->ActiveId = GImGui->MovingWindow->MoveId;
                }
            }
            ImGui::End();
        }
    }

}