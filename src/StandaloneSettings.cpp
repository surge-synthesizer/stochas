/***************************************************************
 ** Copyright (C) 2026 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "StandaloneSettings.h"
#include <JuceHeader.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#include <atomic>

namespace
{
   const char *kSendMidiClockKey = "stochasSendMidiClock";
   const char *kSendMidiTransportKey = "stochasSendMidiTransport";

   std::atomic<bool> sSendClock{ false };
   std::atomic<bool> sSendTransport{ false };

   PropertySet *getSettings()
   {
      auto *holder = StandalonePluginHolder::getInstance();
      return holder != nullptr ? holder->settings.get() : nullptr;
   }
}

void StandaloneSettings::load()
{
   if (auto *s = getSettings()) {
      sSendClock = s->getBoolValue(kSendMidiClockKey, false);
      sSendTransport = s->getBoolValue(kSendMidiTransportKey, false);
   }
}

bool StandaloneSettings::getSendMidiClock()
{
   return sSendClock;
}

void StandaloneSettings::setSendMidiClock(bool send)
{
   if (auto *s = getSettings()) {
      s->setValue(kSendMidiClockKey, send);
      sSendClock = send;
   }
}

bool StandaloneSettings::getSendMidiTransport()
{
   return sSendTransport;
}

void StandaloneSettings::setSendMidiTransport(bool send)
{
   if (auto *s = getSettings()) {
      s->setValue(kSendMidiTransportKey, send);
      sSendTransport = send;
   }
}
