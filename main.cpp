#include </home/hus/plugin/Hyprland/src/plugins/PluginAPI.hpp>
#include </home/hus/plugin/Hyprland/src/event/EventBus.hpp> 
#include </home/hus/plugin/Hyprland/src/Compositor.hpp>
#include </home/hus/plugin/Hyprland/src/render/Renderer.hpp> 
#include </home/hus/plugin/Hyprland/src/render/pass/PassElement.hpp>
#include </home/hus/plugin/Hyprland/src/render/OpenGL.hpp>
#include </home/hus/plugin/Hyprland/src/desktop/state/FocusState.hpp> 
#include </home/hus/plugin/Hyprland/src/desktop/view/Window.hpp>
#include </home/hus/plugin/Hyprland/src/protocols/XWaylandShell.hpp>
#include </home/hus/plugin/Hyprland/src/protocols/XDGShell.hpp>  
#include </home/hus/plugin/Hyprland/src/xwayland/XSurface.hpp>  
#include </home/hus/plugin/Hyprland/src/desktop/DesktopTypes.hpp> 
#include <thread>
#include <chrono>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <filesystem>

using namespace std;
inline HANDLE PHANDLE = nullptr;

int ct = 0;
int ct2;
PHLWORKSPACE workspace;

class CProxyVideoElement : public IPassElement {
public:
    struct SProxyData {
        Hyprutils::Math::CBox logicalBox;
        PHLWINDOWREF pWindowRef;
    };

    CProxyVideoElement(SProxyData data_) : data(data_) {}
    virtual ~CProxyVideoElement() override = default;

    virtual void draw(const CRegion& damage) override {
        try {
            auto pWindow = data.pWindowRef.lock();
            if (!pWindow) {
                return; 
            }

            auto pWlSurface = pWindow->wlSurface();
            if (!pWlSurface || !pWlSurface->resource()) {
                return;
            }

            auto pWindowTexture = pWlSurface->resource()->m_current.texture;
            if (pWindowTexture == nullptr) {
                auto pMonitor = Desktop::focusState()->monitor();
                g_pHyprRenderer->damageMonitor(pMonitor);
                return;
            }
            auto pMonitor = Desktop::focusState()->monitor();
            if (workspace != pMonitor->m_activeWorkspace && !ct2){
                thread([]() {
                    auto path = filesystem::path(__FILE__).parent_path() / "build/plugin.so";
                    auto cmd = string("hyprctl plugin unload ") + path.c_str();
                    system(cmd.c_str());
                }).detach();
                ct2=1;
            }

            CHyprOpenGLImpl::STextureRenderData texRenderData;
            texRenderData.a = 1.0f;
            // texRenderData.blur = true;
            // texRenderData.blockBlurOptimization = true;
            g_pHyprOpenGL->renderTexture(pWindowTexture, data.logicalBox, texRenderData);
            g_pHyprOpenGL->markBlurDirtyForMonitor(pMonitor);
        } catch (...) {
            HyprlandAPI::addNotification(PHANDLE, "[Hyprland-winwrap] Fatal Error", CHyprColor{1.0, 0.0, 0.0, 1.0}, 5000);
        }
    }


    virtual bool needsLiveBlur() override { return false; }
    virtual bool needsPrecomputeBlur() override { return false; }
    virtual const char* passName() override { return "WallpaperElement"; }

private:
    SProxyData data;
};

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string COMPOSITOR_HASH = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();
    
    if (COMPOSITOR_HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[Hyprland-winwrap] Mismatched headers! Can't proceed.",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[Hyprland-winwrap] Version mismatch");
    }

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprland-winwrap:class", Hyprlang::STRING{"kitty-bg"});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprland-winwrap:title", Hyprlang::STRING{""});

    HyprlandAPI::reloadConfig();

    static auto renderHook = Event::bus()->m_events.render.stage.listen([=](eRenderStage stage) {
        try {
            if (stage != eRenderStage::RENDER_POST_WALLPAPER) return;

            auto pMonitor = Desktop::focusState()->monitor();
            if (!pMonitor) return;

            PHLWINDOW pWindow;
            static auto* const PCLASS = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprland-winwrap:class")->getDataStaticPtr();
            static auto* const PTITLE = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprland-winwrap:title")->getDataStaticPtr();

            string pclass = *PCLASS;
            string ptitle = *PTITLE;
            for (auto const& w : g_pCompositor->m_windows) {
                if (w->m_title==ptitle || w->m_class==pclass){ pWindow=w;}
            }
            if (pWindow == nullptr){ 
                if (ct==0){
                    HyprlandAPI::addNotification(PHANDLE, "no bg windows found",CHyprColor{1.0, 1.0, 0.2, 1.0}, 2500); 
                    ct++;
                }
                return; 
            }
            else{ct=0;}

            if (!workspace){workspace = pMonitor->m_activeWorkspace;}

            CProxyVideoElement::SProxyData proxyData;
            proxyData.logicalBox = pMonitor->logicalBox();
            proxyData.pWindowRef = pWindow;
            g_pHyprRenderer->m_renderPass.add(makeUnique<CProxyVideoElement>(proxyData));

        } catch (const exception& e) {
            string errMsg = "[Hyprland-winwrap] Fatal Error: ";
            errMsg += e.what();
            HyprlandAPI::addNotification(PHANDLE, errMsg, CHyprColor{1.0, 0.0, 0.0, 1.0}, 5000);
        } catch (...) {
            HyprlandAPI::addNotification(PHANDLE, "[Hyprland-winwrap] Fatal Error", CHyprColor{1.0, 0.0, 0.0, 1.0}, 5000);
        }
    });

    return {"Hyprland-winwrap", "proxy wallpaper", "hus", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    //...
}
