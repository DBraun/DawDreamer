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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

#if JUCE_IOS
 bool isIOSAppActive = true;
#endif

API_AVAILABLE (macos (10.10))
static auto getNativeQOS (Thread::Priority priority)
{
    switch (priority)
    {
        case Thread::Priority::highest:    return QOS_CLASS_USER_INTERACTIVE;
        case Thread::Priority::high:       return QOS_CLASS_USER_INITIATED;
        case Thread::Priority::low:        return QOS_CLASS_UTILITY;
        case Thread::Priority::background: return QOS_CLASS_BACKGROUND;
        case Thread::Priority::normal:     break;
    }

    return QOS_CLASS_DEFAULT;
}

API_AVAILABLE (macos (10.10))
static auto getJucePriority (qos_class_t qos)
{
    switch (qos)
    {
        case QOS_CLASS_USER_INTERACTIVE:    return Thread::Priority::highest;
        case QOS_CLASS_USER_INITIATED:      return Thread::Priority::high;
        case QOS_CLASS_UTILITY:             return Thread::Priority::low;
        case QOS_CLASS_BACKGROUND:          return Thread::Priority::background;

        case QOS_CLASS_UNSPECIFIED:
        case QOS_CLASS_DEFAULT:             break;
    }

    return Thread::Priority::normal;
}

bool Thread::createNativeThread (Priority priority)
{
    PosixThreadAttribute attr { threadStackSize };

    if (@available (macos 10.10, *))
        pthread_attr_set_qos_class_np (attr.get(), getNativeQOS (priority), 0);
    else
        PosixSchedulerPriority::getNativeSchedulerAndPriority (realtimeOptions, priority).apply (attr);

    threadId = threadHandle = makeThreadHandle (attr, this, [] (void* userData) -> void*
    {
        auto* myself = static_cast<Thread*> (userData);

        JUCE_AUTORELEASEPOOL
        {
            juce_threadEntryPoint (myself);
        }

        return nullptr;
    });

    return threadId != nullptr;
}

void Thread::killThread()
{
    if (threadHandle != nullptr)
        pthread_cancel ((pthread_t) threadHandle.load());
}

Thread::Priority Thread::getPriority() const
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    if (! isRealtime())
    {
        if (@available (macOS 10.10, *))
            return getJucePriority (qos_class_self());

        // fallback for older versions of macOS
        const auto min = jmax (0, sched_get_priority_min (SCHED_OTHER));
        const auto max = jmax (0, sched_get_priority_max (SCHED_OTHER));

        if (min != 0 && max != 0)
        {
            const auto native = PosixSchedulerPriority::findCurrentSchedulerAndPriority().getPriority();
            const auto mapped = jmap (native, min, max, 0, 4);
            return ThreadPriorities::getJucePriority (mapped);
        }
    }

    return {};
}

bool Thread::setPriority (Priority priority)
{
    jassert (Thread::getCurrentThreadId() == getThreadId());

    if (isRealtime())
    {
        // macOS/iOS needs to know how much time you need!
        jassert (realtimeOptions->workDurationMs > 0);

        mach_timebase_info_data_t timebase;
        mach_timebase_info (&timebase);

        const auto periodMs = realtimeOptions->workDurationMs;
        const auto ticksPerMs = ((double) timebase.denom * 1000000.0) / (double) timebase.numer;
        const auto periodTicks = (uint32_t) jmin ((double) std::numeric_limits<uint32_t>::max(), periodMs * ticksPerMs);

        thread_time_constraint_policy_data_t policy;
        policy.period = periodTicks;
        policy.computation = jmin ((uint32_t) 50000, policy.period);
        policy.constraint = policy.period;
        policy.preemptible = true;

        return thread_policy_set (pthread_mach_thread_np (pthread_self()),
                                  THREAD_TIME_CONSTRAINT_POLICY,
                                  (thread_policy_t) &policy,
                                  THREAD_TIME_CONSTRAINT_POLICY_COUNT) == KERN_SUCCESS;
    }

    if (@available (macOS 10.10, *))
        return pthread_set_qos_class_self_np (getNativeQOS (priority), 0) == 0;

   #if JUCE_ARM
    // M1 platforms should never reach this code!!!!!!
    jassertfalse;
   #endif

    // Just in case older versions of macOS support SCHED_OTHER priorities.
    const auto psp = PosixSchedulerPriority::getNativeSchedulerAndPriority ({}, priority);

    struct sched_param param;
    param.sched_priority = psp.getPriority();
    return pthread_setschedparam (pthread_self(), psp.getScheduler(), &param) == 0;
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
   if (SystemStats::isRunningInAppExtensionSandbox())
       return true;

   #if JUCE_MAC
    return [NSApp isActive];
   #else
    return isIOSAppActive;
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess()
{
   #if JUCE_MAC
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [NSApp activateIgnoringOtherApps: YES];
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::hide()
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
    {
       #if JUCE_MAC
        [NSApp hide: nil];
       #elif JUCE_IOS
        [[UIApplication sharedApplication] performSelector: @selector(suspend)];
       #endif
    }
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege() {}
JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege() {}

JUCE_API void JUCE_CALLTYPE Process::setPriority (ProcessPriority) {}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
    struct kinfo_proc info;
    int m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    size_t sz = sizeof (info);
    sysctl (m, 4, &info, &sz, nullptr, 0);
    return (info.kp_proc.p_flag & P_TRACED) != 0;
}

} // namespace juce
