/*
 * Copyright (C) 2004, 2005, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGClipPathElement_h
#define SVGClipPathElement_h

#include "SVGAnimatedBoolean.h"
#include "SVGAnimatedEnumeration.h"
#include "SVGExternalResourcesRequired.h"
#include "SVGGraphicsElement.h"
#include "SVGUnitTypes.h"

namespace WebCore {

class RenderObject;

class SVGClipPathElement final : public SVGGraphicsElement,
                                 public SVGExternalResourcesRequired {
public:
    static Ref<SVGClipPathElement> create(const QualifiedName&, Document&);

private:
    SVGClipPathElement(const QualifiedName&, Document&);

    virtual bool isValid() const override { return SVGTests::isValid(); }
    virtual bool supportsFocus() const override { return false; }
    virtual bool needsPendingResourceHandling() const override { return false; }

    bool isSupportedAttribute(const QualifiedName&);
    virtual void parseAttribute(const QualifiedName&, const AtomicString&) override;
    virtual void svgAttributeChanged(const QualifiedName&) override;
    virtual void childrenChanged(const ChildChange&) override;

    virtual RenderPtr<RenderElement> createElementRenderer(Ref<RenderStyle>&&) override;

    BEGIN_DECLARE_ANIMATED_PROPERTIES(SVGClipPathElement)
        DECLARE_ANIMATED_ENUMERATION(ClipPathUnits, clipPathUnits, SVGUnitTypes::SVGUnitType)
        DECLARE_ANIMATED_BOOLEAN(ExternalResourcesRequired, externalResourcesRequired)
    END_DECLARE_ANIMATED_PROPERTIES
};

}

#endif
