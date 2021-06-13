#include "xplaneplugin.h"
#include "../stkpconnect-server/datarefs/dataref.h"
#include "../stkpconnect-server/datarefs/floatdataref.h"
#include "../stkpconnect-server/datarefs/floatarraydataref.h"
#include "../stkpconnect-server/datarefs/intdataref.h"
#include "../stkpconnect-server/datarefs/intarraydataref.h"
#include "../stkpconnect-server/datarefs/doubledataref.h"
#include "../stkpconnect-server/datarefs/datadataref.h"
#include <console.h>
#include "customdata/navcustomdata.h"
#include "customdata/atccustomdata.h"
#include <clocale>
#include <cstring>
#include <XPLMUtilities.h>
#include <XPLMScenery.h>
#include <XPLMGraphics.h>
#include <math.h>

#define PI 3.141592653589793
#define DEG_TO_RAD_2 PI / 360.0

typedef struct _Eulers
{
	double psi;
	double the;
	double phi;
} Eulers;

typedef struct _Quaternion
{
	double w;
	double x;
	double y;
	double z;
} Quaternion;

XPlanePlugin::XPlanePlugin(QObject *parent) : QObject(parent)
                                              , argc(0)
                                              , argv(nullptr)
                                              , app(nullptr)
                                              , server(nullptr)
                                              , flightLoopInterval(1.0f / 60.f) // Default to 60hz
                                              , g_menu_container_idx(0)
{ }

XPlanePlugin::~XPlanePlugin() { }

float XPlanePlugin::flightLoop(float inElapsedSinceLastCall,
                               float inElapsedTimeSinceLastFlightLoop,
                               int inCounter,
                               void *inRefcon) {
    Q_UNUSED(inElapsedSinceLastCall)
    Q_UNUSED(inElapsedTimeSinceLastFlightLoop)
    Q_UNUSED(inCounter)
    Q_UNUSED(inRefcon)
    // Tell each dataref to update its value through the XPLM api
    for(DataRef *ref : refs) updateDataRef(ref);
    // Tell Qt to process it's own runloop
    // app->processEvents();
    return flightLoopInterval;
}

int XPlanePlugin::pluginStart(char * outName, char * outSig, char *outDesc) {
    // Set plugin info
    INFO << "Plugin started";
    std::strcpy(outName, "STKP-Connector");
    std::strcpy(outSig, "org.xyligo.stkpconnector");
    std::strcpy(outDesc, "STKP Network based connector.");

    g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "STKPConnect", nullptr, 0);
    g_menu_id = XPLMCreateMenu("STKPConnect", XPLMFindPluginsMenu(), g_menu_container_idx, nullptr, nullptr);
    XPLMAppendMenuItem(g_menu_id, "Listening on TCP port " STKPCONNECT_PORT_STR " with protocol " STKPCONNECT_PROTOCOL_STR " version " STKPCONNECT_VERSION_STR ". No GUI yet.", nullptr, 1);

    // Init application and server
    app = new QCoreApplication(argc, &argv);
    setlocale(LC_NUMERIC, "C"); // See http://stackoverflow.com/questions/25661295/why-does-qcoreapplication-call-setlocalelc-all-by-default-on-unix-linux

    server = new TcpServer(this, this);
    connect(server, &TcpServer::setFlightLoopInterval, this, &XPlanePlugin::setFlightLoopInterval);

    // Log that we have started
    XPLMDebugString ("STKPConnect listening on TCP port " STKPCONNECT_PORT_STR " with protocol " STKPCONNECT_PROTOCOL_STR " version " STKPCONNECT_VERSION_STR "\n");

    // Register the nav custom data accessors
    XPLMRegisterDataAccessor("stkpconnect/navdata/5km",
                             xplmType_Data,                                 // The types we support
                             0,                                             // Writable
                             nullptr, nullptr,                                    // Integer accessors
                             nullptr, nullptr,                                    // Float accessors
                             nullptr, nullptr,                                    // Doubles accessors
                             nullptr, nullptr,                                    // Int array accessors
                             nullptr, nullptr,                                    // Float array accessors
                             NavCustomData::DataCallback_5km, nullptr,         // Raw data accessors
                             nullptr, nullptr);                                   // Refcons not used
    XPLMRegisterDataAccessor("stkpconnect/navdata/20km",
                             xplmType_Data,                                 // The types we support
                             0,                                             // Writable
                             nullptr, nullptr,                                    // Integer accessors
                             nullptr, nullptr,                                    // Float accessors
                             nullptr, nullptr,                                    // Doubles accessors
                             nullptr, nullptr,                                    // Int array accessors
                             nullptr, nullptr,                                    // Float array accessors
                             NavCustomData::DataCallback_20km, nullptr,        // Raw data accessors
                             nullptr, nullptr);                                   // Refcons not used
    XPLMRegisterDataAccessor("stkpconnect/navdata/100km",
                             xplmType_Data,                                 // The types we support
                             0,                                             // Writable
                             nullptr, nullptr,                                    // Integer accessors
                             nullptr, nullptr,                                    // Float accessors
                             nullptr, nullptr,                                    // Doubles accessors
                             nullptr, nullptr,                                    // Int array accessors
                             nullptr, nullptr,                                    // Float array accessors
                             NavCustomData::DataCallback_100km, nullptr,       // Raw data accessors
                             nullptr, nullptr);                                   // Refcons not used
    XPLMRegisterDataAccessor("stkpconnect/atc/124thatc/latest",
                             xplmType_Data,                                 // The types we support
                             0,                                             // Writable
                             nullptr, nullptr,                                    // Integer accessors
                             nullptr, nullptr,                                    // Float accessors
                             nullptr, nullptr,                                    // Doubles accessors
                             nullptr, nullptr,                                    // Int array accessors
                             nullptr, nullptr,                                    // Float array accessors
                             ATCCustomData::DataCallback, nullptr,             // Raw data accessors
                             nullptr, nullptr);

    app->processEvents();
    return 1;
}

DataRef* XPlanePlugin::subscribeRef(QString &name) {
    // Search in list of already subscribed datarefs - if found return that
    for(DataRef *ref : refs) {
        if(ref->name() == name) {
            DEBUG << "Already subscribed to " << name;
            ref->setSubscriberCount(ref->subscriberCount() + 1);
            return ref;
        }
    }

    // Not yet subscribed - create a new dataref
    QString realName = refNameWithoutModifiers(name);
    XPLMDataRef ref = XPLMFindDataRef(realName.toLatin1());
    if(ref) {
        XPLMDataTypeID refType = XPLMGetDataRefTypes(ref);
        DataRef *dr = nullptr;
        if(refType & xplmType_Double) {
            dr = new DoubleDataRef(this, name, ref);
        } else if(refType & xplmType_Float) {
            dr = new FloatDataRef(this, name, ref);
        } else if(refType & xplmType_Int) {
            dr = new IntDataRef(this, name, ref);
        } else if (refType & xplmType_FloatArray) {
            dr = new FloatArrayDataRef(this, name, ref);
        } else if (refType & xplmType_IntArray) {
            dr = new IntArrayDataRef(this, name, ref);
        } else if (refType & xplmType_Data) {
            dr = new DataDataRef(this, name, ref);
        }
        if(dr) {
            dr->setSubscriberCount(1);
            dr->setWritable(XPLMCanWriteDataRef(ref));
            DEBUG << "Subscribed to ref " << dr->name()
                  << ", type: " << dr->typeString()
                  << ", writable:" << dr->isWritable();
            refs.append(dr);
            return dr;
        } else {
            server->stkpconnectWarning(QString("Dataref type %1 not supported").arg(refType));
        }
    } else {
        server->stkpconnectWarning(QString("Can't find dataref %1").arg(name));
    }
    return nullptr;
}

void XPlanePlugin::unsubscribeRef(DataRef *ref) {
    Q_ASSERT(refs.contains(ref));

    ref->setSubscriberCount(ref->subscriberCount() - 1);
    if(ref->subscriberCount() == 0) {
        refs.removeOne(ref);
        DEBUG << "Ref " << ref->name() << " not subscribed by anyone - removing.";
        ref->deleteLater();
    }
}

// Called for each ref on every flight loop
void XPlanePlugin::updateDataRef(DataRef *ref) {
    Q_ASSERT(ref);

    switch (ref->type()) {
    case stkpconnectRefTypeFloat:
    {
        float newValue = XPLMGetDataf(ref->ref());
        qobject_cast<FloatDataRef*>(ref)->updateValue(newValue);
        break;
    }
    case stkpconnectRefTypeFloatArray:
    {
        FloatArrayDataRef *faRef = qobject_cast<FloatArrayDataRef*>(ref);
        int arrayLength = faRef->value().length();
        if(arrayLength == 0) {
            arrayLength = XPLMGetDatavf(faRef->ref(), nullptr, 0, 0);
            faRef->setLength(arrayLength);
        }
        int valuesCopied = XPLMGetDatavf(faRef->ref(), faRef->valueArray(), 0, arrayLength);
        Q_ASSERT(valuesCopied == arrayLength);
        faRef->updateValue();
        break;
    }
    case stkpconnectRefTypeIntArray:
    {
        IntArrayDataRef *iaRef = qobject_cast<IntArrayDataRef*>(ref);
        int arrayLength = iaRef->value().length();
        if(arrayLength <= 0) {
            arrayLength = XPLMGetDatavi(iaRef->ref(), nullptr, 0, 0);
            iaRef->setLength(arrayLength);
        }
        int valuesCopied = XPLMGetDatavi(iaRef->ref(), iaRef->valueArray(), 0, arrayLength);
        Q_ASSERT(valuesCopied == arrayLength);
        iaRef->updateValue();
        break;
    }
    case stkpconnectRefTypeInt:
    {
        int newValue = XPLMGetDatai(ref->ref());
        qobject_cast<IntDataRef*>(ref)->updateValue(newValue);
        break;
    }
    case stkpconnectRefTypeDouble:
    {
        double newValue = XPLMGetDatad(ref->ref());
        qobject_cast<DoubleDataRef*>(ref)->updateValue(newValue);
        break;
    }
    case stkpconnectRefTypeData:
    {
        DataDataRef *bRef = qobject_cast<DataDataRef*>(ref);
        Q_ASSERT(bRef);
        int arrayLength = XPLMGetDatab(ref->ref(), nullptr, 0, 0);
        bRef->setLength(arrayLength);
        int valuesCopied = XPLMGetDatab(ref->ref(), bRef->newValue().data(), 0, arrayLength);
        Q_ASSERT(valuesCopied == arrayLength);
        bRef->updateValue();
        break;
    }
    default:
        break;
    }
}

void XPlanePlugin::keyStroke(int keyid) {
    DEBUG << keyid;
    XPLMCommandKeyStroke(keyid);
}

void XPlanePlugin::buttonPress(int buttonid) {
    DEBUG << buttonid;
    XPLMCommandButtonPress(buttonid);
}

void XPlanePlugin::buttonRelease(int buttonid) {
    DEBUG << buttonid;
    XPLMCommandButtonRelease(buttonid);
}

void XPlanePlugin::changeDataRef(DataRef *ref)
{
    if(!ref->isWritable()) {
        server->stkpconnectWarning(QString("Tried to write read-only dataref %1").arg(ref->name()));
        return;
    }

    switch (ref->type()) {
    case stkpconnectRefTypeFloat:
    {
        XPLMSetDataf(ref->ref(), qobject_cast<FloatDataRef*>(ref)->value());
        break;
    }
    case stkpconnectRefTypeFloatArray:
    {
        FloatArrayDataRef *faRef = qobject_cast<FloatArrayDataRef*>(ref);
        XPLMSetDatavf(ref->ref(), faRef->valueArray(), 0, faRef->value().length());
        break;
    }
    case stkpconnectRefTypeIntArray:
    {
        IntArrayDataRef *iaRef = qobject_cast<IntArrayDataRef*>(ref);
        XPLMSetDatavi(ref->ref(), iaRef->valueArray(), 0, iaRef->value().length());
        break;
    }
    case stkpconnectRefTypeInt:
    {
        XPLMSetDatai(ref->ref(), qobject_cast<IntDataRef*>(ref)->value());
        break;
    }
    case stkpconnectRefTypeDouble:
    {
        XPLMSetDatad(ref->ref(), qobject_cast<DoubleDataRef*>(ref)->value());
        break;
    }
    default:
        break;
    }
}

void XPlanePlugin::command(QString &name, stkpconnectCommandType type)
{
    XPLMCommandRef cmdRef = XPLMFindCommand(name.toUtf8().constData());
    if (cmdRef) {
        switch (type) {
        case stkpconnectCommandTypeOnce:
            XPLMCommandOnce(cmdRef);
            break;
        case stkpconnectCommandTypeBegin:
            XPLMCommandBegin(cmdRef);
            break;
        case stkpconnectCommandTypeEnd:
            XPLMCommandEnd(cmdRef);
            break;
        default:
            break;
        }
    } else {
        server->stkpconnectWarning(QString("Command %1 not found").arg(name));
    }

}

void XPlanePlugin::setFlightLoopInterval(float newInterval) {
    if(newInterval > 0) {
        flightLoopInterval = newInterval;
        DEBUG << "New interval" << flightLoopInterval;
    } else {
        server->stkpconnectWarning(QString("Invalid interval %1").arg(newInterval));
    }
}

QString XPlanePlugin::refNameWithoutModifiers(QString &original) {
    return original.contains(":") ? original.left(original.indexOf(":")) : original;
}

/**
 * @brief XPlanePlugin::loadSituation
 * @param name :  situation file location -
 * relative to XPlane root folder, e.g. Output/situations/XXX.sit
 */
bool XPlanePlugin::loadSituation(QString sitFileLocation) {

    // Remove quotes from filename
    sitFileLocation = sitFileLocation.replace("\"", "");

    // XPLMLoadDataFile's return value is not documented, assuming it returns
    // 1 on success and 0 on fail. TODO: Check this.

    return XPLMLoadDataFile(xplm_DataFile_Situation, sitFileLocation.toUtf8().data());
}

/**
 * @brief XPlanePlugin::addFMSEntryLatLon
 * @param fmsEntryLine - string in format "id,latitude,longitude,altitude"
 * example: "1,50.0267,8.51,198"
 */
void XPlanePlugin::addFMSEntryLatLon(QString fmsEntryLine){
    //verify if string is in valid format
    int commaCount = fmsEntryLine.count(QLatin1Char(','));
    if(commaCount != 3){
        return;
    }

    QStringList params = fmsEntryLine.split(",", QString::SkipEmptyParts);
    int id = params.value(0).toInt();
    float lat = params.value(1).toFloat();
    float lon = params.value(2).toFloat();
    int alt = params.value(3).toInt();

    XPLMSetFMSEntryLatLon(id, lat, lon, alt);
}

/**
 * @brief XPlanePlugin::clearAllFmsEntries
 * removes all entries inserted to FMS
 */
void XPlanePlugin::clearAllFmsEntries(){
    int count = XPLMCountFMSEntries();
    for(int i = 0; i < count;i++){
        XPLMClearFMSEntry(i);
    }
}
/**
 * @brief XPlanePlugin::setDestinationFmsEntry
 * @param index entry the FMS is flying the aircraft toward.
 */
void XPlanePlugin::setDestinationFmsEntry(int index) {
    XPLMSetDestinationFMSEntry(index);
}

void XPlanePlugin::move(QString l){
    //verify if string is in valid format
    QStringList parts = l.split(",", QString::SkipEmptyParts);
    double lat = parts.value(0).toDouble();
    double lon = parts.value(1).toDouble();
    double xalt = parts.value(2).toDouble();
    double head = parts.value(3).toDouble();
    double spd = parts.value(4).toDouble();
    XPLMProbeRef ref_probe;

    int _xy = 1;
    XPLMSetDatavi(XPLMFindDataRef("sim/operation/override/override_planepath"), &_xy, 0, 1);

    double x, y, z, alt, foo;
    ref_probe = XPLMCreateProbe(xplm_ProbeY);
    XPLMProbeInfo_t probeinfo;
    
    probeinfo.structSize = sizeof(XPLMProbeInfo_t);
    XPLMWorldToLocal(lat, lon, xalt, &x, &y, &z);	// 1
    XPLMProbeTerrainXYZ(ref_probe, x, y, z, &probeinfo);			// 2
    XPLMLocalToWorld(probeinfo.locationX, probeinfo.locationY, probeinfo.locationZ, &foo, &foo, &alt);	// 3

    XPLMWorldToLocal(lat, lon, alt, &x, &y, &z);

    if (x == 0 || y == 0 || z == 0) {
        return;
    }

    XPLMSetDatad(XPLMFindDataRef("sim/flightmodel/position/local_x"), x);
    XPLMSetDatad(XPLMFindDataRef("sim/flightmodel/position/local_y"), y + (xalt / 3.2804));
    XPLMSetDatad(XPLMFindDataRef("sim/flightmodel/position/local_z"), z);

    Quaternion q;
    Eulers ypr;
    ypr.psi = head;
    ypr.phi = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/phi"));
    ypr.the = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/theta"));
    char buffer[256];  // make sure this is big enough!!!
    snprintf(buffer, sizeof(buffer), "Input Data (%9.7f, %9.7f, %9.7f, %9.7f, %9.7f)\n", lat, lon, xalt, head, spd);
    XPLMDebugString(buffer);
    snprintf(buffer, sizeof(buffer), "(%9.7f, %9.7f, %9.7f)\n", ypr.psi, ypr.phi, ypr.the);
    XPLMDebugString(buffer);

    double spsi = sin(ypr.psi * DEG_TO_RAD_2);
    double sthe = sin(ypr.the * DEG_TO_RAD_2);
    double sphi = sin(ypr.phi * DEG_TO_RAD_2);
    double cpsi = cos(ypr.psi * DEG_TO_RAD_2);
    double cthe = cos(ypr.the * DEG_TO_RAD_2);
    double cphi = cos(ypr.phi * DEG_TO_RAD_2);
    snprintf(buffer, sizeof(buffer), "(%9.7f, %9.7f, %9.7f, %9.7f, %9.7f, %9.7f)\n", spsi, sthe, sphi, cpsi, cthe, cphi);
    XPLMDebugString(buffer);

    double qw = cphi * cthe * cpsi + sphi * sthe * spsi;
    double qx = sphi * cthe * cpsi - cphi * sthe * spsi;
    double qy = sphi * cthe * spsi + cphi * sthe * cpsi;
    double qz = cphi * cthe * spsi - sphi * sthe * cpsi;
    snprintf(buffer, sizeof(buffer), "(%9.7f, %9.7f, %9.7f, %9.7f)\n", qw, qx, qy, qz);
    XPLMDebugString(buffer);

    double e = sqrt(qw * qw + qx * qx + qy * qy + qz * qz);
    snprintf(buffer, sizeof(buffer), "(%9.7f)\n", e);
    XPLMDebugString(buffer);

    q.w = qw / e;
    q.x = qx / e;
    q.y = qy / e;
    q.z = qz / e;
    snprintf(buffer, sizeof(buffer), "(%9.7f, %9.7f, %9.7f, %9.7f)\n", q.w, q.x, q.y, q.z);
    XPLMDebugString(buffer);

    XPLMDataRef gPlaneQuat; // The declarations.
    float gtempq1[4];

    gPlaneQuat = XPLMFindDataRef("sim/flightmodel/position/q"); // The assignment.
    memset(gtempq1, 0, sizeof(gtempq1));

    gtempq1[0] = q.w; // Load up the array.
    gtempq1[1] = 0;
    gtempq1[2] = 0;
    gtempq1[3] = q.z;
                    
    snprintf(buffer, sizeof(buffer), "(%9.7f, %9.7f, %9.7f, %9.7f - in speed: %9.7f)\n", q.w, q.x, q.y, q.z, spd);
    XPLMDebugString(buffer);

    _xy = 0;
    XPLMSetDatavi(XPLMFindDataRef("sim/operation/override/override_planepath"), &_xy, 0, 1);
    XPLMSetDatavf(gPlaneQuat, gtempq1, 0, 4);

    if (spd > 5.0) {

        float indicated_airspeed = spd * 0.5144; //THIS CONVERTS KNOTS TO SI UNITS

        double pressure = 101325 * exp(-9.80665*0.0289644*xalt / (8.31447 * 220));
        double density = pressure / (287.05 * 220);
        double true_velocity = indicated_airspeed / sqrt(density / 1.225);

        snprintf(buffer, sizeof(buffer), "(True Vel :%9.7f)\n", true_velocity);
        XPLMDebugString(buffer);

        //Assuming no additional vertical velocity
        const double degToRadFactor = PI / 180;
        
        double vx = true_velocity*sin(head * degToRadFactor);
        double vy = 0;
        double vz = -true_velocity*cos(head * degToRadFactor);

        XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/position/local_vx"), (float)vx);
        XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/position/local_vy"), (float)vy);
        XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/position/local_vz"), (float)vz);
        snprintf(buffer, sizeof(buffer), "(%9.7f, %9.7f, %9.7f)\n", vx, vy, vz);
        XPLMDebugString(buffer);
    }

    _xy = 0;
    XPLMSetDatavi(XPLMFindDataRef("sim/operation/override/override_planepath"), &_xy, 0, 1);
}

void XPlanePlugin::pluginStop() {
    DEBUG;
    XPLMDestroyMenu(g_menu_id);
    app->processEvents();
    server->disconnectClients();
    delete server;
    server = nullptr;
    app->quit();
    app->processEvents();
    delete app;
    app = nullptr;
    qDeleteAll(refs);
    refs.clear();
}
