/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef __LFPDISPLAYOPTIONS_H__
#define __LFPDISPLAYOPTIONS_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"

#include "EventDisplayInterface.h"
#include "ShowHideOptionsButton.h"

namespace LfpViewer {

/**
 
    Holds the LfpDisplay UI controls
 
 */
class LfpDisplayOptions : 
    public Component,
    public ComboBox::Listener,
    public Button::Listener
{
public:

    /** Construtor */
    LfpDisplayOptions(LfpDisplayCanvas*,
                      LfpDisplaySplitter*, 
                      LfpTimescale*, 
                      LfpDisplay*, 
                      LfpDisplayNode*);

    /** Destructor */
    ~LfpDisplayOptions() { }

    /** Paint background*/
    void paint(Graphics& g);

    /** Set sub-component locations*/
    void resized();

    /** Respond to combo box selection */
    void comboBoxChanged(ComboBox* cb);

    /** Respond to button clicks */
    void buttonClicked(Button* button);

    /** Set range selection combo box to correct value if it has been changed by scolling etc. */
    void setRangeSelection(float range, bool canvasMustUpdate = false); 
    
    /** Set spread selection combo box to correct value if it has been changed by scolling etc. */
    void setSpreadSelection(int spread, bool canvasMustUpdate = false, bool deferDisplayRefresh = false); 

    /** Changes the timebase value used by LfpTimescale and LfpDisplayCanvas. */
    void setTimebaseAndSelectionText(float timebase);

    /** Returns the selected channel height*/
    int getChannelHeight();

    /** Returns true if channel polarity is inverted */
    bool getInputInvertedState();

    /** Returns true if channel names should be shown*/
	bool getChannelNameState();

    /** Toggles pause button (e.g. if space bar is pressed) */
    void togglePauseButton(bool sendUpdate = true);

    /** Saves all options to XML */
    void saveParameters(XmlElement* xml);

    /** Loads options from XML */
    void loadParameters(XmlElement* xml);

    /** Returns the channel type of a given channel index */
	ContinuousChannel::Type getChannelType(int index);

    /** Returns the selected channel type for the range editor */
    ContinuousChannel::Type getSelectedType();

    /** Returns the name for a given channel type (DATA, AUX, ADC) */
    String getTypeName(ContinuousChannel::Type type);

    /** Returns the range step size for a given channel type (DATA, AUX, ADC) */
	int getRangeStep(ContinuousChannel::Type type);

    /** Set the selected channel type (DATA, AUX, ADC) */
	void setSelectedType(ContinuousChannel::Type type, bool toggleButton = true);

    /** Sets whether channel order should be reversed */
    void setChannelsReversed(bool);

    /** Sets whether channels should be sorted by depth*/
    void setSortByDepth(bool);

    /** Sets whether signal polarity should be inverted */
    void setInputInverted(bool);

    /** Sets whether the median of each channel should be subtracted */
    void setMedianOffset(bool);

    /** Sets whether to use averaging in triggered display*/
    void setAveraging(bool);
    
    /** Sets whether channel numbers should be shown instead of names */
    void setShowChannelNumbers(bool);

    int selectedSpread;
    String selectedSpreadValue;

    int selectedTimebase;
    String selectedTimebaseValue;

    int selectedOverlap;
    String selectedOverlapValue;
    
    int selectedChannelDisplaySkip;
    String selectedChannelDisplaySkipValue;
    
    int selectedSpikeRasterThreshold;
    String selectedSpikeRasterThresholdValue;

    // this enum is a candidate option for refactoring, not used yet
    enum ChannelDisplaySkipValue {
        None = 0,
        One,
        Two,
        Four,
        Eight,
        Sixteen,
        ThirtyTwo
    } enum_selectedChannelDisplaySkipValue = None;
    
    float selectedSaturationValueFloat; 

private:

    LfpDisplayCanvas* canvas;
    LfpDisplaySplitter* canvasSplit;
    LfpDisplay* lfpDisplay;
    LfpTimescale* timescale;
    LfpDisplayNode* processor;
    
    Font labelFont;
    Colour labelColour;

    // Main options
    ScopedPointer<ComboBox> timebaseSelection;
    ScopedPointer<ComboBox> spreadSelection;
    ScopedPointer<ComboBox> rangeSelection;
    OwnedArray<UtilityButton> typeButtons;
    
    ScopedPointer<ComboBox> overlapSelection; // what do we do with this?
    
    OwnedArray<EventDisplayInterface> eventDisplayInterfaces;
    ScopedPointer<UtilityButton> pauseButton;
    ScopedPointer<ComboBox> colourSchemeOptionSelection;
    ScopedPointer<ComboBox> colorGroupingSelection;
    
    ScopedPointer<ShowHideOptionsButton> showHideOptionsButton;

    // THRESHOLDS SECTION
    ScopedPointer<ComboBox> spikeRasterSelection;
    ScopedPointer<ComboBox> saturationWarningSelection; // optionally raise hell if the actual data is saturating
    ScopedPointer<ComboBox> clipWarningSelection; // optinally draw (subtle) warning if data is clipped in display
    
    // CHANNELS SECTION
    ScopedPointer<UtilityButton> reverseChannelsDisplayButton;
    ScopedPointer<UtilityButton> sortByDepthButton;
    ScopedPointer<ComboBox> channelDisplaySkipSelection;
    ScopedPointer<UtilityButton> showChannelNumberButton;

    // SIGNAL PROCESSING SECTION
    ScopedPointer<UtilityButton> medianOffsetPlottingButton;
    ScopedPointer<UtilityButton> invertInputButton;

    // TRIGGERED DISPLAY SECTION
    ScopedPointer<ComboBox> triggerSourceSelection;
    ScopedPointer<UtilityButton> averageSignalButton;
    ScopedPointer<UtilityButton> resetButton;
     
    StringArray voltageRanges[CHANNEL_TYPES];
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels
    StringArray colorGroupings; // option for coloring every N channels the same
    StringArray triggerSources; // option for trigger source event channel
    StringArray overlaps; //
    StringArray saturationThresholds; //default values for when different amplifiers saturate
    StringArray clipThresholds;
    StringArray spikeRasterSelectionOptions;
    StringArray channelDisplaySkipOptions;
    StringArray sectionTitles;
    
	ContinuousChannel::Type selectedChannelType;
    int selectedVoltageRange[CHANNEL_TYPES];
    String selectedVoltageRangeValues[CHANNEL_TYPES];
    float rangeGain[CHANNEL_TYPES];
    StringArray rangeUnits;
    StringArray typeNames;
    int rangeSteps[CHANNEL_TYPES];

    bool medianOffsetOnForSpikeRaster;
};
    
}; // namespace

#endif
