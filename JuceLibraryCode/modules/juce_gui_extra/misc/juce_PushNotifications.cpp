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

namespace juce
{

//==============================================================================
#if ! JUCE_ANDROID && ! JUCE_IOS && ! JUCE_MAC
bool PushNotifications::Notification::isValid() const noexcept { return true; }
#endif

PushNotifications::Notification::Notification (const Notification& other)
    : identifier (other.identifier),
      title (other.title),
      body (other.body),
      subtitle (other.subtitle),
      groupId (other.groupId),
      badgeNumber (other.badgeNumber),
      soundToPlay (other.soundToPlay),
      properties (other.properties),
      category (other.category),
      triggerIntervalSec (other.triggerIntervalSec),
      repeat (other.repeat),
      icon (other.icon),
      channelId (other.channelId),
      largeIcon (other.largeIcon),
      tickerText (other.tickerText),
      actions (other.actions),
      progress (other.progress),
      person (other.person),
      type (other.type),
      priority (other.priority),
      lockScreenAppearance (other.lockScreenAppearance),
      publicVersion (other.publicVersion.get() != nullptr ? new Notification (*other.publicVersion) : nullptr),
      groupSortKey (other.groupSortKey),
      groupSummary (other.groupSummary),
      accentColour (other.accentColour),
      ledColour (other.ledColour),
      ledBlinkPattern (other.ledBlinkPattern),
      vibrationPattern (other.vibrationPattern),
      shouldAutoCancel (other.shouldAutoCancel),
      localOnly (other.localOnly),
      ongoing (other.ongoing),
      alertOnlyOnce (other.alertOnlyOnce),
      timestampVisibility (other.timestampVisibility),
      badgeIconType (other.badgeIconType),
      groupAlertBehaviour (other.groupAlertBehaviour),
      timeoutAfterMs (other.timeoutAfterMs)
{
}

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (PushNotifications)

PushNotifications::PushNotifications()
  #if JUCE_PUSH_NOTIFICATIONS
    : pimpl (new Pimpl (*this))
  #endif
{
}

PushNotifications::~PushNotifications() { clearSingletonInstance(); }

void PushNotifications::addListener (Listener* l)      { listeners.add (l); }
void PushNotifications::removeListener (Listener* l)   { listeners.remove (l); }

void PushNotifications::requestPermissionsWithSettings ([[maybe_unused]] const PushNotifications::Settings& settings)
{
  #if JUCE_PUSH_NOTIFICATIONS && (JUCE_IOS || JUCE_MAC)
    pimpl->requestPermissionsWithSettings (settings);
  #else
    listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
  #endif
}

void PushNotifications::requestSettingsUsed()
{
  #if JUCE_PUSH_NOTIFICATIONS && (JUCE_IOS || JUCE_MAC)
    pimpl->requestSettingsUsed();
  #else
    listeners.call ([] (Listener& l) { l.notificationSettingsReceived ({}); });
  #endif
}

bool PushNotifications::areNotificationsEnabled() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    return pimpl->areNotificationsEnabled();
  #else
    return false;
  #endif
}

void PushNotifications::getDeliveredNotifications() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->getDeliveredNotifications();
  #endif
}

void PushNotifications::removeAllDeliveredNotifications()
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removeAllDeliveredNotifications();
  #endif
}

String PushNotifications::getDeviceToken() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    return pimpl->getDeviceToken();
  #else
    return {};
  #endif
}

void PushNotifications::setupChannels ([[maybe_unused]] const Array<ChannelGroup>& groups, [[maybe_unused]] const Array<Channel>& channels)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->setupChannels (groups, channels);
  #endif
}

void PushNotifications::getPendingLocalNotifications() const
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->getPendingLocalNotifications();
  #endif
}

void PushNotifications::removeAllPendingLocalNotifications()
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removeAllPendingLocalNotifications();
  #endif
}

void PushNotifications::subscribeToTopic ([[maybe_unused]] const String& topic)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->subscribeToTopic (topic);
  #endif
}

void PushNotifications::unsubscribeFromTopic ([[maybe_unused]] const String& topic)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->unsubscribeFromTopic (topic);
  #endif
}


void PushNotifications::sendLocalNotification ([[maybe_unused]] const Notification& n)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->sendLocalNotification (n);
  #endif
}

void PushNotifications::removeDeliveredNotification ([[maybe_unused]] const String& identifier)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removeDeliveredNotification (identifier);
  #endif
}

void PushNotifications::removePendingLocalNotification ([[maybe_unused]] const String& identifier)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->removePendingLocalNotification (identifier);
  #endif
}

void PushNotifications::sendUpstreamMessage ([[maybe_unused]] const String& serverSenderId,
                                             [[maybe_unused]] const String& collapseKey,
                                             [[maybe_unused]] const String& messageId,
                                             [[maybe_unused]] const String& messageType,
                                             [[maybe_unused]] int timeToLive,
                                             [[maybe_unused]] const StringPairArray& additionalData)
{
  #if JUCE_PUSH_NOTIFICATIONS
    pimpl->sendUpstreamMessage (serverSenderId,
                                collapseKey,
                                messageId,
                                messageType,
                                timeToLive,
                                additionalData);
  #endif
}

//==============================================================================
void PushNotifications::Listener::notificationSettingsReceived ([[maybe_unused]] const Settings& settings) {}
void PushNotifications::Listener::pendingLocalNotificationsListReceived ([[maybe_unused]] const Array<Notification>& notifications) {}
void PushNotifications::Listener::handleNotification ([[maybe_unused]] bool isLocalNotification,
                                                      [[maybe_unused]] const Notification& notification) {}
void PushNotifications::Listener::handleNotificationAction ([[maybe_unused]] bool isLocalNotification,
                                                            [[maybe_unused]] const Notification& notification,
                                                            [[maybe_unused]] const String& actionIdentifier,
                                                            [[maybe_unused]] const String& optionalResponse) {}
void PushNotifications::Listener::localNotificationDismissedByUser ([[maybe_unused]] const Notification& notification) {}
void PushNotifications::Listener::deliveredNotificationsListReceived ([[maybe_unused]] const Array<Notification>& notifications) {}
void PushNotifications::Listener::deviceTokenRefreshed ([[maybe_unused]] const String& token) {}
void PushNotifications::Listener::remoteNotificationsDeleted() {}
void PushNotifications::Listener::upstreamMessageSent ([[maybe_unused]] const String& messageId) {}
void PushNotifications::Listener::upstreamMessageSendingError ([[maybe_unused]] const String& messageId,
                                                               [[maybe_unused]] const String& error) {}

} // namespace juce
