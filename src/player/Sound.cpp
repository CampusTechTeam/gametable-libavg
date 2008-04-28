//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2006 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//
#include "Sound.h"
#include "Player.h"
#include "NodeDefinition.h"

#include "../base/Exception.h"
#include "../base/Logger.h"
#include "../base/ScopeTimer.h"
#include "../base/XMLHelper.h"

#include "../audio/AudioEngine.h"

#include "../video/AsyncVideoDecoder.h"
#include "../video/FFMpegDecoder.h"

#include <iostream>
#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace boost::python;
using namespace std;

namespace avg {

NodeDefinition Sound::getNodeDefinition()
{
    return NodeDefinition("sound", Node::buildNode<Sound>)
        .extendDefinition(Node::getNodeDefinition())
        .addArg(Arg<string>("href", "", false, offsetof(Sound, m_href)))
        .addArg(Arg<bool>("loop", false, false, offsetof(Sound, m_bLoop)))
        ;
}

Sound::Sound (const ArgList& Args, Player * pPlayer)
    : Node(pPlayer),
      m_Filename(""),
      m_pEOFCallback(0),
      m_bAudioEnabled(true),
      m_pDecoder(0),
      m_Volume(1.0)
{
    Args.setMembers(this);
    m_Filename = m_href;
    if (m_Filename != "") {
        initFilename(getPlayer(), m_Filename);
    }
    VideoDecoderPtr pSyncDecoder(new FFMpegDecoder());
    m_pDecoder = new AsyncVideoDecoder(pSyncDecoder);

    getPlayer()->registerFrameListener(this);
}

Sound::~Sound ()
{
    getPlayer()->unregisterFrameListener(this);
    if (m_pDecoder) {
        delete m_pDecoder;
        m_pDecoder = 0;
    }
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
}

long long Sound::getDuration() const
{
    if (m_State != Unloaded) {
        return m_pDecoder->getDuration();
    } else {
        AVG_TRACE(Logger::WARNING,
               "Error in Sound::getDuration: Sound not loaded.");
        return -1;
    }
}

long long Sound::getCurTime() const
{
    if (m_State != Unloaded) {
        return m_pDecoder->getCurTime();
    } else {
        AVG_TRACE(Logger::WARNING,
                "Error in Sound::GetCurTime: Sound not loaded.");
        return -1;
    }
}

void Sound::seekToTime(long long Time)
{
    if (m_State != Unloaded) {
        seek(Time);
    } else {
        AVG_TRACE(Logger::WARNING,
                "Error in Sound::SeekToTime: Sound "+getID()+" not loaded.");
    }
}

bool Sound::getLoop() const
{
    return m_bLoop;
}

void Sound::setEOFCallback(PyObject * pEOFCallback)
{
    if (m_pEOFCallback) {
        Py_DECREF(m_pEOFCallback);
    }
    Py_INCREF(pEOFCallback);
    m_pEOFCallback = pEOFCallback;
}

void Sound::setRenderingEngines(DisplayEngine * pDisplayEngine, AudioEngine * pAudioEngine)
{
    checkReload();
    Node::setRenderingEngines(pDisplayEngine, pAudioEngine);
}

void Sound::disconnect()
{
    changeSoundState(Unloaded);
    Node::disconnect();
}

void Sound::play()
{
    changeSoundState(Playing);
}

void Sound::stop()
{
    changeSoundState(Unloaded);
}

void Sound::pause()
{
    changeSoundState(Paused);
}

const string& Sound::getHRef() const
{
    return m_Filename;
}

void Sound::setHRef(const string& href)
{
    m_href = href;
    checkReload();
}

void Sound::setVolume(double Volume)
{
    m_Volume = Volume;
    if (m_pDecoder) {
        m_pDecoder->setVolume(Volume);
    }
}

void Sound::checkReload()
{
    string fileName (m_href);
    if (m_href != "") {
        initFilename(getPlayer(), fileName);
        if (fileName != m_Filename) {
            changeSoundState(Unloaded);
            m_Filename = fileName;
            changeSoundState(Paused);
        }
    } else {
        changeSoundState(Unloaded);
        m_Filename = "";
    }
}

string Sound::getTypeStr ()
{
    return "Sound";
}

void Sound::onFrameEnd()
{
    if (m_pDecoder->isEOF(SS_AUDIO)) {
        onEOF();
    }
}

void Sound::setAudioEnabled(bool bEnabled)
{
    m_bAudioEnabled = bEnabled;
    if (m_pDecoder) {
        m_pDecoder->setAudioEnabled(m_bAudioEnabled);
    }
}

int Sound::fillAudioBuffer(AudioBufferPtr pBuffer)
{
    if (m_State == Playing) {
        return m_pDecoder->fillAudioBuffer(pBuffer);
    } else {
        return 0;
    }
}

void Sound::changeSoundState(SoundState NewSoundState)
{
    if (isDisplayAvailable()) {
        long long CurTime = getPlayer()->getFrameTime(); 
        if (NewSoundState != m_State) {
            if (m_State == Unloaded) {
                m_StartTime = CurTime;
                m_PauseTime = 0;
                open();
            }
            if (NewSoundState == Paused) {
                m_PauseStartTime = CurTime;
            } else if (NewSoundState == Playing && m_State == Paused) {
                m_PauseTime += (CurTime-m_PauseStartTime
                        - (long long)(1000.0/m_pDecoder->getFPS()));
            } else if (NewSoundState == Unloaded) {
                close();
            }
        }
    }
    m_State = NewSoundState;
}

void Sound::seek(long long DestTime) 
{
    m_pDecoder->seek(DestTime);
    m_StartTime = getPlayer()->getFrameTime() - DestTime;
    m_PauseTime = 0;
    m_PauseStartTime = getPlayer()->getFrameTime();
}

void Sound::open()
{
    m_pDecoder->open(m_Filename, getAudioEngine()->getParams(), OGL_NONE, true);
    m_pDecoder->setAudioEnabled(m_bAudioEnabled);
    m_pDecoder->setVolume(m_Volume);
    if (getAudioEngine()) {
        getAudioEngine()->addSource(this);
    }
}

void Sound::close()
{
    if (getAudioEngine()) {
        getAudioEngine()->removeSource(this);
    }
    m_pDecoder->close();
}

void Sound::onEOF()
{
    if (m_pEOFCallback) {
        PyObject * arglist = Py_BuildValue("()");
        PyObject * result = PyEval_CallObject(m_pEOFCallback, arglist);
        Py_DECREF(arglist);
        if (!result) {
            throw error_already_set();
        }
        Py_DECREF(result);
    }
}

}
