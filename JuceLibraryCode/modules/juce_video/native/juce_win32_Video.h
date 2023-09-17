/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace VideoRenderers
{
    //==============================================================================
    struct Base
    {
        virtual ~Base() = default;

        virtual HRESULT create (ComSmartPtr<ComTypes::IGraphBuilder>&, ComSmartPtr<ComTypes::IBaseFilter>&, HWND) = 0;
        virtual void setVideoWindow (HWND) = 0;
        virtual void setVideoPosition (HWND) = 0;
        virtual void repaintVideo (HWND, HDC) = 0;
        virtual void displayModeChanged() = 0;
        virtual HRESULT getVideoSize (long& videoWidth, long& videoHeight) = 0;
    };

    //==============================================================================
    struct VMR7  : public Base
    {
        VMR7() {}

        HRESULT create (ComSmartPtr<ComTypes::IGraphBuilder>& graphBuilder,
                        ComSmartPtr<ComTypes::IBaseFilter>& baseFilter, HWND hwnd) override
        {
            ComSmartPtr<ComTypes::IVMRFilterConfig> filterConfig;

            HRESULT hr = baseFilter.CoCreateInstance (ComTypes::CLSID_VideoMixingRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"VMR-7");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (filterConfig);
            if (SUCCEEDED (hr))   hr = filterConfig->SetRenderingMode (ComTypes::VMRMode_Windowless);
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (windowlessControl);
            if (SUCCEEDED (hr))   hr = windowlessControl->SetVideoClippingWindow (hwnd);
            if (SUCCEEDED (hr))   hr = windowlessControl->SetAspectRatioMode (ComTypes::VMR_ARMODE_LETTER_BOX);

            return hr;
        }

        void setVideoWindow (HWND hwnd) override
        {
            windowlessControl->SetVideoClippingWindow (hwnd);
        }

        void setVideoPosition (HWND hwnd) override
        {
            long videoWidth = 0, videoHeight = 0;
            windowlessControl->GetNativeVideoSize (&videoWidth, &videoHeight, nullptr, nullptr);

            RECT src, dest;
            SetRect (&src, 0, 0, videoWidth, videoHeight);
            GetClientRect (hwnd, &dest);

            windowlessControl->SetVideoPosition (&src, &dest);
        }

        void repaintVideo (HWND hwnd, HDC hdc) override
        {
            windowlessControl->RepaintVideo (hwnd, hdc);
        }

        void displayModeChanged() override
        {
            windowlessControl->DisplayModeChanged();
        }

        HRESULT getVideoSize (long& videoWidth, long& videoHeight) override
        {
            return windowlessControl->GetNativeVideoSize (&videoWidth, &videoHeight, nullptr, nullptr);
        }

        ComSmartPtr<ComTypes::IVMRWindowlessControl> windowlessControl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VMR7)
    };


    //==============================================================================
    struct EVR  : public Base
    {
        EVR() = default;

        HRESULT create (ComSmartPtr<ComTypes::IGraphBuilder>& graphBuilder,
                        ComSmartPtr<ComTypes::IBaseFilter>& baseFilter, HWND hwnd) override
        {
            ComSmartPtr<ComTypes::IMFGetService> getService;

            HRESULT hr = baseFilter.CoCreateInstance (ComTypes::CLSID_EnhancedVideoRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"EVR");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (getService);
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            if (SUCCEEDED (hr))   hr = getService->GetService (ComTypes::MR_VIDEO_RENDER_SERVICE, __uuidof (ComTypes::IMFVideoDisplayControl),
                                                               (void**) videoDisplayControl.resetAndGetPointerAddress());
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetVideoWindow (hwnd);
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetAspectRatioMode (ComTypes::MFVideoARMode_PreservePicture);

            return hr;
        }

        void setVideoWindow (HWND hwnd) override
        {
            videoDisplayControl->SetVideoWindow (hwnd);
        }

        void setVideoPosition (HWND hwnd) override
        {
            const ComTypes::MFVideoNormalizedRect src { 0.0f, 0.0f, 1.0f, 1.0f };

            RECT dest;
            GetClientRect (hwnd, &dest);

            videoDisplayControl->SetVideoPosition (&src, &dest);
        }

        void repaintVideo (HWND, HDC) override
        {
            videoDisplayControl->RepaintVideo();
        }

        void displayModeChanged() override {}

        HRESULT getVideoSize (long& videoWidth, long& videoHeight) override
        {
            SIZE sz = { 0, 0 };
            HRESULT hr = videoDisplayControl->GetNativeVideoSize (&sz, nullptr);
            videoWidth  = sz.cx;
            videoHeight = sz.cy;
            return hr;
        }

        ComSmartPtr<ComTypes::IMFVideoDisplayControl> videoDisplayControl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EVR)
    };
}

//==============================================================================
struct VideoComponent::Pimpl  : public Component,
                                private ComponentPeer::ScaleFactorListener
{
    Pimpl (VideoComponent& ownerToUse, bool)
        : owner (ownerToUse)
    {
        setOpaque (true);
        context.reset (new DirectShowContext (*this));
        componentWatcher.reset (new ComponentWatcher (*this));
    }

    ~Pimpl() override
    {
        close();
        context = nullptr;
        componentWatcher = nullptr;

        if (currentPeer != nullptr)
            currentPeer->removeScaleFactorListener (this);
    }

    Result loadFromString (const String& fileOrURLPath)
    {
        close();
        auto r = context->loadFile (fileOrURLPath);

        if (r.wasOk())
        {
            videoLoaded = true;
            context->updateVideoPosition();
        }

        return r;
    }

    Result load (const File& file)
    {
        auto r = loadFromString (file.getFullPathName());

        if (r.wasOk())
            currentFile = file;

        return r;
    }

    Result load (const URL& url)
    {
        auto r = loadFromString (URL::removeEscapeChars (url.toString (true)));

        if (r.wasOk())
            currentURL = url;

        return r;
    }

    void close()
    {
        stop();
        context->release();

        videoLoaded = false;
        currentFile = File();
        currentURL = {};
    }

    bool isOpen() const
    {
        return videoLoaded;
    }

    bool isPlaying() const
    {
        return context->state == DirectShowContext::runningState;
    }

    void play()
    {
        if (videoLoaded)
            context->play();
    }

    void stop()
    {
        if (videoLoaded)
            context->pause();
    }

    void setPosition (double newPosition)
    {
        if (videoLoaded)
            context->setPosition (newPosition);
    }

    double getPosition() const
    {
        return videoLoaded ? context->getPosition() : 0.0;
    }

    void setSpeed (double newSpeed)
    {
        if (videoLoaded)
            context->setSpeed (newSpeed);
    }

    double getSpeed() const
    {
        return videoLoaded ? context->getSpeed() : 0.0;
    }

    Rectangle<int> getNativeSize() const
    {
        return videoLoaded ? context->getVideoSize()
                           : Rectangle<int>();
    }

    double getDuration() const
    {
        return videoLoaded ? context->getDuration() : 0.0;
    }

    void setVolume (float newVolume)
    {
        if (videoLoaded)
            context->setVolume (newVolume);
    }

    float getVolume() const
    {
        return videoLoaded ? context->getVolume() : 0.0f;
    }

    void paint (Graphics& g) override
    {
        if (videoLoaded)
            context->handleUpdateNowIfNeeded();
        else
            g.fillAll (Colours::grey);
    }

    void updateContextPosition()
    {
        context->updateContextPosition();

        if (getWidth() > 0 && getHeight() > 0)
            if (auto* peer = getTopLevelComponent()->getPeer())
                context->updateWindowPosition ((peer->getAreaCoveredBy (*this).toDouble()
                                                * peer->getPlatformScaleFactor()).toNearestInt());
    }

    void updateContextVisibility()
    {
        context->showWindow (isShowing());
    }

    void recreateNativeWindowAsync()
    {
        context->recreateNativeWindowAsync();
        repaint();
    }

    void playbackStarted()
    {
        if (owner.onPlaybackStarted != nullptr)
            owner.onPlaybackStarted();
    }

    void playbackStopped()
    {
        if (owner.onPlaybackStopped != nullptr)
            owner.onPlaybackStopped();
    }

    void errorOccurred (const String& errorMessage)
    {
        if (owner.onErrorOccurred != nullptr)
            owner.onErrorOccurred (errorMessage);
    }

    File currentFile;
    URL currentURL;

private:
    VideoComponent& owner;
    ComponentPeer* currentPeer = nullptr;
    bool videoLoaded = false;

    //==============================================================================
    void nativeScaleFactorChanged (double /*newScaleFactor*/) override
    {
        if (videoLoaded)
            updateContextPosition();
    }

    //==============================================================================
    struct ComponentWatcher   : public ComponentMovementWatcher
    {
        ComponentWatcher (Pimpl& c)  : ComponentMovementWatcher (&c), owner (c)
        {
        }

        using ComponentMovementWatcher::componentMovedOrResized;
        void componentMovedOrResized (bool, bool) override
        {
            if (owner.videoLoaded)
                owner.updateContextPosition();
        }

        void componentPeerChanged() override
        {
            if (owner.currentPeer != nullptr)
                owner.currentPeer->removeScaleFactorListener (&owner);

            if (owner.videoLoaded)
                owner.recreateNativeWindowAsync();
        }

        using ComponentMovementWatcher::componentVisibilityChanged;
        void componentVisibilityChanged() override
        {
            if (owner.videoLoaded)
                owner.updateContextVisibility();
        }

        Pimpl& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentWatcher)
    };

    std::unique_ptr<ComponentWatcher> componentWatcher;

    //==============================================================================
    struct DirectShowContext    : public AsyncUpdater
    {
        DirectShowContext (Pimpl& c)  : component (c)
        {
            [[maybe_unused]] const auto result = CoInitialize (nullptr);
        }

        ~DirectShowContext() override
        {
            release();
            CoUninitialize();
        }

        //==============================================================================
        void updateWindowPosition (const Rectangle<int>& newBounds)
        {
            nativeWindow->setWindowPosition (newBounds);
        }

        void showWindow (bool shouldBeVisible)
        {
            nativeWindow->showWindow (shouldBeVisible);
        }

        //==============================================================================
        void repaint()
        {
            if (hasVideo)
                videoRenderer->repaintVideo (nativeWindow->hwnd, nativeWindow->hdc);
        }

        void updateVideoPosition()
        {
            if (hasVideo)
                videoRenderer->setVideoPosition (nativeWindow->hwnd);
        }

        void displayResolutionChanged()
        {
            if (hasVideo)
                videoRenderer->displayModeChanged();
        }

        //==============================================================================
        void peerChanged()
        {
            deleteNativeWindow();

            mediaEvent->SetNotifyWindow (0, 0, 0);

            if (videoRenderer != nullptr)
                videoRenderer->setVideoWindow (nullptr);

            createNativeWindow();

            mediaEvent->CancelDefaultHandling (ComTypes::EC_STATE_CHANGE);
            mediaEvent->SetNotifyWindow ((ComTypes::OAHWND) hwnd, graphEventID, 0);

            if (videoRenderer != nullptr)
                videoRenderer->setVideoWindow (hwnd);
        }

        void handleAsyncUpdate() override
        {
            if (hwnd != nullptr)
            {
                if (needToRecreateNativeWindow)
                {
                    peerChanged();
                    needToRecreateNativeWindow = false;
                }

                if (needToUpdateViewport)
                {
                    updateVideoPosition();
                    needToUpdateViewport = false;
                }

                repaint();
            }
            else
            {
                triggerAsyncUpdate();
            }
        }

        void recreateNativeWindowAsync()
        {
            needToRecreateNativeWindow = true;
            triggerAsyncUpdate();
        }

        void updateContextPosition()
        {
            needToUpdateViewport = true;
            triggerAsyncUpdate();
        }

        //==============================================================================
        Result loadFile (const String& fileOrURLPath)
        {
            jassert (state == uninitializedState);

            if (! createNativeWindow())
                return Result::fail ("Can't create window");

            HRESULT hr = graphBuilder.CoCreateInstance (ComTypes::CLSID_FilterGraph);

            // basic playback interfaces
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaControl);
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaPosition);
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaEvent);
            if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (basicAudio);

            // video renderer interface
            if (SUCCEEDED (hr))
            {
                if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
                {
                    videoRenderer.reset (new VideoRenderers::EVR());
                    hr = videoRenderer->create (graphBuilder, baseFilter, hwnd);

                    if (FAILED (hr))
                        videoRenderer = nullptr;
                }

                if (videoRenderer == nullptr)
                {
                    videoRenderer.reset (new VideoRenderers::VMR7());
                    hr = videoRenderer->create (graphBuilder, baseFilter, hwnd);
                }
            }

            // build filter graph
            if (SUCCEEDED (hr))
            {
                hr = graphBuilder->RenderFile (fileOrURLPath.toWideCharPointer(), nullptr);

                if (FAILED (hr))
                {
                   #if JUCE_MODAL_LOOPS_PERMITTED
                    // Annoyingly, if we don't run the msg loop between failing and deleting the window, the
                    // whole OS message-dispatch system gets itself into a state, and refuses to deliver any
                    // more messages for the whole app. (That's what happens in Win7, anyway)
                    MessageManager::getInstance()->runDispatchLoopUntil (200);
                   #endif
                }
            }

            // remove video renderer if not connected (no video)
            if (SUCCEEDED (hr))
            {
                if (isRendererConnected())
                {
                    hasVideo = true;
                }
                else
                {
                    hasVideo = false;
                    graphBuilder->RemoveFilter (baseFilter);
                    videoRenderer = nullptr;
                    baseFilter = nullptr;
                }
            }

            // set window to receive events
            if (SUCCEEDED (hr))
            {
                mediaEvent->CancelDefaultHandling (ComTypes::EC_STATE_CHANGE);
                hr = mediaEvent->SetNotifyWindow ((ComTypes::OAHWND) hwnd, graphEventID, 0);
            }

            if (SUCCEEDED (hr))
            {
                state = stoppedState;
                pause();
                return Result::ok();
            }

            // Note that if you're trying to open a file and this method fails, you may
            // just need to install a suitable codec. It seems that by default DirectShow
            // doesn't support a very good range of formats.
            release();
            return getErrorMessageFromResult (hr);
        }

        static Result getErrorMessageFromResult (HRESULT hr)
        {
            switch (hr)
            {
                case ComTypes::VFW_E_INVALID_FILE_FORMAT:         return Result::fail ("Invalid file format");
                case ComTypes::VFW_E_NOT_FOUND:                   return Result::fail ("File not found");
                case ComTypes::VFW_E_UNKNOWN_FILE_TYPE:           return Result::fail ("Unknown file type");
                case ComTypes::VFW_E_UNSUPPORTED_STREAM:          return Result::fail ("Unsupported stream");
                case ComTypes::VFW_E_CANNOT_CONNECT:              return Result::fail ("Cannot connect");
                case ComTypes::VFW_E_CANNOT_LOAD_SOURCE_FILTER:   return Result::fail ("Cannot load source filter");
            }

            TCHAR messageBuffer[512] = { 0 };

            FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr, (DWORD) hr, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                           messageBuffer, (DWORD) numElementsInArray (messageBuffer) - 1, nullptr);

            return Result::fail (String (messageBuffer));
        }

        void release()
        {
            if (mediaControl != nullptr)
                mediaControl->Stop();

            if (mediaEvent != nullptr)
                mediaEvent->SetNotifyWindow (0, 0, 0);

            if (videoRenderer != nullptr)
                videoRenderer->setVideoWindow (nullptr);

            hasVideo = false;
            videoRenderer = nullptr;
            baseFilter = nullptr;
            basicAudio = nullptr;
            mediaEvent = nullptr;
            mediaPosition = nullptr;
            mediaControl = nullptr;
            graphBuilder = nullptr;

            state = uninitializedState;

            if (nativeWindow != nullptr)
                deleteNativeWindow();
        }

        void graphEventProc()
        {
            LONG ec = 0;
            LONG_PTR p1 = {}, p2 = {};

            jassert (mediaEvent != nullptr);

            while (SUCCEEDED (mediaEvent->GetEvent (&ec, &p1, &p2, 0)))
            {
                mediaEvent->FreeEventParams (ec, p1, p2);

                switch (ec)
                {
                    case ComTypes::EC_REPAINT:
                        component.repaint();
                        break;

                    case ComTypes::EC_COMPLETE:
                        component.stop();
                        component.setPosition (0.0);
                        break;

                    case ComTypes::EC_ERRORABORT:
                    case ComTypes::EC_ERRORABORTEX:
                        component.errorOccurred (getErrorMessageFromResult ((HRESULT) p1).getErrorMessage());
                        // intentional fallthrough
                    case ComTypes::EC_USERABORT:
                        component.close();
                        break;

                    case ComTypes::EC_STATE_CHANGE:
                        switch (p1)
                        {
                            case ComTypes::State_Paused:  component.playbackStopped(); break;
                            case ComTypes::State_Running: component.playbackStarted(); break;
                            default: break;
                        }

                    default:
                        break;
                }
            }
        }

        //==============================================================================
        void play()
        {
            mediaControl->Run();
            state = runningState;
        }

        void stop()
        {
            mediaControl->Stop();
            state = stoppedState;
        }

        void pause()
        {
            mediaControl->Pause();
            state = pausedState;
        }

        //==============================================================================
        Rectangle<int> getVideoSize() const noexcept
        {
            long width = 0, height = 0;

            if (hasVideo)
                videoRenderer->getVideoSize (width, height);

            return { (int) width, (int) height };
        }

        //==============================================================================
        double getDuration() const
        {
            ComTypes::REFTIME duration;
            mediaPosition->get_Duration (&duration);
            return duration;
        }

        double getSpeed() const
        {
            double speed;
            mediaPosition->get_Rate (&speed);
            return speed;
        }

        double getPosition() const
        {
            ComTypes::REFTIME seconds;
            mediaPosition->get_CurrentPosition (&seconds);
            return seconds;
        }

        void setSpeed (double newSpeed)     { mediaPosition->put_Rate (newSpeed); }
        void setPosition (double seconds)   { mediaPosition->put_CurrentPosition (seconds); }
        void setVolume (float newVolume)    { basicAudio->put_Volume (convertToDShowVolume (newVolume)); }

        // in DirectShow, full volume is 0, silence is -10000
        static long convertToDShowVolume (float vol) noexcept
        {
            if (vol >= 1.0f) return 0;
            if (vol <= 0.0f) return -10000;

            return roundToInt ((vol * 10000.0f) - 10000.0f);
        }

        float getVolume() const
        {
            long volume;
            basicAudio->get_Volume (&volume);
            return (float) (volume + 10000) / 10000.0f;
        }

        enum State { uninitializedState, runningState, pausedState, stoppedState };
        State state = uninitializedState;

    private:
        //==============================================================================
        enum { graphEventID = WM_APP + 0x43f0 };

        Pimpl& component;
        HWND hwnd = {};
        HDC hdc = {};

        ComSmartPtr<ComTypes::IGraphBuilder> graphBuilder;
        ComSmartPtr<ComTypes::IMediaControl> mediaControl;
        ComSmartPtr<ComTypes::IMediaPosition> mediaPosition;
        ComSmartPtr<ComTypes::IMediaEventEx> mediaEvent;
        ComSmartPtr<ComTypes::IBasicAudio> basicAudio;
        ComSmartPtr<ComTypes::IBaseFilter> baseFilter;

        std::unique_ptr<VideoRenderers::Base> videoRenderer;

        bool hasVideo = false, needToUpdateViewport = true, needToRecreateNativeWindow = false;

        //==============================================================================
        bool createNativeWindow()
        {
            jassert (nativeWindow == nullptr);

            if (auto* topLevelPeer = component.getTopLevelComponent()->getPeer())
            {
                nativeWindow.reset (new NativeWindow ((HWND) topLevelPeer->getNativeHandle(), this));

                hwnd = nativeWindow->hwnd;
                component.currentPeer = topLevelPeer;
                component.currentPeer->addScaleFactorListener (&component);

                if (hwnd != nullptr)
                {
                    hdc = GetDC (hwnd);
                    component.updateContextPosition();
                    component.updateContextVisibility();
                    return true;
                }

                nativeWindow = nullptr;
            }
            else
            {
                jassertfalse;
            }

            return false;
        }

        void deleteNativeWindow()
        {
            jassert (nativeWindow != nullptr);
            ReleaseDC (hwnd, hdc);
            hwnd = {};
            hdc = {};
            nativeWindow = nullptr;
        }

        bool isRendererConnected()
        {
            ComSmartPtr<ComTypes::IEnumPins> enumPins;

            HRESULT hr = baseFilter->EnumPins (enumPins.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                hr = enumPins->Reset();

            ComSmartPtr<ComTypes::IPin> pin;

            while (SUCCEEDED (hr)
                    && enumPins->Next (1, pin.resetAndGetPointerAddress(), nullptr) == S_OK)
            {
                ComSmartPtr<ComTypes::IPin> otherPin;

                hr = pin->ConnectedTo (otherPin.resetAndGetPointerAddress());

                if (SUCCEEDED (hr))
                {
                    ComTypes::PIN_DIRECTION direction;
                    hr = pin->QueryDirection (&direction);

                    if (SUCCEEDED (hr) && direction == ComTypes::PINDIR_INPUT)
                        return true;
                }
                else if (hr == ComTypes::VFW_E_NOT_CONNECTED)
                {
                    hr = S_OK;
                }
            }

            return false;
        }

        //==============================================================================
        struct NativeWindowClass   : private DeletedAtShutdown
        {
            bool isRegistered() const noexcept              { return atom != 0; }
            LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) (pointer_sized_uint) MAKELONG (atom, 0); }

            JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (NativeWindowClass)

        private:
            NativeWindowClass()
            {
                String windowClassName ("JUCE_DIRECTSHOW_");
                windowClassName << (int) (Time::currentTimeMillis() & 0x7fffffff);

                HINSTANCE moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

                TCHAR moduleFile [1024] = {};
                GetModuleFileName (moduleHandle, moduleFile, 1024);

                WNDCLASSEX wcex = {};
                wcex.cbSize         = sizeof (wcex);
                wcex.style          = CS_OWNDC;
                wcex.lpfnWndProc    = (WNDPROC) wndProc;
                wcex.lpszClassName  = windowClassName.toWideCharPointer();
                wcex.hInstance      = moduleHandle;

                atom = RegisterClassEx (&wcex);
                jassert (atom != 0);
            }

            ~NativeWindowClass()
            {
                if (atom != 0)
                    UnregisterClass (getWindowClassName(), (HINSTANCE) Process::getCurrentModuleInstanceHandle());

                clearSingletonInstance();
            }

            static LRESULT CALLBACK wndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                if (auto* c = (DirectShowContext*) GetWindowLongPtr (hwnd, GWLP_USERDATA))
                {
                    switch (msg)
                    {
                        case WM_NCHITTEST:          return HTTRANSPARENT;
                        case WM_ERASEBKGND:         return 1;
                        case WM_DISPLAYCHANGE:      c->displayResolutionChanged(); break;
                        case graphEventID:          c->graphEventProc(); return 0;
                        default:                    break;
                    }
                }

                return DefWindowProc (hwnd, msg, wParam, lParam);
            }

            ATOM atom = {};

            JUCE_DECLARE_NON_COPYABLE (NativeWindowClass)
        };

        //==============================================================================
        struct NativeWindow
        {
            NativeWindow (HWND parentToAddTo, void* userData)
            {
                auto* wc = NativeWindowClass::getInstance();

                if (wc->isRegistered())
                {
                    DWORD exstyle = 0;
                    DWORD type = WS_CHILD;

                    hwnd = CreateWindowEx (exstyle, wc->getWindowClassName(),
                                           L"", type, 0, 0, 0, 0, parentToAddTo, nullptr,
                                           (HINSTANCE) Process::getCurrentModuleInstanceHandle(), nullptr);

                    if (hwnd != nullptr)
                    {
                        hdc = GetDC (hwnd);
                        SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) userData);
                    }
                }

                jassert (hwnd != nullptr);
            }

            ~NativeWindow()
            {
                if (hwnd != nullptr)
                {
                    SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) 0);
                    DestroyWindow (hwnd);
                }
            }

            void setWindowPosition (Rectangle<int> newBounds)
            {
                SetWindowPos (hwnd, nullptr, newBounds.getX(), newBounds.getY(),
                              newBounds.getWidth(), newBounds.getHeight(),
                              SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            }

            void showWindow (bool shouldBeVisible)
            {
                ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
            }

            HWND hwnd = {};
            HDC hdc = {};

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeWindow)
        };

        std::unique_ptr<NativeWindow> nativeWindow;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowContext)
    };

    std::unique_ptr<DirectShowContext> context;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

JUCE_IMPLEMENT_SINGLETON (VideoComponent::Pimpl::DirectShowContext::NativeWindowClass)
