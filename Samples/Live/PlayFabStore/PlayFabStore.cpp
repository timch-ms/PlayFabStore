//--------------------------------------------------------------------------------------
// PlayFabStore.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "PlayFabStore.h"

#include "ATGColors.h"
#include "FindMedia.h"
#include "StringUtil.h"

#include "playfab\PlayFabClientApi.h"

// Playfab StoreID
#define	ITEM_POTION             "Potion"
#define	ITEM_POTIONS5           "Potionsx5"
#define	ITEM_POTIONS25          "Potionsx25"
#define	ITEM_SHIELD             "Shield"
#define	ITEM_SWORD              "Sword"
#define	ITEM_BOW                "Bow"
#define	ITEM_GEM                "Gem"
#define	ITEM_MYSTERY            "Mystery"
#define	ITEM_ARROW              "Arrow"
#define	ITEM_ARROWS5            "Arrowsx5"
#define	ITEM_ARROWS25           "Arrowsx25"
#define	ITEM_ARROWS100          "Arrowsx100"

// Items value
#define	ITEM_POTIONS5_VALUE     (5)
#define	ITEM_POTIONS25_VALUE    (20)
#define	ITEM_SHIELD_VALUE       (2)
#define	ITEM_SWORD_VALUE        (2)
#define	ITEM_BOW_VALUE          (5)
#define	ITEM_MYSTERY_VALUE      (20)
#define	ITEM_ARROWS100_VALUE    (75)
#define	ITEM_ARROWS25_VALUE     (20)
#define	ITEM_ARROWS5_VALUE      (5)

// VC StoreID
#define	ITEM_TC100              "9NFCCXVKB0LQ"
#define	ITEM_TC500              "9P50F9QFBKPP"
#define	ITEM_TC1000             "9N6C45MDKS9C"

enum
{
    potion = 0,
    shield,
    sword,
    bow,
    gem,
    arrow,
    item_all
};

static std::string playfab_items[item_all] = { ITEM_GEM,ITEM_SHIELD,ITEM_BOW,ITEM_SWORD,ITEM_ARROW,ITEM_POTION };

extern void ExitSample() noexcept;

using namespace DirectX;
using namespace ATG::UITK;
using namespace PlayFab;

using Microsoft::WRL::ComPtr;

static const char* c_vc = "TC";
static const char* c_catalog = "PlayFabStore 1.0";

namespace
{
    Sample* s_sample;

    template <size_t bufferSize = 2048>
    void ConsoleWriteLine(std::string_view format, ...)
    {
        assert(format.size() < bufferSize && "format string is too large, split up the string or increase the buffer size");

        static char buffer[bufferSize] = "";

        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, format.data(), args);
        va_end(args);

        OutputDebugStringA(buffer);
        OutputDebugStringA("\n");

        if (s_sample)
        {
            auto console = s_sample->GetConsole();
            if (console)
            {
                console->AppendLineOfText(buffer);
            }
        }
    }

    template <size_t bufferSize = 512>
    void ShowPopup(std::string_view format, ...)
    {
        assert(format.size() < bufferSize && "format string is too large, split up the string or increase the buffer size");

        static char buffer[bufferSize] = "";

        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, format.data(), args);
        va_end(args);

        ConsoleWriteLine(buffer);

        if (s_sample)
        {
            auto popup = s_sample->GetPopup();
            popup->GetTypedSubElementById<UIStaticText>(ID("PopupText"))->SetDisplayText(buffer);

            auto button = popup->GetTypedSubElementById<UIButton>(ID("PopupButton"));
            button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
                [popup](UIButton*)
                {
                    popup->SetVisible(false);
                });

            popup->SetVisible(true);
        }
    }

    const std::map<XStoreProductKind, std::string> c_kindMap =
    {
        { XStoreProductKind::Game, "Game"},
        { XStoreProductKind::Durable, "Durable" },
        { XStoreProductKind::Consumable, "Consumable" },
        { XStoreProductKind::UnmanagedConsumable, "Consumable" },
        { XStoreProductKind::Pass, "Pass" },
        { XStoreProductKind::None, "None" }
    };

    std::string ProductKindToString(XStoreProductKind& kind)
    {
        return c_kindMap.at(kind);
    }

}

Sample::Sample() noexcept(false) :
    m_frame(0),
    m_asyncQueue(nullptr),
    m_closeMenu(false),
    m_showConsole(false)
{
    DX::ThrowIfFailed(
        XTaskQueueCreate(XTaskQueueDispatchMode::ThreadPool, XTaskQueueDispatchMode::Manual, &m_asyncQueue)
    );

    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
    m_liveResources = std::make_unique<ATG::LiveResources>(m_asyncQueue);
    m_liveInfoHUD = std::make_unique<ATG::LiveInfoHUD>("");

}

Sample::~Sample()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }

    XStoreCloseContextHandle(m_xStoreContext);

    if (m_asyncQueue)
    {
        XTaskQueueCloseHandle(m_asyncQueue);
        m_asyncQueue = nullptr;
    }
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(HWND window, int width, int height)
{
    s_sample = this;

    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

    m_mouse = std::make_unique<Mouse>();
#ifdef _GAMING_DESKTOP
    m_mouse->SetWindow(window);
#endif

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_liveResources->SetUserChangedCallback([this](XUserHandle user)
    {
        m_liveInfoHUD->SetUser(user, m_asyncQueue);

        ConsoleWriteLine("Xbox Live: gamertag \"%s\" signed in", m_liveResources->GetGamertag().c_str());

#ifdef _GAMING_XBOX
        HRESULT hr = XStoreCreateContext(m_liveResources->GetUser(), &m_xStoreContext);
#else
        HRESULT hr = XStoreCreateContext(nullptr, &m_xStoreContext);
#endif
        ConsoleWriteLine("XStoreCreateContext 0x%x", hr);

        PlayFabSignIn();
    });

    m_liveResources->SetUserSignOutCompletedCallback([this](XUserHandle /*user*/)
    {
        m_liveInfoHUD->SetUser(nullptr, m_asyncQueue);
    });

    m_liveResources->SetErrorHandler([this](HRESULT error)
    {
        if (error == E_GAMEUSER_NO_DEFAULT_USER || error == E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED)
        {
            m_liveResources->SignInWithUI();
        }
        else // Handle other error cases
        {

        }
    });

    // Before we can make an Xbox Live call we need to ensure that the Game OS has intialized the network stack
    // For sample purposes we block user interaction with the sample.  A game should wait for the network to be
    // initialized before the main menu appears.  For samples, we will wait at the end of initialization.
    while (!m_liveResources->IsNetworkAvailable())
    {
        SwitchToThread();
    }

    m_liveResources->Initialize();
    m_liveInfoHUD->Initialize();


    m_closeMenu = true;

    m_mainLayout = m_uiManager.LoadLayoutFromFile("Assets/UILayout.json");
    m_uiManager.AttachTo(m_mainLayout, m_uiManager.GetRootElement());

    // log window for debug
    m_console = m_uiManager.FindTypedById<UIPanel>(ID("ConsolePanel"));
    ConsoleWriteLine("PlayFabStore sample start");
    m_console->SetVisible(false);

    m_itemMenu = m_uiManager.FindTypedById<UIStackPanel>(ID("ItemMenu"));
    m_itemMenu->SetRelativePositionInRefUnits(Vector2(200.f, 200.f));

    m_itemMenuPF = m_uiManager.FindTypedById<UIStackPanel>(ID("ItemMenuPF"));
    m_itemMenuPF->SetRelativePositionInRefUnits(Vector2(200.f, 200.f));

    // need refactaring
    m_TC = m_uiManager.FindTypedById<UIStaticText>(ID("TCBalance"));
    m_gem = m_uiManager.FindTypedById<UIStaticText>(ID("GemBalance"));
    m_shield = m_uiManager.FindTypedById<UIStaticText>(ID("ShieldBalance"));
    m_bow = m_uiManager.FindTypedById<UIStaticText>(ID("BowBalance"));
    m_sword = m_uiManager.FindTypedById<UIStaticText>(ID("SwordBalance"));
    m_allow = m_uiManager.FindTypedById<UIStaticText>(ID("ArrowBalance"));
    m_potion = m_uiManager.FindTypedById<UIStaticText>(ID("PotionBalance"));
    m_selectedItemImage = m_uiManager.FindTypedById<UIImage>(ID("SelectedItemImage"));
    m_selectedItemTC = m_uiManager.FindTypedById<UIStaticText>(ID("SelectedItemTC"));

    m_menuLeftSelect = m_uiManager.FindTypedById<UIPanel>(ID("MenuLeftSelectPanel"));
    m_menuRightSelect = m_uiManager.FindTypedById<UIPanel>(ID("MenuRightSelectPanel"));
    m_menuLeftSelect->SetVisible(true);
    m_menuRightSelect->SetVisible(false);

    m_items[0] = m_uiManager.FindTypedById<UIStaticText>(ID("GemBalance"));
    m_items[1] = m_uiManager.FindTypedById<UIStaticText>(ID("ShieldBalance"));
    m_items[2] = m_uiManager.FindTypedById<UIStaticText>(ID("BowBalance"));
    m_items[3] = m_uiManager.FindTypedById<UIStaticText>(ID("SwordBalance"));
    m_items[4] = m_uiManager.FindTypedById<UIStaticText>(ID("ArrowBalance"));
    m_items[5] = m_uiManager.FindTypedById<UIStaticText>(ID("PotionBalance"));

#ifdef _GAMING_DESKTOP
    auto highlightState = UIButton::State::Hovered;
#else
    auto highlightState = UIButton::State::Focused;
#endif

    auto button = CastPtr<UIButton>(m_itemMenu->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase 100 TC");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchaseStoreProduct(ITEM_TC100);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("TCSImageStyle"));
        m_selectedItemTC->SetDisplayText("$0");
    });

    button = CastPtr<UIButton>(m_itemMenu->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase 500 TC");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchaseStoreProduct(ITEM_TC500);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("TCMImageStyle"));
        m_selectedItemTC->SetDisplayText("$0");
    });

    button = CastPtr<UIButton>(m_itemMenu->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase 1000 TC");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchaseStoreProduct(ITEM_TC1000);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("TCLImageStyle"));
        m_selectedItemTC->SetDisplayText("$0");
    });
    button = CastPtr<UIButton>(m_itemMenu->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Query PlayFab Catalog");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        QueryPlayFabProducts();
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("CatalogStyle"));
        m_selectedItemTC->SetDisplayText("");
    });
    button = CastPtr<UIButton>(m_itemMenu->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Query PlayFab Inventory");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        QueryPlayFabInventory();
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("CatalogStyle"));
        m_selectedItemTC->SetDisplayText("");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Arrow x 100\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_ARROWS100, ITEM_ARROWS100_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("ArrowStyle"));
        m_selectedItemTC->SetDisplayText("75 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Arrow x 25\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_ARROWS25, ITEM_ARROWS25_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("ArrowStyle"));
        m_selectedItemTC->SetDisplayText("20 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Arrow x 5\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_ARROWS5, ITEM_ARROWS5_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("ArrowStyle"));
        m_selectedItemTC->SetDisplayText("5 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Bow\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_BOW, ITEM_BOW_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("BowStyle"));
        m_selectedItemTC->SetDisplayText("5 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Mystery\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_MYSTERY, ITEM_MYSTERY_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("MysteryStyle"));
        m_selectedItemTC->SetDisplayText("20 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Potion x 25\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_POTIONS25, ITEM_POTIONS25_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("PotionStyle"));
        m_selectedItemTC->SetDisplayText("20 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Potion x 5\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_POTIONS5, ITEM_POTIONS5_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("Potion5Style"));
        m_selectedItemTC->SetDisplayText("5 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Shield\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_SHIELD, ITEM_SHIELD_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("ShieldStyle"));
        m_selectedItemTC->SetDisplayText("2 TC");
    });

    button = CastPtr<UIButton>(m_itemMenuPF->AddChildFromPrefab("#menu_item_prefab"));
    button->GetTypedSubElementById<UIStaticText>(ID("MenuItemButtonText"))->SetDisplayText("Purchase \"Sword\"");
    button->ButtonState().AddListenerWhen(UIButton::State::Pressed,
        [this](UIButton*)
    {
        PurchasePlayFabItem(ITEM_SWORD, ITEM_SWORD_VALUE);
    });
    button->ButtonState().AddListenerWhen(highlightState,
        [this](UIButton*)
    {
        m_selectedItemImage->SetStyleId(ID("SwordStyle"));
        m_selectedItemTC->SetDisplayText("2 TC");
    });

    m_itemMenu->SetVisible(m_closeMenu);
    m_itemMenuPF->SetVisible(!m_closeMenu);
}

#pragma region Frame Update
// Executes basic render loop.
void Sample::Tick()
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Frame %llu", m_frame);

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();

    PIXEndEvent();
    m_frame++;
}

// Updates the world.
void Sample::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    elapsedTime;


    // Push logs to UI
    for (size_t i = 0; i < m_logQueue.size(); ++i)
    {
        m_consoleWindow->AppendLineOfText(m_logQueue[i]);
    }
    m_logQueue.clear();


    PlayFabClientAPI::Update();

    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        if (m_gamePadButtons.menu == GamePad::ButtonStateTracker::PRESSED)
        {
            if (!m_liveResources->IsUserSignedIn())
            {
                m_liveResources->SignInSilently();
            }
            else
            {
                m_liveResources->SignInWithUI();
            }
        }

        if (m_gamePadButtons.leftShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            m_closeMenu = true;
            m_menuLeftSelect->SetVisible(true);
            m_menuRightSelect->SetVisible(false);
        }
        if (m_gamePadButtons.rightShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            m_closeMenu = false;
            m_menuLeftSelect->SetVisible(false);
            m_menuRightSelect->SetVisible(true);
        }

        // consume items
        if (m_gamePadButtons.leftTrigger == GamePad::ButtonStateTracker::PRESSED)
        {
            ConsumePlayFabItem(ITEM_ARROW);
        }
        if (m_gamePadButtons.rightTrigger == GamePad::ButtonStateTracker::PRESSED)
        {
            ConsumePlayFabItem(ITEM_POTION);
        }
    }
    else
    {
        m_gamePadButtons.Reset();
    }

    m_itemMenu->SetVisible(m_closeMenu);
    m_itemMenuPF->SetVisible(!m_closeMenu);

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    if (kb.Escape)
    {
        ExitSample();
    }

    if (m_keyboardButtons.IsKeyReleased(Keyboard::Keys::Tab))
    {
        if (!m_liveResources->IsUserSignedIn())
        {
            m_liveResources->SignInSilently();
        }
        else
        {
            m_liveResources->SignInWithUI();
        }
    }

    m_liveInfoHUD->Update(m_deviceResources->GetCommandQueue());

    m_inputState.Update(elapsedTime, *m_gamePad, *m_keyboard, *m_mouse);
    m_uiManager.Update(elapsedTime, m_inputState);

    auto gamepadButtons = m_inputState.GetGamePadButtons(0);
    auto keys = m_inputState.GetKeyboardKeys();
    auto mouse = m_inputState.GetMouseState();
    auto mouseButtons = m_inputState.GetMouseButtons();

    if (gamepadButtons.view == GamePad::ButtonStateTracker::PRESSED ||
        keys.IsKeyReleased(Keyboard::Keys::OemTilde))
    {
        m_showConsole = !m_showConsole;
        if (m_console)
        {
            m_console->SetVisible(m_showConsole);

        }
    }

    while (XTaskQueueDispatch(m_asyncQueue, XTaskQueuePort::Completion, 0))
    {
    }

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Sample::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    ID3D12DescriptorHeap* heap = m_resourceDescriptors->Heap();
    commandList->SetDescriptorHeaps(1, &heap);

    m_liveInfoHUD->Render(commandList);

    m_uiManager.Render();

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
    PIXEndEvent();
}

// Helper method to clear the back buffers.
void Sample::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, ATG::Colors::Background, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Sample::OnActivated()
{
}

void Sample::OnDeactivated()
{
}

void Sample::OnSuspending()
{
    m_deviceResources->Suspend();
}

void Sample::OnResuming()
{
    m_deviceResources->Resume();
    m_timer.ResetElapsedTime();
    m_gamePadButtons.Reset();
    m_keyboardButtons.Reset();
    m_liveResources->Refresh();
    m_inputState.Reset();
}

void Sample::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Sample::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void Sample::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(), m_deviceResources->GetDepthBufferFormat());

    m_resourceDescriptors = std::make_unique<DirectX::DescriptorPile>(device,
        Descriptors::Count,
        Descriptors::Reserve
        );

    ResourceUploadBatch resourceUpload(device);
    resourceUpload.Begin();

    m_liveInfoHUD->RestoreDevice(device, rtState, resourceUpload, *m_resourceDescriptors);

    auto uploadResourcesFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());
    uploadResourcesFinished.wait();

    device;

    auto styleRenderer = std::make_unique<UIStyleRendererD3D>(*this);
    m_uiManager.GetStyleManager().InitializeStyleRenderer(std::move(styleRenderer));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    m_liveInfoHUD->SetViewport(m_deviceResources->GetScreenViewport());
    auto size = m_deviceResources->GetOutputSize();
    m_uiManager.SetWindowSize(size.right, size.bottom);
}

void Sample::OnDeviceLost()
{
    m_graphicsMemory.reset();
    m_liveInfoHUD->ReleaseDevice();
    m_resourceDescriptors.reset();
    m_uiManager.GetStyleManager().ResetStyleRenderer();
}

void Sample::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

void Sample::PlayFabSignIn()
{
    m_playFabResources = std::make_unique<ATG::PlayFabResources>(
        "4E29",
        m_liveResources->GetUser(),
        m_liveResources->GetGamertag().c_str(),
        m_asyncQueue,
        [this](HRESULT hr, const char* msg)->bool
        {
            if (SUCCEEDED(hr))
            {
                ConsoleWriteLine("PlayFabClientAPI::LoginWithXbox() success!");
                QueryPlayFabInventory();
            }
            else
            {
                ConsoleWriteLine("Error: %s (0x%0x)", msg, hr);
            }

            return SUCCEEDED(hr);
        });

}

void Sample::QueryStoreProducts()
{
    auto async = new XAsyncBlock{};
    async->context = this;
    async->queue = m_asyncQueue;
    async->callback = [](XAsyncBlock* async)
    {
        XStoreProductQueryHandle queryHandle = nullptr;
        HRESULT hr = XStoreQueryAssociatedProductsResult(async, &queryHandle);
        if (SUCCEEDED(hr))
        {
            hr = XStoreEnumerateProductsQuery(
                queryHandle,
                async->context,
                [](const XStoreProduct* product, void*)->bool
                {
                    uint32_t quantity = 0;
                    auto productKind = product->productKind;
                    if (productKind == XStoreProductKind::Consumable)
                    {
                        for (uint32_t i = 0; i < product->skusCount; i++)
                        {
                            quantity += product->skus[i].collectionData.quantity;
                        }
                    }

                    ConsoleWriteLine("[%s] %s \"%s\" %s %s",
                        product->storeId,
                        ProductKindToString(productKind).c_str(),
                        product->title,
                        product->isInUserCollection ? "owned" : "",
                        quantity > 0 ? std::string("quantity: " + std::to_string(quantity)).c_str() : "");

                    return true;
                });

            if (SUCCEEDED(hr))
            {
                if (XStoreProductsQueryHasMorePages(queryHandle))
                {
                    ConsoleWriteLine("Has more pages!");
                    // Normally XStoreProductsQueryNextPageAsync must be called
                    // but with the small number of products this sample uses
                    // this is omitted for brevity
                }
                else
                {
                    delete async;
                }
            }
            else
            {
                delete async;
                ShowPopup("Error calling XStoreEnumerateProductsQuery : 0x%08X", hr);
            }
        }

    };

    XStoreProductKind typeFilter = XStoreProductKind::Consumable | XStoreProductKind::Durable;

    const char* storeIds[] = { "9N2GGK1HN2NZ" };
    HRESULT hr = XStoreQueryProductsAsync(
        m_xStoreContext,
        typeFilter,
        storeIds,
        ARRAYSIZE(storeIds),
        nullptr,
        0,
        async);

    if (FAILED(hr))
    {
        delete async;
        ShowPopup("Error calling XStoreQueryAssociatedProductsAsync : 0x%08X", hr);
    }
}

void Sample::PurchaseStoreProduct(const char* storeId)
{
    auto async = new XAsyncBlock{};
    async->context = this;
    async->queue = m_asyncQueue;
    async->callback = [](XAsyncBlock *async)
    {
        auto pThis = static_cast<Sample*>(async->context);

        HRESULT hr = XStoreShowPurchaseUIResult(async);

        ConsoleWriteLine("XStoreShowPurchaseUIResult 0x%x", hr);

        if (SUCCEEDED(hr))
        {
            pThis->QueryStoreProducts();
            pThis->ConsumeStoreProducts();
            pThis->QueryPlayFabInventory();
        }

        delete async;
    };

    HRESULT hr = XStoreShowPurchaseUIAsync(
        m_xStoreContext,
        storeId,
        nullptr,
        nullptr,
        async);

    if (FAILED(hr))
    {
        delete async;
        ShowPopup("Error calling XStoreShowPurchaseUIAsync : 0x%x", hr);
    }
}

void Sample::QueryPlayFabProducts()
{
    ClientModels::GetCatalogItemsRequest req;
    req.CatalogVersion = c_catalog;

    // https://docs.microsoft.com/en-us/rest/api/playfab/client/title-wide-data-management/get-catalog-items
    PlayFabClientAPI::GetCatalogItems(req,
        [this](const PlayFab::ClientModels::GetCatalogItemsResult& result, void*)
        {
            for (auto& item : result.Catalog)
            {
                ConsoleWriteLine("Catalog Item ID: %s DisplayName: %s",
                    item.ItemId.c_str(),
                    item.DisplayName.c_str());
            }
        },
        [this](const PlayFab::PlayFabError& error, void*)
        {
            ConsoleWriteLine("Error calling GetCatalogItems %d %s", error.ErrorCode, error.ErrorMessage.c_str());
        });
}

void Sample::QueryPlayFabInventory()
{
    // https://docs.microsoft.com/en-us/rest/api/playfab/client/player-item-management/get-user-inventory
    PlayFab::ClientModels::GetUserInventoryRequest req;

    PlayFabClientAPI::GetUserInventory(req,
        [this](const PlayFab::ClientModels::GetUserInventoryResult& result, void*)
        {
            for (auto& item : result.Inventory)
            {
                for (int i = 0; i < item_all; ++i) {
                    if (item.ItemId.c_str() != playfab_items[i])
                    {
                        m_items[i]->SetDisplayText("0");
                    };
                }
            }
            for (auto& item : result.Inventory)
            {
                ConsoleWriteLine("Inventory Item ID: %s DisplayName: %s RemainingUses: %d ItemInstanceId %s",
                    item.ItemId.c_str(),
                    item.DisplayName.c_str(),
                    item.RemainingUses,
                    item.ItemInstanceId.c_str()
                );
                for (int i = 0; i < item_all; ++i) {
                    if (item.ItemId.c_str() == playfab_items[i])
                    {
                        m_items[i]->SetDisplayText(std::to_string(item.RemainingUses));
                    };
                }
            }
            for (auto& item : result.VirtualCurrency)
            {
                ConsoleWriteLine("Virtual Currency ID: %s Balance: %d",
                    item.first.c_str(),
                    item.second);
                m_TC->SetDisplayText(std::to_string(item.second));
            }
        },
        [this](const PlayFab::PlayFabError& error, void*)
        {
            ConsoleWriteLine("Error calling GetUserInventory %d %s", error.ErrorCode, error.ErrorMessage.c_str());
        });
}

void Sample::PurchasePlayFabItem(const char* itemId, uint32_t price)
{
    PlayFab::ClientModels::PurchaseItemRequest req;

    req.ItemId = itemId;
    req.CatalogVersion = c_catalog;
    req.Price = int32_t(price);
    req.VirtualCurrency = c_vc;

    PlayFabClientAPI::PurchaseItem(req,
        [this](const PlayFab::ClientModels::PurchaseItemResult& result, void*)
        {
            for (auto& item : result.Items)
            {
                ConsoleWriteLine("Purchased Item ID: %s DisplayName: %s\n",
                    item.ItemId.c_str(),
                    item.DisplayName.c_str());
            }
            QueryPlayFabInventory();
        },
        [this](const PlayFab::PlayFabError& error, void*)
        {
            ConsoleWriteLine("Error calling PurchaseItem %d %s", error.ErrorCode, error.ErrorMessage.c_str());
        });
}

void Sample::ConsumePlayFabItem(const char* itemId)
{
    // First iterate through inventory to get the first instance Id
    PlayFab::ClientModels::GetUserInventoryRequest req;

    PlayFabClientAPI::GetUserInventory(req,
        [this, itemId](const PlayFab::ClientModels::GetUserInventoryResult& result, void*)
        {
            for (auto& item : result.Inventory)
            {
                if (std::string(itemId) == item.ItemId && item.RemainingUses > 0)
                {
                    ConsoleWriteLine("Consuming 1 of %s InstanceId %s", item.ItemId.c_str(), item.ItemInstanceId.c_str());

                    PlayFab::ClientModels::ConsumeItemRequest req;
                    req.ItemInstanceId = item.ItemInstanceId;
                    req.ConsumeCount = 1;

                    PlayFabClientAPI::ConsumeItem(req,
                        [this, itemId](const PlayFab::ClientModels::ConsumeItemResult& result, void*)
                        {
                            ConsoleWriteLine("%s consumed, %d remaining", itemId, result.RemainingUses);
                        },
                        [this](const PlayFab::PlayFabError& error, void*)
                        {
                            ConsoleWriteLine("Error calling ConsumeItem %d %s", error.ErrorCode, error.ErrorMessage.c_str());
                        });
                }
            }
            QueryPlayFabInventory();
        },
        [this](const PlayFab::PlayFabError& error, void*)
        {
            ConsoleWriteLine("Error calling GetUserInventory %d %s", error.ErrorCode, error.ErrorMessage.c_str());
        });
}

void Sample::ConsumeStoreProducts()
{
    PlayFab::ClientModels::ConsumeMicrosoftStoreEntitlementsRequest req;

    req.CatalogVersion = c_catalog;
    req.MarketplaceSpecificData.XboxToken = m_playFabResources->GetXToken();

    // https://docs.microsoft.com/en-us/rest/api/playfab/client/platform-specific-methods/consume-microsoft-store-entitlements
    PlayFabClientAPI::ConsumeMicrosoftStoreEntitlements(req,
        [this](const PlayFab::ClientModels::ConsumeMicrosoftStoreEntitlementsResponse& result, void*)
        {
            for (auto& item : result.Items)
            {
                ConsoleWriteLine("Catalog Item ID: %s DisplayName: %s consumed, %d remaining",
                    item.ItemId.c_str(),
                    item.DisplayName.c_str(),
                    item.RemainingUses);
            }

        },
        [this](const PlayFab::PlayFabError& error, void*)
        {
            ConsoleWriteLine("Error calling ConsumeMicrosoftStoreEntitlements %d %s", error.ErrorCode, error.ErrorMessage.c_str());
        });
}

