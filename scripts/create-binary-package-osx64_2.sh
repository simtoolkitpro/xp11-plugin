
echo "STKPConnect Fat Plugin Assembler"

# Config
VERSION="0.1"
PLATFORM="osx"
ARC="64"

QT_LIB_PATH="/Users/dan/qt5-static/lib"
STKPCONNECT_PROJECT_PATH="/Users/dan/stkp-xp11-connectplugin"
PLUGIN_NAME="STKPConnect"
PACKAGE_PATH="/Users/dan/stkpconnect-packages/$PLUGIN_NAME"

# Show config
echo " "
echo "VERSION = $VERSION";
echo "PLATFORM = $PLATFORM";
echo "ARC = $ARC";
echo "QT_LIB_PATH = $QT_LIB_PATH";
echo "STKPCONNECT_PROJECT_PATH = $STKPCONNECT_PROJECT_PATH";
echo "PACKAGE_PATH = $PACKAGE_PATH";

echo "Copying Qt Frameworks..."
rm -rf $PACKAGE_PATH/$ARC/*.framework
cp -Rf $QT_LIB_PATH/QtCore.framework $PACKAGE_PATH/$ARC/
cp -Rf $QT_LIB_PATH/QtNetwork.framework $PACKAGE_PATH/$ARC/

echo "Change framework ids..."
LINK_PATH="@executable_path/../../../Resources/plugins/$PLUGIN_NAME/$ARC"
install_name_tool -id $LINK_PATH/QtCore.framework/Versions/5/QtCore $PACKAGE_PATH/$ARC/QtCore.framework/Versions/5/QtCore
install_name_tool -id $LINK_PATH/QtNetwork.framework/Versions/5/QtNetwork $PACKAGE_PATH/$ARC/QtNetwork.framework/Versions/5/QtNetwork

# Change framework paths in QtNetwork
echo "Changing framework paths in QtNetwork.framework..."
install_name_tool -change $QT_LIB_PATH/QtCore.framework/Versions/5/QtCore $LINK_PATH/QtCore.framework/Versions/5/QtCore $PACKAGE_PATH/$ARC/QtNetwork.framework/Versions/5/QtNetwork

echo "Changing framework paths in xpl..."
install_name_tool -change $QT_LIB_PATH/QtCore.framework/Versions/5/QtCore $LINK_PATH/QtCore.framework/Versions/5/QtCore $PACKAGE_PATH/$ARC/mac.xpl
install_name_tool -change $QT_LIB_PATH/QtNetwork.framework/Versions/5/QtNetwork $LINK_PATH/QtNetwork.framework/Versions/5/QtNetwork $PACKAGE_PATH/$ARC/mac.xpl
#install_name_tool -change @executable_path/../../../Resources/plugins/XPLM.framework/XPLM @executable_path/../../XPLM.framework/XPLM $PACKAGE_PATH/$ARC/mac.xpl

echo "Stripping debug symbols..."
find $PACKAGE_PATH/$ARC/QtNetwork.framework -iname *_debug* -exec rm -rf {} \;
find $PACKAGE_PATH/$ARC/QtCore.framework -iname *_debug* -exec rm -rf {} \;

# Show bindings...
#otool -L $PACKAGE_PATH/$ARC/mac.xpl

echo "Done."

