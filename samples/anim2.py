#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2010-2021 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de

from libavg import *

def startAnim():
    animObj.start()

canvas = player.createMainCanvas(size=(640,480))
rootNode = canvas.getRootNode()
node = avg.RectNode(pos=(10,10), size=(100,100), fillcolor="FF0000", fillopacity=1.0,
        parent=rootNode)

animObj = ParallelAnim(
    [LinearAnim(node, "pos", 2000, (0,0), (200,10)),
     LinearAnim(node, "fillcolor", 2000, "00FFFF", "FF0000")])
player.setTimeout(0, startAnim)

player.play()

