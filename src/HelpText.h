/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/
#ifndef HELP_TEXT_OK
#error "This file shouldn't be included"
#endif

#if JUCE_MAC
#define CTRL "Cmd"
#else
#define CTRL "Ctrl"
#endif


HelpPair gHelpText[] = {
   {"main",          "Move the mouse over a component to get more information about that component"},
   {"optionsPanel",  "Options on this pane are saved with the project/patch" },
   {"stepPanelStepMode",    "Left click selects a cell. Click multiple times, or click and drag to change value. Drag near right border of selected cell to change length. Shift-drag to select multiple cells. Arrow keys move selected cells. Delete key clears selected cells" },
   {"stepPanelVeloMode", "Click cell multiple times or click and drag to change velocity on valid cells"},
   {"stepPanelChainMode", "Click and drag on valid cells to create chains to other cells. "
                           CTRL "-drag to create a negative chain. "
                           CTRL "-shift-drag to create a custom chain. "
                           "Right-click a target cell to clear chains leading to that cell" },
   {"stepPanelOffsMode", "Shift cells forward and backward in time. Negative values will cause the cell to play sooner. Positive values will cause the cell to play later. A value of -50 will cause the cell to play halfway between the previous step and the step it is on"},
   { "sectionSelect" , "Select which section of the current pattern to make visible for editing"},
   {"editButton",    "Opens a dialog to allow copying, clearing and bulk editing"},
   {"helpButton",    "Launches the browser to display a user manual" },
   {"lengthCursor",  "Drag left or right to adjust play length of the cell. Right-click to reset. To create a length past the end of the sequence, drag left of the cell to wrap around"},
   {"playLight",     "Indicates the step number, and will be lit when the step is currently playing"},
   {"notePanel",     "Indicate the note for this row. In custom mode you may adjust this by dragging note values (or double-clicking note value). Double-click near the center to edit label in custom mode. Hit play button to preview sound for this note" },
   {"tgMuted",       "Prevents the current layer from producing any notes"},
   {"btnLdCust",     "Load a custom set of note values and labels from a file"},
   {"btnSvCust",     "Save note values and labels to a file" },
   {"numHumanVelo",  "Varies the velocity randomly either up or down. A higher value results in more variation. For best results, cells should have a velocity lower than 127 to allow variance in both directions"},
   { "numHumanPos",  "Varies the position of played cells randomly either forward or backward in time. A higher value results in more variation. A value of 50% will allow cells to move halfway to the next or previous cell. This is additive with groove/swing" },
   { "numHumanLength",  "Varies the length of the played cells. A higher value results in more variation. The value selected is a percentage of the actual cell length, so it will always subtract from the length. A value of 100% would allow the cell length to decrease all the way to 0" },
   {"patternSelect", "Select the active pattern for playback. Normally this will change to the same pattern for all layers" },
   {"midiIndicator", "Will be lit when incoming MIDI data is received"},
   {"editModeSelect","Select which edit mode is currently active. In 'prob' mode you can add/remove cells and edit lengths. In 'velo' mode you can change velocity values for each cell. In 'chain' mode you can create dependencies between cells"},
   { "tgMP" ,        "Mono mode - a single note is randomly selected for each step, based on the cells that are on. Poly mode - multiple notes may play for each step in the sequence. For drum programming use poly mode"},
   { "tgClkDiv",     "Specifies how fast to play the sequence. A value of 1 will play all of the steps in a measure in time with the DAW tempo. A value of 1/2 will play at half the speed of the DAW tempo"},
   {"tgScale" ,      "Standard scales are preset to their respective intervals. Custom scales may be edited"},
   {"tgPL" ,         "Specifies the length of the sequence in steps. All patterns for a specific layer will have the same length. Different layers may have different lengths"},
   {"tgDC" ,         "Specifies how long a note will play for a given step. A value of 50% will play for half the length of time of each step. Note that if a step has a length greater than 1 cell wide, only the final cell in it's width will be reduced according to the duty cycle. Values greater than 100 will result in legato playing"},
   {"tgMIDI",        "Specifies the MIDI channel that will be used to output notes for the current layer" },
   {"tgSPM",         "Number of steps to play in one measure of time. The default value of 16 means that there are 16 steps in each measure. Specify a lower value for alternate time signatures. For example a value of 12 would be appropriate for a 3/4 time signature" },
   {"tgRows" ,       "Number of note rows available for the layer. Reduce this if you do not need all available notes" },
   {"cbbScale" ,     "Select a scale to populate the list of notes at the left of the sequencer" },
   {"cbbKey" ,       "Select a key. This will be used in combination with the scale to populate notes at the left of the sequencer" },
   {"cbbOct" ,       "Select a starting octave to populate the list of notes at the left of the sequencer" },
   {"toggleLabel",   "Select the currently active layer. Shift-Click to Mute/Unmute layer. " CTRL "-Click to Solo/Unsolo layer. All layers play simultaneously. Individual layers have their own set of options as reflected on the 'Layer Options' tab"},
   {"randomToggle",  "Setting this value to stable will cause the same random selections to occur at any given point in the DAW project. If you set this after any playback has occurred, the random selections used in the last playback will persist and be used for subsequent playback. If set to varying, new random values will be generated every time playback starts or loops"},
   { "tglChord",     "Select a chord then click on the cell representing the root note to draw in that chord. Use up and down arrow keys to create chord inversions" },
   { "Layer Options","Edit options that are specific to the currently selected layer" },
   { "Patch Options","Edit options that are shared by all layers" },
   { "Groove/Swing", "Assign a swing or groove pattern to all layers" },
   { "Chords",       "Input multiple cells representing chords" },
   { "Settings",     "Edit user preferences that are saved on your workstation (not with the patch)" },
   { "setDefPoly",   "Default value for cells entered in poly-mode" },
   { "setDefMono",   "Default value for cells entered in mono-mode" },
   { "setRtMouse",   "Behaviour when a cell is right-clicked. Delete will clear the cell. Cycle down will reduce it's value to the next step down" },
   { "setMouseSense","A higher value will cause values to increase/decrease faster when the mouse is dragged over them" },
   { "setDefVelo",   "Default velocity for newly entered cells" },
   { "setOctave",    "Determines what is considered as the lowest octave in the scale (for display purposes only). Different programs have different ideas of where the octave scale starts, so this helps with compatibility with those programs" },
   { "setColor",     "Determines whether a dark or a light colour scheme is used" },
   { "midiMap",      "Create mappings for incoming MIDI values. This allows you to control various things with a MIDI track, a controller or another instance of Stochas" },
   { "midiRespond",  "Whether Stochas responds to incoming MIDI messages or not. If this is set to no, then incoming messages will be ignored. If passthru is set to Unhandled in this case, then all messages will be passed through" },
   { "midiPass",     "Whether Stochas passes incoming MIDI messages on to the next plugin in the chain or to the output. If passthru is set to Unhandled, then any messages that were not used by Stochas (via MIDI Mapping) will be passed along" },
   { "mainScroll",   "Scroll through the range of visible note rows. Mouse wheel also scrolls" },
   { "grv",          "Groove is active when swing is set to 0%. Adjust the groove for each 16th step either forward or backward" },
   { "grvSwing",     "Adjust the amount of swing. Swing overrides groove" },
   { "grvClr",       "When swing is active this copies swing to groove and makes groove active. When groove is active this clears groove" },
   { "grvLoadMidi",  "Extract groove information from a MIDI file"},
   { "grvSaveMidi",  "Save groove information to a MIDI file"},
   { "shiftRev",     "Normally, shift-drag does a cell selection, while regular mouse drag changes cell contents or creates chains in Chain mode. Reversing it causes regular mouse drag to do cell selection, and shift-drag will change cell values or create chains"},
   { "loadPatch",    "Load all data from a file"},
   { "savePatch",    "Save all data to a file"},
   {"numMaxPoly",    "Maximum number of cells that will be selected to play at the same time on a given step"},
   {"numPolyBias",   "Add to, or subtract from the probability of all notes. Can be controlled via MIDI CC"},
   { "combineToggle" , "This determines behavior for when a note is triggered while it's already playing. The playing note can be trimmed in length, or the two notes can be combined as a single note"},
   { "lblLayerName", "Double-Click to edit the descriptive name for this layer"},
   { "lblPatternName", "Double-Click to edit the descriptive name for this pattern" },
   { "setPosOffset",  "Advanced setting. This makes micro adjustments to the perceived play head position. Should stay set to 0 in most cases"},
   { "undoButton" ,   "Click once to undo the last operation. Click again to redo the last operation"},
   { "recordButton" , "Toggle record mode. Recording takes place only when playback is started, and automatically ends when playback stops.  MIDI notes received by Stochas which are not mapped to anything will be recorded to the current pattern and layer" },
   { "playButton",    "Toggle manual playback mode. If quantizing manual playback, this will indicate standby until quantization division is reached. Playback will only start when DAW transport is playing"},
   { "playbackMode",  "Specify whether to sync playback with DAW (Auto) or to manually start playback when DAW is playing. If set to step, beat, or measure, then playback will start when that division is reached after Play is pressed. If set to Instant, playback will start as soon as Play is pressed"},
   { "lblBPM",        "Set the beats per minute for playback"},
   { "standaloneBPM", "Set the beats per minute for playback"},
   { "setUIScale",    "Sets the overall size of the UI, allowing you to enlarge the controls for better visibility on large screens."},
   {0,0} // end sentinel
};
