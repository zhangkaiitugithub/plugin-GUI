/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2018 Open Ephys

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

#include "BinaryFileSource.h"

using namespace BinarySource;

BinaryFileSource::BinaryFileSource() 
	: m_samplePos(0), 
	  hasEventData(false), 
	  loopCount(0)
{

}

bool BinaryFileSource::open(File file)
{
	m_jsonData = JSON::parse(file);
	if (m_jsonData.isVoid())
	{
		std::cout << "Invalid JSON." << std::endl;
		return false;
	}
		

	if (m_jsonData["GUI version"].isVoid())
	{
		std::cout << "No GUI version." << std::endl;
		return false;
	}
		
	
	var cont = m_jsonData["continuous"];
	if (cont.isVoid() || cont.size() <= 0)
	{
		std::cout << "No continuous data found." << std::endl;
		return false;
	}

	var event = m_jsonData["events"];
	if (!(event.isVoid() || event.size() <= 0))
	{
		hasEventData = true;
	}

	m_rootPath = file.getParentDirectory();

	return true;
}

void BinaryFileSource::fillRecordInfo()
{

	const int maxSensibleFileSize = 2 * 1024 * 1024; 

	var continuousData = m_jsonData["continuous"];

	//create identifiers to speed up stuff
	Identifier idFolder("folder_name");
	Identifier idSampleRate("sample_rate");
	Identifier idNumChannels("num_channels");
	Identifier idChannels("channels");
	Identifier idChannelName("channel_name");
	Identifier idBitVolts("bit_volts");

	int numProcessors = continuousData.size();

	for (int i = 0; i < numProcessors; i++)
	{
		var record = continuousData[i];
		if (record.isVoid()) continue;

		var channels = record[idChannels];
		if (channels.isVoid() || channels.size() <= 0) continue;

		RecordInfo info;

		String folderName = record[idFolder];
		folderName = folderName.trimCharactersAtEnd("/");

		File dataFile = m_rootPath.getChildFile("continuous").getChildFile(folderName).getChildFile("continuous.dat");
		if (!dataFile.existsAsFile()) continue;

		int numChannels = record[idNumChannels];
		int64 numSamples = (dataFile.getSize() / numChannels) / sizeof(int16);

		info.name = folderName;
		info.sampleRate = record[idSampleRate];
		info.numSamples = numSamples;
		
		//Get start timestsamp to align any associated events with
		File tsFile = m_rootPath.getChildFile("continuous").getChildFile(folderName).getChildFile("timestamps.npy");

		if (tsFile.exists())
		{
			std::unique_ptr<FileInputStream> tsDataStream = tsFile.createInputStream();
			MemoryBlock tsData;
			if (!(tsDataStream->readIntoMemoryBlock(tsData, maxSensibleFileSize))) continue;
			int64* startTimestamp = (int64*)tsData.getData() + EVENT_HEADER_SIZE_IN_BYTES;
			info.startTimestamp = startTimestamp[0];
		}
		else {
			info.startTimestamp = 0;
		}
		

		for (int c = 0; c < numChannels; c++)
		{
			var chan = channels[c];
			RecordedChannelInfo cInfo;

			cInfo.name = chan[idChannelName];
			cInfo.bitVolts = chan[idBitVolts];
			
			info.channels.add(cInfo);
		}
		
		infoArray.add(info);
		numRecords++;	

		m_dataFileArray.add(dataFile);
		
	}

	if (hasEventData)
	{

		var eventData = m_jsonData["events"];
		
		/* Create identifiers for efficiency */
		Identifier idFolder("folder_name");
		Identifier idChannelName("channel_name");
		Identifier idDescription("description");
		Identifier idIdentifier("identifier");
		Identifier idSampleRate("sample_rate");
		Identifier idType("type");
		Identifier idNumChannels("num_channels");
		Identifier idChannels("channels");
		Identifier idSourceProcessor("source_processor");

		int numEventProcessors = eventData.size();

		for (int i = 0; i < numEventProcessors; i++) 
		{

			var events = eventData[i];

			String folderName = events[idFolder];
			folderName = folderName.trimCharactersAtEnd("/");

			File channelStatesFile = m_rootPath.getChildFile("events").getChildFile(folderName).getChildFile("states.npy");
			std::unique_ptr<MemoryMappedFile> channelStatesFileMap(new MemoryMappedFile(channelStatesFile, MemoryMappedFile::readOnly)); 

			File timestampsFile = m_rootPath.getChildFile("events").getChildFile(folderName).getChildFile("samples.npy");
			std::unique_ptr<MemoryMappedFile> timestampsFileMap(new MemoryMappedFile(timestampsFile, MemoryMappedFile::readOnly));

			int channelFileSize = channelStatesFile.getSize();

			if (!channelFileSize) continue;

			int nEvents = (channelFileSize - EVENT_HEADER_SIZE_IN_BYTES) / BYTES_PER_EVENT; 
		
			EventInfo eventInfo;

			for (int i = 0; i < nEvents; i++)
			{
				int16* data = static_cast<int16*>(channelStatesFileMap->getData()) + (EVENT_HEADER_SIZE_IN_BYTES / 2) + i*sizeof(int16) / 2;
				eventInfo.channels.push_back(abs(*data));

				eventInfo.channelStates.push_back(*data > 0);

				LOGC("Got channel: ", *data, " got state: ", *data > 0);

				int64* tsData = static_cast<int64*>(timestampsFileMap->getData()) + (EVENT_HEADER_SIZE_IN_BYTES / 8) + i*sizeof(int64) / 8;
				eventInfo.timestamps.push_back(*tsData - infoArray[0].startTimestamp); //FIXME: Offset should be dynamic

			}

			if (nEvents)
				eventInfoArray.add(eventInfo);

		}
	}

}

void BinaryFileSource::processEventData(EventInfo &eventInfo, int64 start, int64 stop)
{

	int local_start = start % getActiveNumSamples();;
	int local_stop = stop % getActiveNumSamples();
	int loop_count = start / getActiveNumSamples();

	for (auto info : eventInfoArray)
	{
		int i = 0;
		
		while (i < info.timestamps.size())
		{
			if (info.timestamps[i] >= local_start && info.timestamps[i] <= local_stop)
			{
				eventInfo.channels.push_back(info.channels[i] - 1);
				eventInfo.channelStates.push_back((info.channelStates[i]));
				eventInfo.timestamps.push_back(info.timestamps[i] + loop_count*getActiveNumSamples());
			}
			i++;
		}
	}


}

void BinaryFileSource::updateActiveRecord(int index)
{
    m_dataFile.reset();
	m_dataFile = std::make_unique<MemoryMappedFile>(m_dataFileArray[index], MemoryMappedFile::readOnly);
	m_samplePos = 0;
	numActiveChannels = getActiveNumChannels();

	bitVolts.clear();

	for (int i = 0; i < numActiveChannels; i++)
		bitVolts.add(getChannelInfo(index, i).bitVolts);
}

void BinaryFileSource::seekTo(int64 sample)
{
	m_samplePos = sample % getActiveNumSamples();
}

int BinaryFileSource::readData(int16* buffer, int nSamples)
{
	int64 samplesToRead;

	if (m_samplePos + nSamples > getActiveNumSamples())
	{
		samplesToRead = getActiveNumSamples() - m_samplePos;
	}
	else
	{
		samplesToRead = nSamples;
	}

	int16* data = static_cast<int16*>(m_dataFile->getData()) + (m_samplePos * numActiveChannels);

	//FIXME: Can crash here (heap overflow?), secondary either to wrong index or scrubbing too fast? Not sure yet. 
	memcpy(buffer, data, samplesToRead * numActiveChannels * sizeof(int16));
    m_samplePos += samplesToRead;
	return samplesToRead;
}

void BinaryFileSource::processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples)
{

	for (int i = 0; i < numSamples; i++)
	{
		*(outBuffer + i) = *(inBuffer + (numActiveChannels * i) + channel) * bitVolts[channel];
	}
}
