/*
 * Segment.cpp
 *****************************************************************************
 * Copyright (C) 2010 - 2011 Klagenfurt University
 *
 * Created on: Aug 10, 2010
 * Authors: Christopher Mueller <christopher.mueller@itec.uni-klu.ac.at>
 *          Christian Timmerer  <christian.timmerer@itec.uni-klu.ac.at>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Segment.h"
#include "Representation.h"

#include <cassert>

using namespace dash::mpd;
using namespace dash::http;

ISegment::ISegment(const ICanonicalUrl *parent):
    ICanonicalUrl( parent ),
    startByte  (0),
    endByte    (0)
{

}

dash::http::Chunk* ISegment::toChunk() const
{
    Chunk *chunk = new Chunk();
    if (!chunk)
        return NULL;

    if(startByte != endByte)
    {
        chunk->setStartByte(startByte);
        chunk->setEndByte(endByte);
    }

    chunk->setUrl(getUrlSegment());

    return chunk;
}

bool ISegment::isSingleShot() const
{
    return true;
}
void ISegment::done()
{
    //Only used for a SegmentTemplate.
}

void ISegment::setByteRange(size_t start, size_t end)
{
    startByte = start;
    endByte   = end;
}

std::string ISegment::toString() const
{
    return std::string("    Segment url=").append(getUrlSegment());
}

Segment::Segment(Representation *parent, bool isinit, bool tosplit) :
        ISegment(parent),
        parentRepresentation( parent ),
        init( isinit ),
        needssplit( tosplit )
{
    assert( parent != NULL );
    if ( parent->getSegmentInfo() != NULL && parent->getSegmentInfo()->getDuration() >= 0 )
        this->size = parent->getBandwidth() * parent->getSegmentInfo()->getDuration();
    else
        this->size = -1;
}

Segment::~Segment()
{
    std::vector<SubSegment*>::iterator it;
    for(it=subsegments.begin();it!=subsegments.end();it++)
        delete *it;
}

void                    Segment::setSourceUrl   ( const std::string &url )
{
    if ( url.empty() == false )
        this->sourceUrl = url;
}

bool                    Segment::needsSplit() const
{
    return needssplit;
}

Representation *Segment::getRepresentation() const
{
    return parentRepresentation;
}

std::string Segment::getUrlSegment() const
{
    std::string ret = getParentUrlSegment();
    if (!sourceUrl.empty())
        ret.append(sourceUrl);
    return ret;
}

dash::http::Chunk* Segment::toChunk() const
{
    Chunk *chunk = ISegment::toChunk();
    if (chunk)
        chunk->setBitrate(parentRepresentation->getBandwidth());
    return chunk;
}

std::vector<ISegment*> Segment::subSegments()
{
    std::vector<ISegment*> list;
    if(!subsegments.empty())
    {
        std::vector<SubSegment*>::iterator it;
        for(it=subsegments.begin();it!=subsegments.end();it++)
            list.push_back(*it);
    }
    else
    {
        list.push_back(this);
    }
    return list;
}

std::string Segment::toString() const
{
    if (init)
        return std::string("    InitSeg url=")
                    .append(getUrlSegment());
    else
        return ISegment::toString();
}

SubSegment::SubSegment(Segment *main, size_t start, size_t end) :
    ISegment(main), parent(main)
{
    setByteRange(start, end);
}

std::string SubSegment::getUrlSegment() const
{
    return getParentUrlSegment();
}

std::vector<ISegment*> SubSegment::subSegments()
{
    std::vector<ISegment*> list;
    list.push_back(this);
    return list;
}

Representation *SubSegment::getRepresentation() const
{
    return parent->getRepresentation();
}
