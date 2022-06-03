//--------------------------------------------------------------------------------------
// PlayFabStore.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "LiveResources.h"
#include "PlayFabResources.h"
#include "LiveInfoHUD.h"
#include "StepTimer.h"
#include "StepTimer.h"
#include "UITK.h"


// A basic sample implementation that creates a D3D12 device and
// provides a render loop.
class Sample final : public DX::IDeviceNotify, public ATG::UITK::D3DResourcesProvider
{
public:

    Sample() noexcept(false);
    ~Sample();

    Sample(Sample&&) = default;
    Sample& operator= (Sample&&) = default;

    Sample(Sample const&) = delete;
    Sample& operator= (Sample const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic render loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

    // ATG::UITK::D3DResourcesProvider
    ID3D12Device* GetD3DDevice() override { return m_deviceResources->GetD3DDevice(); }
    ID3D12CommandQueue* GetCommandQueue() const override { return m_deviceResources->GetCommandQueue(); }
    ID3D12GraphicsCommandList* GetCommandList() const override { return m_deviceResources->GetCommandList(); }

    // UI Methods
    std::shared_ptr<ATG::UITK::UIConsoleWindow> GetConsole() const
    {
        return (m_console) ?
            m_console->GetTypedSubElementById<ATG::UITK::UIConsoleWindow>(ATG::UITK::ID("ConsoleWindow")) :
            nullptr;
    }

    std::shared_ptr<ATG::UITK::UIPanel> GetPopup() const
    {
        return m_uiManager.FindTypedById<ATG::UITK::UIPanel>(ATG::UITK::ID("OkPopup"));
    }

    // Sample methods

    void PlayFabSignIn();

    void QueryStoreProducts();
    void ConsumeStoreProducts();
    void PurchaseStoreProduct(const char* storeId);

    void QueryPlayFabProducts();
    void QueryPlayFabInventory();
    void PurchasePlayFabItem(const char* itemId, uint32_t price);
    void ConsumePlayFabItem(const char* itemId);


private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>        m_deviceResources;

    // Rendering loop timer.
    uint64_t                                    m_frame;
    DX::StepTimer                               m_timer;

    XTaskQueueHandle m_asyncQueue;

    // UITK
    ATG::UITK::UIManager                        m_uiManager;
    ATG::UITK::UIInputState                     m_inputState;

    ATG::UITK::UIElementPtr                     m_mainLayout;
    ATG::UITK::UIElementPtr                     m_menuLayout;

    std::shared_ptr<ATG::UITK::UIStackPanel>    m_itemMenu;
    std::shared_ptr<ATG::UITK::UIStackPanel>    m_itemMenuPF;
    bool                                        m_closeMenu;
    std::shared_ptr<ATG::UITK::UIPanel>         m_console;
    bool                                        m_showConsole;

    std::vector<std::string>                    m_logQueue;
    std::shared_ptr<ATG::UITK::UIConsoleWindow> m_consoleWindow;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_TC;
    std::shared_ptr<ATG::UITK::UIPanel>         m_menuLeftSelect;
    std::shared_ptr<ATG::UITK::UIPanel>         m_menuRightSelect;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_potion;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_shield;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_allow;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_sword;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_bow;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_gem;
    std::shared_ptr<ATG::UITK::UIImage>         m_selectedItemImage;
    std::shared_ptr<ATG::UITK::UIStaticText>    m_selectedItemTC;    
    std::shared_ptr<ATG::UITK::UIStaticText>    m_items[6];
    
    // Input devices.
    std::unique_ptr<DirectX::GamePad>           m_gamePad;
    std::unique_ptr<DirectX::Keyboard>          m_keyboard;
    std::unique_ptr<DirectX::Mouse>             m_mouse;

    DirectX::GamePad::ButtonStateTracker        m_gamePadButtons;
    DirectX::Keyboard::KeyboardStateTracker     m_keyboardButtons;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>    m_graphicsMemory;
    std::unique_ptr<DirectX::DescriptorPile>    m_resourceDescriptors;

    // Xbox Live objects.
    std::unique_ptr<ATG::LiveResources>         m_liveResources;
    std::unique_ptr<ATG::LiveInfoHUD>           m_liveInfoHUD;

    std::unique_ptr<ATG::PlayFabResources>      m_playFabResources;

    XStoreContextHandle                         m_xStoreContext;


    enum Descriptors
    {
        // TODO: Put your static descriptors here
        Reserve,
        Count = 32,
    };
};
