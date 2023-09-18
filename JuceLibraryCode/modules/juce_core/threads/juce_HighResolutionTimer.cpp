/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class HighResolutionTimer::Impl : private PlatformTimerListener
{
public:
    explicit Impl (HighResolutionTimer& o)
        : owner { o } {}

    void startTimer (int newIntervalMs)
    {
        shouldCancelCallbacks.store (true);

        const auto shouldWaitForPendingCallbacks = [&]
        {
            const std::scoped_lock lock { timerMutex };

            if (timer.getIntervalMs() > 0)
                timer.cancelTimer();

            jassert (timer.getIntervalMs() == 0);

            if (newIntervalMs > 0)
                timer.startTimer (jmax (0, newIntervalMs));

            return callbackThreadId != std::this_thread::get_id()
                && timer.getIntervalMs() <= 0;
        }();

        if (shouldWaitForPendingCallbacks)
            std::scoped_lock lock { callbackMutex };
    }

    int getIntervalMs() const
    {
        const std::scoped_lock lock { timerMutex };
        return timer.getIntervalMs();
    }

    bool isTimerRunning() const
    {
        return getIntervalMs() > 0;
    }

private:
    void onTimerExpired() final
    {
        callbackThreadId.store (std::this_thread::get_id());

        {
            std::scoped_lock lock { callbackMutex };

            if (isTimerRunning())
            {
                try
                {
                    owner.hiResTimerCallback();
                }
                catch (...)
                {
                    // Exceptions thrown in a timer callback won't be
                    // propagated to the main thread, it's best to find
                    // a way to avoid them if possible
                    jassertfalse;
                }
            }
        }

        callbackThreadId.store ({});
    }

    HighResolutionTimer& owner;
    mutable std::mutex timerMutex;
    std::mutex callbackMutex;
    std::atomic<std::thread::id> callbackThreadId{};
    std::atomic<bool> shouldCancelCallbacks { false };
    PlatformTimer timer { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Impl)
    JUCE_DECLARE_NON_MOVEABLE (Impl)
};

//==============================================================================
HighResolutionTimer::HighResolutionTimer()
    : impl (std::make_unique<Impl> (*this)) {}

HighResolutionTimer::~HighResolutionTimer()
{
    // You *must* call stopTimer from the derived class destructor to
    // avoid data races on the timer's vtable
    jassert (! isTimerRunning());
    stopTimer();
}

void HighResolutionTimer::startTimer (int newIntervalMs)
{
    impl->startTimer (newIntervalMs);
}

void HighResolutionTimer::stopTimer()
{
    impl->startTimer (0);
}

int HighResolutionTimer::getTimerInterval() const noexcept
{
    return impl->getIntervalMs();
}

bool HighResolutionTimer::isTimerRunning() const noexcept
{
    return impl->isTimerRunning();
}

//==============================================================================
#if JUCE_UNIT_TESTS

class HighResolutionTimerTests : public UnitTest
{
public:
    HighResolutionTimerTests()
        : UnitTest ("HighResolutionTimer", UnitTestCategories::threads) {}

    void runTest() override
    {
        constexpr int maximumTimeoutMs {30'000};

        beginTest ("Start/stop a timer");
        {
            WaitableEvent timerFiredOnce;
            WaitableEvent timerFiredTwice;

            Timer timer {[&, callbackCount = 0] () mutable
            {
                switch (++callbackCount)
                {
                    case 1: timerFiredOnce.signal(); return;
                    case 2: timerFiredTwice.signal(); return;
                    default: return;
                }
            }};

            expect (! timer.isTimerRunning());
            expect (timer.getTimerInterval() == 0);

            timer.startTimer (1);
            expect (timer.isTimerRunning());
            expect (timer.getTimerInterval() == 1);
            expect (timerFiredOnce.wait (maximumTimeoutMs));
            expect (timerFiredTwice.wait (maximumTimeoutMs));

            timer.stopTimer();
            expect (! timer.isTimerRunning());
            expect (timer.getTimerInterval() == 0);
        }

        beginTest ("Stop a timer from the timer callback");
        {
            WaitableEvent stoppedTimer;

            auto timerCallback = [&](Timer& timer)
            {
                expect (timer.isTimerRunning());
                timer.stopTimer();
                expect (! timer.isTimerRunning());
                stoppedTimer.signal();
            };

            Timer timer {[&]{ timerCallback (timer); }};
            timer.startTimer (1);
            expect (stoppedTimer.wait (maximumTimeoutMs));
        }

        beginTest ("Restart a timer from the timer callback");
        {
            WaitableEvent restartTimer;
            WaitableEvent timerRestarted;
            WaitableEvent timerFiredAfterRestart;

            Timer timer {[&, callbackCount = 0] () mutable
            {
                switch (++callbackCount)
                {
                    case 1:
                        expect (restartTimer.wait (maximumTimeoutMs));
                        expect (timer.getTimerInterval() == 1);

                        timer.startTimer (2);
                        expect (timer.getTimerInterval() == 2);
                        timerRestarted.signal();
                        return;

                    case 2:
                        expect (timer.getTimerInterval() == 2);
                        timerFiredAfterRestart.signal();
                        return;

                    default:
                        return;
                }
            }};

            timer.startTimer (1);
            expect (timer.getTimerInterval() == 1);

            restartTimer.signal();
            expect (timerRestarted.wait (maximumTimeoutMs));
            expect (timer.getTimerInterval() == 2);
            expect (timerFiredAfterRestart.wait (maximumTimeoutMs));

            timer.stopTimer();
        }

        beginTest ("Calling stopTimer on a timer, waits for any timer callbacks to finish");
        {
            WaitableEvent timerCallbackStarted;
            WaitableEvent stoppingTimer;
            std::atomic<bool> timerCallbackFinished { false };

            Timer timer {[&, callbackCount = 0] () mutable
            {
                switch (++callbackCount)
                {
                    case 1:
                        timerCallbackStarted.signal();
                        expect (stoppingTimer.wait (maximumTimeoutMs));
                        Thread::sleep (10);
                        timerCallbackFinished = true;
                        return;

                    default:
                        return;
                }
            }};

            timer.startTimer (1);
            expect (timerCallbackStarted.wait (maximumTimeoutMs));

            stoppingTimer.signal();
            timer.stopTimer();
            expect (timerCallbackFinished);
        }

        beginTest ("Calling stopTimer on a timer, waits for any timer callbacks to finish, even if the timer callback calls stopTimer first");
        {
            WaitableEvent stoppedFromInsideTimerCallback;
            WaitableEvent stoppingFromOutsideTimerCallback;
            std::atomic<bool> timerCallbackFinished { false };

            Timer timer {[&]()
            {
                timer.stopTimer();
                stoppedFromInsideTimerCallback.signal();
                expect (stoppingFromOutsideTimerCallback.wait (maximumTimeoutMs));
                Thread::sleep (10);
                timerCallbackFinished = true;

            }};

            timer.startTimer (1);
            expect (stoppedFromInsideTimerCallback.wait (maximumTimeoutMs));

            stoppingFromOutsideTimerCallback.signal();
            timer.stopTimer();
            expect (timerCallbackFinished);
        }

        beginTest ("Adjusting a timer period from outside the timer callback doesn't cause data races");
        {
            WaitableEvent timerCallbackStarted;
            WaitableEvent timerRestarted;
            WaitableEvent timerFiredAfterRestart;
            std::atomic<int> lastCallbackCount {0};

            Timer timer {[&, callbackCount = 0] () mutable
            {
                switch (++callbackCount)
                {
                    case 1:
                        expect (timer.getTimerInterval() == 1);
                        timerCallbackStarted.signal();
                        Thread::sleep (10);
                        lastCallbackCount = 1;
                        return;

                    case 2:
                        expect (timerRestarted.wait (maximumTimeoutMs));
                        expect (timer.getTimerInterval() == 2);
                        lastCallbackCount = 2;
                        timerFiredAfterRestart.signal();
                        return;

                    default:
                        return;
                }
            }};

            timer.startTimer (1);
            expect (timerCallbackStarted.wait (maximumTimeoutMs));

            timer.startTimer (2);
            timerRestarted.signal();

            expect (timerFiredAfterRestart.wait (maximumTimeoutMs));
            expect (lastCallbackCount == 2);

            timer.stopTimer();
            expect (lastCallbackCount == 2);
        }

        beginTest ("A timer can be restarted externally, after being stopped internally");
        {
            WaitableEvent timerStopped;
            WaitableEvent timerFiredAfterRestart;

            Timer timer {[&, callbackCount = 0] () mutable
            {
                switch (++callbackCount)
                {
                    case 1:
                        timer.stopTimer();
                        timerStopped.signal();
                        return;

                    case 2:
                        timerFiredAfterRestart.signal();
                        return;

                    default:
                        return;
                }
            }};

            expect (! timer.isTimerRunning());
            timer.startTimer (1);
            expect (timer.isTimerRunning());

            expect (timerStopped.wait (maximumTimeoutMs));
            expect (! timer.isTimerRunning());

            timer.startTimer (1);
            expect (timer.isTimerRunning());
            expect (timerFiredAfterRestart.wait (maximumTimeoutMs));
        }

        beginTest ("Calls to `startTimer` and `getTimerInterval` succeed while a callback is blocked");
        {
            WaitableEvent timerBlocked;
            WaitableEvent unblockTimer;

            Timer timer {[&]
            {
                timerBlocked.signal();
                unblockTimer.wait();
                timer.stopTimer();
            }};

            timer.startTimer (1);
            timerBlocked.wait();

            expect (timer.getTimerInterval() == 1);
            timer.startTimer (2);
            expect (timer.getTimerInterval() == 2);

            unblockTimer.signal();
            timer.stopTimer();
        }

        beginTest ("Stress test");
        {
            constexpr auto maxNumTimers { 100 };

            std::vector<std::unique_ptr<Timer>> timers;
            timers.reserve (maxNumTimers);

            for (int i = 0; i < maxNumTimers; ++i)
            {
                auto timer = std::make_unique<Timer> ([]{});
                timer->startTimer (1);

                if (! timer->isTimerRunning())
                    break;

                timers.push_back (std::move (timer));
            }

            expect (timers.size() >= 16);
        }
    }

    class Timer : public HighResolutionTimer
    {
    public:
        explicit Timer (std::function<void()> fn)
            : callback (std::move (fn)) {}

        ~Timer() override { stopTimer(); }

        void hiResTimerCallback() override { callback(); }

    private:
        std::function<void()> callback;
    };
};

static HighResolutionTimerTests highResolutionTimerTests;

#endif

} // namespace juce
