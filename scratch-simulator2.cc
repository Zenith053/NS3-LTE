#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/config-store-module.h>
#include <ns3/node-container.h>
#include "ns3/internet-module.h"
#include "ns3/epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"
// #include "ns3/mobility-module.h"
// #include <constant-position-mobility-model.h>

using namespace ns3;

int main(int argc, char *argv[]){
    CommandLine cmd;
    cmd.Parse (argc, argv);

    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();

    cmd.Parse (argc, argv);
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

    NodeContainer enbNodes;
    enbNodes.Create(4);
    NodeContainer ueNodes;
    ueNodes.Create(40);

    //placing enodebs in a square topology

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(ueNodes);


    //enodeb 0
    Ptr<ListPositionAllocator> loc0 = CreateObject<ListPositionAllocator>();
    loc0 -> Add(Vector(0,0,0));
    loc0 -> Add(Vector(5000,0,0));
    loc0 -> Add(Vector(5000,5000,0));
    loc0 -> Add(Vector(0,5000,0));
    mobility.SetPositionAllocator(loc0);
    mobility.Install(enbNodes);

    //setting up UL and DL RB
    lteHelper->SetEnbDeviceAttribute("DlBandwidth",UintegerValue(50));
    lteHelper->SetEnbDeviceAttribute("UlBandwidth",UintegerValue(50));


     //installing lte protocol stack on Enbs
    NetDeviceContainer enbDevs;
    enbDevs = lteHelper->InstallEnbDevice(enbNodes);

    //installing lte protocol stack on UEs
    NetDeviceContainer ueDevs;
    ueDevs = lteHelper->InstallUeDevice(ueNodes);


    //setting up transmission power in Enbs and UE

    Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(40.0));

    // dynamically allocate UE's

    
    //if the speed is 0

    for (int j = 0; j < 10; j++)
    {
        lteHelper->Attach(ueDevs.Get(j), enbDevs.Get(0));
    }

    for (int j = 10; j < 20; j++)
    {
        lteHelper->Attach(ueDevs.Get(j), enbDevs.Get(1));
    }

    for (int j = 20; j < 30; j++)
    {
        lteHelper->Attach(ueDevs.Get(j), enbDevs.Get(2));
    }

    for (int j = 30; j < 40; j++)
    {
        lteHelper->Attach(ueDevs.Get(j), enbDevs.Get(3));
    }

    //creating EPC network with interconnected nodes

    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper (epcHelper);


    //accessing the Pgateway that was created in the epcHelper class
    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    //setting up remote host
    NodeContainer InternetNode;
    InternetNode.Create (1);
    InternetStackHelper internet;
    internet.Install (InternetNode);

    Ptr<Node> remHost = InternetNode.Get(0);
    //connecting the pgw with 1Gbps link speed to remote host
    
    PointToPointHelper p2p;    
    p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate("1Gb/s")));
    // p2p.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    // p2p.SetChannelAttribute ("Delay", TimeValue (Seconds(0.010)));
    NetDeviceContainer internetDevices = p2p.Install(pgw,remHost);

    Ipv4AddressHelper ip;
    ip.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer ipinterface = ip.Assign(internetDevices);
    Ipv4Address hostaddr = ipinterface.GetAddress(1);
    Ipv4StaticRoutingHelper iproute;

    //adding static routing to remote host for simplification 
    Ptr<Ipv4StaticRouting> ihostroute= iproute.GetStaticRouting(remHost->GetObject<Ipv4> ());
    ihostroute->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"),Ipv4Mask ("255.0.0.0"), 1);

    //assigning internet to UE's
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIPs;
    ueIPs = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));

    for(uint32_t i=0;i<ueNodes.GetN();++i){
        Ptr<Node> ue = ueNodes.Get (i);
        Ptr<Ipv4StaticRouting> ueroute = iproute.GetStaticRouting(ue->GetObject<Ipv4> ());
        ueroute->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    //Data radio bearer
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    // lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

    // configuring rem plot
    Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper>();
    remHelper->SetAttribute("Channel", PointerValue(lteHelper->GetDownlinkSpectrumChannel()));
    remHelper->SetAttribute("OutputFile", StringValue("rem.out"));
    remHelper->SetAttribute("XMin", DoubleValue(-400.0));
    remHelper->SetAttribute("XMax", DoubleValue(400.0));
    remHelper->SetAttribute("XRes", UintegerValue(100));
    remHelper->SetAttribute("YMin", DoubleValue(-300.0));
    remHelper->SetAttribute("YMax", DoubleValue(300.0));
    remHelper->SetAttribute("YRes", UintegerValue(75));
    remHelper->SetAttribute("Z", DoubleValue(0.0));
    remHelper->SetAttribute("UseDataChannel", BooleanValue(true));
    remHelper->SetAttribute("RbId", IntegerValue(10));
    remHelper->Install();

//    lteHelper->EnablePhyTraces();
   lteHelper->EnableMacTraces();
   lteHelper->EnableRlcTraces();
   lteHelper->EnablePdcpTraces();
    // lteHelper->EnableRrcTraces();

    Simulator::Stop(Seconds(5));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}