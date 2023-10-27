#! /usr/bin/bash

# This script compiles and installs most libraries into "external" directory
SCRIPT_PATH=$(pwd)/$0
SCRIPT_PATH=$(dirname $SCRIPT_PATH)

INSTALLATION_PATH=$SCRIPT_PATH/installation
BASE_INSTALL_PATH=$SCRIPT_PATH
BASE_BUILD_PATH=$SCRIPT_PATH/build
CONFIGS="Release Debug"

# clean previous files
for FILE in assimp build freetype2 glfw glm json tinyxml2 boost_1_83_0 FreeImage glew-2.2.0 ImGui irrKlang-64bit-1.6.0 lua-5.4.2 MSVC_Redist zlib-1.3
do
  rm -fr $SCRIPT_PATH/$FILE
done

# create the build directory
if [ ! -d $BASE_BUILD_PATH ]
then
	mkdir -p $BASE_BUILD_PATH
fi

###########################################################################
# Unzip libraries
##########################################################################

echo "========================================================="
echo " Unzip files"
echo "========================================================="	

unzip $INSTALLATION_PATH/external_zip/FreeImage3180Win32Win64.zip -d $BASE_INSTALL_PATH
unzip $INSTALLATION_PATH/external_zip/glew-2.2.0-win32.zip -d $BASE_INSTALL_PATH
unzip $INSTALLATION_PATH/external_zip/irrKlang-64bit-1.6.0.zip -d $BASE_INSTALL_PATH
unzip $INSTALLATION_PATH/external_zip/lua-5.4.2_Win64_vc16_lib -d $BASE_INSTALL_PATH/lua-5.4.2
unzip $INSTALLATION_PATH/external_zip/ImGui -d $BASE_INSTALL_PATH
unzip $INSTALLATION_PATH/external_zip/MSVC_Redist.zip -d $BASE_INSTALL_PATH

###########################################################################
# Compile libraries (except zlib)
###########################################################################

function BeginBuildLibrary()
{
	LIBRARY=$1
	CONFIG_TYPE=$2
	SRC_PATH=$INSTALLATION_PATH/$LIBRARY
	BUILD_PATH=$BASE_BUILD_PATH/$LIBRARY/$CONFIG_TYPE
	INSTALL_PATH=$BASE_INSTALL_PATH/$LIBRARY/$CONFIG_TYPE
	
	echo "========================================================="
	echo "  -> Building: " $LIBRARY
	echo "       CONFIG_TYPE:  " $CONFIG_TYPE	
	echo "       SRC_PATH:     " $SRC_PATH
	echo "       BUILD_PATH:   " $BUILD_PATH
	echo "       INSTALL_PATH: " $INSTALL_PATH
	echo "========================================================="
}

for CONFIG in $CONFIGS
do
	for LIBRARY in assimp freetype2 glm json tinyxml2 glfw
	do
	
		BeginBuildLibrary $LIBRARY $CONFIG
		cmake -S $SRC_PATH -B $BUILD_PATH
		cmake --build $BUILD_PATH --config $CONFIG
		cmake --install $BUILD_PATH --prefix $INSTALL_PATH --config $CONFIG
	
	done
done

###########################################################################
# zlib-1.3: the --prefix is not work working for this lib. Use -DCMAKE_INSTALL_PREFIX instead
###########################################################################

cd $INSTALLATION_PATH/external_zip
gunzip --stdout $INSTALLATION_PATH/external_zip/zlib-1.3.tar.gz | tar -x
mv zlib-1.3 $INSTALLATION_PATH

for CONFIG in $CONFIGS
do
	BeginBuildLibrary "zlib-1.3" $CONFIG
	cmake -S $SRC_PATH -B $BUILD_PATH -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH
	cmake --build $BUILD_PATH --config $CONFIG
	cmake --install $BUILD_PATH --config $CONFIG
done	

rm -fr $INSTALLATION_PATH/zlib-1.3
cd -

###########################################################################
# Boost
###########################################################################

echo "========================================================="
echo " Install boost"
echo "========================================================="	

curl -L https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.zip --output $BASE_BUILD_PATH/boost_1_83_0.zip  --ssl-no-revoke
unzip $BASE_BUILD_PATH/boost_1_83_0.zip -d $BASE_BUILD_PATH
cd $BASE_BUILD_PATH/boost_1_83_0
$BASE_BUILD_PATH/boost_1_83_0/bootstrap.bat
$BASE_BUILD_PATH/boost_1_83_0/b2
$BASE_BUILD_PATH/boost_1_83_0/b2 install --prefix=$BASE_INSTALL_PATH/boost_1_83_0
cd -

###########################################################################
# Some clean
###########################################################################

rm -fr $BASE_BUILD_PATH