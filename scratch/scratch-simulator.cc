#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store-module.h>
#include <ns3/spectrum-module.h>

//NS_LOG_COMPONENT_DEFINE("abcd");

using namespace ns3;

int main(int argc, char *argv[]){
    CommandLine cmd;
    cmd.Parse (argc, argv);

    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();

    cmd.Parse (argc, argv);

//    LogComponentEnable("abcd", LOG_LEVEL_INFO);

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
//    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
//    lteHelper->SetEpcHelper(epcHelper);

    NodeContainer enbNodes;
    enbNodes.Create(4);
    NodeContainer ueNodes;
    ueNodes.Create(40);

    //Setting up Frequency bands for uplink and downlink

//    lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(100));
//    lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(18100));

    //configuring transmission power and uplink power control

    Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(40.0));
//    Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(20.0));
//    Config::SetDefault("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue(true));

    //configuring mimo

//    Config::SetDefault("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue(1));

    //resource scheduling - round robin

//    lteHelper->SetSchedulerType(("ns3::FdMtFfMacScheduler"));

    //handover algorithms

//    lteHelper->SetHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
//    lteHelper->SetHandoverAlgorithmAttribute("ServingCellThreshold", UintegerValue(1));
//    lteHelper->SetHandoverAlgorithmAttribute("NeighbourCellOffset", UintegerValue(1));

    //interference algorithms

//    lteHelper->SetFfrAlgorithmType("ns3::LteFrHardAlgorithm");
//
    //placing enodebs in a square topology

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    //enodeb in topology
    Ptr<ListPositionAllocator> loc0 = CreateObject<ListPositionAllocator>();
    loc0 -> Add(Vector(-2500,-2500,0));
    loc0 -> Add(Vector(2500,-2500,0));
    loc0 -> Add(Vector(2500,2500,0));
    loc0 -> Add(Vector(-2500,2500,0));
    mobility.SetPositionAllocator(loc0);
    mobility.Install(enbNodes);

    //for uenodes defining stationary mobility model
    MobilityHelper mobility1;
    mobility1.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-5000, 5000, -5000, 5000)),
                              "Time", StringValue("1s"),
                              "Mode", StringValue("Time"),
                              "Speed", StringValue("ns3::ConstantRandomVariable[Constant=10.0]"));

    //first 10 ues
    mobility1.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                   "X", StringValue("-2500"),
                                   "Y", StringValue("-2500.0"),
                                   "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));

    for (int i = 0; i < 10; i++) {
        mobility1.Install(ueNodes.Get(i));
    }

    //10 - 20 ues
    mobility1.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                   "X", StringValue("2500"),
                                   "Y", StringValue("-2500.0"),
                                   "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));

    for (int i = 10; i < 20; i++) {
        mobility1.Install(ueNodes.Get(i));
    }

    //20-30 ues
    mobility1.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                   "X", StringValue("2500"),
                                   "Y", StringValue("2500.0"),
                                   "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));

    for (int i = 20; i < 30; i++) {
        mobility1.Install(ueNodes.Get(i));
    }

    //30-40 ues
    mobility1.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                   "X", StringValue("-2500"),
                                   "Y", StringValue("2500.0"),
                                   "Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=500]"));

    for (int i = 30; i < 40; i++) {
        mobility1.Install(ueNodes.Get(i));
    }

    //installing lte protocol stack on eNBs
    NetDeviceContainer enbDevs;
    enbDevs = lteHelper->InstallEnbDevice(enbNodes);

    //installing lte protocol stack on UEs
    NetDeviceContainer ueDevs;
    ueDevs = lteHelper->InstallUeDevice(ueNodes);

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

    //Data radio bearer
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    lteHelper->ActivateDataRadioBearer(ueDevs, bearer);


    // configuring rem plot
    Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper>();
//    remHelper->SetAttribute("Channel", PointerValue(lteHelper->GetDownlinkSpectrumChannel()));
    remHelper->SetAttribute("ChannelPath", StringValue ("/ChannelList/0"));
    remHelper->SetAttribute("OutputFile", StringValue("rem.out"));
    remHelper->SetAttribute("XMin", DoubleValue(-5000.0));
    remHelper->SetAttribute("XMax", DoubleValue(5000.0));
//    remHelper->SetAttribute("XRes", UintegerValue(100));
    remHelper->SetAttribute("YMin", DoubleValue(-5000.0));
    remHelper->SetAttribute("YMax", DoubleValue(5000.0));
//    remHelper->SetAttribute("YRes", UintegerValue(75));
    remHelper->SetAttribute("Z", DoubleValue(0.0));
//    remHelper->SetAttribute("UseDataChannel", BooleanValue(true));
//    remHelper->SetAttribute("RbId", IntegerValue(10));
    remHelper->Install();

    //Enable traces

//    lteHelper->EnablePhyTraces();
//    lteHelper->EnableMacTraces();
//    lteHelper->EnableRlcTraces();
//    lteHelper->EnablePdcpTraces();
//    lteHelper->EnableTraces();

    Simulator::Stop(Seconds(0.005));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
