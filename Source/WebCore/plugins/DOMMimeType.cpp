/*
 *  Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "DOMMimeType.h"

#include "DOMPlugin.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "MainFrame.h"
#include "Page.h"
#include "PluginData.h"
#include "Settings.h"
#include "SubframeLoader.h"
#include <wtf/text/StringBuilder.h>

namespace WebCore {

DOMMimeType::DOMMimeType(PassRefPtr<PluginData> pluginData, Frame* frame, unsigned index)
    : FrameDestructionObserver(frame)
    , m_pluginData(pluginData)
    , m_index(index)
{
}

DOMMimeType::~DOMMimeType()
{
}

String DOMMimeType::type() const
{
    return mimeClassInfo().type;
}

String DOMMimeType::suffixes() const
{
    const Vector<String>& extensions = mimeClassInfo().extensions;

    StringBuilder builder;
    for (size_t i = 0; i < extensions.size(); ++i) {
        if (i)
            builder.append(',');
        builder.append(extensions[i]);
    }
    return builder.toString();
}

String DOMMimeType::description() const
{
    return mimeClassInfo().desc;
}

MimeClassInfo DOMMimeType::mimeClassInfo() const
{
    Vector<MimeClassInfo> mimes;
    Vector<size_t> mimePluginIndices;
    m_pluginData->getWebVisibleMimesAndPluginIndices(mimes, mimePluginIndices);
    return mimes[m_index];
}

PassRefPtr<DOMPlugin> DOMMimeType::enabledPlugin() const
{
    if (!m_frame || !m_frame->page() || !m_frame->page()->mainFrame().loader().subframeLoader().allowPlugins(NotAboutToInstantiatePlugin))
        return 0;

    Vector<MimeClassInfo> mimes;
    Vector<size_t> mimePluginIndices;
    m_pluginData->getWebVisibleMimesAndPluginIndices(mimes, mimePluginIndices);
    return DOMPlugin::create(m_pluginData.get(), m_frame, mimePluginIndices[m_index]);
}

} // namespace WebCore
