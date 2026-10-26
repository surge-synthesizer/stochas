/***************************************************************
 ** Copyright (C) 2026 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef STANDALONESETTINGS_H_
#define STANDALONESETTINGS_H_

/*
Settings that only apply to the standalone app. They are kept in the same
property file JUCE's standalone wrapper uses for its Audio/MIDI settings
(the "audioSetup" entry). Values are cached in atomics so the getters are safe
to call from the audio thread. When not running in the standalone wrapper the
setters do nothing and the getters return false.
*/
namespace StandaloneSettings
{
   // refresh the cached values from the saved settings. Not for the audio thread.
   void load();

   // whether midi clock should be sent (continuously, whether or not we are playing)
   bool getSendMidiClock();
   void setSendMidiClock(bool send);

   // whether midi start/continue/stop should be sent
   bool getSendMidiTransport();
   void setSendMidiTransport(bool send);
}

#endif
