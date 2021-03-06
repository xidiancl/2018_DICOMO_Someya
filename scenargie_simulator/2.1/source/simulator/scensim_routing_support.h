// Copyright (c) 2007-2016 by Space-Time Engineering, LLC ("STE").
// All Rights Reserved.
//
// This source code is a part of Scenargie Software ("Software") and is
// subject to STE Software License Agreement. The information contained
// herein is considered a trade secret of STE, and may not be used as
// the basis for any other software, hardware, product or service.
//
// Refer to license.txt for more specific directives.

#ifndef SCENSIM_ROUTING_SUPPORT_H
#define SCENSIM_ROUTING_SUPPORT_H

#include "scensim_netsim.h"

namespace ScenSim {

void SetupRoutingProtocol(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const NodeIdType& nodeId,
    const shared_ptr<SimulationEngineInterface>& simulationEngineInterfacePtr,
    const shared_ptr<NetworkLayer>& networkLayerPtr,
    const RandomNumberGeneratorSeedType& nodeSeed);

}//namespace//

#endif

