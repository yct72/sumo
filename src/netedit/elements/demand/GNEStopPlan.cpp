/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2001-2023 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNEStopPlan.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Oct 2023
///
// Representation of Stops in netedit
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/changes/GNEChange_ToggleAttribute.h>
#include <netedit/frames/common/GNEMoveFrame.h>
#include <netedit/frames/demand/GNEStopFrame.h>
#include <utils/gui/div/GLHelper.h>

#include "GNEStopPlan.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNEStopPlan*
GNEStopPlan::buildPersonStopPlan(GNENet* net, GNEDemandElement* personParent,
                                 GNEEdge* edge, GNEAdditional* busStop, GNEAdditional* trainStop, const double endPos,
                                 const SUMOTime duration, const SUMOTime until, const std::string& actType,
                                 bool friendlyPos, const int parameterSet) {
    // declare icon an tag
    const auto iconTag = getPersonStopTagIcon(edge, busStop, trainStop);
    // declare containers
    std::vector<GNEEdge*> edges;
    std::vector<GNEAdditional*> additionals;
    // continue depending of input parameters
    if (edge) {
        edges.push_back(edge);
    } else if (busStop) {
        additionals.push_back(busStop);
    } else if (trainStop) {
        additionals.push_back(trainStop);
    }
    return new GNEStopPlan(net, iconTag.first, iconTag.second, personParent, edges, additionals,
                           endPos, duration, until, actType, friendlyPos, parameterSet);
}


GNEStopPlan*
GNEStopPlan::buildContainerStopPlan(GNENet* net, GNEDemandElement* personParent,
                                    GNEEdge* edge, GNEAdditional* containerStop, const double endPos,
                                    const SUMOTime duration, const SUMOTime until, const std::string& actType,
                                    bool friendlyPos, const int parameterSet) {
    // declare icon an tag
    const auto iconTag = getContainerStopTagIcon(edge, containerStop);
    // declare containers
    std::vector<GNEEdge*> edges;
    std::vector<GNEAdditional*> additionals;
    // continue depending of input parameters
    if (edge) {
        edges.push_back(edge);
    } else if (containerStop) {
        additionals.push_back(containerStop);
    }
    return new GNEStopPlan(net, iconTag.first, iconTag.second, personParent, edges, additionals,
                           endPos, duration, until, actType, friendlyPos, parameterSet);
}


GNEStopPlan::GNEStopPlan(SumoXMLTag tag, GNENet* net) :
    GNEDemandElement("", net, GLO_STOP_PLAN, tag, GUIIconSubSys::getIcon(GUIIcon::STOP),
                     GNEPathManager::PathElement::Options::DEMAND_ELEMENT, {}, {}, {}, {}, {}, {}),
GNEDemandElementPlan(this, -1, -1) {
    // reset default values
    resetDefaultValues();
}


GNEStopPlan::~GNEStopPlan() {}


GNEMoveOperation*
GNEStopPlan::getMoveOperation() {
    return getPlanMoveOperation();
}


void
GNEStopPlan::writeDemandElement(OutputDevice& device) const {
    // open tag
    device.openTag(SUMO_TAG_STOP);
    // write plan attributes
    writePlanAttributes(device);
    // write stop attributes
    if (isAttributeEnabled(SUMO_ATTR_DURATION)) {
        device.writeAttr(SUMO_ATTR_DURATION, getAttribute(SUMO_ATTR_DURATION));
    }
    if (isAttributeEnabled(SUMO_ATTR_UNTIL)) {
        device.writeAttr(SUMO_ATTR_UNTIL, getAttribute(SUMO_ATTR_UNTIL));
    }
    if (isAttributeEnabled(SUMO_ATTR_ACTTYPE) && (myActType.size() > 0)) {
        device.writeAttr(SUMO_ATTR_ACTTYPE, myActType);
    }
    if (myTagProperty.hasAttribute(SUMO_ATTR_FRIENDLY_POS) && myFriendlyPos) {
        device.writeAttr(SUMO_ATTR_FRIENDLY_POS, myFriendlyPos);
    }
    // close tag
    device.closeTag();
}


GNEDemandElement::Problem
GNEStopPlan::isDemandElementValid() const {
    return isPlanPersonValid();
}


std::string
GNEStopPlan::getDemandElementProblem() const {
    return getPersonPlanProblem();
}


void
GNEStopPlan::fixDemandElementProblem() {
    // currently the only solution is removing stop
}


SUMOVehicleClass
GNEStopPlan::getVClass() const {
    return SVC_PASSENGER;
}


const RGBColor&
GNEStopPlan::getColor() const {
    return myNet->getViewNet()->getVisualisationSettings().colorSettings.stopPersonColor;
}


void
GNEStopPlan::updateGeometry() {
    // update geometry depending of parent
    if (getParentAdditionals().size() > 0) {
        // get busStop shape
        const PositionVector& busStopShape = getParentAdditionals().front()->getAdditionalGeometry().getShape();
        // update demand element geometry using both positions
        myDemandElementGeometry.updateGeometry(busStopShape, busStopShape.length2D() - 0.6, busStopShape.length2D(), 0);
    } else if (getParentEdges().size() > 0) {
        // get front and back lane
        const GNELane* frontLane = getParentEdges().front()->getLanes().front();
        const GNELane* backLane = getParentEdges().front()->getLanes().back();
        // get lane drawing constants
        GNELane::LaneDrawingConstants laneDrawingConstantsFront(myNet->getViewNet()->getVisualisationSettings(), frontLane);
        GNELane::LaneDrawingConstants laneDrawingConstantBack(myNet->getViewNet()->getVisualisationSettings(), backLane);
        // calculate front position
        const Position frontPosition = frontLane->getLaneShape().positionAtOffset2D(getAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS), laneDrawingConstantsFront.halfWidth);
        // calulate length between both shapes
        const double length = backLane->getLaneShape().distance2D(frontPosition, true);
        // calculate back position
        const Position backPosition = frontLane->getLaneShape().positionAtOffset2D(getAttributeDouble(GNE_ATTR_PLAN_GEOMETRY_ENDPOS), (length + laneDrawingConstantBack.halfWidth - laneDrawingConstantsFront.halfWidth) * -1);
        // update demand element geometry using both positions
        myDemandElementGeometry.updateGeometry({frontPosition, backPosition});
    }
}


Position
GNEStopPlan::getPositionInView() const {
    return getPlanPositionInView();
}


std::string
GNEStopPlan::getParentName() const {
    return getParentDemandElements().front()->getID();
}


double
GNEStopPlan::getExaggeration(const GUIVisualizationSettings& s) const {
    return s.addSize.getExaggeration(s, this);
}


Boundary
GNEStopPlan::getCenteringBoundary() const {
    return getPlanCenteringBoundary();
}


void
GNEStopPlan::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEStopPlan::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double exaggeration = getExaggeration(s);
    // check if stop can be draw
    if ((getTagProperty().isPlanStopPerson() && checkDrawPersonPlan()) ||
            (getTagProperty().isPlanStopContainer() && checkDrawContainerPlan())) {
        // check if draw stopPerson over busStop oder over lane
        if (getParentAdditionals().size() > 0) {
            drawStopOverStoppingPlace(s, exaggeration);
        } else {
            drawStopOverEdge(s, exaggeration);
        }
        // check if draw plan parent
        if (getParentDemandElements().at(0)->getPreviousChildDemandElement(this) == nullptr) {
            getParentDemandElements().at(0)->drawGL(s);
        }
    }
}


void
GNEStopPlan::computePathElement() {
    // only update geometry
    updateGeometry();
}


void
GNEStopPlan::drawLanePartialGL(const GUIVisualizationSettings& /*s*/, const GNEPathManager::Segment* /*segment*/, const double /*offsetFront*/) const {
    // Stops don't use drawJunctionPartialGL
}


void
GNEStopPlan::drawJunctionPartialGL(const GUIVisualizationSettings& /*s*/, const GNEPathManager::Segment* /*segment*/, const double /*offsetFront*/) const {
    // Stops don't use drawJunctionPartialGL
}


GNELane*
GNEStopPlan::getFirstPathLane() const {
    return getFirstPlanPathLane();
}


GNELane*
GNEStopPlan::getLastPathLane() const {
    return getLastPlanPathLane();
}


std::string
GNEStopPlan::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_DURATION:
            if (isAttributeEnabled(key)) {
                return time2string(myDuration);
            } else {
                return "";
            }
        case SUMO_ATTR_UNTIL:
            if (isAttributeEnabled(key)) {
                return time2string(myUntil);
            } else {
                return "";
            }
        case SUMO_ATTR_ACTTYPE:
            return myActType;
        case SUMO_ATTR_FRIENDLY_POS:
            return toString(myFriendlyPos);
        default:
            return getPlanAttribute(key);
    }
}


double
GNEStopPlan::getAttributeDouble(SumoXMLAttr key) const {
    return getPlanAttributeDouble(key);
}


Position
GNEStopPlan::getAttributePosition(SumoXMLAttr key) const {
    return getPlanAttributePosition(key);
}


void
GNEStopPlan::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    switch (key) {
        case SUMO_ATTR_DURATION:
        case SUMO_ATTR_UNTIL:
        case SUMO_ATTR_ACTTYPE:
        case SUMO_ATTR_FRIENDLY_POS:
            GNEChange_Attribute::changeAttribute(this, key, value, undoList);
            break;
        default:
            setPlanAttribute(key, value, undoList);
            break;
    }
}


bool
GNEStopPlan::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_DURATION:
        case SUMO_ATTR_UNTIL:
            if (canParse<SUMOTime>(value)) {
                return parse<SUMOTime>(value) >= 0;
            } else {
                return false;
            }
        case SUMO_ATTR_ACTTYPE:
            return true;
        case SUMO_ATTR_FRIENDLY_POS:
            return canParse<bool>(value);
        default:
            return isPlanValid(key, value);
    }
}


void
GNEStopPlan::enableAttribute(SumoXMLAttr key, GNEUndoList* undoList) {
    switch (key) {
        case SUMO_ATTR_DURATION:
        case SUMO_ATTR_UNTIL:
            undoList->add(new GNEChange_ToggleAttribute(this, key, true), true);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEStopPlan::disableAttribute(SumoXMLAttr key, GNEUndoList* undoList) {
    switch (key) {
        case SUMO_ATTR_DURATION:
        case SUMO_ATTR_UNTIL:
            undoList->add(new GNEChange_ToggleAttribute(this, key, false), true);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEStopPlan::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_DURATION:
            return (myParametersSet & STOP_DURATION_SET) != 0;
        case SUMO_ATTR_UNTIL:
            return (myParametersSet & STOP_UNTIL_SET) != 0;
        default:
            return isPlanAttributeEnabled(key);
    }
}


std::string
GNEStopPlan::getPopUpID() const {
    return getTagStr();
}


std::string
GNEStopPlan::getHierarchyName() const {
    return getPlanHierarchyName();
}


const Parameterised::Map&
GNEStopPlan::getACParametersMap() const {
    return getParametersMap();
}

// ===========================================================================
// protected
// ===========================================================================

void
GNEStopPlan::drawStopOverEdge(const GUIVisualizationSettings& s, const double exaggeration) const {
    // declare stop color
    const RGBColor stopColor = drawUsingSelectColor() ? s.colorSettings.selectedPersonPlanColor : s.colorSettings.stopColor;
    // avoid draw invisible elements
    if (stopColor.alpha() != 0) {
        // Start drawing adding an gl identificator
        GLHelper::pushName(getGlID());
        // Add layer matrix matrix
        GLHelper::pushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
        // declare stop color
        // declare central line color
        const RGBColor centralLineColor = drawUsingSelectColor() ? stopColor.changedBrightness(-32) : RGBColor::WHITE;
        // set base color
        GLHelper::setColor(stopColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GUIGeometry::drawGeometry(s, myNet->getViewNet()->getPositionInformation(), myDemandElementGeometry, 0.3 * exaggeration);
        // move to front
        glTranslated(0, 0, .1);
        // set central color
        GLHelper::setColor(centralLineColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GUIGeometry::drawGeometry(s, myNet->getViewNet()->getPositionInformation(), myDemandElementGeometry, 0.05 * exaggeration);
        // move to icon position and front
        glTranslated(myDemandElementGeometry.getShape().front().x(), myDemandElementGeometry.getShape().front().y(), .1);
        // rotate over lane
        GUIGeometry::rotateOverLane((myDemandElementGeometry.getShapeRotations().front() * -1) + 90);
        // move again
        glTranslated(0, s.additionalSettings.vaporizerSize * exaggeration, 0);
        // Draw icon depending of Route Probe is selected and if isn't being drawn for selecting
        if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.laneTextures, exaggeration)) {
            // set color
            glColor3d(1, 1, 1);
            // rotate texture
            glRotated(180, 0, 0, 1);
            // draw texture
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GUITexture::STOPPERSON_SELECTED), s.additionalSettings.vaporizerSize * exaggeration);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GUITexture::STOPPERSON), s.additionalSettings.vaporizerSize * exaggeration);
            }
        } else {
            // rotate
            glRotated(22.5, 0, 0, 1);
            // set stop color
            GLHelper::setColor(stopColor);
            // move matrix
            glTranslated(0, 0, 0);
            // draw filled circle
            GLHelper::drawFilledCircle(0.1 + s.additionalSettings.vaporizerSize, 8);
        }
        // pop layer matrix
        GLHelper::popMatrix();
        // Pop name
        GLHelper::popName();
        // draw lock icon
        GNEViewNetHelper::LockIcon::drawLockIcon(this, getType(), getPositionInView(), exaggeration);
    }
    // check if mouse is over element
    mouseWithinGeometry(myDemandElementGeometry.getShape(), 0.3);
    // draw dotted geometry
    myContour.drawDottedContourExtruded(s, myDemandElementGeometry.getShape(), 0.3, exaggeration, true, true,
                                        s.dottedContourSettings.segmentWidth);
}


void
GNEStopPlan::drawStopOverStoppingPlace(const GUIVisualizationSettings& s, const double exaggeration) const {
    // declare stop color
    const RGBColor stopColor = drawUsingSelectColor() ? s.colorSettings.selectedPersonPlanColor : s.colorSettings.stopColor;
    // avoid draw invisible elements
    if (stopColor.alpha() != 0) {
        // Start drawing adding an gl identificator
        GLHelper::pushName(getGlID());
        // Add layer matrix matrix
        GLHelper::pushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
        // set base color
        GLHelper::setColor(stopColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        if (getParentAdditionals().front()->getTagProperty().getTag() == SUMO_TAG_TRAIN_STOP) {
            GUIGeometry::drawGeometry(s, myNet->getViewNet()->getPositionInformation(), myDemandElementGeometry, s.stoppingPlaceSettings.trainStopWidth * exaggeration);
        } else {
            GUIGeometry::drawGeometry(s, myNet->getViewNet()->getPositionInformation(), myDemandElementGeometry, s.stoppingPlaceSettings.busStopWidth * exaggeration);
        }
        // move to icon position and front
        glTranslated(myDemandElementGeometry.getShape().getLineCenter().x(), myDemandElementGeometry.getShape().getLineCenter().y(), .1);
        // rotate over lane
        GUIGeometry::rotateOverLane((myDemandElementGeometry.getShapeRotations().front() * -1) + 90);
        // move again
        glTranslated(s.stoppingPlaceSettings.busStopWidth * exaggeration * -2, 0, 0);
        // Draw icon depending of Route Probe is selected and if isn't being drawn for selecting
        if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.laneTextures, exaggeration)) {
            // set color
            glColor3d(1, 1, 1);
            // rotate texture
            glRotated(-90, 0, 0, 1);
            // draw texture
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GUITexture::STOPPERSON_SELECTED),
                                                   s.additionalSettings.vaporizerSize * exaggeration);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GUITexture::STOPPERSON),
                                                   s.additionalSettings.vaporizerSize * exaggeration);
            }
        } else {
            // rotate
            glRotated(22.5, 0, 0, 1);
            // set stop color
            GLHelper::setColor(stopColor);
            // move matrix
            glTranslated(0, 0, 0);
            // draw filled circle
            GLHelper::drawFilledCircle(0.1 + s.additionalSettings.vaporizerSize, 8);
        }
        // pop layer matrix
        GLHelper::popMatrix();
        // Pop name
        GLHelper::popName();
        // draw lock icon
        GNEViewNetHelper::LockIcon::drawLockIcon(this, getType(), getPositionInView(), exaggeration);
        // draw dotted geometry
        myContour.drawDottedContourExtruded(s, myDemandElementGeometry.getShape(), 0.3, exaggeration, true, true,
                                            s.dottedContourSettings.segmentWidth);
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEStopPlan::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_DURATION:
            if (value.empty()) {
                toggleAttribute(key, false);
            } else {
                toggleAttribute(key, true);
                myDuration = string2time(value);
            }
            break;
        case SUMO_ATTR_UNTIL:
            if (value.empty()) {
                toggleAttribute(key, false);
            } else {
                toggleAttribute(key, true);
                myUntil = string2time(value);
            }
            break;
        case SUMO_ATTR_ACTTYPE:
            myActType = value;
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            myFriendlyPos = parse<bool>(value);
            break;
        default:
            setPlanAttribute(key, value);
            break;
    }
}


void
GNEStopPlan::toggleAttribute(SumoXMLAttr key, const bool value) {
    switch (key) {
        case SUMO_ATTR_DURATION:
            if (value) {
                myParametersSet |= STOP_DURATION_SET;
            } else {
                myParametersSet &= ~STOP_DURATION_SET;
            }
            break;
        case SUMO_ATTR_UNTIL:
            if (value) {
                myParametersSet |= STOP_UNTIL_SET;
            } else {
                myParametersSet &= ~STOP_UNTIL_SET;
            }
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEStopPlan::setMoveShape(const GNEMoveResult& moveResult) {
    // change endPos
    myArrivalPosition = moveResult.newFirstPos;
    // update geometry
    updateGeometry();
}


void
GNEStopPlan::commitMoveShape(const GNEMoveResult& moveResult, GNEUndoList* undoList) {
    undoList->begin(this, "endPos of " + getTagStr());
    // now adjust start position
    setAttribute(SUMO_ATTR_ENDPOS, toString(moveResult.newFirstPos), undoList);
    undoList->end();
}


GNEStopPlan::GNEStopPlan(GNENet* net, SumoXMLTag tag, GUIIcon icon, GNEDemandElement* personParent, const std::vector<GNEEdge*>& edges,
                         const std::vector<GNEAdditional*>& additionals, const double endPos, const SUMOTime duration, const SUMOTime until,
                         const std::string& actType, bool friendlyPos, const int parameterSet) :
    GNEDemandElement(personParent, net, GLO_STOP_PLAN, tag, GUIIconSubSys::getIcon(icon),
                     GNEPathManager::PathElement::Options::DEMAND_ELEMENT, {}, edges, {}, additionals, {personParent}, {}),
GNEDemandElementPlan(this, -1, endPos),
myDuration(duration),
myUntil(until),
myActType(actType),
myFriendlyPos(friendlyPos),
myParametersSet(parameterSet) {
}

/****************************************************************************/
