/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __GENERICEDITOR_H_DD406E71__
#define __GENERICEDITOR_H_DD406E71__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../../CoreServices.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include "../Settings/InfoObject.h"

#include <stdio.h>

class GenericProcessor;
class DrawerButton;
class TriangleButton;
class UtilityButton;
class ParameterEditor;
class StreamSelector;
class TTLMonitor;
class DelayMonitor;

/**
    Base class for creating processor editors.

    If a processor doesn't havesign an editor defined, a GenericEditor will be used.

    Classes derived from this class must place their controls as child components.
    They shouldn't try to re-draw any aspects of their background.

    @see GenericProcessor, EditorViewport
*/
class PLUGIN_API GenericEditor  : public AudioProcessorEditor
                                , public Timer
                                , public Button::Listener
                                , public Slider::Listener
{
public:
    /** Constructor. Loads fonts and creates default buttons.
        useDefaultParameter Editors false means custom parameter editors will be used.*/
    GenericEditor (GenericProcessor* owner, bool useDefaultParameterEditors, bool showDrawerButton = true);

    /** Destructor.*/
    virtual ~GenericEditor();

    /*
    ========================================================================
    ============================= JUCE METHODS =============================
    ========================================================================

    */
    /** Draws the editor's background.*/
    void paint (Graphics& g) override;

    /** Called whenever a key is pressed and the editor has keyboard focus.*/
    bool keyPressed (const KeyPress& key) override;

    /** Handles button clicks for all editors. Deals with clicks on the editor's
        title bar and channel selector drawer. */
    virtual void buttonClicked (Button* buttonThatWasClicked) override;

    /** Called when the boundaries of the editor are updated. */
    virtual void resized() override;

    /** Handles slider events for all editors. */
    virtual void sliderValueChanged (Slider* sliderWhichValueHasChanged) override;

    // =====================================================================
    // =====================================================================
    // =====================================================================


    /** Toggles the editor's selection state.*/
    void switchSelectedState();

    /** Highlights an editor and calls editorWasClicked().*/
    void select();

    /** Highlights an editor.*/
    void highlight();

    /** Makes an editor visible if it's not already.*/
    void makeVisible();

    /** Deselects an editor.*/
    void deselect();

    /** Returns an editor's selection state.*/
    bool getSelectionState();

    /** Used to set desired width of editor. */
    void setDesiredWidth (int width);

    /** Called at the start of a recording **/
    void startRecording();

    /** Called at the end of a recording **/
    void stopRecording();

    /** Called just prior to the start of acquisition, to allow the editor to prepare.*/
    void editorStartAcquisition();

	/** Called just prior to the start of acquisition, to allow custom commands. */
	virtual void startAcquisition() { }

    /** Called after the end of acquisition.*/
    void editorStopAcquisition();

	/** Called after the end of acquisition, to allow custom commands .*/
	virtual void stopAcquisition() { }

    /** Returns the name of the editor.*/
    String getName();

    /** Updates name if processor ID changes. */
    void updateName();

    /** Updates name on title bar. */
    void setDisplayName(const String& string);

    /** Get name on title bar. */
    String getDisplayName();

	/** Returns a custom channel number for the Channel Selector buttons. Useful for channel mappers */
	virtual int getChannelDisplayNumber(int chan) const;

    /** Determines how wide the editor will be drawn. */
    int desiredWidth;

    /** The unique integer ID of the editor's processor. */
    int nodeId;

    /** Sets the number of the editor's associated tab in the DataViewport. */
    virtual void tabNumber(int t);

    /** Returns the number of the editor's associated tab in the DataViewport. */
    int tabNumber();

    /** Required for MergerEditor only.*/
    virtual void switchSource(int);

    /** Required for MergerEditor only.*/
    virtual void switchSource();

    /** Returns the processor associated with an editor.*/
    GenericProcessor* getProcessor() const;

    /** Causes the editor to fade in when it first appears in the EditorViewport. */
    void fadeIn();

    /** Indicates whether or not the editor is in the processof fading in. */
    bool isFading;

    /** Used to control the speed at which the editor fades in. */
    float accumulator;

    /** Required for SplitterEditor only.*/
    virtual void switchDest();

    /** Required for SplitterEditor and MergerEditor only.*/
    virtual void switchIO (int);

    /** Required for SplitterEditor and MergerEditor only.*/
    virtual int getPathForEditor (GenericEditor* editor);

    /** Used by GraphViewer */
    bool isSplitter();

    /** Used by GraphViewer */
    bool isMerger();

    bool isUtility();

    /** Called by buttonClicked(). Deals with clicks on custom buttons. Subclasses of
        GenericEditor should modify this method only.*/
    virtual void buttonEvent (Button* button);

    /** Called by sliderValueChanged(). Deals with clicks on custom sliders. Subclasses
        of GenericEditor should modify this method only.*/
    virtual void sliderEvent (Slider* slider);

    /** Required for opening displays in a VisualizerEditor. Hopefully will be deprecated soon.*/
    virtual void editorWasClicked();

    /** Checks to see if a button click occurred on the ChannelSelector drawer button.*/
    bool checkDrawerButton (Button* button);

    /** Selects all the channels in the input array.*/
    void selectChannels (Array<int>);

    /** Refreshes an editor's background colors when the user selects new ones with the ColourSelector.*/
    void refreshColors();

    /** Called when an editor's processor updates its settings (mainly to update channel count).*/
    void update(bool isEnabled);

    /** Allows other UI elements to use background color of editor. */
    Colour getBackgroundColor();

    /** Changes the background color of this editor. */
    void setBackgroundColor(Colour colour);

    /** Allows other elements to use background gradient of editor. */
    ColourGradient getBackgroundGradient();

    /** Called by the update() method to allow the editor to update its custom settings.*/
    virtual void updateSettings();

    /** Called when the editor needs to update the view of its parameters.*/
    void updateView();

    /** Called when the editor needs to update the view of its parameters.*/
    virtual void updateCustomView();

    /** Allows an editor to update the settings of its visualizer (such as channel count and sample rate).*/
    virtual void updateVisualizer();

    /** An array of pointers to ParameterEditors created based on the Parameters of an editor's underlying processor. */
    OwnedArray<ParameterEditor> parameterEditors;

    /** Stores the font used to display the editor's name. */
    Font titleFont;

    /** True if data acquisition has begun. */
    bool acquisitionIsActive;

    /** Writes editor state to xml */
    void saveToXml (XmlElement* xml);

    /** Writes editor state to xml */
    void loadFromXml (XmlElement* xml);

    /** Writes editor state to xml */
    virtual void saveCustomParametersToXml (XmlElement* xml);

    /** Writes editor state to xml */
    virtual void loadCustomParametersFromXml (XmlElement* xml);

    /** Checks to see whether or not an editor is collapsed */
    bool getCollapsedState();

    /**  Collapses an editor if it's open, and opens it if it's collpased*/
    void switchCollapsedState();

    /**  Notifies the editor that the collapsed state changed, for non-standard function. */
    virtual void collapsedStateChanged();

    /** Returns the editor of this processor's source */
    GenericEditor* getSourceEditor();

    /** Returns the editor of this processor's destination */
    GenericEditor* getDestEditor();

    /** Returns the editors a splitter or merger is connected to */
    virtual Array<GenericEditor*> getConnectedEditors();

    /** Changes the state of the TTLMonitor */
    void setTTLState(uint16 streamId, int bit, bool state);

    /** Notify editor about changes in the StreamSelector */
    void updateSelectedStream(uint16 streamId);

    /** Get the ID of the stream that's currently selected.*/
    uint16 getCurrentStream() { return selectedStream; }

    /** Notifies editor that the selected stream has changed.*/
    virtual void selectedStreamHasChanged();

    /** Notifies editor that the selected stream has changed.*/
    virtual void streamEnabledStateChanged(uint16 streamId, bool enabledState);

    /** Updates the mean latency for a particular data stream (called by LatencyMeter class)*/
    void setMeanLatencyMs(uint16 streamId, float latencyMs);

protected:
    /** A pointer to the button that opens the drawer for the ChannelSelector. */
    std::unique_ptr<DrawerButton> drawerButton;

    /** Determines the width of the ChannelSelector drawer when opened. */
    int drawerWidth;

    /** Saves the open/closed state of the ChannelSelector drawer. */
    bool drawerOpen;

    /** Adds the default editor for a parameter of a given name. */
    void addDefaultParameterEditor (const String& name, int xPos, int yPos);

    /** Adds a custom editor for a parameter of a given name. */
    void addCustomParameterEditor(ParameterEditor* editor, int xPos, int yPos);

    /** A pointer to the editor's StreamSelector. */
    std::unique_ptr<StreamSelector> streamSelector;

    uint16 selectedStream;


private:
    /** Used for fading in the editor. */
    virtual void timerCallback() override;

    /** Stores the editor's background color. */
    Colour backgroundColor;

    /** Stores the editor's background gradient. */
    ColourGradient backgroundGradient;

    bool isSelected;
    bool isEnabled;
    bool isCollapsed;

    int tNum;
    int originalWidth;

    /**initializing function Used to share constructor functions*/
    void constructorInitialize (GenericProcessor* owner, bool useDefaultParameterEditors, bool showDrawerButton);

    String name;
    String displayName;

    std::map<uint16, DelayMonitor*> delayMonitors;
    std::map<uint16, TTLMonitor*> ttlMonitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);
};


/**
  Used to show and hide the StreamSelector.

  Appears on the right-hand size of all plugins that process
  at least one DataStream (except RecordNodeEditor).

  @see GenericEditor, StreamSelector
*/
class PLUGIN_API DrawerButton : public Button
{
public:
    DrawerButton (const String& name);
    ~DrawerButton();

private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;
};


/**
  A button that displays a triangle facing up or down.

  Useful for incrementing or decrementing values (as in SpikeDetectorEditor).

  @see GenericEditor
*/
class PLUGIN_API TriangleButton : public Button
{
public:
    TriangleButton (int direction_);
    ~TriangleButton();

private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    int direction;
};


/**
  A button that displays a "load" icon.

  @see GenericEditor
*/
class PLUGIN_API LoadButton : public ImageButton
{
public:
    LoadButton();
    ~LoadButton();
};


/**
  A button that displays a "save" icon.

  @see GenericEditor
*/
class PLUGIN_API SaveButton : public ImageButton
{
public:
    SaveButton();
    ~SaveButton();
};


/**
  A button that displays text.

  @see GenericEditor
*/
class PLUGIN_API UtilityButton : public Button
{
public:
    UtilityButton (String label_, Font font_);
    ~UtilityButton();

    void setCorners(bool UL, bool UR, bool LL, bool LR);
    void setRadius(float r);

    void setEnabledState (bool);
    bool getEnabledState();

    void setLabel (String label);
    String getLabel();


private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    String label;
    Font font;
    bool roundUL, roundUR, roundLL, roundLR;
    float radius;
    ColourGradient selectedGrad, selectedOverGrad, neutralGrad, neutralOverGrad;
    Colour fontColor;
    Path outlinePath;

    bool isEnabled;

    void resized() override;;
};


class PLUGIN_API ColorButton : public Button
{
public:
    ColorButton (String label_, Font font_);
    ~ColorButton();

    void setEnabledState (bool);
    bool getEnabledState();

    void setColors (Colour foreground, Colour background);
    void setLabel (String label);
    String getLabel();

    void setVerticalOrientation (bool state);
    void setUserDefinedData (int d);
    int getUserDefinedData();

    void setShowEnabled (bool state);


private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    int userDefinedData;
    bool vert;
    String label;
    Font font;
    Colour fontColor, backgroundColor;
    bool showEnabledStatus;
    bool isEnabled;
};


/**
  Used to change the spike detection threshold.

  @see SpikeDetectorEditor
*/
class PLUGIN_API ThresholdSlider : public Slider
{
public:
    ThresholdSlider (Font f);
    ~ThresholdSlider();

    void setActive (bool);

    void setValues (Array<double>);


private:
    void paint (Graphics& g) override;

    Path makeRotaryPath (double, double, double);

    Font font;

    bool isActive;

    Array<double> valueArray;
};


#endif  // __GENERICEDITOR_H_DD406E71__
