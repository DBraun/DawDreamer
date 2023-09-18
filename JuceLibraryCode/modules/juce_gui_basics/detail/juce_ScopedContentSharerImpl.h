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

namespace juce::detail
{

class ConcreteScopedContentSharerImpl : public ScopedMessageBoxImpl,
                                        private AsyncUpdater
{
public:
    static ScopedMessageBox show (std::unique_ptr<ScopedContentSharerInterface>&& native,
                                  ContentSharer::Callback callback)
    {
        return ScopedMessageBox (runAsync (std::move (native), std::move (callback)));
    }

    ~ConcreteScopedContentSharerImpl() override
    {
        cancelPendingUpdate();
    }

    void close() override
    {
        cancelPendingUpdate();
        nativeImplementation->close();
        self.reset();
    }

private:
    static std::shared_ptr<ConcreteScopedContentSharerImpl> runAsync (std::unique_ptr<ScopedContentSharerInterface>&& p,
                                                                      ContentSharer::Callback&& c)
    {
        std::shared_ptr<ConcreteScopedContentSharerImpl> result (new ConcreteScopedContentSharerImpl (std::move (p), std::move (c)));
        result->self = result;
        result->triggerAsyncUpdate();
        return result;
    }

    ConcreteScopedContentSharerImpl (std::unique_ptr<ScopedContentSharerInterface>&& p,
                                     ContentSharer::Callback&& c)
        : callback (std::move (c)), nativeImplementation (std::move (p)) {}

    void handleAsyncUpdate() override
    {
        nativeImplementation->runAsync ([weakRecipient = std::weak_ptr<ConcreteScopedContentSharerImpl> (self)] (bool result, const String& error)
                                        {
                                            const auto notifyRecipient = [result, error, weakRecipient]
                                            {
                                                if (const auto locked = weakRecipient.lock())
                                                {
                                                    NullCheckedInvocation::invoke (locked->callback, result, error);
                                                    locked->self.reset();
                                                }
                                            };

                                            if (MessageManager::getInstance()->isThisTheMessageThread())
                                                notifyRecipient();
                                            else
                                                MessageManager::callAsync (notifyRecipient);
                                        });
    }

    ContentSharer::Callback callback;
    std::unique_ptr<ScopedContentSharerInterface> nativeImplementation;

    /*  The 'old' native message box API doesn't have a concept of content sharer owners.
        Instead, content sharers have to clean up after themselves, once they're done displaying.
        To allow this mode of usage, the implementation keeps an owning reference to itself,
        which is cleared once the content sharer is closed or asked to quit. To display a content
        sharer box without a scoped lifetime, just create a Pimpl instance without using
        the ScopedContentSharer wrapper, and the Pimpl will destroy itself after it is dismissed.
    */
    std::shared_ptr<ConcreteScopedContentSharerImpl> self;
};

} // namespace juce::detail
