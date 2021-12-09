/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 /Users/franman/Documents/Programming/GitHub/plugin-GUI/Plugins/BasicSpikeDisplay/SpikeDisplayNode/SpikeDisplayCanvas.h

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SPIKEDISPLAY_H_
#define SPIKEDISPLAY_H_

#include <VisualizerWindowHeaders.h>

#include "SpikeDisplayCanvas.h"

class SpikeDisplay;
class SpikePlot;

class SpikeThresholdCoordinator;

/** 
    Component that holds multiple spike plots

    @see SpikeDisplayCanvas
*/
class SpikeDisplay : public Component
{
public:

    /** Constructor*/
    SpikeDisplay(SpikeDisplayCanvas*, Viewport*);

    /** Destructor */
    ~SpikeDisplay() { }

    /** Removes spike plots*/
    void removePlots();

    /** Clears spike plots*/
    void clear();

    /** Adds a spike plot with 1, 2, or 3 channels*/
    SpikePlot* addSpikePlot(int numChannels, int electrodeNum, String name);

    /** Returns a spike plot based on index*/
    SpikePlot* getSpikePlot(int index);

    /** Renders gray background */
    void paint(Graphics& g);

    /** Sets the locations of available spike plots*/
    void resized();

    /** Sends a spike to a given spike plot*/
    void plotSpike(const Spike* spike, int electrodeNum);

    /** Tells available plots to reverse spike direction*/
    void invertSpikes(bool);

    /** Returns the total height of the component*/
    int getTotalHeight() { return totalHeight; }

    /** Returns the total number of plots*/
    int getNumPlots();

    /** Returns the total number of channels for a given plot*/
    int getNumChannelsForPlot(int plotNum);

    /** Returns the threshold for a given plot*/
    float getThresholdForWaveAxis(int plotNum, int axisNum);

    /** Sets the threshold for a given plot*/
    void setThresholdForWaveAxis(int plotNum, int axisNum, float threshold);

    /** Returns the range for a given plot*/
    float getRangeForWaveAxis(int plotNum, int axisNum);

    /** Sets the range for a given plot*/
    void setRangeForWaveAxis(int plotNum, int axisNum, float range);

    /** Allows the threshold coordinator to change thresholds synchronously*/
    void registerThresholdCoordinator(SpikeThresholdCoordinator* stc);

private:

    int totalHeight;

    SpikeDisplayCanvas* canvas;
    Viewport* viewport;

    OwnedArray<SpikePlot> spikePlots;

    bool shouldInvert;

    SpikeThresholdCoordinator* thresholdCoordinator;

};

/** 
    Allows spike plot thresholds to be adjusted synchronously
*/
class SpikeThresholdCoordinator
{
public:

    /** Constructor*/
    SpikeThresholdCoordinator();

    /** Destructor*/
    ~SpikeThresholdCoordinator();

    /** Registers a plot to interact with this coordinator*/
    void registerSpikePlot(SpikePlot* sp);

    /** De-registers a plot to interact with this coordinators*/
    void deregisterSpikePlot(SpikePlot* sp);

    /** Sets the lock threshold state*/
    void setLockThresholds(bool en);

    /** Returns the lock threshold state*/
    bool getLockThresholds();

    /** Sets the thresholds of all registered plots*/
    void thresholdChanged(float displayThreshold, float range);

private:

    bool lockThresholds;
    Array<SpikePlot*> registeredPlots;

    WeakReference<SpikeThresholdCoordinator>::Master masterReference;
    friend class WeakReference<SpikeThresholdCoordinator>;

};


#endif  // SPIKEDISPLAYCANVAS_H_