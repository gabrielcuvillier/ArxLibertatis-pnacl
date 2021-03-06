/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "audio/Audio.h"

#include "Configure.h"

#include "audio/AudioResource.h"
#include "audio/Mixer.h"
#include "audio/Sample.h"
#include "audio/Ambiance.h"
#include "audio/AudioGlobal.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"
#include "audio/AudioEnvironment.h"
#if ARX_HAVE_OPENAL
	#include "audio/openal/OpenALBackend.h"
#endif

#include "io/log/Logger.h"

#include "platform/Lock.h"
#include "platform/Time.h"
#include "platform/profiler/Profiler.h"

namespace audio {

namespace {
static Lock * mutex = NULL;
}

aalError init(const std::string & backendName, const std::string & deviceName) {
	
	// Clean any initialized data
	clean();
	
	LogDebug("Init");
	
	stream_limit_bytes = DEFAULT_STREAMLIMIT;
	
	bool autoBackend = (backendName == "auto");
	aalError error = AAL_ERROR_INIT;
	
	for(int i = 0; i < 2 && !backend; i++) {
		bool first = (i == 0);
		
		bool matched = false;
		
		#if ARX_HAVE_OPENAL
		if(!backend && first == (autoBackend || backendName == "OpenAL")) {
			matched = true;
			LogDebug("initializing OpenAL backend");
			OpenALBackend * _backend = new OpenALBackend();
			if(!deviceName.empty() && deviceName != "auto") {
				error = _backend->init(deviceName.c_str());
				if(error) {
					LogWarning << "Could not open device \"" << deviceName
					           << "\", retrying with default device";
				}
			}
			if(error) {
				error = _backend->init();
			}
			if(!error) {
				backend = _backend;
			} else {
				delete _backend;
			}
		}
		#endif
		
		if(first && !matched) {
			LogError << "Unknown backend: " << backendName;
		}
	}
	
	#if !ARX_HAVE_OPENAL
	ARX_UNUSED(autoBackend);
	#endif
	
	if(!backend) {
		LogError << "No working backend available";
		return error;
	}
	
	mutex = new Lock();
	
	session_time = platform::getTimeMs();
	
	return AAL_OK;
}

aalError clean() {
	
	if(!backend) {
		return AAL_OK;
	}
	
	LogDebug("Clean");
	
	_amb.clear();
	_sample.clear();
	_mixer.clear();
	_env.clear();
	
	delete backend, backend = NULL;
	
	sample_path.clear();
	ambiance_path.clear();
	environment_path.clear();
	
	delete mutex, mutex = NULL;
	
	return AAL_OK;
}

#define AAL_ENTRY \
	if(!backend) { \
		return AAL_ERROR_INIT; \
	} \
	Autolock lock(mutex);

#define AAL_ENTRY_V(value) \
	if(!backend) { \
		return (value); \
	} \
	Autolock lock(mutex);

std::vector<std::string> getDevices() {
	
	AAL_ENTRY_V(std::vector<std::string>())
	
	return backend->getDevices();
}

aalError setStreamLimit(size_t limit) {
	
	AAL_ENTRY
	
	stream_limit_bytes = limit;
	
	return AAL_OK;
}

aalError setSamplePath(const res::path & path) {
	
	AAL_ENTRY
	
	sample_path = path;
	
	return AAL_OK;
}

aalError setAmbiancePath(const res::path & path) {
	
	AAL_ENTRY
	
	ambiance_path = path;
	
	return AAL_OK;
}

aalError setEnvironmentPath(const res::path & path) {
	
	AAL_ENTRY
	
	environment_path = path;
	
	return AAL_OK;
}

aalError setReverbEnabled(bool enable) {
	
	AAL_ENTRY
	
	return backend->setReverbEnabled(enable);
}

bool isReverbSupported() {
	
	AAL_ENTRY_V(false)
	
	return backend->isReverbSupported();
}

aalError update() {
	
	ARX_PROFILE_FUNC();
	
	AAL_ENTRY
	
	session_time = platform::getTimeMs();
	
	// Update sources
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd();) {
		Source * source = *p;
		if(source && (source->update(), source->isIdle())) {
			p = backend->deleteSource(p);
		} else {
			++p;
		}
	}
	
	// Update ambiances
	for(size_t i = 0; i < _amb.size(); i++) {
		Ambiance * ambiance = _amb[i];
		if(ambiance) {
			ambiance->update();
			if(ambiance->getChannel().flags & FLAG_AUTOFREE && ambiance->isIdle()) {
				_amb.remove(i);
			}
		}
	}
	
	// Update samples
	for(size_t i = 0; i < _sample.size(); i++) {
		Sample * sample = _sample[i];
		if(sample && sample->isReferenced() < 1) {
			_sample.remove(i);
		}
	}
	
	return AAL_OK;
}

// Resource creation

MixerId createMixer() {
	
	AAL_ENTRY_V(MixerId())
	
	Mixer * mixer = new Mixer();
	
	MixerId id = MixerId(_mixer.add(mixer));
	if(id == MixerId()) {
		delete mixer;
	}
	
	return id;
}

SampleId createSample(const res::path & name) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	Sample * sample = new Sample(name);
	
	SampleId s_id = INVALID_ID;
	if(sample->load() || (s_id = _sample.add(sample)) == INVALID_ID) {
		delete sample;
	} else {
		sample->reference();
	}
	
	return Backend::clearSource(s_id);
}

AmbianceId createAmbiance(const res::path & name) {
	
	AAL_ENTRY_V(AmbianceId())
	
	Ambiance * ambiance = new Ambiance(name);
	AmbianceId a_id = AmbianceId();
	if(ambiance->load() || (a_id = AmbianceId(_amb.add(ambiance))) == AmbianceId()) {
		delete ambiance;
		LogError << "Ambiance " << name << " not found";
	}
	
	return a_id;
}

EnvId createEnvironment(const res::path & name) {
	
	AAL_ENTRY_V(EnvId())
	
	Environment * env = new Environment(name);
	EnvId e_id = EnvId();
	if(env->load() || (e_id = EnvId(_env.add(env))) == EnvId()) {
		delete env;
		LogError << "Environment " << name << " not found";
	}
	
	return e_id;
}

// Resource destruction

aalError deleteSample(SampleId sample_id) {
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	if(!_sample.isValid(s_id)) {
		return AAL_ERROR_HANDLE;
	}
	
	_sample.remove(s_id);
	
	return AAL_OK;
}

aalError deleteAmbiance(AmbianceId a_id) {
	
	AAL_ENTRY
	
	_amb.remove(a_id.handleData());
	
	return AAL_OK;
}

AmbianceId getAmbiance(const res::path & name) {
	
	AAL_ENTRY_V(AmbianceId())
	
	for(size_t i = 0; i < _amb.size(); i++) {
		if(_amb[i] && name == _amb[i]->getName()) {
			return AmbianceId(i);
		}
	}
	
	return AmbianceId();
}

EnvId getEnvironment(const res::path & name) {
	
	AAL_ENTRY_V(EnvId())
	
	for(size_t i = 0; i < _env.size(); i++) {
		if(_env[i] && name == _env[i]->name) {
			return EnvId(i);
		}
	}
	
	return EnvId();
}

// Retrieve next resource by ID

AmbianceId getNextAmbiance(AmbianceId ambiance_id) {
	
	AAL_ENTRY_V(AmbianceId())
	
	size_t i = _amb.isValid(ambiance_id.handleData()) ? ambiance_id.handleData() + 1 : 0;
	
	for(; i < _amb.size(); i++) {
		if(_amb[i]) {
			return AmbianceId(i);
		}
	}
	
	return AmbianceId();
}

// Listener settings

aalError setUnitFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug("SetUnitFactor " << factor);
	
	return backend->setUnitFactor(factor);
}

aalError setRolloffFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug("SetRolloffFactor " << factor);
	
	return backend->setRolloffFactor(factor);
}

aalError setListenerPosition(const Vec3f & position) {
	
	AAL_ENTRY
	
	return backend->setListenerPosition(position);
}

aalError setListenerDirection(const Vec3f & front, const Vec3f & up) {
	
	AAL_ENTRY
	
	return backend->setListenerOrientation(front, up);
}

aalError setListenerEnvironment(EnvId e_id) {
	
	AAL_ENTRY
	
	if(!_env.isValid(e_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetListenerEnvironment " << _env[e_id.handleData()]->name);
	
	return backend->setListenerEnvironment(*_env[e_id.handleData()]);
}

// Mixer setup

aalError setMixerVolume(MixerId m_id, float volume) {
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetMixerVolume " << m_id.handleData() << " volume=" << volume);
	
	return _mixer[m_id.handleData()]->setVolume(volume);
}

aalError setMixerParent(MixerId m_id, MixerId pm_id) {
	
	AAL_ENTRY
	
	if(m_id == pm_id || !_mixer.isValid(m_id.handleData()) || !_mixer.isValid(pm_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetMixerParent " << m_id.handleData() << " parent=" << pm_id.handleData());
	
	return _mixer[m_id.handleData()]->setParent(_mixer[pm_id.handleData()]);
}

// Mixer control 

aalError mixerStop(MixerId m_id) {
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("MixerStop " << m_id.handleData());
	
	return _mixer[m_id.handleData()]->stop();
}

aalError mixerPause(MixerId m_id) {
	
	AAL_ENTRY;
	
	if(!_mixer.isValid(m_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("MixerPause " << m_id.handleData());
	
	return _mixer[m_id.handleData()]->pause();
}

aalError mixerResume(MixerId m_id) {
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("MixerResume " << m_id.handleData());
	
	return _mixer[m_id.handleData()]->resume();
}

// Sample setup

aalError setSampleVolume(SourceId sample_id, float volume) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	return source->setVolume(volume);
}

aalError setSamplePitch(SourceId sample_id, float pitch) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	return source->setPitch(pitch);
}

aalError setSamplePosition(SourceId sample_id, const Vec3f & position) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	return source->setPosition(position);
}

// Sample status

aalError getSampleName(SampleId sample_id, res::path & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	if(!_sample.isValid(s_id)) {
		return AAL_ERROR_HANDLE;
	}
	
	name = _sample[s_id]->getName();
	
	return AAL_OK;
}

aalError getSampleLength(SampleId sample_id, size_t & length, TimeUnit unit) {
	
	length = 0;
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	if(!_sample.isValid(s_id)) {
		return AAL_ERROR_HANDLE;
	}
	
	Sample * sample = _sample[s_id];
	length = bytesToUnits(sample->getLength(), sample->getFormat(), unit);
	
	return AAL_OK;
}

bool isSamplePlaying(SourceId sample_id) {
	
	AAL_ENTRY_V(false)
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return false;
	}
	
	bool ret = source->isPlaying();
	
	return ret;
}

// Sample control

aalError samplePlay(SampleId & sample_id, const Channel & channel, unsigned play_count) {
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	sample_id = Backend::clearSource(sample_id);
	if(!_sample.isValid(s_id) || !_mixer.isValid(channel.mixer.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SamplePlay " << _sample[s_id]->getName() << " play_count=" << play_count);
	
	Source * source = backend->getSource(sample_id);
	if(source) {
		if(channel.flags == source->getChannel().flags) {
			source = NULL;
		} else if(channel.flags & FLAG_RESTART) {
			source->stop();
		} else if(channel.flags & FLAG_ENQUEUE) {
			source->play(play_count);
		} else if(source->isIdle()) {
			source->setMixer(channel.mixer);
			source->setVolume(channel.volume);
			source->setPitch(channel.pitch);
			source->setPan(channel.pan);
			source->setPosition(channel.position);
			source->setVelocity(channel.velocity);
			source->setDirection(channel.direction);
			source->setCone(channel.cone);
			source->setFalloff(channel.falloff);
		} else {
			source = NULL;
		}
	}
	
	if(!source) {
		source = backend->createSource(s_id, channel);
		if(!source) {
			return AAL_ERROR_SYSTEM;
		}
	}
	
	if(aalError error = source->play(play_count)) {
		return error;
	}
	
	sample_id = source->getId();
	
	if(channel.flags & FLAG_AUTOFREE) {
		_sample[s_id]->dereference();
	}
	
	return AAL_OK;
}

aalError sampleStop(SourceId & sample_id) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SampleStop " << source->getSample()->getName());
	
	sample_id = Backend::clearSource(sample_id);
	
	return source->stop();
}

// Ambiance setup

aalError setAmbianceUserData(AmbianceId a_id, void * data) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetAmbianceUserData " << _amb[a_id.handleData()]->getName() << " " << data);
	
	_amb[a_id.handleData()]->setUserData(data);
	
	return AAL_OK;
}

aalError setAmbianceVolume(AmbianceId a_id, float volume) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetAmbianceVolume " << _amb[a_id.handleData()]->getName() << " " << volume);
	
	return _amb[a_id.handleData()]->setVolume(volume);
}

// Ambiance status

aalError getAmbianceName(AmbianceId a_id, res::path & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	name = _amb[a_id.handleData()]->getName();
	
	return AAL_OK;
}

aalError getAmbianceUserData(AmbianceId a_id, void ** data) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	*data = _amb[a_id.handleData()]->getUserData();
	
	return AAL_OK;
}

aalError getAmbianceVolume(AmbianceId a_id, float & _volume) {
	
	_volume = DEFAULT_VOLUME;
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	if(!(_amb[a_id.handleData()]->getChannel().flags & FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	_volume = _amb[a_id.handleData()]->getChannel().volume;
	
	return AAL_OK;
}

bool isAmbianceLooped(AmbianceId a_id) {
	
	AAL_ENTRY_V(false)
	
	if(!_amb.isValid(a_id.handleData())) {
		return false;
	}
	
	return _amb[a_id.handleData()]->isLooped();
}

// Ambiance control

aalError ambiancePlay(AmbianceId a_id, const Channel & channel, bool loop, size_t fade_interval) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData()) || !_mixer.isValid(channel.mixer.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("AmbiancePlay " << _amb[a_id.handleData()]->getName() << " loop=" << loop << " fade=" << fade_interval);
	
	return _amb[a_id.handleData()]->play(channel, loop, fade_interval);
}

aalError ambianceStop(AmbianceId a_id, size_t fade_interval) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id.handleData())) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("AmbianceStop " << _amb[a_id.handleData()]->getName() << " " << fade_interval);
	
	_amb[a_id.handleData()]->stop(fade_interval);
	
	return AAL_OK;
}

} // namespace audio
