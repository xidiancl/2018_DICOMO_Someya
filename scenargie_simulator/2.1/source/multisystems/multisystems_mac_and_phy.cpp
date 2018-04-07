// Copyright (c) 2007-2016 by Space-Time Engineering, LLC ("STE").
// All Rights Reserved.
//
// This source code is a part of Scenargie Software ("Software") and is
// subject to STE Software License Agreement. The information contained
// herein is considered a trade secret of STE, and may not be used as
// the basis for any other software, hardware, product or service.
//
// Refer to license.txt for more specific directives.

#include "sim.h"

using Aloha::AlohaMac;
using Aloha::AlohaPhy;

using T109::T109App;

using Wave::WsmpLayer;
using Wave::DsrcMessageApplication;
using Wave::WaveMacPhyInput;
using Wave::ChannelNumberIndexType;
using Wave::WaveMac;
using Wave::GetChannelCategoryAndNumber;
using Wave::ChannelCategoryType;

using GeoNet::GeoNetworkingProtocol;
using GeoNet::BasicTransportProtocol;
using GeoNet::GeoNetMac;

using Its::ItsBroadcastApplication;

using Dot15::Dot15Mac;
using Dot15::Dot15AbstractTransportProtocol;
using Dot15::Dot15ApplicationMaker;

using ScenSim::InterfaceOutputQueue;

class NoNetworkQueue : public InterfaceOutputQueue {
public:
    NoNetworkQueue() { }

    virtual bool InsertWithFullPacketInformationModeIsOn() const { return false; }

    virtual void Insert(
        unique_ptr<Packet>& packetPtr,
        const NetworkAddress& nextHopAddress,
        const PacketPriorityType priority,
        EnqueueResultType& enqueueResult,
        unique_ptr<Packet>& packetToDropPtr,
        const EtherTypeFieldType etherType = ScenSim::ETHERTYPE_IS_NOT_SPECIFIED) {
        (*this).OutputUtilizationError();
    }

    virtual void InsertWithFullPacketInformation(
        unique_ptr<Packet>& packetPtr,
        const NetworkAddress& nextHopAddress,
        const NetworkAddress& sourceAddress,
        const unsigned short int sourcePort,
        const NetworkAddress& destinationAddress,
        const unsigned short int destinationPort,
        const unsigned char protocolCode,
        const PacketPriorityType priority,
        const unsigned short int ipv6FlowLabel,
        EnqueueResultType& enqueueResult,
        unique_ptr<Packet>& packetToDropPtr) {
        (*this).OutputUtilizationError();
    }

    virtual bool IsEmpty() const { return true; }

    virtual bool IsFull(const PacketPriorityType priority, const size_t forPacketSizeBytes = 0) const { return false; }

    virtual void DequeuePacket(
        unique_ptr<Packet>& packetPtr,
        NetworkAddress& nextHopAddress,
        PacketPriorityType& priority,
        EtherTypeFieldType& etherType) { assert(false); }

protected:

    void OutputUtilizationError() {
        cerr << "T109Mac can't send IP packets generated by CBR, VBR, FTP, MultiFTP, VoIP, Video, HTTP, Flooding or any general application." << endl;
        cerr << "Only T109App is available for T109Mac." << endl;
        exit(1);
    }

};//NoNetworkQueue//

shared_ptr<ObjectMobilityModel> SimNode::GetNodeOrAttachedMobility(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const InterfaceIdType& interfaceId)
{
    // Antenna Mobility
    if (AttachedAntennaMobilityModel::AntennaIsAttachedAntenna(
            theParameterDatabaseReader, nodeId, interfaceId)) {

        return shared_ptr<ObjectMobilityModel>(
            new AttachedAntennaMobilityModel(
                theParameterDatabaseReader,
                nodeId,
                interfaceId,
                nodeMobilityModelPtr));
    }

    return nodeMobilityModelPtr;
}//GetNodeOrAttachedMobility//



void SimNode::SetupInterfaces(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const shared_ptr<GisSubsystem>& gisSubsystemPtr,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr)
{
    const unsigned int numberInterfaces = networkLayerPtr->NumberOfInterfaces();
    interfaces.resize(numberInterfaces);

    for(unsigned int interfaceIndex = 0; interfaceIndex < numberInterfaces; interfaceIndex++) {
        const InterfaceIdType interfaceId = networkLayerPtr->GetInterfaceId(interfaceIndex);

        if (InterfaceIsWired(theParameterDatabaseReader, nodeId, interfaceId)) {

            (*this).SetupWiredInterface(
                theParameterDatabaseReader,
                theGlobalNetworkingObjectBag,
                interfaceId,
                interfaceIndex);
        }
        else if (IsNoInterface(theParameterDatabaseReader, nodeId, interfaceId)) {

            // no MAC protocol

        }
        else {

            // MAC protocol
            const string macProtocol = MakeLowerCaseString(
                theParameterDatabaseReader.ReadString("mac-protocol", nodeId, interfaceId));

            InterfaceType& interfaceToBeInitialized = interfaces[interfaceIndex];

            if (macProtocol == "aloha") {
                (*this).SetupAlohaInterface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "dot11") {
                (*this).SetupDot11Interface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "dot11ad") {
                (*this).SetupDot11adInterface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "dot11ah") {
                (*this).SetupDot11ahInterface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "dot15") {
                (*this).SetupDot15Interface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "lte") {
                (*this).SetupLteInterface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "wave") {
                (*this).SetupWaveInterface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "geonet") {
                (*this).SetupGeoNetInterface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "t109") {
                (*this).SetupT109Interface(
                    theParameterDatabaseReader,
                    theGlobalNetworkingObjectBag,
                    interfaceId,
                    interfaceIndex,
                    channelModelSetPtr,
                    interfaceToBeInitialized);
            }
            else if (macProtocol == "no") {

                // no mac

            }
            else {
                cerr << "Error: bad mac-protocol format: " << macProtocol
                     << " for node: " << nodeId << endl;
                exit(1);
            }

            networkLayerPtr->SetInterfaceMacLayer(interfaceIndex, interfaceToBeInitialized.macPtr);
        }//if//

        // Add interface routing or application protocols from here. -------------------


        // -----------------------------------------------------------------------------

    }//for//

    (*this).CompleteAntennaNumberAssignment();

}//SetupInterfaces//

void SimNode::SetupWiredInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex)
{
    // no mac protocol
    // Abstract Network has virtual connections between their network layers.
}//SetupWiredInterface//

void SimNode::SetupAlohaInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<Aloha::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetAlohaPropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.alohaPropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId,
        interfaceId,
        interfaceIndex);

    // Aloha MAC/PHY
    interfaceToBeInitialized.macPtr =
        Aloha::AlohaFactory(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            antenna.alohaPropagationModelInterfacePtr,
            nodeId,
            interfaceId,
            networkLayerPtr,
            nodeSeed);

}//SetupAlohaInterface//

void SimNode::SetupDot11Interface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    using Dot11::Dot11Mac;
    using Dot11::Dot11Phy;

    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<Dot11::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetDot11PropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.dot11PropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId,
        interfaceId,
        interfaceIndex);

#ifdef ITS_EXTENSION
    // Dot11 PHY
    shared_ptr<Dot11Phy> phyLayerPtr(
        new Dot11Phy(
            theParameterDatabaseReader,
            nodeId,
            interfaceId,
            simulationEngineInterfacePtr,
            antenna.dot11PropagationModelInterfacePtr,
            channelModelSetPtr->GetDot11BitOrBlockErrorRateCurveDatabase(),
            nodeSeed));

    // Dot11 MAC
    interfaceToBeInitialized.macPtr =
        Dot11Mac::Create(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            nodeId,
            interfaceId,
            interfaceIndex,
            networkLayerPtr,
            phyLayerPtr,
            nodeSeed);
#else
    shared_ptr<MimoChannelModel> mimoChannelModelPtr;
    shared_ptr<FrequencySelectiveFadingModel> frequencySelectiveFadingModelPtr;

    channelModelSetPtr->GetDot11ChannelModel(
        GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId),
        propagationModelPtr->GetBaseChannelNumber(),
        propagationModelPtr->GetChannelCount(),
        GetListOfCarrierFreqenciesMhz(*propagationModelPtr),
        GetListOfChannelBandwidthsMhz(*propagationModelPtr),
        mimoChannelModelPtr,
        frequencySelectiveFadingModelPtr);

    if (mimoChannelModelPtr != nullptr) {
        mimoChannelModelPtr->CreateNewMimoChannelModelInterface(
            theParameterDatabaseReader,
            nodeId,
            interfaceId,
            antenna.antennaMobilityModelPtr,
            antenna.mimoChannelModelInterfacePtr);
    }//if//

    if (frequencySelectiveFadingModelPtr != nullptr) {
        assert(mimoChannelModelPtr == nullptr);

        frequencySelectiveFadingModelPtr->GetNewFadingModelInterface(
            nodeId,
            antenna.frequencySelectiveFadingModelInterfacePtr);
    }//if//

    interfaceToBeInitialized.macPtr =
        Dot11Mac::Create(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            antenna.dot11PropagationModelInterfacePtr,
            antenna.mimoChannelModelInterfacePtr,
            antenna.frequencySelectiveFadingModelInterfacePtr,
            channelModelSetPtr->GetDot11BitOrBlockErrorRateCurveDatabase(),
            nodeId,
            interfaceId,
            interfaceIndex,
            networkLayerPtr,
            nodeSeed);
#endif

}//SetupDot11Interface//

void SimNode::SetupDot11adInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    using Dot11ad::Dot11Mac;
    using ScenSim::CustomAntennaModel;

    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<Dot11ad::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetDot11adPropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.dot11adPropagationModelInterfacePtr =
        propagationModelPtr->GetNewPropagationModelInterface(
            simulationEngineInterfacePtr,
            antenna.antennaModelPtr,
            antenna.antennaMobilityModelPtr,
            nodeId,
            interfaceId,
            interfaceIndex);

    shared_ptr<MimoChannelModel> mimoChannelModelPtr;
    shared_ptr<FrequencySelectiveFadingModel> frequencySelectiveFadingModelPtr;

    channelModelSetPtr->GetDot11adChannelModel(
        GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId),
        propagationModelPtr->GetBaseChannelNumber(),
        propagationModelPtr->GetChannelCount(),
        GetListOfCarrierFreqenciesMhz(*propagationModelPtr),
        GetListOfChannelBandwidthsMhz(*propagationModelPtr),
        mimoChannelModelPtr,
        frequencySelectiveFadingModelPtr);

    if (mimoChannelModelPtr != nullptr) {
        mimoChannelModelPtr->CreateNewMimoChannelModelInterface(
            theParameterDatabaseReader,
            nodeId,
            interfaceId,
            antenna.antennaMobilityModelPtr,
            antenna.mimoChannelModelInterfacePtr);
    }//if//

    if (frequencySelectiveFadingModelPtr != nullptr) {
        assert(mimoChannelModelPtr == nullptr);

        frequencySelectiveFadingModelPtr->GetNewFadingModelInterface(
            nodeId,
            antenna.frequencySelectiveFadingModelInterfacePtr);
    }//if//

    shared_ptr<CustomAntennaModel> customAntennaModelPtr =
        std::dynamic_pointer_cast<CustomAntennaModel>(antenna.antennaModelPtr);

    if (customAntennaModelPtr == nullptr) {
        cerr << "Error: Use custom antenna model for Dot11ad" << endl;
        exit(1);
    }//if/

    interfaceToBeInitialized.macPtr =
        Dot11Mac::Create(
            theParameterDatabaseReader,
            *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr,
            simulationEngineInterfacePtr,
            antenna.dot11adPropagationModelInterfacePtr,
            antenna.mimoChannelModelInterfacePtr,
            antenna.frequencySelectiveFadingModelInterfacePtr,
            customAntennaModelPtr,
            channelModelSetPtr->GetDot11AdBitOrBlockErrorRateCurveDatabase(),
            nodeId,
            interfaceId,
            interfaceIndex,
            networkLayerPtr,
            nodeSeed);

}//SetupDot11adInterface//

void SimNode::SetupDot11ahInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    using Dot11ah::Dot11Mac;

    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<Dot11ah::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetDot11ahPropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.dot11ahPropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId,
        interfaceId,
        interfaceIndex);

    interfaceToBeInitialized.macPtr =
        Dot11Mac::Create(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            antenna.dot11ahPropagationModelInterfacePtr,
            channelModelSetPtr->GetDot11AhBitOrBlockErrorRateCurveDatabase(),
            nodeId,
            interfaceId,
            interfaceIndex,
            networkLayerPtr,
            nodeSeed);

}//SetupDot11ahInterface//

void SimNode::SetupDot15Interface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<Dot15::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetDot15PropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.dot15PropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId,
        interfaceId,
        interfaceIndex);

    //Application and Transport
    shared_ptr<Dot15AbstractTransportProtocol> dot15AbstractTransportLayerPtr = 
        shared_ptr<Dot15AbstractTransportProtocol>(
            new Dot15AbstractTransportProtocol(
                simulationEngineInterfacePtr));
    
    Dot15ApplicationMaker appMaker(
        simulationEngineInterfacePtr,
        appLayerPtr,
        dot15AbstractTransportLayerPtr,
        nodeId,
        nodeSeed);

    appMaker.ReadApplicationLineFromConfig(theParameterDatabaseReader);
    
    // Dot15 MAC/PHY
    shared_ptr<Dot15Mac> macPtr =
        shared_ptr<Dot15Mac>(
            new Dot15Mac(
                theParameterDatabaseReader,
                simulationEngineInterfacePtr,
                antenna.dot15PropagationModelInterfacePtr,
                channelModelSetPtr->GetDot15BitOrBlockErrorRateCurveDatabase(),
                nodeId,
                interfaceId,                
                nodeSeed));
        
    dot15AbstractTransportLayerPtr->ConnectToMacLayer(macPtr);

    interfaceToBeInitialized.macPtr = macPtr;
                
}//SetupDot15Interface//

void SimNode::SetupLteInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    const shared_ptr<Lte::LteGlobalParameters> lteGlobalPtr = channelModelSetPtr->GetLteGlobals();
    const ChannelInstanceIdType downlinkInstanceId = lteGlobalPtr->GetDownlinkPropagationInstanceId();
    const ChannelInstanceIdType uplinkInstanceId = lteGlobalPtr->GetUplinkPropagationInstanceId();

    // Propagation Interface
    const shared_ptr<Lte::DownlinkPropagationModel> downlinkPropagationModelPtr =
        channelModelSetPtr->GetLteDownlinkPropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, downlinkInstanceId));

    const shared_ptr<Lte::UplinkPropagationModel> uplinkPropagationModelPtr =
        channelModelSetPtr->GetLteUplinkPropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, uplinkInstanceId));

    antenna.lteDownlinkPropagationModelInterfacePtr = downlinkPropagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId//,
        // interfaceId
        );

    antenna.lteUplinkPropagationModelInterfacePtr = uplinkPropagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId//,
        // interfaceId
        );

    shared_ptr<ScenSim::MimoChannelModel> downlinkMimoChannelModelPtr;
    shared_ptr<ScenSim::MimoChannelModel> uplinkMimoChannelModelPtr;
    shared_ptr<ScenSim::FrequencySelectiveFadingModel> downlinkFrequencySelectiveFadingModelPtr;
    shared_ptr<ScenSim::FrequencySelectiveFadingModel> uplinkFrequencySelectiveFadingModelPtr;

    shared_ptr<ScenSim::FrequencySelectiveFadingModel> frequencySelectiveFadingModelPtr;

    channelModelSetPtr->GetLteMimoOrFadingModelPtr(
        downlinkInstanceId,
        uplinkInstanceId,
        downlinkMimoChannelModelPtr,
        uplinkMimoChannelModelPtr,
        downlinkFrequencySelectiveFadingModelPtr,
        uplinkFrequencySelectiveFadingModelPtr);

    assert((downlinkMimoChannelModelPtr == nullptr) ||
           (downlinkFrequencySelectiveFadingModelPtr == nullptr));

    // Lte MAC/PHY
    interfaceToBeInitialized.macPtr =
        Lte::LteFactory(
            theParameterDatabaseReader,
            nodeId,
            interfaceId,
            networkLayerPtr->LookupInterfaceIndex(interfaceId),
            simulationEngineInterfacePtr,
            antenna.lteDownlinkPropagationModelInterfacePtr,
            antenna.lteUplinkPropagationModelInterfacePtr,
            downlinkMimoChannelModelPtr,
            uplinkMimoChannelModelPtr,
            downlinkFrequencySelectiveFadingModelPtr,
            uplinkFrequencySelectiveFadingModelPtr,
            channelModelSetPtr->GetLteBitOrBlockErrorRateCurveDatabase(),
            lteGlobalPtr,
            nodeSeed,
            networkLayerPtr,
            *appLayerPtr);

}//SetupLteInterface//




void SimNode::SetupWaveInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    using Dot11::Dot11Phy;

    const string phyDvicesNames = theParameterDatabaseReader.ReadString(
        "its-wave-device-names", nodeId, interfaceId);

    istringstream phyDeviceStream(phyDvicesNames);
    vector<WaveMacPhyInput> phyInputs;

    // WAVE interface can have a MAC with multiple PHY devices.
    while (!phyDeviceStream.eof()) {

        phyInputs.push_back(WaveMacPhyInput());
        WaveMacPhyInput& phyInput = phyInputs.back();

        phyDeviceStream >> phyInput.phyDeviceName;

        // save propagation interface index
        size_t propagationInterfaceIndex = 0;
        for(size_t i = 0; i < interfaces.size(); i++) {
            propagationInterfaceIndex += interfaces[i].antennaPtrs.size();
        }

        interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(phyInput.phyDeviceName)));
        AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

        // Antenna
        antenna.antennaModelPtr = CreateAntennaModel(
            theParameterDatabaseReader,
            nodeId,
            phyInput.phyDeviceName,
            *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

        // Antenna Mobility
        antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
            theParameterDatabaseReader,
            phyInput.phyDeviceName);

        // Propagation Interface
        const shared_ptr<Dot11::PropagationModel> propagationModelPtr =
            channelModelSetPtr->GetDot11PropagationModel(
                GetChannelInstanceId(theParameterDatabaseReader, nodeId, phyInput.phyDeviceName));

        antenna.dot11PropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
            simulationEngineInterfacePtr,
            antenna.antennaModelPtr,
            antenna.antennaMobilityModelPtr,
            nodeId,
            phyInput.phyDeviceName,
            static_cast<unsigned int>(propagationInterfaceIndex));

#ifdef ITS_EXTENSION
        // Dot11 PHY
        phyInput.phyPtr.reset(
            new Dot11Phy(
                theParameterDatabaseReader,
                nodeId,
                phyInput.phyDeviceName,
                simulationEngineInterfacePtr,
                antenna.dot11PropagationModelInterfacePtr,
                channelModelSetPtr->GetDot11BitOrBlockErrorRateCurveDatabase(),
                nodeSeed));
#endif

        const string channelNumbersString =
            theParameterDatabaseReader.ReadString(
                "its-wave-phy-support-channels", nodeId, phyInput.phyDeviceName);

        istringstream channelNumberStream(channelNumbersString);

        while (!channelNumberStream.eof()) {
            string channelNumberString;
            channelNumberStream >> channelNumberString;

            ChannelNumberIndexType channelNumberId;
            ChannelCategoryType channelCategory;

            GetChannelCategoryAndNumber(channelNumberString, channelCategory, channelNumberId);

            phyInput.channelNumberIds[channelCategory].push_back(channelNumberId);
        }//while//
    }//while//

    // WAVE MAC
    shared_ptr<WaveMac> waveMacPtr(
        new WaveMac(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            nodeId,
            interfaceId,
            interfaceIndex,
            networkLayerPtr,
            phyInputs,
            nodeSeed));

    interfaceToBeInitialized.macPtr = waveMacPtr;

    // WSMP
    interfaceToBeInitialized.wsmpLayerPtr.reset(
        new WsmpLayer(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            nodeId,
            interfaceId,
            (*this).GetNodeSeed(),
            nodeMobilityModelPtr,
            networkLayerPtr,
            waveMacPtr));

    if (theParameterDatabaseReader.ParameterExists("its-bsm-app-traffic-start-time", nodeId)) {
        (*this).GetAppLayerPtr()->AddApp(
            shared_ptr<DsrcMessageApplication>(
                new DsrcMessageApplication(
                    theParameterDatabaseReader,
                    simulationEngineInterfacePtr,
                    interfaceToBeInitialized.wsmpLayerPtr,
                    nodeId,
                    nodeSeed,
                    nodeMobilityModelPtr)));
    }//if//

}//SetupWaveInterface//

void SimNode::SetupGeoNetInterface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    using Dot11::Dot11Phy;

    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<Dot11::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetDot11PropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.dot11PropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId,
        interfaceId,
        interfaceIndex);

    shared_ptr<Dot11Phy> dot11PhyLayerPtr;

#ifdef ITS_EXTENSION
    // GeoNet PHY
    dot11PhyLayerPtr.reset(
        new Dot11Phy(
            theParameterDatabaseReader,
            nodeId,
            interfaceId,
            simulationEngineInterfacePtr,
            antenna.dot11PropagationModelInterfacePtr,
            channelModelSetPtr->GetDot11BitOrBlockErrorRateCurveDatabase(),
            nodeSeed));
#endif

    // GeoNet MAC
    shared_ptr<GeoNetMac> geoNetMacLayerPtr(
        new GeoNetMac(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            nodeId,
            interfaceId,
            interfaceIndex,
            networkLayerPtr,
            dot11PhyLayerPtr,
            nodeSeed));

    interfaceToBeInitialized.macPtr = geoNetMacLayerPtr;

    // GeoNet Network
    shared_ptr<GeoNetworkingProtocol> geoNetworkingPtr(
        new GeoNetworkingProtocol(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            nodeId,
            interfaceId,
            interfaceIndex,
            nodeMobilityModelPtr,
            geoNetMacLayerPtr,
            networkLayerPtr));

    shared_ptr<BasicTransportProtocol> btpPtr(
        new BasicTransportProtocol(
            simulationEngineInterfacePtr));

    btpPtr->ConnectToNetworkLayer(geoNetworkingPtr);

    // Application
    if (theParameterDatabaseReader.ParameterExists("its-broadcast-app-traffic-start-time", nodeId)) {

        shared_ptr<ItsBroadcastApplication> appPtr =
            shared_ptr<ItsBroadcastApplication>(
                new ItsBroadcastApplication(
                    theParameterDatabaseReader,
                    simulationEngineInterfacePtr,
                    nodeId,
                    interfaceId,
                    ItsBroadcastApplication::BTP_MODE,
                    btpPtr));

        (*this).GetAppLayerPtr()->AddApp(appPtr);
        appPtr->CompleteInitialization(theParameterDatabaseReader);
    }//if//

}//SetupGeoNetInterface//

void SimNode::SetupT109Interface(
    const ParameterDatabaseReader& theParameterDatabaseReader,
    const GlobalNetworkingObjectBag& theGlobalNetworkingObjectBag,
    const InterfaceIdType& interfaceId,
    const unsigned int interfaceIndex,
    const shared_ptr<ChannelModelSet>& channelModelSetPtr,
    InterfaceType& interfaceToBeInitialized)
{
    interfaceToBeInitialized.antennaPtrs.push_back(shared_ptr<AntennaType>(new AntennaType(interfaceId)));
    AntennaType& antenna = (*interfaceToBeInitialized.antennaPtrs.back());

    // Antenna
    antenna.antennaModelPtr = CreateAntennaModel(
        theParameterDatabaseReader,
        nodeId,
        interfaceId,
        *theGlobalNetworkingObjectBag.antennaPatternDatabasePtr);

    // Antenna Mobility
    antenna.antennaMobilityModelPtr = (*this).GetNodeOrAttachedMobility(
        theParameterDatabaseReader,
        interfaceId);

    // Propagation Interface
    const shared_ptr<T109::PropagationModel> propagationModelPtr =
        channelModelSetPtr->GetT109PropagationModel(
            GetChannelInstanceId(theParameterDatabaseReader, nodeId, interfaceId));

    antenna.t109PropagationModelInterfacePtr = propagationModelPtr->GetNewPropagationModelInterface(
        simulationEngineInterfacePtr,
        antenna.antennaModelPtr,
        antenna.antennaMobilityModelPtr,
        nodeId,
        interfaceId,
        interfaceIndex);

    // T109 Application/MAC/PHY
    shared_ptr<T109App> t109AppPtr(
        new T109App(
            theParameterDatabaseReader,
            simulationEngineInterfacePtr,
            antenna.t109PropagationModelInterfacePtr,
            channelModelSetPtr->GetT109BitOrBlockErrorRateCurveDatabase(),
            nodeId,
            interfaceId,
            interfaceIndex,
            (*this).GetNodeSeed(),
            nodeMobilityModelPtr));

    interfaceToBeInitialized.macPtr = t109AppPtr->GetMacPtr();

    (*this).GetAppLayerPtr()->AddApp(t109AppPtr);

    networkLayerPtr->SetInterfaceOutputQueue(
        interfaceIndex,
        shared_ptr<InterfaceOutputQueue>(new NoNetworkQueue()));
}//SetupT109Interface//

//----------------------------------------------------------------------------------------

void SimNode::CompleteAntennaNumberAssignment()
{
    unsigned int antennaNumber = 0;

    for(size_t interfraceNumber = 0; interfraceNumber < interfaces.size(); interfraceNumber++) {
        const vector<shared_ptr<AntennaType> >& antennaPtrs = interfaces[interfraceNumber].antennaPtrs;

        for(size_t i = 0; i < antennaPtrs.size(); i++) {
            antennaPtrs[i]->antennaNumber = antennaNumber;
            antennaNumber++;
        }//for//
    }//for//
}//CompleteAntennaNumberAssignment//

bool SimNode::HasAntenna(const InterfaceIdType& channelId) const
{
    for(size_t interfraceNumber = 0; interfraceNumber < interfaces.size(); interfraceNumber++) {
        const vector<shared_ptr<AntennaType> >& antennaPtrs = interfaces[interfraceNumber].antennaPtrs;

        for(size_t i = 0; i < antennaPtrs.size(); i++) {

            if (antennaPtrs[i]->channelModelId == channelId) {
                return true;
            }
        }
    }
    return false;
}//HasAntenna//

unsigned int SimNode::GetAntennaNumber(const InterfaceIdType& interfaceName)
{
    for(size_t interfraceNumber = 0; interfraceNumber < interfaces.size(); interfraceNumber++) {
        const vector<shared_ptr<AntennaType> >& antennaPtrs = interfaces[interfraceNumber].antennaPtrs;

        for(size_t i = 0; i < antennaPtrs.size(); i++) {
            const AntennaType& antenna = (*antennaPtrs[i]);

            if (antenna.channelModelId == interfaceName) {
                return antenna.antennaNumber;
            }
        }
    }

    cerr << "Invalid antenna instace name " << interfaceName << endl;
    exit(1);

    return static_cast<unsigned int>(-1);
}//GetAntennaIdNumber//

shared_ptr<SimNode::AntennaType> SimNode::GetAntennaPtr(const unsigned int antennaNumber) const
{
    for(size_t interfraceNumber = 0; interfraceNumber < interfaces.size(); interfraceNumber++) {
        const vector<shared_ptr<AntennaType> >& antennaPtrs = interfaces[interfraceNumber].antennaPtrs;

        for(size_t i = 0; i < antennaPtrs.size(); i++) {
            const shared_ptr<AntennaType>& antennaPtr = antennaPtrs[i];

            if (antennaPtr->antennaNumber == antennaNumber) {
                return antennaPtr;
            }
        }
    }

    cerr << "Invalid antenna number " << antennaNumber << endl;
    exit(1);

    return shared_ptr<AntennaType>();
}//GetAntennaPtr//

shared_ptr<AntennaModel> SimNode::GetAntennaModelPtr(const unsigned int antennaNumber) const
{
    return (*this).GetAntennaPtr(antennaNumber)->antennaModelPtr;
}//GetAntennaModelPtr//

ObjectMobilityPosition SimNode::GetAntennaLocation(const unsigned int antennaNumber) const
{
    const TimeType currentTime = simulationEngineInterfacePtr->CurrentTime();
    ObjectMobilityPosition position;
    GetAntennaPtr(antennaNumber)->antennaMobilityModelPtr->GetPositionForTime(currentTime, position);
    return (position);
}//GetAntennaLocation//

