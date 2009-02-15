//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#ifndef _LineNode_H_
#define _LineNode_H_

#include "../api.h"
#include "VectorNode.h"

#include "../graphics/Pixel32.h"

namespace avg {

class AVG_API LineNode : public VectorNode
{
    public:
        static NodeDefinition createDefinition();
        
        LineNode(const ArgList& Args, bool bFromXML);
        virtual ~LineNode();

        double getX1() const;
        void setX1(double x);
        
        double getY1() const;
        void setY1(double y);

        const DPoint& getPos1() const;
        void setPos1(const DPoint& pt);

        double getX2() const;
        void setX2(double x);
        
        double getY2() const;
        void setY2(double y);

        const DPoint& getPos2() const;
        void setPos2(const DPoint& pt);

        virtual int getNumVertexes();
        virtual int getNumIndexes();
        virtual void calcVertexes(VertexArrayPtr& pVertexArray, double opacity);

    private:
        DPoint m_P1;
        DPoint m_P2;
};

}

#endif

