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

#include "LfpDisplayNode.h"

#include <stdio.h>

#define MS_FROM_START Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - start) * 1000


using namespace LfpViewer;


LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor  ("LFP Viewer")
{
    for (int displayIndex = 0; displayIndex <= 3; displayIndex++)
    {
        triggerChannels.add(-1);
        latestTrigger.add(-1);
        latestCurrentTrigger.add(-1);
    }
    
}


AudioProcessorEditor* LfpDisplayNode::createEditor()
{
    editor = std::make_unique<LfpDisplayEditor> (this);
    return editor.get();
}


void LfpDisplayNode::initialize(bool signalChainIsLoading)
{
    if (!signalChainIsLoading)
    {
        LfpDisplayEditor* editor = (LfpDisplayEditor*)getEditor();
        editor->initialize(signalChainIsLoading);
    }
}


void LfpDisplayNode::updateSettings()
{

    LOGD("Setting num inputs on LfpDisplayNode to ", getNumInputs());

    int64 start = Time::getHighResolutionTicks();

    for (auto displayBuffer : displayBuffers)
    {
        displayBuffer->prepareToUpdate();
    }

    for (int ch = 0; ch < getNumInputs(); ch++)
    {
        const ContinuousChannel* channel = continuousChannels[ch];

        uint16 streamId = channel->getStreamId();

        if (displayBufferMap.count(streamId) == 0)
        {
            String name = channel->getStreamName();

            displayBuffers.add(new DisplayBuffer(streamId, name, channel->getSampleRate()));
            displayBufferMap[streamId] = displayBuffers.getLast();

        }
        else {
            displayBufferMap[streamId]->sampleRate = channel->getSampleRate();
        }

        displayBufferMap[streamId]->addChannel(channel->getName(), // name
                                               ch, // index
                                               channel->getChannelType(), // type
                                               0, // group 
                                               channel->position.y // ypos
                                                );
    }

    Array<DisplayBuffer*> toDelete;
    
    for (auto displayBuffer : displayBuffers)
    {

        if (displayBuffer->isNeeded)
        {
            displayBuffer->update();
        }
        else {

            displayBufferMap.erase(displayBuffer->id);
            toDelete.add(displayBuffer);

            for (auto splitID : displayBuffer->displays)
            {
                LfpDisplayEditor* ed = (LfpDisplayEditor*)getEditor();
                ed->removeBufferForDisplay(splitID);
            }
        }
        
    }

    for (auto displayBuffer : toDelete)
    {
        displayBuffers.removeObject(displayBuffer, true);
    }

    LOGDD("    Finished creating buffers in ", MS_FROM_START, " milliseconds");

}

void LfpDisplayNode::setSplitDisplays(Array<LfpDisplaySplitter*> splits)
{
    splitDisplays = splits;
}

uint16 LfpDisplayNode::getEventSourceId(const EventChannel* event)
{
    return event->getStreamId();
}

uint16 LfpDisplayNode::getChannelSourceId(const ChannelInfoObject* chan)
{
    return chan->getStreamId();
}

Array<DisplayBuffer*> LfpDisplayNode::getDisplayBuffers()
{
    Array<DisplayBuffer*> buffers;

    for (auto displayBuffer : displayBuffers)
    {
        if (displayBuffer->numChannels > 0)
            buffers.add(displayBuffer);
    }

    return buffers;
}


bool LfpDisplayNode::startAcquisition()
{

    LfpDisplayEditor* editor = (LfpDisplayEditor*)getEditor();
    editor->enable();

    return true;

}


bool LfpDisplayNode::stopAcquisition()
{

    LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
    editor->disable();

    return true;

}


void LfpDisplayNode::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex < 99)
    {
        triggerChannels.set(int(newValue), parameterIndex);
    }
    else {


        ContinuousChannel* chan = continuousChannels[int(newValue)];

        String msg = "AUDIO SELECT ";
        msg += String(chan->getStreamId()) + " ";
        msg += String(chan->getLocalIndex() + 1) + " ";

        broadcastMessage(msg);

    }
}


void LfpDisplayNode::handleTTLEvent(TTLEventPtr event)
{
 
    const int eventId = event->getState() ? 1 : 0;
    const int eventChannel = event->getLine();
    const uint16 eventStreamId = event->getChannelInfo()->getStreamId();
    const int eventSourceNodeId = event->getChannelInfo()->getSourceNodeId();
    const int eventTime = event->getTimestamp() - getSourceTimestamp(eventStreamId);

    if (eventId == 1)
    {
        for (int i = 0; i < 3; i++)

        {
            if (triggerChannels[i] == eventChannel)
            {
                // if an event came in on the trigger channel
                //std::cout << "Setting latest current trigger to " << eventTime << std::endl;
                latestCurrentTrigger.set(i, eventTime);

            }
        }
    }

    if (displayBufferMap.count(eventStreamId))
    {
        displayBufferMap[eventStreamId]->addEvent(eventTime, eventChannel, eventId,
            getNumSourceSamples(eventStreamId)
        );
    }

    else {

        for (auto displayBuffer : displayBuffers)
        {
            displayBuffer->addEvent(eventTime, eventChannel, eventId,
                getNumSourceSamples(displayBuffer->id)
            );
                
        }

    }

}


void LfpDisplayNode::initializeEventChannels()
{
    latestCurrentTrigger.insertMultiple(0, -1, 3); // reset to -1

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSourceSamples(displayBuffer->id);
        displayBuffer->initializeEventChannel(numSamples);
    }
}

void LfpDisplayNode::finalizeEventChannels()
{
    for (int i = 0; i < 3; i++)
    {
        if (latestTrigger[i] == -1 && latestCurrentTrigger[i] > -1) // received a trigger, but not yet acknowledged
        {
            int triggerSample = latestCurrentTrigger[i] + splitDisplays[i]->displayBuffer->displayBufferIndices.getLast();
            //std::cout << "Setting latest trigger to " << triggerSample << std::endl;
            latestTrigger.set(i, triggerSample);
        }
    }

    for (auto displayBuffer : displayBuffers)
    {
        int numSamples = getNumSourceSamples(displayBuffer->id);
        displayBuffer->finalizeEventChannel(numSamples);
    }

}

void LfpDisplayNode::process (AudioBuffer<float>& buffer)
{

    initializeEventChannels();
    checkForEvents();
    finalizeEventChannels();

    for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
    {
        uint16 subProcId = getChannelSourceId(continuousChannels[chan]);

        const int nSamples = getNumSamples(chan);

        displayBufferMap[subProcId]->addData(buffer, chan, nSamples);
    }
}

int64 LfpDisplayNode::getLatestTriggerTime(int id) const
{
    return latestTrigger[id];
}

void LfpDisplayNode::acknowledgeTrigger(int id)
{
    latestTrigger.set(id, -1);
}
