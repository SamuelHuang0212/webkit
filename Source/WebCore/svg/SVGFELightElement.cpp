/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Oliver Hunt <oliver@nerget.com>
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

#include "config.h"
#include "SVGFELightElement.h"

#include "Attribute.h"
#include "ElementIterator.h"
#include "RenderObject.h"
#include "RenderSVGResource.h"
#include "SVGFEDiffuseLightingElement.h"
#include "SVGFESpecularLightingElement.h"
#include "SVGFilterElement.h"
#include "SVGFilterPrimitiveStandardAttributes.h"
#include "SVGNames.h"
#include <wtf/NeverDestroyed.h>

namespace WebCore {

// Animated property definitions
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::azimuthAttr, Azimuth, azimuth)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::elevationAttr, Elevation, elevation)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::xAttr, X, x)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::yAttr, Y, y)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::zAttr, Z, z)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::pointsAtXAttr, PointsAtX, pointsAtX)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::pointsAtYAttr, PointsAtY, pointsAtY)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::pointsAtZAttr, PointsAtZ, pointsAtZ)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::specularExponentAttr, SpecularExponent, specularExponent)
DEFINE_ANIMATED_NUMBER(SVGFELightElement, SVGNames::limitingConeAngleAttr, LimitingConeAngle, limitingConeAngle)

BEGIN_REGISTER_ANIMATED_PROPERTIES(SVGFELightElement)
    REGISTER_LOCAL_ANIMATED_PROPERTY(azimuth)
    REGISTER_LOCAL_ANIMATED_PROPERTY(elevation)
    REGISTER_LOCAL_ANIMATED_PROPERTY(x)
    REGISTER_LOCAL_ANIMATED_PROPERTY(y)
    REGISTER_LOCAL_ANIMATED_PROPERTY(z)
    REGISTER_LOCAL_ANIMATED_PROPERTY(pointsAtX)
    REGISTER_LOCAL_ANIMATED_PROPERTY(pointsAtY)
    REGISTER_LOCAL_ANIMATED_PROPERTY(pointsAtZ)
    REGISTER_LOCAL_ANIMATED_PROPERTY(specularExponent)
    REGISTER_LOCAL_ANIMATED_PROPERTY(limitingConeAngle)
END_REGISTER_ANIMATED_PROPERTIES

SVGFELightElement::SVGFELightElement(const QualifiedName& tagName, Document& document)
    : SVGElement(tagName, document)
    , m_specularExponent(1)
{
    registerAnimatedPropertiesForSVGFELightElement();
}

SVGFELightElement* SVGFELightElement::findLightElement(const SVGElement* svgElement)
{
    for (auto& child : childrenOfType<SVGElement>(*svgElement)) {
        if (is<SVGFEDistantLightElement>(child) || is<SVGFEPointLightElement>(child) || is<SVGFESpotLightElement>(child))
            return static_cast<SVGFELightElement*>(const_cast<SVGElement*>(&child));
    }
    return nullptr;
}

PassRefPtr<LightSource> SVGFELightElement::findLightSource(const SVGElement* svgElement)
{
    SVGFELightElement* lightNode = findLightElement(svgElement);
    if (!lightNode)
        return 0;
    return lightNode->lightSource();
}

bool SVGFELightElement::isSupportedAttribute(const QualifiedName& attrName)
{
    static NeverDestroyed<HashSet<QualifiedName>> supportedAttributes;
    if (supportedAttributes.get().isEmpty()) {
        supportedAttributes.get().add(SVGNames::azimuthAttr);
        supportedAttributes.get().add(SVGNames::elevationAttr);
        supportedAttributes.get().add(SVGNames::xAttr);
        supportedAttributes.get().add(SVGNames::yAttr);
        supportedAttributes.get().add(SVGNames::zAttr);
        supportedAttributes.get().add(SVGNames::pointsAtXAttr);
        supportedAttributes.get().add(SVGNames::pointsAtYAttr);
        supportedAttributes.get().add(SVGNames::pointsAtZAttr);
        supportedAttributes.get().add(SVGNames::specularExponentAttr);
        supportedAttributes.get().add(SVGNames::limitingConeAngleAttr);
    }
    return supportedAttributes.get().contains<SVGAttributeHashTranslator>(attrName);
}

void SVGFELightElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
{
    if (!isSupportedAttribute(name)) {
        SVGElement::parseAttribute(name, value);
        return;
    }

    if (name == SVGNames::azimuthAttr) {
        setAzimuthBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::elevationAttr) {
        setElevationBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::xAttr) {
        setXBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::yAttr) {
        setYBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::zAttr) {
        setZBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::pointsAtXAttr) {
        setPointsAtXBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::pointsAtYAttr) {
        setPointsAtYBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::pointsAtZAttr) {
        setPointsAtZBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::specularExponentAttr) {
        setSpecularExponentBaseValue(value.toFloat());
        return;
    }

    if (name == SVGNames::limitingConeAngleAttr) {
        setLimitingConeAngleBaseValue(value.toFloat());
        return;
    }

    ASSERT_NOT_REACHED();
}

void SVGFELightElement::svgAttributeChanged(const QualifiedName& attrName)
{
    if (!isSupportedAttribute(attrName)) {
        SVGElement::svgAttributeChanged(attrName);
        return;
    }

    InstanceInvalidationGuard guard(*this);
    
    if (attrName == SVGNames::azimuthAttr
        || attrName == SVGNames::elevationAttr
        || attrName == SVGNames::xAttr
        || attrName == SVGNames::yAttr
        || attrName == SVGNames::zAttr
        || attrName == SVGNames::pointsAtXAttr
        || attrName == SVGNames::pointsAtYAttr
        || attrName == SVGNames::pointsAtZAttr
        || attrName == SVGNames::specularExponentAttr
        || attrName == SVGNames::limitingConeAngleAttr) {
        ContainerNode* parent = parentNode();
        if (!parent)
            return;

        RenderObject* renderer = parent->renderer();
        if (!renderer || !renderer->isSVGResourceFilterPrimitive())
            return;

        if (parent->hasTagName(SVGNames::feDiffuseLightingTag)) {
            SVGFEDiffuseLightingElement* diffuseLighting = static_cast<SVGFEDiffuseLightingElement*>(parent);
            diffuseLighting->lightElementAttributeChanged(this, attrName);
            return;
        } else if (parent->hasTagName(SVGNames::feSpecularLightingTag)) {
            SVGFESpecularLightingElement* specularLighting = static_cast<SVGFESpecularLightingElement*>(parent);
            specularLighting->lightElementAttributeChanged(this, attrName);
            return;
        }
    }

    ASSERT_NOT_REACHED();
}

void SVGFELightElement::childrenChanged(const ChildChange& change)
{
    SVGElement::childrenChanged(change);

    if (change.source == ChildChangeSourceParser)
        return;
    ContainerNode* parent = parentNode();
    if (!parent)
        return;
    RenderElement* renderer = parent->renderer();
    if (renderer && renderer->isSVGResourceFilterPrimitive())
        RenderSVGResource::markForLayoutAndParentResourceInvalidation(*renderer);
}

}
