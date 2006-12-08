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

#include "TrackerThread.h"
#include "ConnectedComps.h"
#include "../base/Logger.h"
#include "../graphics/Filter.h"
#include "../graphics/Filterfill.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

namespace avg {

TrackerThread::TrackerThread(CameraPtr pCamera, int Threshold, 
        BitmapPtr ppBitmaps[NUM_TRACKER_IMAGES],
        MutexPtr pMutex,
        CmdQueue& CmdQ,
        IBlobTarget *target,
        Filter *preproc
        )
    : WorkerThread<TrackerThread>(CmdQ),
      m_Threshold(Threshold),
      m_pMutex(pMutex),
      m_bHistoryInitialized(false),
      m_pCamera(pCamera),
      m_pTarget(target),
      m_pPreProcessor(preproc)

{
    for (int i=0; i<NUM_TRACKER_IMAGES; i++) {
        m_pBitmaps[i] = ppBitmaps[i];
    }
}

TrackerThread::~TrackerThread()
{
}

bool TrackerThread::init()
{
    m_pHistoryBmp = BitmapPtr(new Bitmap(m_pBitmaps[0]->getSize(), I16));
    memset(m_pHistoryBmp->getPixels(), 0, 
            m_pHistoryBmp->getSize().y*m_pHistoryBmp->getStride());
    AVG_TRACE(Logger::CONFIG, "Tracker thread started.");
    m_pCamera->open();
    return true;
}

bool TrackerThread::work()
{
    BitmapPtr pTempBmp = m_pCamera->getImage(true);
    BitmapPtr pTempBmp1;
    while (pTempBmp1 = m_pCamera->getImage(false)) {
        pTempBmp = pTempBmp1;
    }
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        *(m_pBitmaps[TRACKER_IMG_CAMERA]) = *pTempBmp;
    }
    m_pPreProcessor->applyInPlace(pTempBmp);
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        m_pBitmaps[TRACKER_IMG_NOHISTORY]->copyPixels(*pTempBmp);
    }
    //get bloblist
    //BlobListPtr comps = connected_components(m_pBitmaps[TRACKER_IMG_NOHISTORY], m_Threshold);
    BlobListPtr comps = connected_components(pTempBmp, m_Threshold);
    {
        boost::mutex::scoped_lock Lock(*m_pMutex);
        FilterFill<Pixel8>(0x00).applyInPlace(m_pBitmaps[TRACKER_IMG_COMPONENTS]);
        for (BlobList::iterator it=comps->begin(); it != comps->end(); ++it) {
            render(&(*m_pBitmaps[TRACKER_IMG_COMPONENTS]), *it, 0xFF);
        }
    }
    AVG_TRACE(Logger::EVENTS2, "connected components found "<<comps->size()<<" blobs.");
    //feed the IBlobTarget
    m_pTarget->update(comps);
    return true;
}

void TrackerThread::deinit()
{
    m_pCamera->close();
}

void TrackerThread::setThreshold(int Threshold) 
{
    m_Threshold = Threshold;
}

void TrackerThread::setBrightness(int Brightness) 
{
    m_pCamera->setFeature("brightness", Brightness);
}

void TrackerThread::setExposure(int Exposure) 
{
    m_pCamera->setFeature("exposure", Exposure);
}

void TrackerThread::setGain(int Gain) 
{
    m_pCamera->setFeature("gain", Gain);
}

void TrackerThread::setShutter(int Shutter) 
{
    m_pCamera->setFeature("shutter", Shutter);
}


}
