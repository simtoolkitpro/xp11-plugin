
echo "STKPConnect Fat Plugin Assembler"

# Config
VERSION="0.1"
PLATFORM="osx"
ARC="64"

QT_LIB_PATH="/usr/local/Cellar/qt/5.15.0/lib"
STKPCONNECT_PROJECT_PATH="/Users/dan/stkp-xp11-connectplugin"
PLUGIN_NAME="stkpconnector"
PACKAGE_PATH="/Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector"

# Show config
echo " "
echo "VERSION = $VERSION";
echo "PLATFORM = $PLATFORM";
echo "ARC = $ARC";
echo "QT_LIB_PATH = $QT_LIB_PATH";
echo "STKPCONNECT_PROJECT_PATH = $STKPCONNECT_PROJECT_PATH";
echo "PACKAGE_PATH = $PACKAGE_PATH";

echo "Copying Qt Frameworks..."
# rm -rf $PACKAGE_PATH/*.framework
# cp -Rf /usr/local/Cellar/qt/5.15.0/lib/QtCore.framework $PACKAGE_PATH/
# cp -Rf /usr/local/Cellar/qt/5.15.0/lib/QtNetwork.framework $PACKAGE_PATH/

echo "Change framework ids..."
install_name_tool -id @executable_path/../../../Resources/plugins/stkpconnector/QtCore.framework/Versions/5/QtCore /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtCore.framework/Versions/5/QtCore
install_name_tool -id @executable_path/../../../Resources/plugins/stkpconnector/QtNetwork.framework/Versions/5/QtNetwork /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtNetwork.framework/Versions/5/QtNetwork

# Change framework paths in QtNetwork
echo "Changing framework paths in QtNetwork.framework..."
install_name_tool -change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @executable_path/../../../Resources/plugins/stkpconnector/QtCore.framework/Versions/5/QtCore /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtNetwork.framework/Versions/5/QtNetwork

echo "Changing framework paths in xpl..."
install_name_tool -change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @executable_path/../../../Resources/plugins/stkpconnector/QtCore.framework/Versions/5/QtCore /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/mac.xpl
install_name_tool -change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../../../Resources/plugins/stkpconnector/QtNetwork.framework/Versions/5/QtNetwork /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/mac.xpl
#install_name_tool -change @executable_path/../../../Resources/plugins/XPLM.framework/XPLM @executable_path/../../XPLM.framework/XPLM $PACKAGE_PATH/$ARC/mac.xpl

echo "Stripping debug symbols..."
find /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtNetwork.framework -iname *_debug* -exec rm -rf {} \;
find /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtCore.framework -iname *_debug* -exec rm -rf {} \;

# THIS ALL NEEDS A MAJOR CLEANUP!

install_name_tool -change /usr/local/Cellar/qt/5.15.0/lib/QtCore.framework/Versions/5/QtCore @executable_path/../../../Resources/plugins/stkpconnector/QtCore.framework/Versions/5/QtCore /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtNetwork.framework/Versions/5/QtNetwork 
install_name_tool -change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @executable_path/../../../Resources/plugins/stkpconnector/QtCore.framework/Versions/5/QtCore /Users/dan/stkp-xp11-connectplugin/stkpconnect-plugin/stkpconnector/QtNetwork.framework/Versions/5/QtNetwork

# Show bindings...
otool -L $PACKAGE_PATH/mac.xpl

echo "Done."

