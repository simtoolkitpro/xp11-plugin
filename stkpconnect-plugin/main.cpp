#include <cstdlib>
#include <QtGlobal>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <XPLMPlugin.h>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include "xplaneplugin.h"
#include "console.h"

XPlanePlugin *globalPlugin = nullptr;

// Sets up a new Qt message handler for to handle logging for the plugin. Logs are outputted to
// the file "stkpconnector.log" in the X-Plane root directory.
void stkpConnectMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString dateTime = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QString logMessage;
    switch (type) {
        case QtDebugMsg:
            logMessage = QString("%1 DEBUG: %2").arg(dateTime, msg);
            break;
        case QtInfoMsg:
            logMessage = QString("%1 INFO: %2").arg(dateTime, msg);
            break;
        case QtWarningMsg:
            logMessage = QString("%1 WARNING: %2").arg(dateTime, msg);
            break;
        case QtCriticalMsg:
            logMessage = QString("%1 CRITICAL: %2").arg(dateTime, msg);
            break;
        case QtFatalMsg:
            logMessage = QString("%1 FATAL: %2").arg(dateTime, msg);
            break;
    }

    // Output plugin logs to a file in <X-Plane root directory>/Output.
    QFile outFile("Output/stkpconnector.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream textStream(&outFile);
    textStream << logMessage << endl;
}

PLUGIN_API float MyFlightLoopCallback(
        float inElapsedSinceLastCall,
        float inElapsedTimeSinceLastFlightLoop,
        int inCounter,
        void *inRefcon) {
    if(globalPlugin)
        return globalPlugin->flightLoop(inElapsedSinceLastCall, inElapsedTimeSinceLastFlightLoop, inCounter, inRefcon);
    return 1;
}

PLUGIN_API int XPluginStart(
        char * outName,
        char * outSig,
        char *outDesc) {
    XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, 0.01f, NULL);
    Q_ASSERT(!globalPlugin);
    globalPlugin = new XPlanePlugin();

    // Install custom message handler.
    qInstallMessageHandler(stkpConnectMessageHandler);

    return globalPlugin->pluginStart(outName, outSig, outDesc);
}

PLUGIN_API void XPluginStop() {
    DEBUG;
    XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, 0);
    globalPlugin->pluginStop();
    delete globalPlugin;
    globalPlugin = nullptr;
}

PLUGIN_API void XPluginDisable() {
    DEBUG;
}

PLUGIN_API int XPluginEnable() {
    DEBUG;
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(
        XPLMPluginID inFromWho,
        long inMessage,
        void *inParam) {
    Q_UNUSED(inFromWho)
    Q_UNUSED(inMessage)
    Q_UNUSED(inParam)
}
